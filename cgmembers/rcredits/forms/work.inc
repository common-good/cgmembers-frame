<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Get information about member's calling and work life.
 */
function formWork($form, &$sta, $args = '') {
  global $mya;

  $title = t('Your Work');

  $calling = textFld(t('Life Calling:'), [t('Your calling in life (optional)'), t('One of the main purposes of %PROJECT is to provide you the opportunity to do what you feel called to do in life. What would bring you joy to do (maybe something you\'re not doing yet)?')]);

  $companyOptions = array(
    'owner' => t('owner'),
    'employee' => t('employee'),
    'contractor' => t('other worker (contractor, supplier, etc.)'),
  );
  $copts = boxOpt('companyOptions', $companyOptions, $sta);

  $companyInfo = array(
    'company_text' => item(t('Are you connected to a local company, or to any %PROJECT member company? Otherwise leave this part blank.')),
    'company' => textFld(t('Company: '), [t('Your company')]),
    'companyPhon' => textFld(t('Company Ph<span>&nbsp;</span>one:'), [t('Company phone number')]), // Google Chrome bug requires obfuscation of "phone"
    'companyOptions' => boxesFld('', '', @$copts, $companyOptions), 
  );
  $companyInfo = fieldSet('companyInfo', $companyInfo, BARE);


  $submit = t('Save');
  
  return cgform(compact(ray('title calling companyInfo submit')));
}

function formWork_validate($form, &$sta) {
  global $mya;
  extract(just('company companyPhon companyOptions', $sta['input']));

  if (@$company or @$companyPhon) { // NOT or $gotrelation (let user get by without)
    foreach (ray('company companyPhon') as $one) {
      if (u\nonish(@$$one)) $$one = '';
      // (no) return say('missing field', array('field' => $one), $one);
    }
    if (@$company and $err = u\badName($company)) return say($err, array('field' => 'Company'), 'company');
    if (@$companyPhon and !$companyPhone = u\fmtPhone($companyPhon, '+n')) return say('bad company phone', compact('companyPhone'), 'companyPhone');
    if (!$gotrelation) return say('what relation', 'company');
    $signupCo = @compact(ray('company companyPhone employee owner contractor'));
  } else $signupCo = NULL;  

  u\preRay(compact(ray('signupCo')), $sta['input']);
}

function formWork_submit($form, &$sta) {
  global $mya;
  extract(just('calling signupCo', $sta['input']));

  $mya->update(@compact(ray('calling signupCo')));
  if (@$signupCo) {
    suCompanyRelation($mya->id, $signupCo);
    $mya->update(compact('signupCo'));
  }
  
  return w\goNextStep('work');
}

/**
 * Gather the just-submitted checkbox values as which-boxes-to-check for boxesFld.
 * @param string $field: name of boxesFld field
 * @param assoc $opts: display names of the boxes, keyed by their index
 * @param assoc $sta: form status
 * @return flat array of values (not display names) of checked boxes
 */
function boxOpt($field, $opts, $sta) {
  foreach ((@$sta['input'][$field] ?: []) as $k => $v) $res[] = $k; // $opts[$k]; // note that k==v
  return @$res ?: [];
}