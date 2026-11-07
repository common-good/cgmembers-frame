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
  extract(just('pass', $sta['input'], NULL));

  if (!$mya or !$mya->admin) {
    if (isset($pass) and !$mya->passwordOkay($pass, 'pass', $err)) return say($err, 'pass');
  }
  if ($mya and $mya->admin and !u\strongPass(nn($sta['input']['pw']), array_merge(ray($mya->fullName), [strstr($mya->email, '@', TRUE)]))) {
    return say(t('An administrator password must be at least 12 characters, with at least 8 different ones, and hard to guess (no name, email or common word). A long random one from a password manager is best.'), 'pw');
  }
}

function formChangePassword_submit($form, &$sta) {
  extract($info = just('pw', $sta['input']));
  global $mya;
  $mya->update('pass', $pw);
  say(t('Your new password has been saved.'));
  return w\go('dashboard');
}
