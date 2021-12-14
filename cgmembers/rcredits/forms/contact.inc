<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Display, for editing, contact info for the current account.
 */
function formContact($form, &$sta) {
  global $mya;

  $nameDesc = t('usename desc');
  if ($mya->stepIsDone('contact')) $nameDesc .= '<br>' . ($mya->co ? t('Your %PROJECT webpage address will also change.') : '');
  $title = t('Contact Information');
  $fullName = textFld($mya->co ? t('Company Name:') : t('Your Name:'), $nameDesc, required($mya->fullName));
//  $onchange = w\onchange("jQuery('.form-item-pass').show();");
  $email = emailFld(t('Email:'), '', required($mya->email));
  $pass = $mya->cAdmin ? NULL : passFld(t('Password:'), t('Required for changing email address'));

  $submit = submit();
  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost

  $form = compact(ray('title fullName email pass')) 
        + contactFields([], $mya->id)
        + w\setupFoot();

        
  jsx('contact');

  return cgform($form);
}

function formContact_validate($form, &$sta) {
  global $mya;
  $myid = $mya->id;
  extract(u\normalizeCase(just($normals = 'fullName city address postalAddr', $sta['input'])));
  extract(just('email pass phone state zip years months', $sta['input']));

  if ($fullName != $mya->fullName) {
    if ($err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');
    $name = r\shortName($fullName); // get a unique short version
  }

  if ($err = u\badZip($zip)) return say($err, 'zip');
  if (!emailOkay($email, $mya->co, $mya)) return; // say('bad email', ray('email', $email), 'email');
  if (!$mya->cAdmin and $email != $mya->email and !$mya->agentA->passwordOkay($pass, 'pass', $err)) return say($err, 'pass');
//$pin != $mya->agentA->pin) return say('bad pin', 'pin');
  if ($err = u\badPhone($phone)) return say($err, 'phone');
  if (u\poAddress($address)) return say('po in location', 'address');

  if (isset($years)) { // showing tenure field
    if (is_null($tenure = w\tenure($years, $months, $err))) return say($err, 'years');
    $extras = ' tenure';
  } else $extras = '';
  
  foreach (ray($normals . ' email phone name' . $extras) as $k) if (isset($$k)) $sta['input'][$k] = $$k;
}

function formContact_submit($form, &$sta) {
  global $mya;
  global $partnerInfo; // information used within eachPartner function argument (so must be global)
  extract($info = just('fullName name email phone faxetc zip state city address postalAddr tenure owns', $sta['input']));
  unset($info['owns']);
  
  if ($mya->id == 1 OR $mya->isRegion or $mya->partner) unset($info['name']); // keep name the same for admin, region, or partner
  
  $info = u\normalizeCase(just('fullName city address postalAddr', $info)) + $info;
  if ("$address $city $state" != "$mya->address $mya->city $mya->state") $info += ray('latitude longitude', 0, 0); // regeolocate
  
  $partnerInfo = just('email phone address city state zip postalAddr', $info); // what to tell partner about

  $mya->eachPartner(function($coA, $custA, $custCode) { // tell partner what changed
    global $partnerInfo, $mya;
    foreach ($partnerInfo as $k => $v) if ($custA->$k == $v) unset($partnerInfo[$k]);
    if ($partnerInfo and $api = $coA->api()) u\post($api, ray('partnerCode qid op name', $coA->emailCode, $mya->mainQid, 'change', $custA->fullName) + $partnerInfo);
  });
  
  if (isset($owns)) $mya->setRisk('rents', !$owns);
  $mya->update($info);
  return w\goNextStep('contact', 'info saved', $info);
}