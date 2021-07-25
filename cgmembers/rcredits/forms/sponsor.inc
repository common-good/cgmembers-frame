<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Accept an application for fiscal sponsorship.
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
  $gross = numFld(REQ . t('Expected Income:'),[t('Gross annual income'), t('How much income do you expect in the next 12 months, before expenses?')], dft($isCo ? $mya->gross : ''));
  $employees = intFld(REQ . t('Employees:'), [t('Employee count'), t('How many employees do you have?')], dft($isCo ? $mya->employees : ''));
  $checksIn = intFld(REQ . t('Checks In:'), [t('Monthly incoming checks'), t('Number of donation checks you expect to receive in a typical month.')], dft($isCo ? $mya->checksIn : ''));
  $checksOut = intFld(REQ . t('Checks Out:'), [t('Number of outgoing checks'), t('Number of payments you expect to make in a typical month.')], dft($isCo ? $mya->checksOut : ''));
  $comments = areaFld(t('Comments:'), [t('(optional)')]);
  
  if ($isCo and ($mya->ok or $mya->admin)) {
    if (!$mya->can(B_MANAGE)) return say('no sponsor perm', 'err');
    $contact = $fullName = $phone = $email = $zip = $country = $source = NULL;
    if ($mya->coCan(CO_SPONSORED) or u\starts($mya->fullName, t('%PROJECT FBO '))) $gross = $employees = $comments = NULL;
  } else {
    $contact = textFld(REQ . t('Your Name:'), [t('Your full name')]);
    $fullName = textFld(REQ . t('Organization:'), [t('Organization or project name')]);
    $phone = phoneFld(REQ . t('Org Phone:'), [t('Your phone number')]);
    $email = emailFld(REQ . t('Org Email:'), [t('Email')]);
    $country = selectFld(t('Country:'), t(''), dft(US_COUNTRY_ID), r\countryList()); // can't use required() here
    $zip = zipFld();
    $source = sourceFld();
  }

  $submit = isset($gross) ? t('Submit') : t('Update');
  
  return cgform(compact(ray('title subtext contact fullName phone email country zip source mission activities gross employees checksIn checksOut comments submit')));
}

function formSponsor_validate($form, &$sta) {
  $mya = w\eLinkAcct($sta);
  extract(just('fullName phone email zip gross employees checksIn checksOut', $sta['input'], NULL));
  foreach (ray($flds = 'gross employees checksIn checksOut') as $k) if (isset($$k) and $err = u\badAmount($$k, '>=0')) return say($k . ': ' . $err, $k);
  
  if (!($mya and $mya->co)) {
    if ($err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');
    $fullName = u\normalizeCase($fullName);
    
    if ($err = u\badPhone($phone)) return say($err, 'phone');
    if (!emailOkay($email, FALSE, nn($myid) ? r\acct($myid) : '')) return;
    if ($err = u\badZip($zip)) return say($err, 'zip');
    $flds .= ' fullName phone email zip';
  }
  u\preray(compact(ray($flds)), $sta['input']);
}

function formSponsor_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta); 

  $dbFlds = 'contact fullName legalName federalId email flags phone zip country notes source city state latitude longitude mission activities gross employees checksIn checksOut';
  extract(just('comments source2 ' . $dbFlds, $sta['input'], NULL));
  $notes = $comments ? (r\stampNote($comments) . ($mya ? $mya->notes : '')) : '';

  if (!isset($contact)) { // using company data
    if (isset($gross)) { // copying from org to new FBO org
      u\EXPECT((bool) $gross, 'no gross copying FBO from co');
      $contact = $mya->agentA->fullName;
      foreach (ray('fullName email phone zip country source city state latitude longitude') as $k) $$k = $mya->$k;
      $mya->update(compact(ray('gross employees')));
    } else {
      $mya->update(compact(ray('mission activities checksIn checksOut')));
      return say('info saved');
    }
  }
  
  list ($city, $state, $latitude, $longitude) = r\zipCity($zip);

  $flags = u\bit(B_NONUDGE) | u\bit(B_CO);
  $fullName = PROJECT . t(' FBO ') . $fullName;
  $legalName = CGF_LEGALNAME;
  $federalId = CGF_EIN;
  r\tellAdmin(t('Fiscal Sponsorship Application'), compact(ray($dbFlds)) + ($mya ? [] : ['NEW ORG!' => 1]));
  $msg = 'got application';

  $acctType = hidFld(CO_CUSTODIAN);
  u\setDft($source, '');
  if ($source2) $source = trim(mb_strtoupper($source2) . "-$source");
  $info = []; foreach (ray($dbFlds) as $k) if (isset($$k)) $info[$k] = $$k;
  if (!$a = new r\Acct($info)) return softErr(t('cannot create account'));
  $msg .= '|meanwhile join';
  $a->stepDone('signup');
  return sayEmpty($msg);
}
