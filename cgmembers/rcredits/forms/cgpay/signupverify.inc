<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

/**
 * @file
 * Companion to /cgpay-signup: the member enters the 6-ish-character verification
 * code they received by email AND picks a real password. On success this
 * endpoint:
 *   1. verifies the shared secret
 *   2. verifies the code against the account's oneTimePass
 *   3. hashes + stores the chosen password
 *   4. marks the account verified (`signed` + `signedBy` + agree step)
 *   5. creates a session for the account (same shape as /cgpay-sso)
 *   6. returns the SSO cookie values so the node app can set them on the browser
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
  $code        = trim((string) nni($body, 'code'));
  $newPassword = (string) nni($body, 'newPassword');

  if (!$uid || $code === '' || $newPassword === '') return exitJust(X_SYNTAX);

  $a = r\acct($uid);
  if (!$a) return exitJust(X_NOTFOUND);

  // Verify the code against the account's one-time pass.
  if (!$a->passwordOkay($code, '', $err)) {
    return exitJson(['errors' => ['code' => t('That code is wrong or has expired.')]], X_SYNTAX);
  }

  // Store the real password (hashed) and clear the one-time pass so the code
  // can't be reused.
  require_once DRUPAL_ROOT . '/includes/password.inc';
  $hash = \user_hash_password($newPassword);
  if (!$hash) return exitJust(X_SYNTAX);
  $a->update('pass', $hash);
  $a->update('oneTimePass', '');

  // Mark the account as having signed the agreement + verified.
  $a->update('signed, signedBy', now(), $a->fullName);
  $a->stepDone('agree');

  if (!$a->ok) return exitJust(X_FORBIDDEN);

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
