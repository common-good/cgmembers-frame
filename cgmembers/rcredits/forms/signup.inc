<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

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
  if (!$args) $args = @$_POST ?: basename($_SERVER['REQUEST_URI']); // because menu_execute_active_handler fails
  if (is_string($args)) $args = urlq2ray($args);

  extract(just('preid source2 joint reid inviter code fullName email helper phone zip partner partnerCode customer source', $args));

  u\setDft($source2, nn($_COOKIE['ad']));
  u\setDft($preid, getPreId($source2));
  $source2 = hidFld($source2);

  if (@$reid) return softErr('old partner link');

  if (@$joint and $a = r\acct() and $a->canManage) {
    if (!$a->co) $myid = w\jointSignup(); else return w\go('', t('A company cannot have a joint account.'), 'err');
  } 
  
  if (test() and is_array(@$myid)) return; // array means returned r\go
  $myid = hidFld($myid0 = @$myid);

  $title = @$joint ? t('Create a Partner Account<br><small>(joined to yours)</small>') : t('Open a Personal Account');

  if (!@$helper) {
    $helper = @$partner ?: 0;
    $forother = item(w\lnk('/signup-co/' . http_build_query($args), t('open an account for your company instead')));
  } else $forother = NULL;

  if (!@$code) $code = 'self ()';

  $notes = '';
  if ($i = mb_strpos(@$code, '(')) { // self-invitation with friend's name and zipcode
    $inviterName = substr($code, 0, $i);
    if (!@$partner) $notes = "knows: $code";
    $code = r\acct(1)->iCardCode(IBY_SELF); // inviter is admin for now; invitation() will set $iCode=IBY_SELF
  }
  $notes = hidFld($notes);
  w\invitation(@$code, '', $inviter, $iCode, $err);

  $subtext = @$joint 
  ? t('<%b>NOTE:</b> From here on, "you" means the person for whom you are creating a new account, which will be joined to an existing account (creating a joint account).') 
  : t(<<<EOF
<p>Opening a %PROJECT Account takes 10-15 minutes and is very much like opening an online bank account, plus a couple extra steps for security and democracy. After this page we will guide you through <%b>several short steps</b>.</p>
<p>If you need help, please <%aHelp>ask</a> (our contact information is at the bottom of every page).</p>
EOF
  , '_b _aHelp', 'b class="loud"', w\emailtag(CGF_EMAIL, t('signup help')));

  $acctType = hidFld(CO_PERSONAL);
//    $pin = passFld(t('PIN:'), [t('Choose a PIN (any 4 digits)'), t('Your security code for online purchases.')]); // first time logging in, get PIN

  jsx('signup', ray('preid', nn($preid))); // must be before $preid = hidFld()
  $submit = t('Next');
  $args = hidFld(@$args);
  $preid = hidFld($preid);
  $confirmed = hidFld(@$iCode == IBY_FORM); // iff inviter used form (or opening a company account), invitation is already confirmed
  $iCode = hidFld(@$iCode + 0);
  $helperA = nn($helper) ? r\acct($helper) : NULL;
  $helper = hidFld($helperA ? $helperA->id : @$inviter);
  $source = $helperA ? hidFld($helperA->fullName) : sourceFld();
  
  $phone = textFld(t('Phone:'), [t('Your phone number')], required(nn($phone) ? u\fmtPhone($phone): ''));
  $zip = zipFld(nn($zip));
  $fullName = textFld(t('Name:'), [t('Your full name'), t('Your full legal name, properly capitalized, as it appears on your social security card, driver\'s license, or other government-issued ID')], required(nn($fullName)));
  $email = textFld(t('Email:'), [t('Email'), t('<b>Type carefully.</b> All emails from the system will be sent to this address. It will not be made public, but will be viewable by accounts you transact with (you can choose greater privacy by clicking "Advanced" on the Preferences Settings page).')], required(nn($email)));
  
  $form = compact(ray('title forother subtext acctType fullName phone email zip source submit myid args confirmed helper iCode notes preid source2'));
//  $sponsorConfirmation = w\whoFldSubmit('sponsor', t('Your sponsoring organization is %name, yes?'));
  return cgform($form);
}

function formSignup_validate($form, &$sta) {
  extract(just('fullName phone email zip myid', $sta['input']));

  if ($err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');
  $fullName = u\normalizeCase($fullName);
  
  if ($err = u\badPhone($phone)) return say($err, 'phone');
  if (!emailOkay($email, FALSE, @$myid ? r\acct($myid) : '')) return;
  if ($err = u\badZip($zip)) return say($err, 'zip');
  
  if (@$companyOptions) extract(just('owner employee contractor', $companyOptions));
  $gotrelation = (@$employee or @$owner or @$contractor);

  u\preray(compact(ray('fullName phone email zip')), $sta['input']);
}

/**
 * Add a note to the notes field about an error in an optional field
 */
function zExtraErr(&$notes, $msg, $field, $value = '') {
  $notes = (nn($notes) ? "$notes\n" : '') . tr($msg, compact('field')) . ": $value";
}

function formSignup_submit($form, &$sta) {
  global $mya;
  $dbFields = 'fullName legalName email flags phone federalId dob pin zip address city state postalAddr helper iCode notes tenure source';
  extract(just('myid preid confirmed acctType args ats owns partner customer source2 ' . $dbFields, $sta['input']));
  if (test() and !@$args) $args = basename($_SERVER['REQUEST_URI']);
  extract(just('code source2', $args)); // source2 may come from either partner or an ad (through our promo site)
  $helper = $helper + 0; // don't let lack of helper stop us (on DEV)

  u\setDft($source, '');
  if (@$notes) list ($source, $notes) = [trim("$notes $source"), ''];
  if (@$source2) $source = trim(mb_strtoupper($source2) . "-$source");
  $flags = nn($confirmed) ? u\bit(B_CONFIRMED) : 0;
  
  list ($city, $state, $latitude, $longitude) = zipCity($zip);


  $info = @compact(ray($dbFields));
  $info += $acctType == CO_PERSONAL ? ['preid' => $preid] : ['coType' => $acctType];
  
  if (@$myid) {
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
  
  if (@$iCode < 0) w\invitation($code, $myid); // mark the invitation used
  $email = u\cry('P', $email);
  db\q('UPDATE r_invites SET invitee=:myid WHERE email=:email', compact('myid', 'email')); // and all such
  if (!@$owns) $mya->setRisk('rents');
  
  if ($h = $mya->helperA and @$args and !$h->cAdmin) suCreatorRelation($helper, $args, $myid);
  $info += compact(ray('acctType qid name'));

  tellStaffNewMember($info); // report to admin

  $pw1 = $mya->oneTimePass($mya->name, TRUE);
  $mya->update('pass', $pass = u\randPass());
  r\notify($myid, 'verify', ray('name qid code pass', $mya->name, $mya->mainQid, $pw1, $pass), TRUE); 

  if (!nn($confirmed)) {
    u\EXPECT((bool) $h, 'unconfirmed with no helper');
    if ($h->id != 1) { // sysadmin helper means the potential inviter is named in the notes field
      $_a1 = $h->makeDo('confirmInvite', $myid); // link to confirm invitedness
      r\message($h->id, 'confirm invite', ray('_a1 fullName', $_a1, $mya->fullName)); // ask inviter to confirm
    }
  }

  return w\goNextStep('signup');
}

/**
 * Handle simplified signup for second person in a joint account.
 * @param acct $a: the account to join, if it exists (otherwise create it) -- UNUSED
 */
function jointSignup($a = NULL) {
  global $mya; // was  $mya = r\acct();
  
  if ($mya->jid) return $mya->jid; // in case form gets run twice
  if (!$a) {
    $fields = 'fullName phone address city state zip postalAddr source minimum achMin';
    $args = (array) $mya->account($fields);
    $args['fullName'] .= t(' PARTNER');
    $a = new r\Acct($args + ['email' => t('name@') . EMAILX]);
    u\EXPECT($a, 'cannot create account');
    $a->update('tenure', round($mya->tenure + ($a->created - $mya->created) / (30 * DAY_SECS)));
    if (@$mya->risk('rents')) $a->setRisk('rents');
    $a->newRelation(ray('other permission', $mya->id, r\perm(B_JOINT)));
  }
  $mya->newRelation(ray('other permission', $a->id, r\perm(B_JOINT)));
  $a->join($mya);
  r\setAcct(NULL); // sign agent out so it looks joint
  return $a->id;
}

/**
 * Return city, state, and coordinates from zip and country
  {"post code": "01002", "country": "United States", "country abbreviation": "US", "places": [{"place name": "Amherst", "longitude": "-72.4646", "state": "Massachusetts", "state abbreviation": "MA", "latitude": "42.3671"}]}
  */
function zipCity($zip, $country = R_COUNTRY_ID) {
  /*
  if ($info = @file_get_contents('http://api.zippopotam.us/us/' . $zip)) {
    if ($info = json_decode($info)) $info = (array) nn($info->places)[0];
    if ($info) return [$info['place name'], r\stateNum($info['state abbreviation']), $info['latitude'], $info['longitude']];
  }
  */

  if ($res = db\get('region,state', 'zip3', 'id=:zip3', ['zip3' => substr($zip, 0, 3)])) extract($res);
  return $res ? [$region, r\stateNum($state), 0, 0] : [NULL, 0, 0, 0];
}
