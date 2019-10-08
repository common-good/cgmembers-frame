<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once __DIR__ . '/changepassword.inc';
include_once __DIR__ . '/signin.inc';


/**
 * Reset password by choosing a new one from an official email link.
 */
function formResetPassword($form, &$sta, $args = '') {
  global $mya;
  extract(just('id code verify', $args)); // linked from an email

  if (flooding($uid = r\loginString(@$id), $sta, $err)) return w\softErr($err); // getting hammered
  $mya = r\acct($uid);

  if (!$mya->passwordOkay(@$code, 'pass', $err)) return w\softErr($err);
  noFlood($sta);
  
  w\setAcct($uid);
  u\setDft($verify, FALSE);
  
  if ($verify) {
    $mya->stepDone('verifyemail'); // this step is unique in that it is complete by visiting the page
    
    $title = t('Verified!');
    $subtext = t('<p>You have successfully verified your email address.</p><p>If you want a different password than the one we emailed you, you can change it here.</p>');
    $showPass = boolFld(t('Change Password?'), t('Do you want to change your assigned password?'));
    $passFlds = compact('showPass') + w\pickPassword(FALSE);
    $buttons = setupFoot();
  } else {
    $title = t('Choose a New Password');
    $subtext = '';
    $passFlds = w\pickPassword(TRUE, REQ);
    $submit = t('Submit');
    $buttons = compact('submit');
  }
  
  $subtext .= t('<p>Your password must be very long OR it must have at least 8 characters, including uppercase, lowercase, digit, and punctuation.</p><p>Consider choosing 4 random words &mdash; hard to guess, but easy for you to remember.');
  $uid = hidFld($uid);
  jsx('verifyemail', compact('verify'));
  $verify = hidFld($verify);

  return cgform(compact(ray('title subtext uid verify')) + $passFlds + $buttons);
}

function formResetPassword_validate($form, &$sta) {if (!$sta['input']['verify']) return formChangePassword_validate($form, $sta);}

function formResetPassword_submit($form, &$sta) {
  global $mya;
  extract(just('pass1 uid verify', $sta['input']));

  $sta['uid'] = $uid;
  $sta['status'] = 1;

  $mya->update('oneTimePass', '');
  if ($pass1) {
    say('pass saved');
    $mya->update('pass', $pass1);
  }

  return $verify ? w\goNextStep('verifyemail') : w\go($mya->admin2 ? 'sadmin/followup' : 'summary');
}
