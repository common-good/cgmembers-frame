<?php
namespace CG\Web;
use CG as r;
use CG\Util as u;

/**
 * @file
 * Server-to-server: deliver a password reset code that the node app made and stored (hashed).
 *
 * POST {uid, channel: email|sms, code} with X-CG-Internal-Token. Node owns the codes and every limit;
 * this only sends the message. Replies {channel} naming the channel actually used, or {channel: null}
 * when nothing was sent (no such account, company-only account, or the send failed). Node shows the
 * member the same thing either way. A text that cannot be sent falls back to email.
 */
function cgpayResetDeliver() {
  global $channel; $channel = TX_API;
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) return exitJust(X_UNAUTH);

  $body = json_decode(test() ? nni($GLOBALS, 'testBody') : file_get_contents('php://input'), TRUE);
  if (!is_array($body)) return exitJust(X_SYNTAX);
  $uid = (int) nni($body, 'uid');
  $by = nni($body, 'channel');
  $code = (string) nni($body, 'code');
  if (!$uid or !in($by, 'email sms') or !preg_match('/^[0-9]{6}$/', $code)) return exitJust(X_SYNTAX);
  u\loga('in', compact(ray('uid by'))); // never log the code

  $a = r\acct($uid);
  $none = ['channel' => NULL];
  if (!$a or ($a->co and !$a->trialco)) return exitJson($none, X_OK); // same rule as the web form: companies reset through their managers

  if ($by == 'sms') {
    require_once R_ROOT . '/cg-sms.inc';
    if (r\smsResetCode($a, $code)) return exitJson(['channel' => 'sms'], X_OK);
  }
  return exitJson($a->tell('reset-code', compact('code')) ? ['channel' => 'email'] : $none, X_OK);
}
