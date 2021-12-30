<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Ask new member to verify their identity.
 */
function formVerifyId($form, &$sta, $args = '') {
  global $mya;
  
  if ($mya->stepIsDone('ssn') and !$mya->admin) return w\goNextStep(''); // don't allow DIY redo after SSN is verified
  extract(just('setMethod', $args, 0));

  if (u\isMobile()) js('x/jquery.finger.min');
  jsx('verifyid', 'method usa', $setMethod, $mya->usa);
  $title = t('Identity Verification');
  $subtext = t('As a registered Money Services Business, we are required by law to verify your identity. Fortunately, we have <%aSecure>top notch security</a>.', '_aSecure', atag(r\promoLink('about-us/privacy-and-security')));
  $legalName = textFld(t('Your Legal Name:'), [t('Legal name'), t('If your legal name is different (or longer), correct it here.')], dft($mya->fullName));
  $methodChoices = ray(t('|<b>Immediate and automatic</b> &mdash; Type your social security number and birth date.|Upload driver\'s license (to be verified by an administrator).|Upload government-issued photo ID (to be verified by an administrator).'));
  //  This is simplest and is required by the IRS if you <i class="loud">accept</i> more than $20,000 a year in payments through %PROJECT, and by the states of Vermont and Massachusetts if you accept more than $%MINIMA1099MAVT a year (not including purchase rebates offered by %PROJECT businesses).
  $method = radiosFld(REQ, '', dft(0), $methodChoices);  
  $idtype = textFld(t('ID Type:'), [t('Type of ID')]);
  $file = fileFld(t('ID File:'), t('Take a photo of your ID and upload it here (or email it us at %CGF_EMAIL).'));
  $federalId = ssnFld(FALSE, suffix(lnk('', t('Show other options'))));
  $dob = dobFld(FALSE);
  $submit = t('Submit');

  $form = compact(ray('title subtext method idtype file legalName federalId dob submit'));
  return cgform($form);
}

function formVerifyId_validate($form, &$sta) {
  global $mya;
  extract(just('method idtype', $sta['input']));

  if (!is_dir(PHOTOID_DIR)) mkdir(PHOTOID_DIR, 0755) or u\FAIL('cannot create photo ID directory'); // required for tests

  $ssnMethod = ($method == 0); // user chose SSN verification
  $checkFlds = $ssnMethod ? 'federalId dob' : 'dob';
  if (!formBasic_validate($form, $sta, $checkFlds)) return w\go('/settings/verifyid/setMethod=' . $method);
  if ($ssnMethod) return;
  
  // not SSN verification, so handle uploaded file
  $path = PHOTOID_DIR . "/$mya->mainQid";
  $fileData = uploadFile($path, NULL);
  if (!is_array($fileData)) return err($fileData . t(' Maybe try emailing it?'), 'file');
  $sta['input']['filedata'] = serialize($fileData + compact('idtype'));
  u\cryFile($path);
}
  
function formVerifyId_submit($form, &$sta) {
  global $mya;
  extract(just('legalName filedata dob federalId', $sta['input'], NULL));

  $mya->update('legalName dob yob', $legalName ?: $mya->fullName, $dob, date('Y', $dob)); // must precede ->verifySsn...

  if (nn($filedata)) {
    r\tellAdmin('id to verify', unserialize($filedata));
  } else {
    $mya->update(compact('federalId'));
    $mya->verifySsnInBackground();
  }

  say(t('info saved'));

  return (!nn($filedata) and $mya->stepCount() > 1) ? w\skipStep('ssn', 'verifyid') : w\goNextStep('verifyid');
}