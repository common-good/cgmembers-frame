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
  global $base_url;
  
  extract(just('then', $arg, NULL));
	
  signout(TRUE); // make sure we're signed out
	if (isDEV) db\q('TRUNCATE sessions'); // this saves a lot of manual truncates when tests mess up
  if (isDEV and !test()) db\q('TRUNCATE test');

  extract(just('qid pw2 vKeyE', nni($sta, 'input')));
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
  $passDesc = t('<%a>Forgot password?</a>', compact('_a'));
  list ($qid, $pass) = w\signinFlds($qid, t('Your account ID or email address'), $passDesc);
  $pw2 = hidFld(nn($pw2) ?: ''); // extra password for admins
  $vKeyE = hidFld(nn($vKeyE) ?: ''); // extra password for admins
  
  $trust = boxFld('trust', NBSP, t('Trust this device'), nni($_COOKIE, 'trust'));
  $then = hidFld($then);
  $submit = t('Sign in');
  $form = compact(ray('title qid pass pw2 vKeyE trust submit subtext then'));

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
  extract(just('qid pass pw2 vKeyE trust', $sta['input'], NULL));

  if (isDEV and $qid == 'admin' and !a(1)) {
    require_once __DIR__ . '/../admin/admin.inc';
    db\del('sessions', 'uid', '1');
    \CG\admin\setupBasicAccounts(); // happens sometimes during testing
  }

  if ($qid == 'admin') $qid = 'AAAAAA';
  if (!$uid = r\loginString($qid)) return say('bad login', 'qid');
  if (w\flooding($uid, $sta, $err)) return say($err, 'pass');
  $a = r\acct($uid);
  $success = (isDEV and $uid == 1) ?: ($a and $a->passwordOkay($pass, 'pass', $err)); // ignore admin pass on dev machine
  w\updateFlood($sta, $success); // should be before pw2 and vKeyE checks
	if (!$success) return say('login failed', 'qid');
  $sta['input']['uid'] = $uid; // especially for cgpay

  if ($a->superAdmin) {
    $pw2 = $pw2 ? u\b64decode($pw2) : (isDEV ? DEV_PW2 : '');
    if ($pw2 and !$a->pw2) a(1)->update('pw2', r\passHash($pw2)); // first ever admin signin sets pw2
    if (!$a->passwordOkay($pw2, 'pw2', $err)) return say(t('Admin requires second pass.'), 'pass');
    r\setCryptCook('pw2', $pw2, 0); // store as cookie, so pw2 is never on server
		if (isDEV) u\setDft($vKeyE, getv('vKeyE', '')); // make it easier to sign in
		if ($vKeyE) { // set cookie for very-secure private key, encrypted (encrypt it further for cookie)
			if (!$vKeyE = u\b64decode($vKeyE)) return w\hack(t('bad vKeyE'));
		  r\setCryptCook('vKeyE', $vKeyE, 0);
			if (isDEV) setv('vKeyE', $vKeyE); // remember new vKeyE, make it easier to sign in (this is not redundant)
		}
  }
  
  if (!$a->superAdmin) { // why does this fail for superAdmin? (but who cares?)
    r\setCryptCook('qid', $a->mainQid, NULL);
    r\setCook('trust', $a->admin2 ? 0 : ($trust ? 1 : 0));
  }
  
  return TRUE; // TRUE is used by formBuy()
}

/**
 * Finish signing in.
 * @param string $goto: what page to start on for non-admin
 */
function formSignin_submit($form, &$sta, $goto = 'dashboard') {
  extract(just('qid then uid', $sta['input'], NULL));
  $mya = r\setAcct($uid, $sta);

  u\loga('signin', ['qid' => $mya->qid]);
  if ($mya->superAdmin) superSignin();
  if ($then and $res = then($then)) return $res;
  return w\go($mya->admin2 ? 'sadmin/followup' : $goto);
}

/**
 * Do periodic stuff that only superAdmin can do.
 */
function superSignin() {
 
  foreach ($kids = db\col('uid', 'users', ':IS_UNDERAGE') as $uid) { // underage bit got set when activating account
    $a = r\acct($uid);
    if (now() - ($a->dob ?: now()) >= MATURITY * YEAR_SECS) $a->setBit(B_UNDERAGE, FALSE); // unset bit (youngster is now of age)
  }
}
