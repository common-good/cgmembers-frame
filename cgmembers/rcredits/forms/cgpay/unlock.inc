<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/unlock: check the token's member's password again (the app asks for it before changing account or settings, if its settings say so).
 *   body: {token, password, ip}; 200 {ok: 1}; 401 {error: bad-token | bad-login}; 429 {error: flooding}
 * Failures count against the member like failed sign-ins (flood control).
 */
function cgpayUnlock() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token'))) return;
  $sta = [];
  if (w\flooding($row->uid, $sta, $err)) return appErr('flooding', X_TOOMANY);
  $a = r\acct($row->uid);
  $success = ($a and $a->passwordOkay((string) nni($in, 'password'), 'pass', $err));
  w\updateFlood($sta, $success, $err);
  return $success ? exitJson(['ok' => 1], X_OK) : appErr('bad-login', X_UNAUTH);
}
