<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once R_ROOT . '/forms/signup.inc'; // for validation

/**
 * Open a trial company account
 * If called from a partner signup, suppress the fields we already have.
 */
function formSignupCo($form, &$sta, $args = '') {
  global $mya;
  global $signupArgs; // for testing
  
  $dftFlds = ray('fullName phone contact email zip source');
  foreach ($dftFlds as $k) $$k = '';
  
  if ($mya) {
    if ($mya->trialco) {
      foreach ($dftFlds as $k) $$k = $mya->$k; // pick up where we left off
      $source = $mya->helperA->fullName;
      $qid = $mya->helperA->mainQid;
    } else {
      $a = ($mya->cAdmin and !$mya->co) ? $mya : $mya->agentA; // let admins open company accounts on behalf of others
      if ($coInfo = $a->signupCo) {
        extract(just('company companyPhone', $coInfo)); // predict co name
        if (!findCompany($company, $companyPhone)) list ($fullName, $phone) = [$company, $companyPhone];
      }
      $qid = $a->mainQid;
    }
  } else $qid = '';

  $title = t('Open a Trial Company Account');
  $subtext = t(<<< X
  <p>Fill out this short form to get your company started accepting %PROJECT payments quickly. You can accept up to %limit in payments before you need to convert to a regular company account (and open a personal account for signing in). There is no charge for a trial account or for a regular company account or a personal account.</p>
X
  , 'limit', u\fmtAmt(TRIALCO_LIMIT));
  
  $forother = item(w\lnk('/signup/' . $args, t('open an individual account instead')));
  
  u\setDft($source, nn($_COOKIE['ad']));
  $source2 = hidFld($source);
  $args = hidFld($args);

  if ($fullName and $email and $phone and $zip and $contact) { // called from partner signup
    foreach (ray('fullName email phone zip contact') as $k) $$k = hidFld($$k);
  } else {
    extract(just('fullName email', w\nameAndEmailFlds(TRUE, $fullName, $email)));
    $email['#help'][1] = preg_replace('/ \\(.*\\)/', '', $email['#help'][1]); // don't talk about settings
    $phone = textFld(t('Company Phone:'), [t('Company phone number')], required($phone ? u\fmtPhone($phone, '-') : ''));
    $zip = zipFld($zip);
    $contact = $qid ? NULL : textFld(t('Your Name:'), [t('Your full name')], required($contact));
  }
  $selling = sellingFld();
  $source = ($qid or $source) ? NULL : sourceFld('');
  $ownPhone = boolFld(t('Own Phone:'), t('Will you be running the CGPay app on your own Android phone? (%PROJECT is happy to supply one, with a $%EQUIP_DEPOSIT refundable equipment deposit).'), FALSE);
  $qid = $qid ? hidFld($qid) : textFld(t('Account ID:'), [t('Your personal account ID'), t('If you are already a member of %PROJECT, type the account ID of your personal account. Otherwise leave this <b class="loud">BLANK</b>.')]);
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

  if ($qid = trim(strtoupper(nn($qid))) and !r\acct($qid)) return say(t('That is not an account ID. Please check again (look in the lower left on your %PROJECT Card).'), 'qid');

  if ($err = u\badZip($zip)) return say($err, 'zip');
  if ($err = u\badPhone($phone)) return say($err, 'phone');
  if (!emailOkay($email, TRUE, NULL)) return;
  if ($err = zBadSelling($selling)) return say($err, 'selling');
  
  u\preray(@compact(ray('contact fullName qid zip phone email selling')), $sta['input']); // fix formatting and add name
}
  
function formSignupCo_submit($form, &$sta) {
  global $mya;
  $dbFields = 'coType contact source selling fullName legalName email flags phone federalId zip state helper';
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
  
  list ($city, $state, $latitude, $longitude) = zipCity($zip);
  $preid = getPreId($source);
  if (@$source2) $source = trim(mb_strtoupper($source2) . "-$source");  
  $flags = ($mya ? $mya->flags : 0) | u\bit(B_CO) | u\bit(B_DEPENDS) | u\bit(B_CONFIRMED);
//  $stepsDone = array_fill_keys(ray('signup discount verify'), FALSE);
  $coType = $coType = CO_CUSTODIAN;
  $info = @compact(ray($dbFields));
  
  if ($mya) {
    $mya->update($info);
  } else {
    if (!$mya = new r\Acct($info)) return say('cannot create account', 'zot');
    w\setAcct($mya->id); // this works for both an individual account (proSe) and a company account (with agent)
  }

  foreach (ray(CO_DFTS) as $i) $mya->setCoBit(APP_CANS + $i);

  list ($myid, $name) = [$mya->id, $mya->name];  
  $qid = $mya->mainQid;
  tellStaffNewMember($info + compact(ray('qid name'))); // report to staff and user
  $pw1 = $mya->oneTimePass($name, TRUE); 
  r\notify($myid, 'verify-trial', ray('name qid code', $name, $qid, $pw1), TRUE); 
  if (nn($code)) w\invitation($code, $myid); // mark invitation used
  
  say('info saved');
  
  if (!$ownPhone) {
    be\invoice(r\acct(CGID), $mya, EQUIP_DEPOSIT, t('equipment deposit'), FOR_NONGOODS);
    say('refundable deposit');
  }
    
  $sta['uid'] = $myid; // tell Drupal we're signed in
  
  return w\goNextStep('signup'); 
}
