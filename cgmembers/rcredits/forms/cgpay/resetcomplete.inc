<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

/**
 * @file
 * Server-to-server: set a new password after the node app has verified a reset code.
 *
 * POST {resetId, uid, newPassword} with X-CG-Internal-Token. Node spends the code (usedAt) just before
 * calling, so this refuses unless that row exists for this uid and was spent in the last RESET_GRACE
 * seconds. The row is deleted here, so a repeat call fails. On success it ends every session the
 * member has, tells them by email, and replies {ok: true}; node then signs them in with cgpay/sso.
 * 400 means the password itself was refused; 403 means no verified reset.
 */
define('RESET_GRACE', 120);

function cgpayResetComplete() {
  global $channel; $channel = TX_API;
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) return exitJust(X_UNAUTH);

  $body = json_decode(test() ? nni($GLOBALS, 'testBody') : file_get_contents('php://input'), TRUE);
  if (!is_array($body)) return exitJust(X_SYNTAX);
  $resetId = (int) nni($body, 'resetId');
  $uid = (int) nni($body, 'uid');
  $pw = (string) nni($body, 'newPassword');
  if (!$resetId or !$uid) return exitJust(X_SYNTAX);
  u\loga('in', compact(ray('resetId uid'))); // never log the password

  $usedAt = db\get('usedAt', 'reset_codes', 'id=:resetId AND uid=:uid', compact(ray('resetId uid')));
  $a = r\acct($uid);
  if (!$usedAt or $usedAt < now() - RESET_GRACE or !$a or ($a->co and !$a->trialco)) return exitJust(X_FORBIDDEN);
  if (strlen($pw) < 8 or strlen($pw) > 128) return exitJust(X_SYNTAX);

  db\del('reset_codes', ['id' => $resetId]);
  $a->update('pass', $pw); // hashes it
  db\del('sessions', 'uid', $uid);
  $a->tell('password-changed');
  return exitJson(['ok' => TRUE], X_OK);
}
