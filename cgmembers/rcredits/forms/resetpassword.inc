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
  global $mya; // not yet saved in session
 
  extract(just('id code', $args)); // linked from an email
  
  $a = r\acct($id);
    
  if (!$mya or $mya != $a) { // no longer signed in to the right account, so sign in
    w\signout(TRUE); // close the wrong old account, if any
    if (flooding($uid = r\loginString(nn($id)), $sta, $err)) return w\softErr($err); // handle getting hammered
    if (!$mya = $a ?: r\acct($uid)) return w\softErr(t('That is not a recognized account ID and one-time password.'));
    if (!$mya->passwordOkay(nn($code), 'pass', $err)) return w\softErr($err);
    noFlood($sta);
  }
  
  $title = t('Choose a New Password');
  $subtext = tr('pass advice');
  $passFlds = w\pickPassword(TRUE, REQ);
  $submit = t('Submit');
  
  $uid = hidFld($uid);

  return cgform(compact(ray('title subtext uid')) + $passFlds + compact('submit'));
}

function formResetPassword_validate($form, &$sta) {return formChangePassword_validate($form, $sta);}

function formResetPassword_submit($form, &$sta) {
  global $mya;
  extract(just('pw uid', $sta['input']));

  $mya = r\setAcct($uid, $sta);

  $mya->update('oneTimePass', '');
  if ($pw) {
    say('pass saved');
    $mya->update('pass', $pw);
  }

  return w\go($mya->admin2 ? 'sadmin/followup' : '');
}
