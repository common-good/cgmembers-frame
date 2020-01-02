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
  extract(just('method', $args, -1));

  $title = t('Identity Verification');
  $subtext = t('As a registered Money Services Business, we are required by law to verify your identity. Fortunately, we have <%aSecure>top notch security</a>. How do you want to do it?', '_aSecure', atag(r\promoLink('about-us/privacy-and-security')));
  $methodChoices = ray(t('Upload driver\'s license.,Upload other state-issued photo ID.,Type your social security number and birth date. This is required by the IRS if you are going to be <i class="loud">accepting</i> payments through %PROJECT (including receiving rebates).'));
  $field = radiosFld(' ', '', required(-1), $methodChoices);  
  $idtype = textFld(t('ID Type:'), [t('Type of ID')]);
  $file = fileFld(t('ID File:'), t('Take a photo of your ID and upload it here (or email it us at %CGF_EMAIL).'));
  extract(ssnFields(!$mya->co), FALSE);
//  $dob = textFld(t('Birth Date:'), [t('Your birth date'), t('Your date of birth (mm/dd/yyyy)')], autofill('off'));
//  $federalId = passFld(t('Soc Sec #:'), [t('ssDesc')], autofill('off'));    
  jsx('verifyid', compact('method'));

  $form = compact(ray('title subtext field idtype file federalId dob')) + w\setupFoot(t('Submit'), [], TRUE);
  return cgform($form);
}

function formVerifyId_validate($form, &$sta) {
  global $mya;
  extract(just('field idtype', $sta['input']));

  if ($field == 2) {
    $res = formBasic_validate($form, $sta, FALSE, 'federalId dob');
    return $res ? '' : w\go('/settings/verifyid/method=2');
  }

  foreach (ray('name type tmp_name error size') as $k) $$k = $_FILES['files'][$k]['file'];
  if ($size > MAX_ID_SIZE * 1024 * 1024) return say(t('That file is too big. It must be under %MAX_ID_SIZE MB'), 'err');
  $path = dirname($tmp_name) . "/$name";
  rename($tmp_name, $path);

  $mimetype = mime_content_type($path);
  $sta['input']['filedata'] = serialize(compact(ray('name path mimetype type error size idtype')));
}

function formVerifyId_submit($form, &$sta) {
  global $mya;
  extract(just('filedata dob federalId', $sta['input']));

  if (nn($filedata)) {
    $filedata = unserialize($filedata);
    r\tellAdmin('id to verify', $filedata, TRUE, $filedata['path']);
  } else {
    $mya->update(compact(ray('dob federalId')));
    $mya->verifySsnInBackground();
  }
  
  say(t('info saved'));
  return (!nn($filedata) and $mya->stepCount() > 1) ? w\skipStep('ssn') : w\goNextStep('verifyid');
}