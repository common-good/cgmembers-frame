<?php
namespace CG\Web;
use CG as r;
use CG\Util as u;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/feedback: a member's message to Common Good, from the store app (with its token, and the app's recent log)
 * or from node's website form (with the member's uid, which node took from the session).
 *   body: {token | uid, message, log?, version?, platform?}; 200 {ok: true}; 400 no message; 401 {error: bad-token}; 404 {error: no-account}
 * Like the member site's help form, the message goes to the account's community administrator. A log goes to the tech team too, with the message.
 */
function cgpayFeedback() {
  if (($in = appIn()) === FALSE) return;
  if (!$message = trim((string) nni($in, 'message'))) return exitJust(X_SYNTAX);
  if (nni($in, 'token')) {
    if (!$row = appRow($in['token'])) return;
    $uid = (int) $row->uid;
  } else $uid = (int) nni($in, 'uid');
  if ($uid <= 0 or !$a = r\acct($uid)) return appErr('no-account');

  $who = "$a->fullName ($a->mainQid)";
  $body = $message;
  r\tellAdmin(t('Feedback'), compact(ray('who body')), $uid);
  if ($log = (string) nni($in, 'log')) {
    $subs = compact(ray('who message')) + just('version platform', $in, '') + compact('log');
    r\rMail('tell-staff', TECH_EMAIL, $subs + ['topic' => t('App feedback'), 'noFrame' => TRUE, 'body' => u\seeAssoc($subs)], $a);
  }
  return exitJson(['ok' => TRUE]);
}
