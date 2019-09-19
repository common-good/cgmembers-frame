<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/signin.inc';

/**
 * Open a trial company account
 */
function formSignupCo($form, &$sta, $args = '') {
  global $mya;
  global $signupArgs; // for testing
  
  $qid = $fullName = $phone = ''; // defaults
  
  if ($mya and !$mya->trialco) {
    $a = ($mya->cAdmin and !$mya->co) ? $mya : $mya->agentA; // let admins open company accounts on behalf of others
    if ($coInfo = $a->signupCo) {
      extract(just('company companyPhone', $coInfo)); // predict co name
      if (!findCompany($company, $companyPhone)) list ($fullName, $phone) = [$company, u\fmtPhone($companyPhone, '-')];
    }
    $qid = $a->mainQid;
  }

  $title = t('Open a Trial Company Account');
  $subtext = t(<<< X
  <p>Fill out this short form to get your company started accepting %PROJECT payments quickly. You can accept up to %limit in payments before you need to convert to a regular company account (and open a personal account for signing in). There is no charge for a trial account or for a regular company account or a personal account.</p>
X
  , 'limit', u\fmtAmt(TRIALCO_LIMIT));
  
  $forother = item(w\lnk('/signup/' . $args, t('open an individual account instead')));
  
  u\setDft($source, nn($_COOKIE['ad']));
  $source2 = hidFld($source);
  $args = hidFld($args);

  extract(just('fullName legalName email', w\nameAndEmailFlds(TRUE, $fullName)));
  $email['#help'][1] = preg_replace('/ \\(.*\\)/', '', $email['#help'][1]); // don't talk about settings
  $phone = textFld(t('Company Phone:'), [t('Company phone number')], required($phone));
  $zip = zipFld();
  $contact = $qid ? NULL : textFld(t('Your Name:'), [t('Your full name')], required());
  $selling = sellingFld();
  $source = $qid ? NULL : sourceFld();
  $ownPhone = boolFld(t('Own Phone:'), t('Will you be running the CGPay app on your own Android phone? (%PROJECT is happy to supply one, with a $%EQUIP_DEPOSIT refundable equipment deposit).'), FALSE);
  $qid = $qid ? NULL : textFld(t('Account ID:'), [t('Your personal account ID'), t('If you are already a member of %PROJECT, type the account ID of your personal account. Otherwise leave this <b class="loud">BLANK</b>.')]);
  $submit = t('Submit');
  return cgform(compact(ray('title forother subtext source2 contact fullName zip phone email selling source ownPhone qid submit args')));
}
  
function formSignupCo_validate($form, &$sta) {
  extract(u\normalizeCase(just('contact fullName qid', $sta['input'])));
  extract(just('zip phone email selling', $sta['input']));

  foreach (ray('contact fullName') as $field) {
    $$field = trim(str_replace('  ', ' ', u\normalizeCase(@$$field))); // @ for testing
    if ($err = u\badName($$field)) return say($err, compact('field'), $field);
  }

  if ($qid = trim(strtoupper($qid)) and !r\acct($qid)) return say(t('That is not an account ID. Please check again (look in the lower left on your %PROJECT Card).'), 'qid');

  if ($err = u\badZip($zip)) return say($err, 'zip');
  if (!phoneOk($phone, US_COUNTRY_ID)) return;
  if (!emailOkay($email, TRUE, NULL)) return;
  if ($err = zBadSelling($selling)) return say($err, 'selling');
  
  u\preray(@compact(ray('contact fullName qid zip phone email selling')), $sta['input']); // fix formatting and add name
}
  
function formSignupCo_submit($form, &$sta) {
  global $mya;
  $dbFields = 'coType contact source selling fullName legalName email flags phone federalId country zip address city state helper stepsDone preid';
  extract(just($dbFields . ' source2 ownPhone qid args', $sta['input']));
  extract(just('code', nn($args)));

//    $a = r\acct($qid);
//    $postalAddr = $a->postalAddr;
//    list ($legalName, $federalId) = [$a->legalName, $a->federalId];
  if ($qid) {
    $contact .= '~' . $qid; 
    $helper = r\acct($qid)->id;
    unset($qid);
  } else $helper = CGID;
  
  list ($legalName, $federalId) = [CGF_LEGALNAME, CGF_EIN];
  
  $country = US_COUNTRY_ID;
  list ($city, $state, $latitude, $longitude) = zipCity($zip, $country);
  $preid = getPreId($source);
  if (@$source2) $source = trim(strtoupper($source2) . "-$source");  
  $flags = u\bit(B_CO) | u\bit(B_DEPENDS) | u\bit(B_CONFIRMED);
  $stepsDone = array_fill_keys(ray('signup discount verify'), FALSE);
  $coType = $acctType = CO_CUSTODIAN;
  if (!$a = new r\Acct($info = @compact(ray($dbFields)))) return say('cannot create account', 'zot');

  list ($myid, $name) = [$a->id, $a->name];  
  $mya = w\setAcct($myid); // this works for both an individual account (proSe) and a company account (with agent)
  $qid = $mya->mainQid;
  tellStaffNewMember($info + compact(ray('acctType qid name'))); // report to staff and user
  $pw1 = $mya->oneTimePass($name, TRUE); 
  r\notify($myid, 'verify-trial', ray('name qid code', $name, $qid, $pw1), TRUE); 
  if (nn($code)) w\invitation($code, $myid); // mark invitation used
  
  say('info saved');
  
  if (!$ownPhone) {
    be\invoice(r\acct(CGID), $mya, EQUIP_DEPOSIT, t('equipment deposit'), FOR_NONGOODS);
    say('refundable deposit');
  }
    
  $sta2['uid'] = $myid; // tell Drupal we're signed in
  $nextStepUrl = $mya->nextStepUrl('signup');
  formSignin_submit($form, $sta2, $nextStepUrl); // lead user to next step   
}

/**
 * Return city, state, and coordinates from zip and country
  {"post code": "01002", "country": "United States", "country abbreviation": "US", "places": [{"place name": "Amherst", "longitude": "-72.4646", "state": "Massachusetts", "state abbreviation": "MA", "latitude": "42.3671"}]}
  */
function zipCity($zip, $country) {
  if ($info = @file_get_contents('http://api.zippopotam.us/us/' . $zip)) {
    if ($info = json_decode($info)) $info = (array) nn($info->places)[0];
  }

  if (!$info) return [NULL, 0, 0, 0];
  return [$info['place name'], $info['state abbreviation'], $info['latitude'], $info['longitude']];
}
