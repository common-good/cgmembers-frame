<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once R_ROOT . '/forms/signin.inc';

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

  if (@$reid) return softErr('old partner link');
    /*
    if (test()) $reid = f('t.uid', $reid);
    if ($args2 = w\continuePartnerSignup($reid, @$customer)) extract($args = $args2);
  } else*/
  if (@$customer and @$partner and @$partnerCode) {
    $myid = w\partnerSignup($args); // info passed from partner signup
  } elseif (@$joint and $a = r\acct() and $a->canManage) {
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
  if ($i = strpos(@$code, '(')) { // self-invitation with friend's name and zipcode
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
  $calling = textFld(t('Life Calling:'), [t('Your calling in life (optional)'), t('One of the main purposes of %PROJECT is to provide you the opportunity to do what you feel called to do in life. What would bring you joy to do, to earn %RCREDITS (maybe something you\'re not doing yet)? What might you have to offer?')]);
  $companyOptions = array(
    'owner' => t('owner'),
    'employee' => t('employee'),
    'contractor' => t('other worker (contractor, supplier, etc.)'),
  );
  $copts = boxOpt('companyOptions', $companyOptions, $sta);

  $companyInfo = array(
    'company_text' => item(t('Are you an owner and/or employee of a local company, or of any %PROJECT member company? Otherwise leave this part blank.')),
//      'company' => textFld(t('Company: '), [t('Your company')], autocomplete('company')),
    'company' => textFld(t('Company: '), [t('Your company')]),
    'companyPhon' => textFld(t('Company Ph<span>&nbsp;</span>one:'), [t('Company phone number')]), // Google Chrome bug require obfuscation of "phone"
    'companyOptions' => boxesFld('', '', @$copts, $companyOptions), 
  );
  $companyInfo = fieldSet('companyInfo', $companyInfo, BARE);

  jsx('signup', ray('preid', nn($preid))); // must be before $preid = hidFld()
  $submit = t('Next');
  $args = hidFld(@$args);
  $preid = hidFld($preid);
  $confirmed = hidFld(@$iCode == IBY_FORM); // iff inviter used form (or opening a company account), invitation is already confirmed
  $iCode = hidFld(@$iCode + 0);
  $helperA = nn($helper) ? r\acct($helper) : NULL;
  $helper = hidFld($helperA ? $helperA->id : @$inviter);
  $source = hidFld($helperA ? $helperA->fullName : '');
  
  $phone = textFld(t('Phone:'), [t('Your phone number')], required(nn($phone) ? u\fmtPhone($phone): ''));
  $zip = zipFld(nn($zip));
  $fullName = textFld(t('Name:'), [t('Your full name'), t('Your full legal name, properly capitalized, as it appears on your social security card, driver\'s license, or other government-issued ID')], required(nn($fullName)));
  $email = textFld(t('Email:'), [t('Email'), t('<b>Type carefully.</b> All emails from the system will be sent to this address. It will not be made public, but will be viewable by accounts you transact with (you can choose greater privacy by clicking "Advanced" on the Preferences Settings page).')], required(nn($email)));
  
  $form = compact(ray('title forother subtext acctType fullName phone email zip submit myid args confirmed helper iCode notes preid source'));
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

  u\preray(compact(ray('fullName phone email zip')), $sta['input']);
}

/**
 * Handle optional extra fields
 */
function validateExtras($sta) {
   extract(just('myid years months pin company companyPhon companyOptions legalName address city state postalAddr', $sta['input']));
   return [];
  if (!nn($state)) return say('bad state', 'state');
  if (!isset($years) and !isset($months)) return say(t('You must say how long you\'ve been there.'), 'years');
  foreach (ray('years months') as $k) {
    if (!nn($$k)) $$k = 0;
    if ($err = u\badAmount($$k, '>=0')) return say(t('How-long: ') . t($err), $k);
  }
  $tenure = $years * 12 + $months;
//  if (@$pin === '') return say('pin required', 'pin');
//  if (@$pin and strlen($pin) != 4) return say('wrong pin len', 'pin');
  if (@$companyOptions) extract(just('owner employee contractor', $companyOptions));
  $gotrelation = (@$employee or @$owner or @$contractor);
  if (@$company or @$companyPhon) { // NOT or $gotrelation (let user get by without)
    foreach (ray('company companyPhon') as $one) {
      if (u\nonish(@$$one)) $$one = '';
      // (no) return say('missing field', array('field' => $one), $one);
    }
    if (@$company and $err = u\badName($company)) return say($err, array('field' => 'Company'), 'company');
    if (@$companyPhon and !$companyPhone = u\fmtPhone($companyPhon, '+n')) return say('bad company phone', compact('companyPhone'), 'companyPhone');
    if (!$gotrelation) return say('what relation', 'company');
    $signupCo = @compact(ray('company companyPhone employee owner contractor'));
  }
  if (u\poAddress($address)) return say('po in location', 'address');
  if (!@$legalName) $legalName = $fullName;
  foreach (ray('fullName legalName address city postalAddr') as $field) {
    if (nn($$field)) $$field = trim(str_replace('  ', ' ', u\normalizeCase($$field)));
    if ($err = u\badName($$field)) zExtraErr($notes, $err, $field, $$field);
  }
  return $extras ?: [];
}

/**
 * Add a note to the notes field about an error in an optional field
 */
function zExtraErr(&$notes, $msg, $field, $value = '') {
  $notes = (nn($notes) ? "$notes\n" : '') . tr($msg, compact('field')) . ": $value";
}

function formSignup_submit($form, &$sta) {
  global $mya;
  $dbFields = 'fullName legalName email flags phone federalId dob pin zip address city state postalAddr helper iCode notes calling signupCo tenure source';
  extract(just('myid preid confirmed acctType args ats owns partner customer source2 ' . $dbFields, $sta['input']));
  if (test() and !@$args) $args = basename($_SERVER['REQUEST_URI']);
  extract(just('code source2', $args)); // source2 may come from either partner or an ad (through our promo site)
  $helper = $helper + 0; // don't let lack of helper stop us (on DEV)

  if (@$notes) list ($source, $notes) = [trim("$notes $source"), ''];
  if (@$source2) $source = trim(strtoupper($source2) . "-$source");
  $flags = nn($confirmed) ? u\bit(B_CONFIRMED) : 0;

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
  $mya = w\setAcct($myid); // this works for both an individual account (proSe) and a company account (with agent)
  $qid = $mya->mainQid;
  
  if (nn($source2) == 'food') $mya->stepDone('food', FALSE); // signed up through Food Fund promo (cg4.us/food), so add this special step
  
  if (@$iCode < 0) w\invitation($code, $myid); // mark the invitation used
  $email = u\cry('P', $email);
  db\q('UPDATE r_invites SET invitee=:myid WHERE email=:email', compact('myid', 'email')); // and all such
  if (!@$owns) $mya->setRisk('rents');
  
  if (@$signupCo) {
    suCompanyRelation($myid, $signupCo);
    $mya->update(compact('signupCo'));
  }
  if ($h = $mya->helperA and @$args and !$h->cAdmin) suCreatorRelation($helper, $args, $myid);
  return finishSignup($info + compact(ray('acctType qid name')), nn($confirmed));    
}

/**
 * Finish signup -- used also by new(partner)Customer
 * @param assoc $info: info to report to admin
 */
function finishSignup($info, $confirmed) {
  global $mya; $myid = $mya->id;
  tellStaffNewMember($info); // report to admin

  $pw1 = $mya->oneTimePass($mya->name, TRUE);
  $h = $mya->helperA;
  
  if ($mya->co) { // company account
    say('company is ready');
    if (!@$h) hack('company no helper');
    if ($mya->email == $h->email) {
      $mya->stepDone('verifyemail');
    } else r\notify($myid, 'verify-co', ['code' => $pw1], TRUE); 
    return w\goNextStep('signup');  
  } 

  // personal account
  $mya->update('pass', $pass = u\randPass());

  r\notify($myid, 'verify', ray('name qid code pass', $mya->name, $mya->mainQid, $pw1, $pass), TRUE); 

  if (!$confirmed) {
    u\EXPECT((bool) $h, 'unconfirmed with no helper');
    if ($h->id != 1) { // sysadmin helper means the potential inviter is named in the notes field
      $_a1 = $h->makeDo('confirmInvite', $myid); // link to confirm invitedness
      r\message($h->id, 'confirm invite', ray('_a1 fullName', $_a1, $mya->fullName)); // ask inviter to confirm
    }
  }
  
  $sta['uid'] = $myid; // tell Drupal we're signed in
  
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
  w\setAcct(NULL); // sign agent out so it looks joint
  return $a->id;
}

/**
 * Handle info passed from partner signup, for use in CG signup
 * @param assoc $args: what the partner passed (includes non-empty customer, partner and partnerCode)
 * @return string $myid: account record ID for the newly-created account
 *   but if person already has an account, we redirect to continue signup
 *   we also redirect if all that needs to be done is to display a message (including if the person is already a member)
 */
function partnerSignup($args) {
  foreach ($args as $k => $v) u\badName($args[$k]); // fix double spaces and leading or trailing
  extract(just('partner partnerCode customer qid phone fullName email', $args));
  if (!$partnerA = r\acct($partner) or $partnerA->emailCode != $partnerCode) return NULL;
  $partnerName = $partnerA->fullName;
  
  $reid = r\relation('reid', 'main=:main AND code=:customer', ray('main customer', $partnerA->id, $customer));
  
  if (!@$qid) { // person is not a member yet
    if ($reid and $uid = r\relation('other', $reid)) return $uid;
    return newCustomer($reid, $args); // mark new or old member as a customer of the partner
  }
    
  // partner sent an already-member
  $valid = ($a = r\acct($qid) and ($a->phone == u\fmtPhone(@$phone, '+n') or (u\similar($a->fullName, @$fullName, 5, .5))));
  r\tellAdmin(t('new customer of %partner: %fullName', compact('partner', 'fullName')) . (@$valid ? '' : t('(invalid)')), $args);

  if (!$valid) return w\softErr(t('That does not appear to be your correct %PROJECT member ID. But no worries. We will try to figure out who you are and match up your %PROJECT account with your account at %partnerName. If you have not yet finished opening your %PROJECT account, <%a>sign in</a> (if you have not yet begun, go to %CG_DOMAIN and click "Sign up").', 'partnerName _a', $partnerName, atag('/signin'))); // member mistyped their qid (a royal pain in the butt)

  r\acct($partnerA->id, $a->id)->setRel('customer code', TRUE, $customer);
  tellPartner($partnerA, $a, 'complete', $customer);
  return w\go($a->member ? 'empty' : "signin?name=$qid"); // finish account setup or report success
}

/**
 * Tell the partner about this signup.
 */
function tellPartner($pA, $a, $op, $customer = NULL) {
  if (!$pA->website) return w\softErr(t('Partner has no website. Fix it on the Company Info settings page.')); // partner should always have a website

  if ($a->member) w\say('new customer done', ray('partnerName', $pA->fullName));
  $fields = ray('op partnerCode qid name email customer', $op, $pA->emailCode, $a->mainQid, $a->fullName, $a->email, $customer);
  u\post($pA->api(), $fields); // tell partner we're starting or done
}

/**
 * Create a new account and a customer relation with a company, if it doesn't already exist.
 */
function newCustomer($reid, $args) {
  foreach (ray('address2 city2 state2 zip2 idVerifiedBy') as $k) $$k = @$args[$k] ?: '?';
  $args += ['postalAddr' => "$address2, $city2, $state2 $zip2"];
  $a = new r\Acct(just('fullName phone email address city state zip postalAddr source idVerifiedBy', $args));
  u\EXPECT($a, 'cannot create account');

  w\setAcct($a->id);

  if (nn($idVerifiedBy)) $a->stepDone('verifyid');
  if (allContactsGiven($args)) $a->stepDone('contact');
  
  if (!$reid) {
    extract(just('partner customer email autopay', $args));
    $partnerA = r\acct($partner);
    $reid = $partnerA->newRelation(ray('other customer code autopay data', $a->id, TRUE, $customer, (bool) @$autopay, serialize($args))); // (bool) is required to avoid NULL
    w\say('partner welcome', ['partner' => $partnerA->fullName]);
/*    try { // give them a way to continue, even if they haven't completed the first page
      $noFrame = TRUE;
      r\rMail('partner-signup', $email, @compact(ray('fullName reid customer noFrame'))); // so customer can continue
    } catch (Exception $e) {r\tellAdmin('bad email addr from partner', $args);} // bad email gets caught in signup submit also
    */
    tellPartner($partnerA, $a, 'start', $customer);
  } 

  return finishSignup($args + ray('acctType', CO_PERSONAL), TRUE);
}

/**
 * Say whether all contact fields are supplied
 */
function allContactsGiven($args) {
  foreach (ray(CONTACT_FLDS) as $k) if (empty($args[$k])) return FALSE;
  return TRUE;
}

/**
 * Customer clicked on link to begin or continue CG signup after completing partner signup.
 * @param int $reid: r_relations record ID linking customer to partner
 * @param int $reid: (RETURNED) set to NULL if not a valid r_relations ID
 * @return assoc of signup args from r_relations record (NULL if none)
 *   redirects to signup step, if already in process
 *//* UNUSED
function continuePartnerSignup(&$reid, $customer) {
  if (!$info = r\relation('other,data', 'reid=:reid AND code=:customer', compact('reid', 'customer'))) return $reid = NULL;
  extract($info);
  $a = r\acct($other);
  if ($a->federalId) return w\go("signin?name=$a->mainQid", t('Sign in to continue your account set-up.'));

  return unserialize($data) + ['myid' => @$a ? $a->id : 0];
}*/

/**
 * Gather the just-submitted checkbox values as which-boxes-to-check for boxesFld.
 * @param string $field: name of boxesFld field
 * @param assoc $opts: display names of the boxes, keyed by their index
 * @param assoc $sta: form status
 * @return flat array of values (not display names) of checked boxes
 */
function boxOpt($field, $opts, $sta) {
///   debug(compact('field','opts','sta'));
  foreach ((@$sta['input'][$field] ?: []) as $k => $v) $res[] = $k; // $opts[$k]; // note that k==v
  return @$res ?: [];
}
