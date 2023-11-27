<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Get the member's security info.
 * @param string $arg: "gotPhoto" if just sucessfully uploaded a photo
 */
function formSecurity($form, &$sta, $arg = '') {
  global $base_url;
  global $mya;

  if ($arg == 'gotPhoto') say('got photo');

  $btns = btn("$base_url/settings/security/change-password", t('password'))
  . ' ' . btn("$base_url/settings/photo", t('photo'));
  
  $title = t('Security Information');
  
  $mfa = boolFld(t('Multifactor:'), t('Use multi-factor authentication?'), $mya->canMfa);
  $change = item($btns, t('Change:'));
  
  $disAdmin = ($mya->admin and !$mya->proSe) ? disabled() : [];
  $questionDesc = t('Type a question no one but you could guess the answer to.');
  $question = $mya->co ? NULL : textFld(REQ . t('Question:'), [t('Choose a security question'), $questionDesc], dft($mya->question) + $disAdmin);
  $answer = $mya->co ? NULL : textFld(REQ . t('Answer:'), [t('The answer to that question')], dft($mya->answer) + $disAdmin);

  $submit = submit();
  $form = compact(ray('title mfa change question answer submit'));

  return cgform($form);
}

function formSecurity_validate($form, &$sta) {
  global $mya;
//  extract(just('', $sta['input']));
}

function formSecurity_submit($form, &$sta) {
  global $mya;
  $info = just('mfa question answer', $sta['input']);

  foreach (just('question answer', $info) as $k => $v) if ($v == R_ON_FILE or $v == nni($form[$k], '#default_value')) unset($info[$k]); // don't change if on file or unchanged

  $mya->update($info);

  say('info saved');
}