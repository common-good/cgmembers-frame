<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Request a new password.
 */
function formPass($form, $sta, $qid = '') {
  $qid = urldecode(u\urlArgs());
  $title = t('Request New Password');
  $qid = textFld(REQ . t('Account:'), t('Your account ID or email address'), dft(u\neq($qid, 'password', '')));
  $submit = t('Request');

  $form = compact(ray('title qid submit'));
  focusOn('qid');
  return cgform($form);
}

function formPass_validate($form, &$sta) {
  extract(just('qid', $sta['input'], NULL));
  if (!$qid) return err('missing field', ['field' => 'account'], 'qid');
  if (!$uid = r\loginString($qid)) return err('bad account id', 'qid'); // "bad account id" is a key
  $a = r\acct($uid);
  if ($a->co and !$a->trialco) return err('no co pass', ['company' => $a->fullName], 'qid');
  $sta['input']['uid'] = $uid;
}

/**
 * Handle request to reset password or resend verification email
 * @param string $goto: where to redirect (defaults to empty page if resetting password)
 * @see formResend_submit()
 */
function formPass_submit($form, &$sta, $goto = '') {
  extract(just('uid', $sta['input'], NULL));
  
  $verify = (bool) $goto;
  $a = r\acct($uid); // defaults to current account when called from formResend_submit (without uid)
  $qid = $a->co ? "$a->mainQid-A" : $a->mainQid;

  list ($template, $pwMsg) = $verify
  ? ($a->co ? ['verify-co', t('Always sign in to your PERSONAL account. Then click your photo to choose the company account.')] : ['verify', ''])
  : ['password-reset', ''];
  
  $code = $a->oneTimePass();
  
  $a->tell($template, compact(ray('qid code pwMsg')));
  if (isDEV or (NOT_PRODUCTION and $a->admin)) {
    $page = $goto ?: 'reset';
    say(t('Attention admin: code=%code Link: <a href="%BASE_URL/%page/qid=%qid&code=%code&verify=%verify">reset</a>', compact(ray('page qid code verify'))));
  }
  return w\go($goto ?: '/empty', $verify ? t('Email sent') : 'password reset');
}
