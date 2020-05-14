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

  jsx('work');
  $title = t('Your Work');

  $calling = textFld(t('Life Calling:'), [t('Your calling in life (optional)'), t('One of the main purposes of %PROJECT is to provide you the opportunity to do what you feel called to do in life. What would bring you joy to do (maybe something you\'re not doing yet)?')], dft($mya->calling));

  $companyOptions = array(
    'owner' => t('owner'),
    'employee' => t('employee'),
    'contractor' => t('other worker (contractor, supplier, etc.)'),
  );
  $copts = boxOpt('companyOptions', $companyOptions, $sta);

  $companyInfo = array(
    'company_text' => item(t('Are you connected to a local company, or to any %PROJECT member company? Otherwise leave this part blank.')),
    'company' => textFld(t('Company: '), [t('Your company')]),
    'companyPhone' => phoneFld(t('Company Phone:'), [t('Company phone number')]),
    'companyOptions' => boxesFld('', '', @$copts, $companyOptions), 
  );
  $companyInfo = fieldSet('companyInfo', $companyInfo, BARE);
  
  return cgform(compact(ray('title calling companyInfo')) + setupFoot());
}

function formWork_validate($form, &$sta) {
  global $mya;
  extract($input = just('company companyPhone companyOptions', $sta['input'], NULL));

  if (@$company or @$companyPhone) { // NOT or $gotrelation (let user get by without)
    foreach (ray('company companyPhone') as $one) {
      if (u\nonish(@$$one)) $$one = '';
      // (no) return say('missing field', array('field' => $one), $one);
    }
    if (@$company and $err = u\badName($company)) return say($err, array('field' => 'Company'), 'company');
    if (@$companyPhone and !$companyPhone = u\fmtPhone($companyPhone, '+n')) return say('bad company phone', compact('companyPhone'), 'companyPhone');
    if (!$companyOptions) return say('what relation', 'company');
    $signupCo = compact(ray('company companyPhone')) + array_fill_keys(array_values($companyOptions), TRUE);

  } else $signupCo = NULL;  

  u\preRay(compact(ray('signupCo')), $sta['input']);
}

function formWork_submit($form, &$sta) {
  global $mya;
  extract($input = just('calling signupCo', $sta['input']));

  $mya->update(@compact(ray('calling')));
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