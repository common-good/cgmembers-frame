<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Do whatever.
 */
function formAgent($form, &$sta, $args = '') {
  global $mya;

  $title = t('Convert from Trial Account');
  $subtext = t('<p>Until now your company account has been a "trial" account. We have discontinued the trial account program, which caused software nightmares.</p><p>If you haven\'t opened a personal account yet, sign out and click "Sign up" to open one. Otherwise type your personal account ID below (you can find it on your personal %PROJECT card, under your name).</p><p>Once you complete this step, you will sign in to your personal account to manage this one. </p><p>We will lead you now through several additional setup steps.</p>');
  
  $agent = db\get('other', 'r_relations', 'main=:main', ['main' => $mya->id]);
  $agent = textFld(REQ . t('Personal Account:'), [t('Account ID')], dft($agent ? r\acct($agent)->mainQid : ''));
  $phone = phoneFld(REQ . t('Phone:'), [t('Phone number'), t('Type the phone number on your personal account, for verification.')]);

  $coTypes = ray(CO_TYPES);
  unset($coTypes[CO_PERSONAL]);
  $coType = radiosFld(t('Account type:'), '', required(), $coTypes);
  $legalName = textFld(t('Legal name:'), [t('Company legal name'), t('Type the full legal name of the company.')], required($mya->fullName));
  extract(ssnFields(FALSE, required()));
  $submit = t('Next');
  
  return cgform(compact(ray('title subtext agent phone coType legalName federalId dob submit'))); // not setupFoot (don't skip this step)
}

function formAgent_validate($form, &$sta) {
  global $mya;
  extract(just('agent phone federalId dob', $sta['input']));

  $agent = strtoupper(trim($agent));
  if (!$a = r\acct($agent) or $a->co or !$a->member) return say(t('That is not an account ID of a completed personal account.'), 'agent');
  if ($err = u\badPhone($phone)) return say($err, 'phone');
  if ($phone != $a->phone) return say(t('The phone number on that account does not match.'), 'phone');
  if (!formBasic_validate($form, $sta, TRUE, 'federalId dob')) return;
  u\preRay(compact(ray('agent phone')), $sta['input']);
}

function formAgent_submit($form, &$sta) {
  global $mya;
  extract(just('agent coType legalName federalId dob', $sta['input']));

  $a = r\acct($agent);
  $mya->newRelation(ray('other permission', $a->id, r\perm(B_MANAGE)));
  $mya->setBit('depends', FALSE);
  $helper = $a->id; // make sure agreement is shown with proper suggested signer
  $mya->update(compact(ray('coType helper legalName federalId dob')));

  return w\goNextStep('agent');
}