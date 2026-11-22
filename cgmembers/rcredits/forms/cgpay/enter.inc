<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/enter: a one-time code (good for a minute) that opens a live page as the token's member, acting for the account the app has chosen.
 *   body: {token, to (path on the live site, default dashboard), account (uid or account code, default the member's own)}; 200 {code, to}; 401 {error: bad-token}; 403 {error: not-allowed | locked}
 * The node server trades the code for a session at /cgpay/redeem, so the live site's cookie is rebuilt from the token.
 */
function cgpayEnter() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token'))) return;
  if (appLocked($row)) return appErr('locked', X_FORBIDDEN); // a cash register opens no live page until a manager unlocks it
  $to = trim((string) nni($in, 'to')) ?: 'dashboard';
  $to = preg_match('~^[a-z0-9/_?&=.%-]*$~i', $to) && !preg_match('~^(/|%2f){2}~i', $to) ? ltrim($to, '/') : 'dashboard'; // a path on the live site, never another site (not even protocol-relative)
  if (!$a = r\acct(nni($in, 'account') ?: $row->uid) or !appMayAct($row->uid, $account = $a->id)) return appErr('not-allowed', X_FORBIDDEN);
  if (($only = appOnly($row)) and $account != $only) return appErr('locked', X_FORBIDDEN);
  $code = bin2hex(random_bytes(16));
  db\insert('app_codes', ['uid' => $row->uid, 'tokenId' => $row->id, 'account' => $account, 'codeHash' => appHash($code), 'dest' => $to, 'expires' => now() + APP_CODE_SECS]);
  return exitJson(['code' => $code, 'to' => $to], X_OK);
}
