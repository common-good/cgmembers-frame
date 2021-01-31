<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Send another member a message.
 */
function formMessage($form, &$sta, $args = '') {
  global $mya;
  
  extract(just('code', $args, NULL));
  
  if ($code) return codeResult($code);
  
  jsx('message');
  $title = t('Send a Message');
  $subtext = t('Complete this form and click Send. The other member will receive an email from your email address. When they reply, you will also have their email address, so you can communicate by email after that (you won\'t need to use this form again).');
  $to = textFld(REQ . t('To:'), [t('Recipient'),t('The account, name, or partial name of the member you want to send a message to.')]);
  $subject = textFld(REQ . t('Subject:'), [t('Subject')], dft(t('a message from %PROJECT member ') . $mya->fullName));
  $message = areaFld(t('Message:'), [t('Your message goes here.')]);
  $addSecret = item(lnk('', t('Add some encrypted data')));
  $secret = areaFld(t('Secret:'), [t('Secret information here'), t('Secret data can be decrypted only by the intended recipient (even if the message is intercepted or forwarded). Max %SECRET_MAXLEN chars.')], maxlen(SECRET_MAXLEN));
  $expires = numFld(t('Expires In:'), '', maxlen(4) + dft(3) + suffix(t(' hours')));
  $secrets = fieldSet('secrets', compact(ray('secret expires')), BARE);
  $submit = t('Send');
  w\whoFldSubmit('to', t('Send to %name?'));

  return cgform(compact(ray('title subtext to subject message addSecret secrets submit')));
}

function formMessage_validate($form, &$sta) {
  extract($info = just('to subject message secret expires', $sta['input'], NULL));
  if (strhas($to, '@') and empty($secret)) return say(t('If you know the member\'s email address, don\'t use this form -- just send them an email.'), 'to');
  if (strlen($secret) and $expires <= 0) return say(t('Expires must be greater than zero.'), 'expires');
  if (!$a = w\whois($to, 'to', $info)) return FALSE;
  if (empty($message) and empty($secret)) return say(t('You forgot to include a message.'), 'message');
  u\preray(ray('uid', $a->id), $sta['input']);
}

function formMessage_submit($form, &$sta) {
  extract(just('uid subject message secret expires', $sta['input'], NULL));
  global $mya, $base_url;
  
  $a = r\acct($uid);
  if (strlen($secret)) { // encrypt secret message
    $code = u\cryRay(ray('qid secret exp', $a->mainQid, $secret, $at = now() + HOUR_SECS * $expires));
    $url = "$base_url/community/message/code=$code";
    $message .= "<br><br>Secret: " . lnk($url, t('click here')) . t('. Link expires at %tm on %dt.', 'tm dt', u\tm($at), u\fmtDate($at));
  }
  r\rMail('', [$a->email => $a->fullName], ray('noFrame subject body', TRUE, $subject, $message), [$mya->email => $mya->fullName]);
  say('sent message', ray('fullName', $a->fullName));
}

/**
 * Decrypt the code and report the result, if appropriate.
 * The intended recipient may be an individual or a company. Only an authorized agent can read the message.
 */
function codeResult($code) {
  global $box;
  if (!$ray = u\decryRay($code)) return softErr(t('That is not a valid link.'));
  extract(just('qid secret exp', $ray, NULL));
  $intendedA = r\acct($qid);
  $agent = db\get('uid', 'r_boxes JOIN users USING(uid)', 'code=:box ORDER BY :IS_CO', compact('box'));
  if (!$a = r\acct($intendedA->id, $agent) or !$a->can(B_READ)) return softErr(t('You are not the intended recipient.'));
  if (now() > $exp) return softErr(t('That secret information is not longer available.'));
  return sayEmpty(t('<h2>Encrypted Message</h2>') . $secret);
}
