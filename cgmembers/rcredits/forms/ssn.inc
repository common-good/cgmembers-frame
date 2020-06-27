<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Ask new member to supply or confirm their SSN and/or birthdate (by typing it again). Possibly from an email link.
 */
function formSsn($form, &$sta, $args = '') {
  $mya = w\eLinkAcct($sta, $args, 'acct');
  
  if ($mya->stepIsDone('ssn') and !$mya->admin) w\go(''); // don't allow DIY redo after SSN is verified

  extract(ssnFields(!$mya->co, REQ));
  
  $ssnData = $mya->ssnData;
  $retry = (!empty($ssnData) or (isDEV and !$mya->member) or $mya->task == 'person');
  
  if ($retry) {
    if ($badBday = (is_array($ssnData) and nni($ssnData, SSN_ERR_INDEX) == t('bad birthdate'))) {
      $thing = t('Birth Date');
      $federalId = NULL;
    } else {
      $thing = $mya->co ? t('EIN') : t('Social Security Number');
      $dob = NULL;
    }
    $title = t('Confirm Your ') . $thing;
    $subtext = t('Our 3rd party service was unable to verify your %thing automatically. Please type it again (carefully) in case there was a typo.', 'thing', strtolower($thing));
  } else {
    $title = t('Verify Your Identity for the IRS');
    $subtext = t('As with any financial institution, we are required to report transactions to the IRS.');
  }

  $retrying = hidFld($retry);
  $submit = t('Submit');
  
  $form = compact(ray('title subtext federalId dob retrying'));
  $form += $retry ? compact('submit') : setupFoot($submit, [], TRUE);
  return cgform($form);
}

function formSsn_validate($form, &$sta) {
  return formBasic_validate($form, $sta, FALSE, 'federalId dob');
}

function formSsn_submit($form, &$sta) {
  extract(just('federalId dob retrying', $sta['input']));
  $mya = w\eLinkAcct($sta);

  foreach (ray('federalId dob') as $k) {
    if (isset($$k)) {
      if (!empty($mya->$k)) {
        if ($same = ($mya->$k == $$k)) $mya->setRisk($k == 'dob' ? 'dobOff' : 'ssnOff');
      }
      $mya->update(compact($k));
    }
  }
 
  if (!r\acct()) return doSay(t('info saved|setup complete')); // r\acct not $mya!
  $mya->verifySsnInBackground();
  if ($mya->stepCount() > 1 and !$retrying) {
    say('info saved|step completed');
    return w\skipStep();
  } else return w\goNextStep('ssn');
//  return ($mya->stepCount() > 1 and !$retrying) ? w\skipStep() : w\goNextStep('ssn');
}