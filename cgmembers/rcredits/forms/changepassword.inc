<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Change from existing password to a new one.
 * @todo: refactor this with formResetPassword
 */
function formChangePassword($form, &$sta) {
  global $mya;
  $title = t('Choose a New Password');
  $subtext = tr('pass advice');
  $pass = $mya->admManageAccts ? NULL : passFld(REQ . t('Current (old) Password:'));
  $submit = t('Save');
  return cgform(compact('title', 'subtext') + w\pickPassword(TRUE, REQ) + compact('pass', 'submit'));
}

function formChangePassword_validate($form, &$sta) {
  global $mya;
  extract(just('pass pw', $sta['input'], [NULL, '']));

  if (!$mya or !$mya->admin) {
    if (isset($pass) and !$mya->passwordOkay($pass, 'pass', $err)) return say($err, 'pass');
  }
  $admin = ($mya and $mya->admin);
  if (($pw !== '' or $admin) and !u\okPass($pw, $admin, $mya ? u\pwAvoid($mya) : [])) {
    return say($admin ? t('An administrator password must score higher than an ordinary one: for example 10 characters with capitals, lower case, digits and punctuation (or longer with fewer kinds). Common words, your name and your email count as one letter each. A generated one from a password manager is best.') : 'weak pass', 'pw');
  }
}

function formChangePassword_submit($form, &$sta) {
  extract($info = just('pw', $sta['input']));
  global $mya;
  $mya->update('pass', $pw);
  say(t('Your new password has been saved.'));
  return w\go('dashboard');
}
