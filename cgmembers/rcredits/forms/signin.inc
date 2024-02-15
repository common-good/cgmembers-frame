<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Completely redo the login form (see user_login() in user.module).
 */
function formSignin($form, &$sta, $arg = '') {
  extract(just('then mfa', $arg, NULL));

  if ($mfa) return mfa($mfa, $then); // send MFA code and display a form to ask for it
  
  signout(TRUE); // make sure we're signed out
  if (isDEV) db\q('TRUNCATE sessions'); // this saves a lot of manual truncates when tests mess up
  if (isDEV and !test()) db\q('TRUNCATE test');

  extract(just('qid pw vKeyPw', nni($sta, 'input'), ''));
  if (!isset($qid)) $qid = nni($_GET, 'qid') ?: nni($_GET, 'name'); // allow links to signin (for example from app) -- remove 'name' after 7/1/2021
  if (mb_strlen(str_replace(' ', '', $qid)) >= ICARD_CODELEN_MIN and !strhas($qid, '@')) { // not a qid or email, must be an invitation code
    \drupal_get_messages(); // drop previous errors
    if (r\iCardAcct($qid)) return w\go("signup/code=$qid"); // invitation card manual signup!
    return w\go('accept', 'bad invite num', 'err');
  }
  unset($form['actions']);
  unset($form['pass']);
  focusOn(($qid == 'admin' or mb_strlen($qid) > 4) ? 'pass' : 'qid');

  $title = t('Welcome to %PROJECT!');
  $subtext = t('<p>Not yet a member? <%aCheck>Check it out</a> or just <%aSignup>Sign up</a>!</p><p><b>TIP</b>: Try not to sign in to your financial accounts on a public computer. Or if you do, be careful! Check the address bar at the top of the screen to make sure you have the right website (<b><em>%BASE_URL/</em>...</b>). And be sure to sign OUT when you\'re done!</p>', '_aCheck _aSignup', atag(r\promoLink('')), atag('/signup'));

  $_a = atag('/settings/password/' . $qid, tabindex(3)); // uses $qid, so must precede $qid = 
  $passDesc = t('<%a>Reset password</a>', compact('_a'));
  list ($qid, $pass) = w\signinFlds($qid, t('Your account ID or email address'), $passDesc);
  $pass['#default_value'] = $pw;
  $vKeyPw = hidFld($vKeyPw); // superAdmin password
  
  $then = hidFld($then);
  $submit = t('Sign in');
  $form = compact(ray('title qid pass vKeyPw submit subtext then'));

  $form['external_input'] = hidFld(serialize(nni($sta, 'input'))); // pass this to rweb_user_login

  $form['#attributes']['class'][] = 'rweb labeled';
  return cgform($form);
}

/**
 * Replacement for user_login_authenticate_validate()
 * See the original function for explanation of flood control.
 * Accept account ID or email, with password.
 * Sets $sta['uid'] to signify successful authentication.
 * @param string $type: 'acct', 'pw', or 'both' -- what parameters to check
 */
function formSignin_validate($form, &$sta, $type = 'both') {
  extract(just('qid pass vKeyPw mfa gotNonce trust then', $sta['input'], NULL)); // mfa/gotNonce/trust are from mfa()

  if ($mfa) { // second page of signin (MFA)
    if (!$ray = u\decryRay($mfa)) return err(t('There was a problem with that page request.'));
    extract(just('uid nonce', $ray));
    if ($gotNonce != $nonce) {
      $retry = nni($sta, 'retry', 0);
      if ($retry > 2) return softErr('bad nonce');
      $sta['retry'] = $retry + 1;
      return err('bad nonce', 'gotNonce');
    }
    $a = r\acct($uid);
    r\setCook($a->trustCook, $trust ? 1 : 0);
    $sta['input']['uid'] = $uid;
    
  } else { // first page of signin
    if ($qid == 'admin' or (isDEV and $qid == 'root')) $qid = 'AAAAAA'; // root is so password managers can handle phpMyAdmin (user root) AND this
    if (isDEV and $qid == 'admin' and !r\acct(1)) {
      require_once __DIR__ . '/../admin/admin.inc';
      db\del('sessions', 'uid', '1');
      \CG\admin\setupBasicAccounts(); // happens sometimes during testing
    }
    if (!$uid = r\loginString($qid)) return say(t('bad login', '_a', atag('/settings/password')), 'qid');
    if (w\flooding($uid, $sta, $err)) return say($err, 'pass');
    $a = r\acct($uid);
    $success = (isDEV and $uid == 1) ?: ($a and $a->passwordOkay($pass, 'pass', $err)); // ignore admin pass on dev machine
    w\updateFlood($sta, $success, $err); // should be before pw2 and vKeyE checks
    if (!$success) return say($err, 'qid');
    $sta['input']['uid'] = $uid;
    if ($a->canMfa) return preMFA($uid, $then); // go show a second authentication page for multi-factor (or not)
  }

/*  if (!$a->id == UID_SUPER) {
    $pw2 = $pw2 ? u\b64decode($pw2) : (isDEV ? DEV_PW2 : '');
    if (!nni($a->vsecure, 'pw2')) r\acct(1)->update('pw2', r\passHash($pw2)); // first ever admin signin sets pw2
    if (!$a->passwordOkay($pw2, 'pw2', $err)) return say(t('Admin requires second pass.'), 'pass');
    r\setCryptCook('pw2', $pw2, 0); // store as cookie, so pw2 is never on server
    if (isDEV) u\setDft($vKeyE, getv('vKeyE', '')); // make it easier to sign in
    if ($vKeyE) { // set cookie for very-secure private key, encrypted (encrypt it further for cookie)
      if (!$vKeyE = u\b64decode($vKeyE)) return w\hack(t('bad vKeyE'));
      r\setCryptCook('vKeyE', $vKeyE, 0);
      if (isDEV) setv('vKeyE', $vKeyE); // remember new vKeyE, make it easier to sign in (this is not redundant)
    }
  }
  if (!$a->id == UID_SUPER) { // why does this fail for superAdmin? (but who cares?)
    r\setCryptCook('qid', $a->mainQid, NULL);
  }*/
  
  if (($uid == UID_SUPER or $a->admin) and $vKeyPw) superSignin($a, $vKeyPw); // include superAdmin permission if asked

  return TRUE; // TRUE is used by formBuy()
}

/**
 * Finish signing in.
 * @param string $goto: what page to start on for non-admin
 */
function formSignin_submit($form, &$sta, $goto = 'dashboard') {
  extract(just('then uid', $sta['input'], NULL));

  $mya = r\setAcct($uid, $sta);
  $agt = strtolower($mya->qid);
  if ($dftAcct = r\cook('dftAcct') and $dftA = r\acct($dftAcct, $uid) and $dftA->can(B_READ)) r\setAcct($dftA->id, $sta);

  u\loga('signin', compact('agt') + ($dftAcct ? compact('dftAcct') : []) );
  if ($then and $res = then($then)) return $res;
  return go($mya->admPanel ? 'sadmin/panel' : $goto);
}

/**
 * Show a form for multi-factor authentication (by email).
 */
function mfa($mfa, $then) {
  $title = t('Confirm');
  $subtext = t('To be sure it is really you signing in, we sent you a confirmation code by email. Copy it into the box below.') . '<br>&nbsp;';
  $gotNonce = intFld(REQ . t('Passcode:'));
  $trust = boxFld('trust', NBSP, t('Trust this device'), 0); // if we get here, trust is off, so default to No

  $submit = submit(t('Submit'));

  $mfa = hidFld($mfa);
  $then = hidFld($then);
  
  return cgform(compact(ray('title subtext gotNonce trust submit mfa then')));
}

function preMFA($uid, $then) {
  $a = r\acct($uid);
  if (r\cook($a->trustCook)) return; // this device is trusted for this account, so don't do MFA

  $nonce = u\randomString(8, 'digits') . ''; // tests get confused unless this is a string
  r\rMail('confirm-signin', $uid, compact('nonce'));
  $uid = $uid . ''; // tests get confused unless this is a string
  $mfa = u\cryRay(compact(ray('uid nonce')));
  return go('signin/' . http_build_query(compact(ray('mfa then'))));
}
