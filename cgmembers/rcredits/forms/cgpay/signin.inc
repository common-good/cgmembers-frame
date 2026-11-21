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
  if (w\flooding($uid, $sta, $err)) return appErr('flooding', X_TOOMANY);
  $a = r\acct($uid);
  $success = ($a and $a->passwordOkay($password, 'pass', $err));
  w\updateFlood($sta, $success, $err);
  if (!$success) return appErr('bad-login', X_UNAUTH);
  if (!$a->ok) return appErr('not-ok', X_FORBIDDEN);
  if (isPRODUCTION and $a->admin and !u\strongPass($password, u\pwAvoid($a))) return appErr('weak-admin-password', X_FORBIDDEN);

  if (!$a->canMfa or appTrusted($uid, $deviceKey)) return appIssue($uid, $label, $deviceKey, FALSE);

  $nonce = u\randomString(8, 'digits') . '';
  r\rMail('confirm-signin', $uid, compact('nonce'));
  $expires = now() + APP_MFA_SECS;
  $uid .= '';
  return exitJson(['mfa' => u\cryRay(compact(ray('uid nonce expires label deviceKey')))], X_OK);
}
