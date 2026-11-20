<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\QR as qr;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/card: sign the store app in by a scanned member card, when the app switches to or from demo.
 *   body: {qr (the card's QR text), password (the cardholder's; not needed on demo), skipMfa (honored only on demo), label, deviceKey, ip}
 *   200 {token, uid, admin, accounts, account (the account the card is for, to act for first)} or {mfa: <handle>}, as for /cgpay/signin
 *   401 {error: bad-card | bad-login | password-needed}, 406 {error: wrong-kind} (a test card here, or a real card on demo), 429 and 403 as for /cgpay/signin
 * A company card signs in its cardholder, acting for the company. On demo the card alone is enough, since demo money is
 * not real; elsewhere the cardholder's password is needed too, so a card alone never opens a real account.
 */
function cgpayCard() {
  if (($in = appIn()) === FALSE) return;
  extract(just('qr password skipMfa label deviceKey', $in, ''));
  include_once R_ROOT . '/cg-qr.inc';

  $sta = [];
  if (w\ipFlooding($sta, $err)) return appErr('flooding', X_TOOMANY); // before the card lookup, so a blocked address learns nothing about cards
  if (!$a = qr\acct(trim($qr), TRUE, $isTestCard)) {w\updateFlood($sta, FALSE); return appErr('bad-card', X_UNAUTH);} // counts against the IP, so card codes cannot be guessed
  $demo = appDemo();
  if ($isTestCard != $demo) return appErr('wrong-kind', X_DENIED);
  if (!$demo and $password === '') return appErr('password-needed', X_UNAUTH);
  $actFor = ($a->ok and appMayAct($a->agentId, $a->id)) ? $a->id : NULL; // the card may be older than a change of permission
  return appVerify($a->agentId, $demo ? NULL : $password, $label, $deviceKey, $demo and $skipMfa, $actFor);
}
