<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const MSG_URL0 = BASE_URL . '/community/message/x/code='; // add code to this

/**
 * Send another member a message.
 */
function formMessage($form, &$sta, $args = '') {
  global $mya;
  
  extract(just('code', $args, NULL));
  
  if ($code) return codeResult($code);
  $a = ($mya->ok and $mya->relPermission >= r\perm(B_REFUND)) ? $mya : $mya->agentA; // explicit use of relPermission handles admins right
  
  jsx('message');
  $title = t('Send a Message');
  $subtext = t('<p>Complete this form and click Send. The other member receives an email from your email address. When they reply, you also have their email address, so you can communicate by email after that (you won\'t need to use this form again).</p><p>If you include an encrypted message, you receive notification when it has been seen. Secret information can be viewed/downloaded only by the intended recipient (even if your message is intercepted or forwarded).</p>');
  $to = textFld(REQ . t('To:'), [t('Recipient'),t('The account, name, or partial name of the member you want to send a message to.')]);
  $subject = textFld(REQ . t('Subject:'), [t('Subject')], dft(t('a message from %PROJECT member ') . $a->fullName));
  $message = areaFld(t('Message:'), [t('Your message goes here.')]);
  $addSecret = item(lnk('', t('Add some encrypted data')));
  $secret = areaFld(t('Secret:'), [t('Secret information here'), t('Max %SECRET_MAXLEN chars.')], maxlen(SECRET_MAXLEN));
  $file = fileFld(t('Secret File:'), t('Upload a file for secure download. Maximum size %MAX_UPLOAD_SIZE MB. Maximum expiration time %MAX_FILE_HOURS hours.'));
  $expires = numFld(t('Expires In:'), '', maxlen(4) + dft(48) + suffix(t(' hours')));
  $secrets = fieldSet('secrets', compact(ray('secret file expires')), BARE);
  $submit = t('Send');
  $fromQid = hidFld($a->mainQid);
  w\whoFldSubmit('to', t('Send to %name?'), 'self-message', '');

  return cgform(compact(ray('title subtext to subject message addSecret secrets submit fromQid')));
}

function formMessage_validate($form, &$sta) {
  extract($info = just('to subject message secret expires', $sta['input'], NULL));
  if (strhas($to, '@') and !strhas($to, ' @ ') and empty($secret) and !gotFile()) return say(t('If you know the member\'s email address, don\'t use this form unless you\'re including an encrypted message or file &mdash; just send them an email.'), 'to'); // !strhas is needed to accept results of autocomplete
  if (strlen($secret) and $expires <= 0) return say(t('Expires must be greater than zero.'), 'expires');
//  if (!$a = w\whois($to, 'to', $info, 'self-message')) return FALSE;
//  u\preray(ray('uid', $a->id), $sta['input']);

  if (gotFile()) {
    $flnm = microtime(TRUE) + HOUR_SECS * $expires;
    if (!is_dir(MSG_DIR)) mkdir(MSG_DIR, 0755) or u\FAIL('cannot create msgs directory'); 
    $path = MSG_DIR . '/' . $flnm;
    $fileData = uploadFile($path, NULL);
    if (!is_array($fileData)) return err($fileData, 'file');
    $sta['input']['fileData'] = serialize($fileData);
    u\cryFile($path);
  } elseif (empty($message) and empty($secret)) return say(t('You forgot to include a message.'), 'message');
}

function formMessage_submit($form, &$sta) {
  extract(just('subject message secret fileData expires fromQid', $sta['input'], NULL));
  global $base_url;
  
  $uid = whoId($sta);
  $a = r\acct($uid);
  
  if ($fileData = unserialize($fileData) or strlen($secret)) { // encrypted file and/or message
    $at = now() + HOUR_SECS * $expires;
    $ray = ray('qid from exp', $a->mainQid, $fromQid, $at);
    if (strlen($secret)) $ray += compact('secret');
    if ($fileData) $ray += just('path name type size', $fileData);

    $message .= t('<p>Secret: <%a>click here</a>.<sup>*</sup></p>', '_a', atag(MSG_URL0 . u\cryRay($ray)));
    $message .= t('<p>Encrypted data expires at %tm on %dt.</p><p class="smallish">* The encrypted message can be viewed only by you, on a device you have used (sometime in the past) to sign in to your %PROJECT account.', 'tm dt', u\tm($at), u\fmtDate($at));
  }

  $fromA = r\acct($fromQid);
  if ($fromA->admin) $fromA = $fromA->agentA;
  $fromNm = $fromA->proSe ? $fromA->fullName : ($fromA->agentA->fullName . " ($fromA->fullName)");
  r\rMail('', [$a->email => $a->fullName], ray('noFrame subject body', TRUE, $subject, $message), [$fromA->email => $fromNm]);
  say('sent message', ray('fullName', $a->fullName));
}

/**
 * Decrypt the code and report the result, if appropriate.
 * The intended recipient may be an individual or a company. Only an authorized agent can read the message.
 */
function codeResult($code) {
  global $box;
  if (!$ray = u\decryRay($code)) return softErr(t('That is not a valid link.'));
  extract(just('qid from secret path name type size exp download', $ray, NULL));
  
  if ($intendedA = r\acct($qid)) $uid = $intendedA->id; else $uid = 0;
  if (!db\exists('r_boxes b LEFT JOIN u_relations r ON r.other=b.uid', ':uid=b.uid OR (:uid=r.main AND permission>=:B_READ-:B_RELATED)', compact('uid'))) return softErr(t('You are not the intended recipient.'));
  if (now() > $exp) return softErr(t('That encrypted information is no longer available.'));
  
  $fromA = r\acct($from);

  if ($download or $secret) {
    $msg = $download ? 'crypt file received' : 'crypt msg received';
    $fromA->tell($msg, ray('who time', $intendedA->fullName, ucwords(strtolower(strftime('%I:%M:%S%p %A %m/%d/%Y')))), t('encrypted message received'));
  }

  if ($download) {
    list ($s, $mimeType) = u\decryFile($path);
    header("Content-type: $mimeType");
    header("Content-Disposition: attachment; filename=\"$name\"");
    header("Content-Length: $size");
    return exitJust($s);
  }
  
  $title = t('Encrypted Message');
  $subtext = t('(from %who)', 'who', $fromA->fullName);
  $note = $secret ? "<br><p>$secret</p>" : '';
  
  if ($path) {
    $download = $name;
    $code = u\cryRay($ray + compact('download'));
    $note .= t('<p>Download encrypted file: <%a>%name</a> (%type %size MB)</p><p>Use right-click to Save As a different filename.</p>', '_a name type size', atag(MSG_URL0 . $code, compact('download')), $name, $type, number_format(round($size / 1000), 1));
    jsx('encrypted', 'msg', t('The file download is underway. It may take a few seconds to appear in your downloads folder.'));
  }
      
  $note = item($note);
  
  return cgform(compact(ray('title subtext note')));
}
