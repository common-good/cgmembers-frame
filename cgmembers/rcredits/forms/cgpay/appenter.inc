<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/apptoken.inc';

/**
 * /cgpay/app-enter: a one-time code (good for a minute) that opens a live page as the token's member.
 *   body: {token, to (path on the live site, default dashboard)}; 200 {code, to}; 401 {error: bad-token}
 * The node server trades the code for a session at /cgpay/app-redeem, so the live site's cookie is rebuilt from the token.
 */
function cgpayAppEnter() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token'))) return;
  $to = trim((string) nni($in, 'to')) ?: 'dashboard';
  $to = preg_match('~^[a-z0-9/_?&=.%-]*$~i', $to) ? ltrim($to, '/') : 'dashboard'; // a path on the live site, never another site
  $code = bin2hex(random_bytes(16));
  db\insert('app_codes', ['uid' => $row->uid, 'codeHash' => appHash($code), 'dest' => $to, 'expires' => now() + APP_CODE_SECS]);
  return exitJson(['code' => $code, 'to' => $to], X_OK);
}
