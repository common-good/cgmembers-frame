<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Update contact fields on a single account from the SvelteKit admin dashboard.
 *
 * Scoped to the admin-safe subset of user fields: name, legal name, email,
 * phone, address, city, state, zip. Flag edits + password resets go through
 * separate endpoints (Phase 5 slice 3) because they need stricter guards.
 *
 * Writes a `changes` row per changed field for the audit trail, matching the
 * existing cgpaygrants.inc pattern so admin edits are reviewable in /sadmin.
 *
 * Flow:
 *   POST /cgpay-admin-user-update
 *     X-CG-Internal-Token: <shared secret>
 *     body: {"ssid": "<SSESS... value>",
 *            "uid": <int>,
 *            "fields": { fullName?, email?, phone?, address?, city?, state?, zip? }}
 *
 *   200 { ok: true, updated: [...changed field names...] }
 *   400 { message, errors: { field: "msg", ... } }
 *   401 bad token, no session, or caller not admin
 *   404 target uid not found
 */
function cgpayAdminUserUpdate() {
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  $body = json_decode(file_get_contents('php://input'), TRUE);
  $ssid = isset($body['ssid']) ? trim((string) $body['ssid']) : '';
  if ($ssid === '') return exitJust(X_SYNTAX);

  $row = db\row('sessions', 'ssid=:sid', ['sid' => $ssid]);
  if (!$row) return exitJust(X_UNAUTH);
  $callerUid = (int) ($row->acct ?? 0) ?: (int) ($row->uid ?? 0);
  if (!$callerUid) return exitJust(X_UNAUTH);
  $me = r\acct($callerUid);
  if (!$me || !$me->can(B_ADMIN)) return exitJust(X_UNAUTH);

  $uid = isset($body['uid']) ? (int) $body['uid'] : 0;
  if ($uid <= 0) return exitJust(X_SYNTAX);

  $target = db\row('users', 'uid=:u', ['u' => $uid]);
  if (!$target) return exitJust(X_NOTFOUND);

  $fieldsIn = isset($body['fields']) && is_array($body['fields']) ? $body['fields'] : [];
  $allowed = ['fullName', 'email', 'phone', 'address', 'city', 'state', 'zip'];

  // Normalize + validate.
  $errors  = [];
  $updates = [];
  foreach ($allowed as $k) {
    if (!array_key_exists($k, $fieldsIn)) continue;
    $v = is_string($fieldsIn[$k]) ? trim($fieldsIn[$k]) : $fieldsIn[$k];

    if ($k === 'email' && $v !== '' && !filter_var($v, FILTER_VALIDATE_EMAIL)) {
      $errors['email'] = 'Please enter a valid email address.';
      continue;
    }
    if ($k === 'fullName' && $v === '') {
      $errors['fullName'] = 'Full name cannot be empty.';
      continue;
    }
    $updates[$k] = $v;
  }

  if ($errors) {
    $summary = t('Some fields need attention - please fix the highlighted entries and try again.');
    return exitJson(['message' => $summary, 'errors' => $errors, 'error' => $summary], X_SYNTAX);
  }
  if (!$updates) return exitJson(['ok' => TRUE, 'updated' => []], X_OK);

  // Diff against current + record audit rows for fields that actually changed.
  $changed = [];
  foreach ($updates as $k => $new) {
    $old = $target->$k ?? NULL;
    if ((string) $old === (string) $new) continue;
    $changed[] = $k;
    db\insert('changes', [
      'table'     => 'users',
      'rid'       => $uid,
      'field'     => $k,
      'oldValue'  => (string) $old,
      'newValue'  => (string) $new,
      'changedBy' => $callerUid,
      'created'   => now(),
    ]);
  }

  if (!$changed) return exitJson(['ok' => TRUE, 'updated' => []], X_OK);

  // Apply the update.
  db\update('users', $updates + ['uid' => $uid], 'uid');

  return exitJson(['ok' => TRUE, 'updated' => $changed], X_OK);
}
