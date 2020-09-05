<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

function formPass($form, $sta, $id = '') {
  $id = urldecode(u\urlArgs());
  $title = t('Request New Password');
  $name = textFld(t('Account:'), t('Username (the letters of your full name, all lowercase), Email, or Account ID'), required(u\neq($id, 'password', '')));
  $submit = t('Request');

  $form = compact(ray('title name submit'));
  focusOn('name');
  return cgform($form);
}

function formPass_validate($form, &$sta) {
  extract(just('name', $sta['input'], NULL));
  if (!$name) return say('missing field', ['field' => 'account'], 'name');
  if (!$uid = r\loginString($name)) return say('bad account id', 'name');
  $a = r\acct($uid);
  if ($a->co and !$a->trialco) return say('no co pass', ['company' => $a->fullName], 'name');
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
  $myid = $uid ?: $a->id;
  $qid = r\acct($myid)->mainQid;
  $name = $a->name;
  $code = $a->oneTimePass($name);
  $verify = (bool) $goto;
//  list ($region, $code) = [strtolower(R_SERVER_ID), $pw];
  $a->tell($verify ? 'verify' : 'password-reset', compact(ray('name qid code')));
  if (isDEV or (NOT_PRODUCTION and $a->cAdmin)) {
    $page = $goto ?: 'reset';
    say(t('Attention admin: code=%code Link: <a href="%BASE_URL/%page/id=%name&code=%code&verify=%verify">reset</a>', compact(ray('page name code verify'))));
  }
//  r\tellAdmin('password reset', compact(ray('name myid code')));
//  say($goto ? t('Email sent') : 'password reset');
  return w\go($goto ?: '/empty', $goto ? t('Email sent') : 'password reset');
}