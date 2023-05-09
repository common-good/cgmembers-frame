<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/agreement.inc';
/**
 * The user signs up for a Common Good account (an individual or company account)
 * @param string $args: if not empty, 
 *   a urlencoded assoc (from an email invitation or the "Create a Company Account" button) OR
 *   an assoc gleaned from POSTed parameters (for combined signup with Coop Power's heating fuel program, for example)
 *
 *   code: the invitation code (or a friend's name with zip in parens)
 *   fullName: the likely name of the account to create
 *   phone: the account's likely phone number
 *   
 */
function formSignup($form, &$sta, $args = '') {
  global $mya;
  
  if (!$args) $args = nn($_POST) ?: basename($_SERVER['REQUEST_URI']); // because menu_execute_active_handler fails
  if (is_string($args)) $args = urlq2ray($args);

  $flds = 'fullName email helper phone zip country source';
  extract(just($flds . ' source2 joint reid inviter code partner partnerCode customer then', $args, NULL));

  u\setDft($source2, nni($_COOKIE, 'ad'));
  $source2 = hidFld($source2);

  if (nn($reid)) return softErr('old partner link');

  if (nn($joint) and $mya and $mya->canManage) {
    if (!$mya->co) $myid = w\jointSignup(); else return w\softErr(t('A company cannot have a joint account.'));
  }
    
  if ($mya) signout(TRUE); // must come after jointSignup
  
  if (test() and is_array(nn($myid))) return; // array means returned r\go
  $myid = hidFld($myid0 = nn($myid));

  $title = nn($joint) ? t('Create a Partner Account<br><small>(joined to yours)</small>') : t('Open a Personal Account');

  if (!nn($helper)) {
    $helper = nn($partner) ?: 0;
    $forother = item(t('If you already have a %PROJECT account, <%aSignin>Sign in</a>. Did you want to open a <%aCo>COMPANY account</a> instead?', '_aSignin _aCo', atag('/signin'), atag('/signup-co/' . http_build_query($args))));
  } else $forother = NULL;

  if (!nn($code)) $code = 'self ()';

  $notes = '';
  if ($i = mb_strpos(nn($code), '(')) { // self-invitation with friend's name and zipcode
    $inviterName = substr($code, 0, $i);
    if (!nn($partner)) $notes = "knows: $code";
    $code = r\acct(1)->iCardCode(IBY_SELF); // inviter is admin for now; invitation() will set $iCode=IBY_SELF
  }
  $notes = hidFld($notes);
  w\invitation(nn($code), '', $inviter, $iCode, $err);

  $subtext = nn($joint)
  ? t('<%b>NOTE:</b> From here on, "you" means the person for whom you are creating a new account, which will be joined to an existing account (creating a joint account).') 
  : t(<<<EOF
<p>Sign-up takes about 5 minutes. If you need help, please <%aHelp>ask</a>.</p>
EOF
  , '_b _aHelp', 'b class="loud"', w\emailtag(CGF_EMAIL, t('signup help')));

  $acctType = hidFld(CO_PERSONAL);
//    $pin = passFld(t('PIN:'), [t('Choose a PIN (any 4 digits)'), t('Your security code for online purchases.')]); // first time logging in, get PIN

  $args = hidFld(nn($args));
  $confirmed = hidFld(nn($iCode) == IBY_FORM); // iff inviter used form (or opening a company account), invitation is already confirmed
  $iCode = hidFld(nn($iCode) + 0);
  $helperA = nn($helper) ? r\acct($helper) : NULL;
  $helper = hidFld($helperA ? $helperA->id : nn($inviter));
  $source = $helperA ? hidFld($helperA->fullName) : sourceFld();
  
  $phone = phoneFld(REQ . t('Phone:'), [t('Your phone number')], dft($phone));
  $country = selectFld(t('Country:'), t(''), dft($country ?: US_COUNTRY_ID), r\countryList()); // can't use required() here

  $zip = zipFld(nn($zip));
  $fullName = textFld(t('Name:'), [t('Your full name'), t('Your full name, properly capitalized, as you would like it to appear to other members and member companies.')], required(nn($fullName)));
  $email = emailFld(t('Email:'), [t('Email'), t('<b>Type carefully.</b> All emails from the system will be sent to this address. It will not be made public, but will be viewable by accounts you transact with (you can choose greater privacy by clicking "Advanced" on the Preferences Settings page).')], required(nn($email)));

  $midtext = t('<h3>%PROJECT Agreement</h3>Please actually read this, so you know what you\'re agreeing to.</p>');

  extract(just('agreement details', formAgreement($form, $sta)));
  list ($cq, $ca) = w\captcha();
  $then = hidFld($then);
  $submit = t('I agree'); //  was t('Next')
    
  $form = compact(ray('title forother subtext acctType fullName phone email country zip source midtext agreement details cq ca submit then args confirmed helper iCode notes source2'));
//  $sponsorConfirmation = w\whoFldSubmit('sponsor', t('Your sponsoring organization is %name, yes?'));
  return cgform($form);
}

function formSignup_validate($form, &$sta) {
  extract(just('fullName phone email zip country', $sta['input'], NULL));

  if ($err = u\badName($fullName)) return err($err, ['field' => 'fullName'], 'fullName');
  $fullName = u\normalizeCase($fullName);
  
  if ($err = u\badPhone($phone)) return err($err, 'phone');
  if (!emailOkay($email, FALSE)) return;
  if (emailDupCo($email)) return err(t('That email is already used by a company. To manage that account, sign in to your personal account and click your photo in the upper right corner. To create another company account, sign in and click "Open Another Company Account".'));
  if ($err = u\badZip($zip, $country)) return err($err, 'zip');
  if (badCaptcha($sta)) return;

  u\preray(compact(ray('fullName phone email zip')), $sta['input']);
}

/**
 * Add a note to the notes field about an error in an optional field
 */
function zExtraErr(&$notes, $msg, $field, $value = '') {
  $notes = (nn($notes) ? "$notes\n" : '') . tr($msg, compact('field')) . ": $value";
}

function formSignup_submit($form, &$sta) {
  $dbFields = 'fullName email flags phone federalId dob pin country zip address city state postalAddr helper iCode notes tenure source';
  extract(just('then myid confirmed acctType args ats owns partner customer source2 ' . $dbFields, $sta['input']));
  if (test() and !nn($args)) $args = basename($_SERVER['REQUEST_URI']);
  extract(just('code source2', $args)); // source2 may come from either partner or an ad (through our promo site)
  $helper = $helper + 0; // don't let lack of helper stop us (on DEV)

  u\setDft($source, '');
  if (nn($notes)) list ($source, $notes) = [trim("$notes $source"), ''];
  if (nn($source2)) $source = trim(mb_strtoupper($source2) . "-$source");
  $flags = nn($confirmed) ? u\bit(B_CONFIRMED) : 0;
  
  list ($city, $state, $latitude, $longitude) = r\zipCity($zip);

  $info = []; foreach (ray($dbFields) as $k) if (isset($$k)) $info[$k] = $$k;
  if ($acctType != CO_PERSONAL) $info += ['coType' => $acctType];
  
  if (nn($myid)) {
    $a = r\acct($myid);
    if ($a->jid) $info['name'] = r\uniqueName($fullName, $info); // make sure name reflects partner name
    $a->update($info);
    $name = $a->name;
  } else {
    if (!$a = new r\Acct($info)) return say('cannot create account', 'zot');
    list ($myid, $name) = [$a->id, $a->name];
  }
  
  $mya = r\setAcct($myid, $sta); // this works for both an individual account (proSe) and a company account (with agent)
  $qid = $mya->mainQid;
  
  if (nn($source2) == 'food') $mya->stepDone('food', FALSE); // signed up through Food Fund promo (cg4.us/food), so add this special step
  
  if (nn($iCode) < 0) w\invitation(nn($code), $myid); // mark the invitation used
  $email = u\cry('P', $email);
  db\q('UPDATE r_invites SET invitee=:myid WHERE email=:email', compact('myid', 'email')); // and all such
  if (!nn($owns)) $mya->setRisk('rents');
  
  if ($h = $mya->helperA and nn($args) and !$h->admHelpSignup) suCreatorRelation($helper, $args, $myid);
  $info += compact(ray('acctType qid name'));

  tellStaffNewMember($info); // report to admin

  $pw1 = $mya->oneTimePass($mya->name, TRUE);
  $mya->tell('verify', ray('name qid code', $mya->name, $mya->mainQid, $pw1));

  if (!nn($confirmed)) {
    u\EXPECT((bool) $h, 'unconfirmed with no helper');
    if ($h->id != 1) { // sysadmin helper means the potential inviter is named in the notes field
      $_a1 = $h->doLink('confirmInvite', $myid); // link to confirm invitedness
      $h->tell('confirm invite', ray('_a1 fullName', $_a1, $mya->fullName)); // ask inviter to confirm
    }
  }

  $mya->update('signed, signedBy', now(), $mya->fullName);
  $mya->stepDone('agree');
  if (nn($then) and $res = then($then)) return $res;
  return w\goNextStep('signup');
}

/**
 * Handle simplified signup for second person in a joint account.
 * @param acct $a: the account to join, if it exists (otherwise create it) -- UNUSED
 */
function jointSignup($a = NULL) {
  global $mya;
  
  if ($mya->jid) return $mya->jid; // in case form gets run twice
  if (!$a) {
    $fields = 'fullName phone address city state zip postalAddr source minimum achMin';
    $args = (array) $mya->account($fields);
    $args['fullName'] .= t(' PARTNER');
    $a = new r\Acct($args + ['email' => t('name@') . EMAILX]);
    u\EXPECT($a, 'cannot create account');
    $a->update('tenure', round(($mya->tenure ?: 0) + ($a->created - $mya->created) / (30 * DAY_SECS)));
    if ($mya->risk('rents')) $a->setRisk('rents');
    $a->newRelation(ray('other permission', $mya->id, r\perm(B_JOINT)));
  }
  $mya->newRelation(ray('other permission', $a->id, r\perm(B_JOINT)));
  $a->join($mya);
  return $a->id;
}
