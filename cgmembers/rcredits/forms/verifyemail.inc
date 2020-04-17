<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/changepassword.inc';

/**
 * Send the new member a link and a code, to verify their email address.
 */
function formVerifyEmail($form, &$sta, $args = '') {
  global $mya;

  extract(just('id code verify', $args, NULL)); // linked from an email
  if (!$verify) { // send link to verify email
    if (!$mya or !$mya->can(B_MANAGE)) return hack(t('tried to verify email without manage permission'));
    $title = t('Verify Your Email Address');
    $subtext = t('sent email', 'email', $mya->email) . t('<p><%a>Resend or change email</a></p>', '_a', atag('/settings/resend'));
    return cgform(compact(ray('title subtext')));
  }
  
  $a = r\acct($id);
  
  if (!$mya or $mya != $a) { // no longer signed in to the right account, so sign in
    w\signout(TRUE); // close the wrong old account, if any
    if (flooding($uid = r\loginString(@$id), $sta, $err)) return w\softErr($err); // handle getting hammered
    $mya = r\acct($id);
    if (!$mya->passwordOkay(@$code, 'pass', $err)) return w\softErr($err);
    noFlood($sta);
  }
  
  $mya->stepDone('verifyemail'); // this step is unique in that it is completed simply by visiting the page
    
  $title = t('Verified!');
  $subtext = t('<p>You have successfully verified your email address.</p>');
  if (!$mya->co) {
    $subtext .= t('<p>If you want a different password than the one we emailed you, you can change it here.</p>') . t('pass advice');
    $showPass = boolFld(t('Change Password?'), t('Do you want to change your assigned password?'));
    $passFlds = compact('showPass') + w\pickPassword(FALSE);
  } else $passFlds = [];
  
  $buttons = setupFoot();
  $uid = hidFld($mya->id);
  jsx('verifyemail', compact('verify'));
  $verify = hidFld($verify);

  return cgform(compact(ray('title subtext uid verify')) + $passFlds + $buttons);
}

function formVerifyEmail_validate($form, &$sta) {if (nn($sta['input']['showPass'])) return formChangePassword_validate($form, $sta);}

function formVerifyEmail_submit($form, &$sta) {
  global $mya;
  extract(just('pass1 uid', $sta['input'], NULL));

  $mya = r\setAcct($uid, $sta);

  $mya->update('oneTimePass', '');
  if ($pass1) {
    say('pass saved');
    $mya->update('pass', $pass1);
  }

  return w\goNextStep('verifyemail');
}
