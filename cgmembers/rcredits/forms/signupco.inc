<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once R_ROOT . '/forms/signup.inc'; // for validation

/**
 * Open a trial company account
 * Four cases:
 *   Signed in as an administrator, creating a company account for a member by clicking the button: do it.
 *   Signed in as an administrator otherwise: sign out and continue.
 *   Signed in as a member: create a related company account.
 *   Not signed in: ask for member ID. If supplied, ask for password for that account. Otherwise create proSe company account.
 */
function formSignupCo($form, &$sta, $args = '') {
  global $mya;
  global $signupArgs; // for testing
  
  $title = t('Open a Company Account');

  extract(just('relate', $args, FALSE));
  if (!$mya) { // not signed in (presumably from promo site)
    $subtext = t(<<< X
      <p>The first step is to open a personal account (even if you never use it) for signing in to your company account. To get started, <%aSignup>click here</a>.</p>
      <p><b>If you already have a personal account</b>, <%aSignin>click here</a> to sign in. Then click "Create a Company Account".</p>
      <p>This extra step is important for security &mdash; by law you can't open a financial account unless your identity has been verified.</p>
X
    , '_aSignin _aSignup', atag(first('/signin', '/signup-co/relate=1&' . $args)), atag('/signup/' . $args));
    return cgform(compact(ray('title subtext')));
  }

  $a = ($mya->admHelpSignup and $relate and !$mya->co) ? $mya : $mya->agentA; // admins open company accounts on behalf of others

  if ($coInfo = $a->signupCo) {
    extract(just('company companyPhone', $coInfo, '')); // predict co name
    if (!findCompany($company, $companyPhone)) list ($fullName, $phone) = [$company, $companyPhone];
  }
  $agentQid = hidFld($a->mainQid);

  $subtext = t(<<< X
    <p>Opening a Common Good Company Account is very much like opening a personal account. We will guide you through several short steps.</p>
    <p>If you need help, please ask (our contact information is at the bottom of every page).</p>
X
  );
  
//  $forother = item(w\lnk('/signup/' . $args, t('open a personal account instead')));
  
  $args = hidFld($args);

  $coType = coTypeFld();
  
  extract(just('fullName email', w\nameAndEmailFlds(TRUE, nn($fullName), '')));

  $legalName = textFld(t('Legal Name:'), [t('Company legal name'), t('Type the full legal name of the company (if different).')]);
  $federalId = passFld(REQ . tr('einTitle'), [tr('einDesc')], autofill('off'));
  $founded = foundedFld($mya->founded, REQ);

  $email['#help'][1] = preg_replace('/ \\(.*\\)/', '', $email['#help'][1]); // don't talk about settings
  $phone = textFld(t('Company Phone:'), [t('Company phone number')], required(nn($phone) ? u\fmtPhone($phone, '-') : ''));
  $zip = zipFld();

  $selling = textFld(t('Selling:'), [t('Products'),t('Enter a <b>VERY SHORT</b> transaction description (for example "food") for when you make a sale. This will appear in the description of the transaction for both you and the customer (for example "$20 for <i>food</i> on %dt")', 'dt', fmtDt(now()))], required($mya->selling) + maxlen(SELLING_MAXLEN));
  $sellCG = boolFld(t('Sell Credit:'), t('Do you want to accept customer cash in exchange for your %PROJECT credit (recommended)?'), 1);

  $midtext = t('<h3>%PROJECT Agreement</h3><p>I commit this company to the terms of the <%a>%PROJECT Agreement</a> (the same agreement I signed as an individual).</p>', '_a', 'a id="show-agreement"');
  extract(just('agreement details', formAgreement($form, $sta)));
  $submit = t('I agree'); // was t('Submit');
  jsx('signupco');
  
  return cgform(compact(ray('title subtext coType fullName legalName email zip phone selling sellCG founded federalId midtext agreement submit args agentQid')));
}
  
function formSignupCo_validate($form, &$sta) {
  global $mya;
  
  extract(u\normalizeCase(just('fullName legalName', $sta['input'], NULL)));
  extract(just('zip phone email selling federalId founded', $sta['input']));

  foreach (ray('fullName legalName') as $field) if ($field != 'legalName' or $$field) {
    $$field = trim(str_replace('  ', ' ', u\normalizeCase(nn($$field))));
    if ($err = u\badName($$field)) return say($err, compact('field'), $field);
  }
  if (r\isCriminal($fullName, TRUE) or r\isCriminal($legalName, TRUE)) {
    r\tellAdmin(t('New potential member flagged as criminal'), $sta['input']);
    return softErr(t('There is a problem with your account application. Please contact our support staff at %CGF_PHONE.'));
  }


  if ($err = u\badEin($federalId, TRUE)) return say($err, 'federalId');
  if ($err = u\badDob($founded, TRUE)) return say($err, 'founded');

  if ($err = u\badZip($zip)) return say($err, 'zip');
  if ($err = u\badPhone($phone)) return say($err, 'phone');
  if (!emailOkay($email, TRUE, NULL)) return;
  if ($err = zBadSelling($selling)) return say($err, 'selling');
  
  u\preray(compact(ray('fullName zip phone email selling federalId founded')), $sta['input']); // fix formatting and add name
}
  
function formSignupCo_submit($form, &$sta) {
  
  $dbFields = 'coType selling fullName legalName email flags phone federalId founded zip state helper';
  extract(just($dbFields . ' sellCG agentQid args', $sta['input'], NULL));
  extract(just('code', nn($args, []), NULL));

  signout(TRUE);
  $agentA = r\acct($agentQid);
  r\setAcct($agentA); // set agent
  $helper = $agentA->id;
  $legalName = $legalName ?: $fullName;
  
  list ($city, $state, $latitude, $longitude) = r\zipCity($zip);
  $flags = u\bit(B_CO) | u\bit(B_CONFIRMED);
  $info = []; foreach (ray($dbFields) as $k) if (isset($$k)) $info[$k] = $$k;

  if (!$newA = new r\Acct($info)) return say('cannot create account', 'zot');
  r\setAcct($newA->id, $sta);
  
  foreach (ray(CO_APP_DFTS) as $i) $newA->setCoBit(APP_CANS + $i);
  if ($sellCG) foreach ([CO_SELLCG, APP_CANS + CO_R4USD, APP_CANS + APP_CAN_BITS + CO_R4USD] as $k) $newA->setCoBit($k);

  list ($myid, $name) = [$newA->id, $newA->name];  
  $qid = $newA->qid;
  tellStaffNewMember($info + compact(ray('qid name'))); // report to staff and user
  
  $pw1 = $newA->oneTimePass($name, TRUE);

  $newA->newRelation(ray('other otherNum owner permission', $agentA->id, 1, TRUE, r\perm(B_MANAGE)));
  $newA->tell('verify-co', ray('qid code pwMsg', $qid, $pw1, t('co nonpass')));

  if ($code) w\invitation($code, $myid); // mark invitation used

  $newA->update('signed, signedBy', now(), $agentA->fullName);
  $newA->stepDone('agree');
  return w\goNextStep('signup'); 
}
