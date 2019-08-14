<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Open a trial company account
 */
function formTrial($form, &$sta, $args = '') {
  global $mya;
  
  $title = t('Open a Trial Account');
  $subtext = t(<<< X
  <p>Fill out this short form to get your company started accepting %PROJECT payments quickly. You can accept up to %limit in payments before you need to convert to a regular company account (and open a personal account for signing in). There is no charge for a trial account and no charge for a regular account (personal and/or company).</p>
X
  , 'limit', u\fmtAmt(TRIALCO_LIMIT));
  
  extract(just('fullName legalName email', w\nameAndEmailFlds(TRUE)));
  $phone = textFld(t('Company Phone:'), [t('Company phone number')], required());
  $zip = zipFld();
  $yourName = textFld(t('Your Name:'), [t('Your full name')], required());
  $ownPhone = boolFld(t('Own Phone:'), t('Will you be running the CGPay app on your own Android phone? (%PROJECT is happy to supply one, with a $%EQUIP_DEPOSIT refundable equipment deposit).'), FALSE);
  $submit = t('Submit');
  return cgform(compact(ray('yourName fullName zip phone email ownPhone submit'));
}
  
function formTrial_validate($form, &$sta) {
  extract(u\normalizeCase(just('yourName fullName', $sta['input'])));
  extract(just('zip phone email', $sta['input']));

  if ($err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');
  $name = u\shortName($fullName);
  if ($err = u\badZip($zip)) return say($err, 'zip');
  if (!phoneOk($phone, US_COUNTRY_ID)) return;
  if (!emailOkay($email, TRUE, NULL)) return;
  
  u\preray(@compact(ray('yourName fullName zip phone email name')), $sta['input']); // fix formatting and add name
}
  
function formTrial_submit($form, &$sta) {
  extract(just('amount', $sta['input']));
  return w\goNextStep('sign');
}