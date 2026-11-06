<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

/**
 * @file
 * Companion to /cgpay/signup: the member enters the verification code they
 * received by email AND picks a real password. The code is `R_CODE_LEN` (32)
 * characters, so it is not something anyone types - the email link pastes the
 * `?code=...&qid=...` into the verify page URL. On success this endpoint:
 *   1. verifies the shared secret
 *   2. looks up the account by `uid` (same-tab flow) or `qid` (email-link flow)
 *   3. refuses an already-signed account so a stale code can't seize it
 *   4. checks the code against the account's oneTimePass
 *   5. verifies the account is ok (before any writes)
 *   6. stores the chosen password (Acct::update1 hashes it) and clears oneTimePass
 *   7. marks the account signed + agree + verifyemail
 *   8. creates a session for the account (same shape as /cgpay/sso)
 *   9. returns the SSO cookie values so the node app can set them on the browser
 *
 * The two-endpoint split is deliberate: node owns the verify-code UI and the
 * password-setup UI as separate pages, so each one has room to grow (e.g.
 * password strength meter, resend-code button) without widening a monster endpoint.
 */
function cgpaySignupVerify() {
  global $channel; $channel = TX_API;

  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  $body = json_decode(file_get_contents('php://input'), TRUE);
  if (!is_array($body)) return exitJust(X_SYNTAX);

  $uid         = (int) nni($body, 'uid');
  $qid         = trim((string) nni($body, 'qid'));
  $code        = trim((string) nni($body, 'code'));
  $newPassword = (string) nni($body, 'newPassword');

  if ($code === '' || $newPassword === '') return exitJust(X_SYNTAX);
  if (!$uid && $qid === '') return exitJust(X_SYNTAX);

  // Accept either a uid (node has it from sessionStorage during the same-tab flow)
  // or a qid (from the ?qid=... in the verification email link, which may open in
  // a different tab/device where sessionStorage is empty). r\acct() takes either.
  $a = $uid ? r\acct($uid) : r\acct(mb_strtoupper($qid));
  if (!$a) return exitJust(X_NOTFOUND);

  // Refuse if this account has already completed the signup/agreement step.
  // Without this check, a stale oneTimePass (for example, a password-reset code
  // issued to an already-active member) would be enough to overwrite `signed`,
  // the password, and the session on an existing account. The password-reset
  // flow lives at /cgpay/reset-verify instead.
  if ((int) $a->signed > 0) {
    return exitJson(['errors' => ['code' => t('That code is wrong or has expired.')]], X_SYNTAX);
  }

  // Verify the code against the account's one-time pass.
  if (!$a->passwordOkay($code, '', $err)) {
    return exitJson(['errors' => ['code' => t('That code is wrong or has expired.')]], X_SYNTAX);
  }

  // Check ok BEFORE writing anything, so a forbidden account doesn't get left
  // half-updated with a new password / signed flag / cleared oneTimePass.
  if (!$a->ok) return exitJust(X_FORBIDDEN);

  // Store the real password and clear the one-time pass so the code can't be
  // reused. Acct::update1() hashes `pass` for us via r\passHash (see acct.class);
  // hashing again here would store a hash-of-a-hash and the member could never
  // sign in. Pass the raw password.
  $a->update('pass', $newPassword);
  $a->update('oneTimePass', '');

  // Mark the account as having signed the agreement + verified the email.
  // `agree` covers the Common Good Agreement acceptance; `verifyemail` covers
  // the one-time code the member just proved they received (same step
  // partnersignup.inc marks when its email code checks out).
  $a->update('signed, signedBy', now(), $a->fullName);
  $a->stepDone('agree');
  $a->stepDone('verifyemail');

  // Create a session for the member - same pattern as cgpaysso.inc.
  if (!drupal_session_started()) drupal_session_start();
  global $user;
  $user = (object) ['uid' => $uid];
  $_SESSION[SVAR_HEADER . 'myid']    = serialize($uid);
  $_SESSION[SVAR_HEADER . 'agentId'] = serialize($uid);
  $cookieVal = session_id();

  // Create a device box row + cookie so downstream features see the device.
  $boxName  = 'box-' . $a->mainQid;
  $boxValue = u\code();
  $boxnum   = (db\get('MAX(boxnum)', 'r_boxes', ['uid' => $uid]) ?: 0) + 1;
  db\insert('r_boxes', [
    'uid'     => $uid,
    'code'    => $boxValue,
    'boxnum'  => $boxnum,
    'created' => now(),
    'access'  => now(),
    'channel' => TX_WEB,
  ]);

  $menu = ['Dashboard', 'History', 'Community'];
  if ($a->co) $menu[] = 'Company';
  if ($a->can(B_ADMIN)) $menu[] = 'Admin';
  $menu[] = 'Settings';

  return exitJson([
    'cookieName'     => session_name(),
    'sid'            => $cookieVal,
    'ssid'           => $cookieVal,
    'menu'           => $menu,
    'boxCookieName'  => $boxName,
    'boxCookieValue' => $boxValue,
    'uid'            => $uid,
    'qid'            => $a->mainQid,
  ], X_OK);
}
