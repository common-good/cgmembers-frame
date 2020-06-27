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
function formEmailUs($form, &$sta, $args = '') {
/**/  die('NYI');
  global $mya;
  extract(just('nothing', $args));

  $title = t('Whatever');
  $subtext = t('Details.');

  $choices = ray('this that theOther');
  $radio = radiosFld(t('Radios:'), t('Details here'), required(2), $choices);
  $text = textFld(t('Text:'), [t('Your text'), t('Details here')], dft('value'));
  jsx('model', compact('choices'));

  $submit = t('Save');
  
  return cgform(compact(ray('title subtext radio text submit')));
}

function formEmailUs_validate($form, &$sta) {
  global $mya;
  extract(just('text', $sta['input']));

  if ($err = u\badAmount($text)) return say($err, 'text');
  u\preRay(compact('text'), $sta['input']);
}

function formEmailUs_submit($form, &$sta) {
  global $mya;
  extract(just('text', $sta['input']));

  $mya->update(compact('text'));

  return w\go('');
}
