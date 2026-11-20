<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Db as db;
use CG\Util as u;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/signin: the store app's first sign-in step.
 *   body: {identifier, password, label (device name), deviceKey (random key the app keeps for this installation), ip (the member's address, for flood control)}
 *   200 {token, uid, admin, accounts} when signed in (see appMember())
 *   200 {mfa: <handle>} when the member must also give the passcode we emailed (send it to /cgpay/mfa)
 *   401 {error: bad-login}, 429 {error: flooding}, 403 {error: not-ok | weak-admin-password}
 * Password check, flood control and MFA rules are the web sign-in's (see forms/signin.inc), so the two cannot drift apart.
 */
function cgpaySignin() {
  if (($in = appIn()) === FALSE) return;
  extract(just('identifier password label deviceKey', $in, ''));
  $identifier = trim($identifier);
  if ($identifier === '' or $password === '') return exitJust(X_SYNTAX);

  $sta = [];
  if (w\ipFlooding($sta, $err)) return appErr('flooding', X_TOOMANY);
  if (!$uid = appFind($identifier)) {w\updateFlood($sta, FALSE); return appErr('bad-login', X_UNAUTH);} // counts against the IP
  return appVerify($uid, $password, $label, $deviceKey);
}
