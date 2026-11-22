<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Db as db;
use CG\QR as qr;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/unlock: check the token's member's password again (the app asks for it before changing account or settings, if its settings say so),
 * or, for a cash register (a token locked to a company, see /cgpay/lock), a scanned card of one of the company's managers.
 *   body: {token, password, ip} or {token, qr, ip}
 *   200 {ok: 1} (plus, for a locked token, what /cgpay/device replies while it is unlocked, and unlockedBy: the uid of the manager whose card it was)
 *   401 {error: bad-token | bad-login | bad-card}; 403 {error: not-manager | not-locked}; 406 {error: wrong-kind}; 429 {error: flooding}
 * Failures count against the member (password) or the address (card), like failed sign-ins. A locked token stays unlocked while it is used, until APP_UNLOCK_SECS pass without use or the app locks it again.
 */
function cgpayUnlock() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token'))) return;
  $qr = trim((string) nni($in, 'qr'));
  $sta = [];
  if ($qr === '') {
    if (w\flooding($row->uid, $sta, $err)) return appErr('flooding', X_TOOMANY);
    $a = r\acct($row->uid);
    $success = ($a and $a->passwordOkay((string) nni($in, 'password'), 'pass', $err));
    w\updateFlood($sta, $success, $err);
    if (!$success) return appErr('bad-login', X_UNAUTH);
    $by = $row->uid;
  } else {
    if (!$row->lockedTo) return appErr('not-locked', X_FORBIDDEN);
    include_once R_ROOT . '/cg-qr.inc';
    if (w\ipFlooding($sta, $err)) return appErr('flooding', X_TOOMANY);
    if (!qr\qid($qr, $code, $isTestCard)) {w\updateFlood($sta, FALSE); return appErr('bad-card', X_UNAUTH);}
    if ((bool) $isTestCard !== appDemo()) return appErr('wrong-kind', X_DENIED);
    if (!$a = qr\acct($qr, TRUE)) {w\updateFlood($sta, FALSE); return appErr('bad-card', X_UNAUTH);}
    if (!appManages($by = $a->agentId, $row->lockedTo)) return appErr('not-manager', X_FORBIDDEN);
  }
  if (!$row->lockedTo) return exitJson(['ok' => 1], X_OK);
  db\update('app_tokens', ['id' => $row->id, 'unlockedUntil' => $row->unlockedUntil = now() + APP_UNLOCK_SECS]);
  return exitJson(['ok' => 1, 'unlockedBy' => (int) $by] + appMember($row->uid, $row), X_OK);
}
