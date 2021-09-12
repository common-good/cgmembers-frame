<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Show a form that can function offline.
 */
function formOffline($form, &$sta, $args = '') {
  global $mya;
  extract(just('nothing', $args, NULL));

  $title = t('Whatever');
  $subtext = t('Details.');

  $choices = ray('this that theOther');
  $radio = radiosFld(REQ . t('Radios:'), t('Details here'), dft(-1), $choices); // no default
  $select = selectFld(REQ . t('Selection:'), t('Details here'), dft(2), $choices);
  $text = textFld(t('Text:'), [t('Hint'), t('Details here')], dft('value'));
  js('offline');

  $submit = t('Save');
  
  return cgform(compact(ray('title subtext radio text submit')));
}

function formOffline_validate($form, &$sta) {
  global $mya;
  extract(just('text', $sta['input'], NULL));

  if ($err = u\badAmount($text)) return say($err, 'text');
  u\preRay(compact('text'), $sta['input']);
}

function formOffline_submit($form, &$sta) {
  global $mya;
  extract(just('text', $sta['input'], NULL));

  $mya->update(compact('text'));

  return go('dashboard');
}
