<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Db as db;
use CG\Util as u;

/**
 * @file
 * Shared code of the store app's token sign-in endpoints (/cgpay/signin, app-mfa, app-device, app-enter, app-redeem, app-revoke).
 *
 * The app signs in once and keeps a revocable token (only its sha256 is stored, in app_tokens). Every call
 * comes from the node server (X-CG-Internal-Token, like /cgpay/sso), never from the browser. Errors are JSON
 * {"error": "<code>"} so the app can word them itself.
 */

const APP_TOKEN_DAYS = 90; // a token not used for this long stops working
const APP_CODE_SECS = 60; // life of a one-time code for opening a live page
const APP_MFA_SECS = 600; // life of the second sign-in step
const APP_API_VERSION = 40206; // the legacy app's version, so the device id has the current format

/**
 * Check the method and shared secret, then return the request fields (posted fields under test), or FALSE after replying with the error.
 */
function appIn() {
  global $channel; $channel = TX_API;
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') {exitJust(X_SYNTAX); return FALSE;}
  $given = (string) nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (CGPAY_SSO_SECRET === '' or !hash_equals(CGPAY_SSO_SECRET, $given)) {exitJust(X_UNAUTH); return FALSE;}
  $in = test() ? $_POST : (json_decode(file_get_contents('php://input'), TRUE) ?: []);
  if (filter_var(nni($in, 'ip'), FILTER_VALIDATE_IP)) $_SERVER['REMOTE_ADDR'] = $in['ip']; // the caller is the node server, so flood control must see the member's address, not its own
  return $in;
}

/**
 * Reply with an error code (and the HTTP status that goes with it).
 */
function appErr($code, $status = X_NOTFOUND) {return exitJson(['error' => $code], $status);}

function appHash($s) {return hash('sha256', (string) $s);}

/**
 * Make a token for the member, record it, and reply with it and the device credentials the transaction screens need.
 * @param bool $trust: this device skips MFA from now on
 */
function appIssue($uid, $label, $deviceKey, $trust) {
  $token = bin2hex(random_bytes(32));
  $created = $lastUsed = now();
  $expires = $created + APP_TOKEN_DAYS * DAY_SECS;
  $tokenHash = appHash($token);
  $deviceHash = $deviceKey ? appHash($deviceKey) : NULL;
  $trusted = $trust ? 1 : 0;
  $label = mb_substr(trim((string) $label), 0, 60);
  db\insert('app_tokens', compact(ray('uid tokenHash deviceHash label trusted created lastUsed expires')));
  return exitJson(['token' => $token, 'uid' => (int) $uid, 'device' => appDevice($uid)], X_OK);
}

/**
 * Return the live token row for the token, slid forward; or FALSE after replying that the token is not good.
 */
function appRow($token) {
  $now = now();
  $row = $token ? db\row('app_tokens', 'tokenHash=:h AND revoked IS NULL AND expires>:now', ['h' => appHash($token), 'now' => $now]) : NULL;
  if (!$row) {appErr('bad-token', X_UNAUTH); return FALSE;}
  db\update('app_tokens', ['id' => $row->id, 'lastUsed' => $now, 'expires' => $now + APP_TOKEN_DAYS * DAY_SECS]);
  return $row;
}

/**
 * Return the account credentials the app's transaction screens use (see accountInfo()), or NULL if this account can't use the app yet.
 */
function appDevice($uid) {
  include_once R_ROOT . '/forms/api.inc';
  $a = r\acct($uid, $uid);
  return $a ? accountInfo($a, $a->fullName, APP_API_VERSION) : NULL;
}

/**
 * Return the uid the identifier names (an account code, email, phone number or name), or NULL.
 */
function appFind($identifier) {
  if ($uid = r\loginString($identifier)) return $uid;
  if (!preg_match('/^[\+\(\)\.\- 0-9]{7,}$/', $identifier)) return NULL;
  $phone = u\cry('P', u\fmtPhone($identifier, '+n'));
  return db\get('uid', 'users', 'phone=:phone ORDER BY :IS_CO', compact('phone')) ?: NULL;
}

/**
 * Return whether this installation is trusted for the member (signed in with MFA before, and not revoked everywhere since).
 */
function appTrusted($uid, $deviceKey) {
  return $deviceKey and db\exists('app_tokens', 'uid=:uid AND deviceHash=:h AND trusted=1', ['uid' => $uid, 'h' => appHash($deviceKey)]);
}
