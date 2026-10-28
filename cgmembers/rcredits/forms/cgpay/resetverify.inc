<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

/**
 * @file
 * Companion to /cgpay/reset-request. The member enters the one-time code they
 * received and picks a new password. On success this endpoint:
 *   1. verifies the shared secret
 *   2. verifies the code against the account's oneTimePass
 *   3. hashes + stores the chosen password
 *   4. clears the one-time pass (one-time-use enforcement per Jose's ask)
 *   5. creates a session + device-box row (same shape as /cgpay/sso)
 *   6. returns the SSO cookie values so the node app can set them on the browser
 *
 * TODO (waiting on William's answer): the identifier-enumeration guard on the
 * request side means a bad identifier returns uid=0 and no code was sent. If
 * the node layer still submits uid=0 here (because the browser faked it), we
 * reject with the generic "code wrong or expired" error rather than something
 * more specific - keeps the two paths consistent from the attacker's point of view.
 */
function cgpayResetVerify() {
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

  $genericCodeErr = ['errors' => ['code' => t('That code is wrong or has expired.')]];

  if (!$uid || $code === '' || $newPassword === '') return exitJson($genericCodeErr, X_SYNTAX);
  if (strlen($newPassword) < 8) return exitJson(['errors' => ['newPassword' => t('Password must be at least 8 characters.')]], X_SYNTAX);

  $a = r\acct($uid);
  if (!$a) return exitJson($genericCodeErr, X_SYNTAX);

  // Verify the code against the account's one-time pass. passwordOkay() clears
  // the oneTimePass on success inside Acct, giving us one-time-use enforcement
  // without a separate call.
  if (!$a->passwordOkay($code, '', $err)) {
    return exitJson($genericCodeErr, X_SYNTAX);
  }

  require_once DRUPAL_ROOT . '/includes/password.inc';
  $hash = \user_hash_password($newPassword);
  if (!$hash) return exitJust(X_SYNTAX);
  $a->update('pass', $hash);
  $a->update('oneTimePass', ''); // defensive clear in case passwordOkay() changes

  if (!$a->ok) return exitJust(X_FORBIDDEN);

  // Create a session for the member - same pattern as cgpay/sso.
  if (!drupal_session_started()) drupal_session_start();
  global $user;
  $user = (object) ['uid' => $uid];
  $_SESSION[SVAR_HEADER . 'myid']    = serialize($uid);
  $_SESSION[SVAR_HEADER . 'agentId'] = serialize($uid);
  $cookieVal = session_id();

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
