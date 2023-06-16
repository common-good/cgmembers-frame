<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Accept an application for fiscal sponsorship.
 * Three cases:
 * - Non-member organization applying (!$mya)
 * - Member organization applying ($mya and $mya->co and !$mya->sponsored)
 * - Displaying existing fiscal sponsorship ($mya and $mya->sponsored)
 */

function formSponsor($form, &$sta, $args = '') {
  $mya = w\eLinkAcct($sta, $args, 'any');
  $isCo = ($mya and $mya->co);
  
  u\setDft($source2, nni($_COOKIE, 'ad'));
  $source2 = hidFld($source2);

  $title = t('Fiscal Sponsorship Application');
  $subtext = t('<p>%PROJECT provides fiscal sponsorship for compatible, mission-aligned organizations and projects working for social, racial, environmental, and economic justice. This allows you to accept tax-deductible contributions without having to apply to the IRS for nonprofit status. %PROJECT provides some extra benefits and charges a lower fee than most other fiscal sponsors. Complete this form to apply, or give us a call.</p><p><span class="loud">If your project already has a %PROJECT account</span>, sign in to that account before applying.</p>');

  $mission = areaFld(REQ . t('Mission:'), [t('Organization\'s mission')], dft($isCo ? $mya->mission : '') + maxlen(250));
  $activities = areaFld(REQ . t('Activities:'), [t('Organization\'s activities'), t('What do you <i>do</i> to advance your mission?')], dft($isCo ? $mya->activities : '') + maxlen(250));
  $gross = dolFld(REQ . t('Expected Income:'),[t('Gross annual income'), t('How much income (in dollars) do you expect in the next 12 months, before expenses?')], dft($isCo ? $mya->gross : ''));
  $employees = intFld(REQ . t('Employees:'), [t('Employee count'), t('How many employees does the organization have?')], dft($isCo ? $mya->employees : ''));
  $checksIn = intFld(REQ . t('Checks In:'), [t('Monthly incoming checks'), t('Number of donation checks (not the dollar amount) you expect to receive in a typical month.')], dft($isCo ? $mya->checksIn : ''));
  $checksOut = intFld(REQ . t('Checks Out:'), [t('Number of outgoing checks'), t('Number of payments (not the dollar amount) you expect to make in a typical month.')], dft($isCo ? $mya->checksOut : ''));
  $helper = hidFld(UID_SUPER);
  
  $comments = areaFld(t('Comments:'), [t('(optional)')]);
  
  if ($isCo) {
    if (!$mya->can(B_MANAGE)) return say('no sponsor perm', 'err');
    if (!$mya->ok and !$mya->sponsored) return say(t('You must finishing opening your company account before you can use it as a model for a fiscal sponsorship account.'));
    $contact = $fullName = $phone = $email = $zip = $country = $source = $federalId = $coType = NULL;
    if ($mya->sponsored) $comments = NULL;
  } else {
    $contact = textFld(REQ . t('Your Name:'), [t('Your full name')]);
    $fullName = textFld(REQ . t('Organization:'), [t('Organization or project name')]);
    $phone = phoneFld(REQ . t('Org Phone:'), [t('Your phone number')]);
    $email = emailFld(REQ . t('Org Email:'), [t('Email')]);
    $country = selectFld(t('Country:'), t(''), dft(US_COUNTRY_ID), r\countryList()); // can't use required() here
    $zip = zipFld();
    $federalId = passFld(REQ . tr('einTitle'), ['', tr('einDesc') . t(' This cannot be an individual\'s SSN. You can get an EIN from the IRS free online at <%a>this link</a>.', '_a', atag('https://www.irs.gov/businesses/small-businesses-self-employed/how-to-apply-for-an-ein'))], autofill('off'));
    $coType = coTypeFld($mya ? $mya->coType : NULL, [CO_PERSONAL, CO_SOLE_PROPRIETOR], t('The "personal" and "sole-proprietor" choices are intentionally omitted here. If you are taking this intitiative on your own, choose "%club". You will need a volunteer oversight board of at least two people who meet with you at least once a quarter.', 'club', ray(CO_TYPES)[CO_CLUB]));
    $source = sourceFld();
  }

  $submit = ($mya and $mya->sponsored) ? t('Update') : t('Submit');
  
  return cgform(compact(ray('title subtext contact fullName phone email country zip federalId coType source mission activities gross employees checksIn checksOut comments submit')));
}

function formSponsor_validate($form, &$sta) {
  $mya = w\eLinkAcct($sta);
  extract(just('fullName phone email zip federalId gross employees checksIn checksOut', $sta['input'], NULL));
  foreach (ray($flds = 'gross employees checksIn checksOut') as $k) if (isset($$k) and $err = u\badAmount($$k, '>=0')) return say($k . ': ' . $err, $k);
  
  if (!($mya and $mya->co)) {
    if ($err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');
    $fullName = u\normalizeCase($fullName);
    
    if ($err = u\badPhone($phone)) return say($err, 'phone');
    if (!emailOkay($email, FALSE, nn($myid) ? r\acct($myid) : '')) return;
    if ($err = u\badZip($zip)) return say($err, 'zip');
    if (isset($federalId) and $err = u\badEin($federalId, TRUE)) return say($err, 'federalId');
    $flds .= ' fullName phone email zip federalId';
  }
  u\preray(compact(ray($flds)), $sta['input']);
}

function formSponsor_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta); 

  $dbFlds = 'contact fullName legalName federalId coType coFlags email flags phone zip country notes source city state latitude longitude mission activities gross employees checksIn checksOut helper';
  extract(just('comments source2 ' . $dbFlds, $sta['input'], NULL));
  
  if ($comments or $federalId) {
    if ($federalId) $comments = "EIN: $federalId. $comments";
    $notes = $comments ? r\stampNote($comments) : '';
  }

  if (!isset($contact)) { // using company data
    if ($mya and $mya->sponsored){ // just updating
      $mya->update(compact(ray('mission activities checksIn checksOut')));
      return say('info saved');
    }
    if ($mya and $mya->co) { // copying from org to new FBO org
      $contact = $mya->agentA->fullName;
      foreach (ray('fullName email phone zip country source city state latitude longitude coType') as $k) $$k = $mya->$k;
      $mya->update(compact(ray('gross employees'))); // update these in old record
    }
  }
  
  list ($city, $state, $latitude, $longitude) = r\zipCity($zip);

  $flags = u\bit(B_NONUDGE) | u\bit(B_CO);
//  $fullName = PROJECT . t(' FBO ') . $fullName;
  $legalName = CGF_LEGALNAME;
  $federalId = CGF_EIN;
  $coFlags = u\bit(CO_SPONSORED);
  if ($coType != CO_NONPROFIT) $coFlags |= u\bit(CO_FLIP); // make sure it gets a donate button on our promo site (not a buy credit button)
  $subs = compact(ray($dbFlds)) + ($mya ? [] : ['NEW ORG!' => 1]);
  $subs['to'] = 'partnerships@' . CG_DOMAIN;
  $subs['state'] = r\realState($state);
  $subs['coType'] = ray(CO_TYPES)[$coType];
  r\tellAdmin(t('Fiscal Sponsorship Application'), $subs);
  $msg = 'got application';

  $acctType = hidFld(CO_CUSTODIAN);
  u\setDft($source, '');
  if ($source2) $source = trim(mb_strtoupper($source2) . "-$source");
  $info = []; foreach (ray($dbFlds) as $k) if (isset($$k)) $info[$k] = $$k;
  if (!$a = new r\Acct($info)) return softErr(t('cannot create account'));
  if (!$mya) $msg .= '|meanwhile join';
  $a->stepDone('signup');
  return sayEmpty($msg);
}
