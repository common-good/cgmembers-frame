<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/apptoken.inc';

/**
 * /cgpay/app-revoke: sign out.
 *   body: {token} signs out that device (it stays trusted for MFA); {token, everywhere: 1} signs out all the member's devices and ends their trust;
 *   {uid} (no token) does the latter, for when the password changes. 200 {revoked: <count>}; 401 {error: bad-token}
 */
function cgpayAppRevoke() {
  if (($in = appIn()) === FALSE) return;
  extract(just('token uid everywhere', $in, ''));
  $now = now();
  if ($token) {
    if (!$row = appRow($token)) return;
    $uid = $row->uid;
    if (!$everywhere) {
      db\update('app_tokens', ['id' => $row->id, 'revoked' => $now]);
      return exitJson(['revoked' => 1], X_OK);
    }
  }
  if (!$uid) return exitJust(X_SYNTAX);
  $n = db\q('UPDATE app_tokens SET revoked=:now, trusted=0 WHERE uid=:uid AND revoked IS NULL', compact('now', 'uid'))->rowCount();
  db\q('UPDATE app_tokens SET trusted=0 WHERE uid=:uid', compact('uid'));
  return exitJson(['revoked' => $n], X_OK);
}
