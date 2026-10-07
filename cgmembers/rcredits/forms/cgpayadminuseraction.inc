<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Admin-triggered state changes on a single account (activate / deactivate).
 *
 * Password reset and admin-flag toggles live in follow-up slices because they
 * carry heavier confirmation requirements. This endpoint covers the common
 * daily action: flip B_OK on or off.
 *
 * Flow:
 *   POST /cgpay-admin-user-action
 *     X-CG-Internal-Token: <shared secret>
 *     body: {"ssid": "<SSESS... value>",
 *            "uid": <int>,
 *            "action": "activate" | "deactivate"}
 *
 *   200 { ok: true, action, flagsAfter: <int> }
 *   400 bad body / unknown action
 *   401 bad token, no session, or caller not admin
 *   403 caller cannot act on their own account
 *   404 target uid not found
 */
function cgpayAdminUserAction() {
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
  if ($uid === $callerUid) return exitJust(X_FORBIDDEN);

  $action = isset($body['action']) ? (string) $body['action'] : '';
  if (!in_array($action, ['activate', 'deactivate'], TRUE)) return exitJust(X_SYNTAX);

  $target = db\row('users', 'uid=:u', ['u' => $uid]);
  if (!$target) return exitJust(X_NOTFOUND);

  $okBit  = 1 << B_OK;
  $before = (int) ($target->flags ?? 0);
  $after  = $action === 'activate' ? ($before | $okBit) : ($before & ~$okBit);
  if ($after === $before) return exitJson(['ok' => TRUE, 'action' => $action, 'flagsAfter' => $after, 'noop' => TRUE], X_OK);

  db\update('users', ['uid' => $uid, 'flags' => $after], 'uid');

  db\insert('changes', [
    'table'     => 'users',
    'rid'       => $uid,
    'field'     => 'flags',
    'oldValue'  => (string) $before,
    'newValue'  => (string) $after,
    'changedBy' => $callerUid,
    'created'   => now(),
  ]);

  return exitJson(['ok' => TRUE, 'action' => $action, 'flagsAfter' => $after], X_OK);
}
