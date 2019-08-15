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
  
  $forother = item(w\lnk('/signup', t('open a regular account instead')));
  
  u\setDft($source, nn($_COOKIE['ad']));
  $preid = getPreId($source);

  extract(just('fullName legalName email', w\nameAndEmailFlds(TRUE)));
  $email['#help'][1] = preg_replace('/ \\(.*\\)/', '', $email['#help'][1]); // don't talk about settings
  $phone = textFld(t('Company Phone:'), [t('Company phone number')], required());
  $zip = zipFld();
  $yourName = textFld(t('Your Name:'), [t('Your full name')], required());
  $ownPhone = boolFld(t('Own Phone:'), t('Will you be running the CGPay app on your own Android phone? (%PROJECT is happy to supply one, with a $%EQUIP_DEPOSIT refundable equipment deposit).'), FALSE);
  $submit = t('Submit');
  return cgform(compact(ray('title forother subtext source preid yourName fullName zip phone email ownPhone submit')));
}
  
function formTrial_validate($form, &$sta) {
  extract(u\normalizeCase(just('yourName fullName', $sta['input'])));
  extract(just('zip phone email', $sta['input']));

  foreach (ray('yourName fullName') as $field) {
    $$field = trim(str_replace('  ', ' ', u\normalizeCase(@$$field))); // @ for testing
    if ($err = u\badName($$field)) return say($err, compact('field'), $field);
  }

  if ($err = u\badZip($zip)) return say($err, 'zip');
  if (!phoneOk($phone, US_COUNTRY_ID)) return;
  if (!emailOkay($email, TRUE, NULL)) return;
  
  u\preray(@compact(ray('yourName fullName zip phone email')), $sta['input']); // fix formatting and add name
}
  
function formTrial_submit($form, &$sta) {
  global $mya;
  $dbFields = 'preid source fullName legalName email flags phone federalId country zip address city state helper notes';
  extract(just($dbFields + ' ownPhone yourName', $sta['input']));
  list ($legalName, $federalId, $country, $helper, $notes) = [CGF_LEGALNAME, CGF_EIN, US_COUNTRY_ID, CGID, $yourname];
//  list ($address, $city, $state) =
  $flags = u\bit(B_CO) | u\bit(B_DEPENDS) | u\bit(B_CONFIRMED);
  $stepsDone = 'verify'; // just one step
  if (!$a = new r\Acct(compact(ray($dbFields)))) return say('cannot create account', 'zot');

  list ($myid, $name) = [$a->id, $a->name];  
  $mya = w\setAcct($myid); // this works for both an individual account (proSe) and a company account (with agent)
  $qid = $mya->mainQid;
  tellStaffNewMember($info + compact(ray('acctType qid name'))); // report to staff and user
  $pw1 = $mya->oneTimePass($name, TRUE); 
  
  say('company is ready');

  return w\goNextStep('signup');  

  r\notify($myid, 'verify - trial', ray('name qid code', $name, $qid, $pw1), TRUE); 
  
  $sta2['uid'] = $myid; // tell Drupal we're signed in
  $nextStepUrl = $mya->nextStepUrl('signup', $msg);
  say($msg);
  formSignin_submit($form, $sta2, $nextStepUrl); // lead user to verification (final) step   
}