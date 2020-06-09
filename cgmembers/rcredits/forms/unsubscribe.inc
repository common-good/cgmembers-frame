<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Send an email to Common Good
 */
function formUnsubscribe($form, &$sta, $args = '') {
  extract(just('code qid', $args, ''));

  if (!u\crypted('P', $code)) return softErr(t('That is not a valid Unsubscribe link.'));
  db\q('UPDATE r_invites SET nonudge=:NOW WHERE email=:code', compact('code'));
  $msg = t('<p>Your request to unsubscribe is received.</p>');  
  $email = u\decry('P', $code);
  r\tellAdmin(t('Unsubscribe #') . $code, compact(ray('email qid')));
  
  if ($qid) {
    $title = t('Unsubscribe');
    $subtext = $msg . t('<p>If you would, please tell us why you are unsubscribing, so we can try to improve. If you simply are not sure what the point of it is, you might want to see our short video (it\'s actually pretty amazing). %vid</p>', '_vid', btn(VIDEO_URL, t('See video')));
    $why = textFld(t('Reason:'), [t('Why (please, but optional)')]);
    $qid = hidFld($qid);
    $email = hidFld($email);
    $submit = t('Unsubscribe');
    return cgform(compact(ray('title subtext why email qid submit')));
  } else { say($msg); }
}

function formUnsubscribe_submit($form, &$sta) {
  extract(just('why email qid', $sta['input'], ''));
  $info = compact(ray('why email'));
  
  if ($a = r\acct($qid) and $email == $a->email) {
    $a->update('notes', r\stampNote('ZAP "' . $why . '"') . $a->notes);
  } else { $info['fishy'] = t('Might be a hack; email disagrees with account.'); }
  
  r\tellCo(t('%nm unsbuscribed (%qid)', 'nm qid', $a->fullName, $qid), $info, $a->id);
  return go('/empty', t('Thank you for telling us!'));
}
