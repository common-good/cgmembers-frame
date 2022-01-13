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
  if (strhas($to, '@') and !strhas($to, ' @ ') and empty($secret)) return say(t('If you know the member\'s email address, don\'t use this form -- just send them an email.'), 'to'); // !strhas is needed to accept results of autocomplete
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
  
  if ($fileData = unserialize($fileData) or strlen($secret)) { // encrypted file or message
    $at = now() + HOUR_SECS * $expires;
    $ray = ray('qid from exp', $a->mainQid, $fromQid, $at);
    $message .= '<br>';
    
    if ($fileData) {
      $code = u\cryRay($ray + just('path name type size', $fileData));
      $url = "$base_url/community/message/x/code=$code";
      $message .= t('<br>Download encrypted file ("%name", %size MB): <%a>click here</a>.', '_a name size', atag($url, ['download'=>1]), $fileData['name'], round($fileData['size'] / 1000, 1));
    }
    if (strlen($secret)) { // encrypt secret message
      $code = u\cryRay($ray + compact('secret'));
      $url = "$base_url/community/message/x/code=$code";
      $message .= t('<br>Secret: <%a>click here</a>.', '_a', atag($url));
    }
    $message .= t('<br>Encrypted data expires at %tm on %dt.', 'tm dt', u\tm($at), u\fmtDate($at));
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
  extract(just('qid from secret path name type size exp', $ray, NULL));
  
  if ($intendedA = r\acct($qid)) $uid = $intendedA->id; else $uid = 0;
  if (!db\exists('r_boxes b LEFT JOIN u_relations r ON r.other=b.uid', ':uid=b.uid OR (:uid=r.main AND permission>=:B_READ-:B_RELATED)', compact('uid'))) return softErr(t('You are not the intended recipient.'));
  if (now() > $exp) return softErr(t('That encrypted information is no longer available.'));
  
  $msg = $secret ? 'crypt msg received' : 'crypt file received';
  r\acct($from)->tell($msg, ray('who time', $intendedA->fullName, ucwords(strtolower(strftime('%I:%M:%S%p %A %m/%d/%Y')))), t('encrypted message received'));
  
  if ($secret) { // short message
    return sayEmpty(t('<h3>Encrypted Message</h3>') . $secret);
  } else { // file
    list ($s, $mimeType) = u\decryFile($path);
    header("Content-type: $mimeType");
    header("Content-Disposition: attachment; filename=\"$name\"");
    header("Content-Length: $size");
    return exitJust($s);
  }
}
