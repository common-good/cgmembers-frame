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
  $a = r\acct($uid); // defaults to current account when called from formResend_submit (without uid)
  $qid = $a->mainQid;
  if ($a->co) $qid .= '-A'; // happens only when opening a company account
  $code = $a->oneTimePass();
  $verify = (bool) $goto;
  $a->tell($verify ? 'verify' : 'password-reset', compact(ray('qid code')));
  if (isDEV or (NOT_PRODUCTION and $a->cAdmin)) {
    $page = $goto ?: 'reset';
    say(t('Attention admin: code=%code Link: <a href="%BASE_URL/%page/qid=%qid&code=%code&verify=%verify">reset</a>', compact(ray('page qid code verify'))));
  }
  return w\go($goto ?: '/empty', $goto ? t('Email sent') : 'password reset');
}
