<?php
namespace CG\Drive;
use CG as r;

/**
 * @file
 * Minimal Google Drive integration for uploading grant-agreement PDFs to the
 * "Grant Contracts YYYY" folder. Uses direct HTTPS + service-account JWT auth
 * (no google/apiclient dependency, to keep vendor/ small).
 *
 * Config (defs.inc):
 *   DRIVE_SERVICE_ACCOUNT_JSON_PATH — path to the service-account credentials JSON
 *   DRIVE_ROOT_FOLDER_ID            — Drive ID of the parent folder that contains
 *                                     the yearly "Grant Contracts YYYY" subfolders
 *                                     (share it with the service account as Editor)
 *
 * If either is empty, driveConfigured() returns FALSE and callers should skip
 * the upload gracefully — the file stays wherever the caller put it.
 *
 * Public API:
 *   driveConfigured(): bool
 *   driveUploadFile($localPath, $filename, $year): array   → ['id' => ..., 'webViewLink' => ...]
 *
 * Errors: functions throw \RuntimeException with a descriptive message; the
 * caller decides whether to surface it to the user, log it, or both.
 */

const GOOGLE_TOKEN_URL = 'https://oauth2.googleapis.com/token';
const DRIVE_UPLOAD_URL = 'https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart&supportsAllDrives=true';
const DRIVE_FILES_URL  = 'https://www.googleapis.com/drive/v3/files';
const DRIVE_SCOPE      = 'https://www.googleapis.com/auth/drive.file';

function driveConfigured() {
  return DRIVE_SERVICE_ACCOUNT_JSON_PATH !== '' && DRIVE_ROOT_FOLDER_ID !== ''
      && is_readable(DRIVE_SERVICE_ACCOUNT_JSON_PATH);
}

/**
 * Upload a local file to the Drive folder "Grant Contracts $year" (creating that
 * subfolder inside DRIVE_ROOT_FOLDER_ID if needed).
 *
 * @param string $localPath  Server-side path of the file to upload.
 * @param string $filename   Filename to store in Drive (per William's naming convention).
 * @param int    $year       Year for the target subfolder name.
 * @return array             ['id' => <driveFileId>, 'webViewLink' => <url>]
 * @throws \RuntimeException on config, auth, or upload failure.
 */
function driveUploadFile($localPath, $filename, $year) {
  if (!driveConfigured()) throw new \RuntimeException('Drive not configured');
  if (!is_readable($localPath)) throw new \RuntimeException("File not readable: $localPath");

  $creds = driveLoadCreds();
  $token = driveAccessToken($creds);
  $folderName = str_replace('%year%', (string) (int) $year, GRANT_CONTRACTS_FOLDER_PATTERN);
  $folderId = driveResolveYearFolder($token, DRIVE_ROOT_FOLDER_ID, $folderName);

  $mime = mime_content_type($localPath) ?: 'application/octet-stream';
  $body = file_get_contents($localPath);
  $meta = ['name' => $filename, 'parents' => [$folderId]];

  $boundary = '----cgdrive' . bin2hex(random_bytes(8));
  $payload = "--$boundary\r\n"
           . "Content-Type: application/json; charset=UTF-8\r\n\r\n"
           . json_encode($meta) . "\r\n"
           . "--$boundary\r\n"
           . "Content-Type: $mime\r\n\r\n"
           . $body . "\r\n"
           . "--$boundary--";

  $resp = driveHttpRequest('POST', DRIVE_UPLOAD_URL . '&fields=id,webViewLink', $payload, [
    "Authorization: Bearer $token",
    "Content-Type: multipart/related; boundary=$boundary",
  ]);
  if (empty($resp['id'])) throw new \RuntimeException('Drive upload returned no id: ' . json_encode($resp));

  return ['id' => $resp['id'], 'webViewLink' => $resp['webViewLink'] ?? ''];
}

function driveLoadCreds() {
  $raw = file_get_contents(DRIVE_SERVICE_ACCOUNT_JSON_PATH);
  $data = json_decode($raw, TRUE);
  if (!$data || empty($data['client_email']) || empty($data['private_key'])) {
    throw new \RuntimeException('Invalid Drive service-account JSON');
  }
  return $data;
}

/**
 * Sign a JWT with the service account's private key and exchange it for a Drive access token.
 * Access tokens are short-lived (~1 hr); we mint a fresh one each call rather than caching,
 * since grant uploads are infrequent.
 */
function driveAccessToken(array $creds) {
  $now = time();
  $header  = base64UrlEncode(json_encode(['alg' => 'RS256', 'typ' => 'JWT']));
  $claim   = base64UrlEncode(json_encode([
    'iss'   => $creds['client_email'],
    'scope' => DRIVE_SCOPE,
    'aud'   => GOOGLE_TOKEN_URL,
    'exp'   => $now + 3600,
    'iat'   => $now,
  ]));
  $signInput = "$header.$claim";
  $key = openssl_pkey_get_private($creds['private_key']);
  if (!$key) throw new \RuntimeException('Invalid Drive private key');
  openssl_sign($signInput, $sig, $key, 'SHA256');
  $jwt = "$signInput." . base64UrlEncode($sig);

  $resp = driveHttpRequest('POST', GOOGLE_TOKEN_URL, http_build_query([
    'grant_type' => 'urn:ietf:params:oauth:grant-type:jwt-bearer',
    'assertion'  => $jwt,
  ]), ['Content-Type: application/x-www-form-urlencoded']);
  if (empty($resp['access_token'])) throw new \RuntimeException('Drive auth failed: ' . json_encode($resp));

  return $resp['access_token'];
}

/**
 * Find (or create) the "Grant Contracts YYYY" subfolder inside DRIVE_ROOT_FOLDER_ID.
 */
function driveResolveYearFolder($token, $parentId, $folderName) {
  $q = "name = '" . addslashes($folderName) . "' and mimeType = 'application/vnd.google-apps.folder' and '$parentId' in parents and trashed = false";
  $url = DRIVE_FILES_URL . '?' . http_build_query(['q' => $q, 'fields' => 'files(id,name)', 'supportsAllDrives' => 'true', 'includeItemsFromAllDrives' => 'true']);
  $resp = driveHttpRequest('GET', $url, NULL, ["Authorization: Bearer $token"]);
  if (!empty($resp['files'][0]['id'])) return $resp['files'][0]['id'];

  // Not found — create it.
  $meta = ['name' => $folderName, 'mimeType' => 'application/vnd.google-apps.folder', 'parents' => [$parentId]];
  $resp = driveHttpRequest('POST', DRIVE_FILES_URL . '?fields=id&supportsAllDrives=true', json_encode($meta), [
    "Authorization: Bearer $token",
    'Content-Type: application/json',
  ]);
  if (empty($resp['id'])) throw new \RuntimeException("Failed to create Drive folder '$folderName': " . json_encode($resp));
  return $resp['id'];
}

function driveHttpRequest($method, $url, $body, array $headers) {
  $ch = curl_init($url);
  curl_setopt_array($ch, [
    CURLOPT_CUSTOMREQUEST  => $method,
    CURLOPT_HTTPHEADER     => $headers,
    CURLOPT_RETURNTRANSFER => TRUE,
    CURLOPT_TIMEOUT        => 30,
    CURLOPT_FOLLOWLOCATION => FALSE,
  ]);
  if ($body !== NULL) curl_setopt($ch, CURLOPT_POSTFIELDS, $body);

  $raw = curl_exec($ch);
  $err = curl_error($ch);
  $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
  curl_close($ch);

  if ($raw === FALSE) throw new \RuntimeException("Drive HTTP error: $err");
  $parsed = json_decode($raw, TRUE);
  if ($code >= 400) throw new \RuntimeException("Drive HTTP $code: $raw");

  return $parsed ?: [];
}

function base64UrlEncode($data) {
  return rtrim(strtr(base64_encode($data), '+/', '-_'), '=');
}
