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

  extract(just('qid code verify', $args, NULL));
  if (!$verify) { // send link to verify email
    if (!$mya or !$mya->can(B_MANAGE)) return softErr(t('You are no longer signed in.'));
    $title = t('Verify Your Email Address');
    $subtext = t('sent email', 'email', $mya->email) . t('<p><%a>Resend or change email</a></p>', '_a', atag('/settings/resend'));
    return cgform(compact(ray('title subtext')));
  }

  if (!$a = r\acct($qid)) return hack(t('Tried to verify nonexistent account: ' . $args));
  
  if (!$mya or $mya->id != $a->id or $mya->agentId != $a->agentId) { // not signed in to the right account, so sign in
    $mya = NULL; // for companies, this flags that we're not signed in right
    w\signout(TRUE); // close the wrong old account, if any
    if (flooding(r\loginString($qid), $sta, $err)) return w\softErr($err); // handle getting hammered
    if (!$a->passwordOkay(nn($code), 'pass', $err)) return w\softErr($err); // check oneTimePass
    noFlood($sta);
    if ($a->co) {
      $a->stepDone('verifyemail');
      $a->nextStepUrl('verifyemail', $msg); // get completion message, if any
      say('verified email');
      say($msg);
      if ($a->ok) {
        return go('empty'); // just say the message if done with signup
      } else {
        say(tr('continue co setup'));
        return go('signin?name=' . $a->agentA->mainQid);
      }
    }
  } elseif ($a->co) {
    say('verified email');
    return goNextStep('verifyemail'); 
  } // Nothing useful on verify page for companies

  $a->stepDone('verifyemail'); // this step is unique in that it is completed simply by visiting the page
  
  $title = t('Verified!');
  $subtext = tr('verified email');
  $submit = submit($a->stepCount() > 1 ? t('Next') : t('Finish')); // can't use setupFoot() here because not signed in

  $subtext .= t('<p>If you want a different password than the one we emailed you, you can change it here.</p>') . t('pass advice');
  $showPass = boolFld(t('Change Password?'), t('Do you want to change your assigned password?'));
  $passFlds = compact('showPass') + w\pickPassword(FALSE);
  
  $uid = hidFld($a->id);
  jsx('verifyemail', compact('verify'));
  $verify = hidFld($verify);

  return cgform(compact(ray('title subtext uid verify')) + $passFlds + compact('submit'));
}

function formVerifyEmail_validate($form, &$sta) {if (nni($sta['input'], 'showPass')) return formChangePassword_validate($form, $sta);}

function formVerifyEmail_submit($form, &$sta) {
  global $mya;
  extract(just('pw uid', $sta['input'], NULL));

  $mya = r\setAcct($uid, $sta);

  $mya->update('oneTimePass', '');
  if ($pw) {
    say('pass saved');
    $mya->update('pass', $pw);
  }

  return w\goNextStep('verifyemail');
}
