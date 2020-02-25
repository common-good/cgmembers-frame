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
  
  extract(just('relate', $args, FALSE));
  if ($mya) {
    if ($mya->cAdmin) { // let admins open company accounts on behalf of others
      if ($relate and !$mya->co) $a = $mya; else signout(TRUE);
    } else $a = $mya->agentA;
  }

  if (nn($a)) {
    $agentQid = $a->mainQid;
    if ($coInfo = $a->signupCo) {
      extract(just('company companyPhone', $coInfo)); // predict co name
      if (!findCompany($company, $companyPhone)) list ($fullName, $phone) = [$company, $companyPhone];
    }
  } else $agentQid = '';

  $title = t('Open a Trial Company Account');
  $subtext = t(<<< X
  <p>Fill out this short form to get your company started accepting %PROJECT payments quickly. You can accept up to $%TRIALCO_AMT_LIMIT in total payments, for %TRIALCO_DAY_LIMIT days, before you need to convert to a regular company account (and open a personal account for signing in). There is no charge for a trial account or for a regular company account or a personal account.</p>
X
  );
  
  $forother = item(w\lnk('/signup/' . $args, t('open an individual account instead')));
  
  u\setDft($source, nn($_COOKIE['ad']));
  $source2 = hidFld($source);
  $args = hidFld($args);

  extract(just('fullName email', w\nameAndEmailFlds(TRUE, nn($fullName), '')));
  $email['#help'][1] = preg_replace('/ \\(.*\\)/', '', $email['#help'][1]); // don't talk about settings
  $phone = textFld(t('Company Phone:'), [t('Company phone number')], required(nn($phone) ? u\fmtPhone($phone, '-') : ''));
  $zip = zipFld();
  $contact = $agentQid ? NULL : textFld(t('Your Name:'), [t('Your full name')], required());

  $selling = sellingFld();
  $source = ($agentQid or $source) ? hidFld($agentQid ?: $source) : sourceFld('');
  $ownPhone = boolFld(t('Own Phone:'), t('Will you be running the CGPay app on your own Android phone? (%PROJECT is happy to supply one, with a $%EQUIP_DEPOSIT refundable equipment deposit).'), FALSE);

  if ($agentQid) {
    $agentQid = hidFld($agentQid);
    $pass = NULL;
  } else {
    $agentQid = textFld(t('Account ID:'), [t('Your personal account ID'), t('If you are already a member of %PROJECT, type the account ID of your personal account. Otherwise leave this <b class="loud">BLANK</b>.')]);
    list ($zot, $pass) = w\signinFlds('zot', '', '');
    $pass['#required'] = FALSE;
  }
  
  $submit = t('Submit');
  jsx('signupco');
  
  return cgform(compact(ray('title forother subtext source2 contact fullName zip phone email selling source ownPhone agentQid pass submit args')));
}
  
function formSignupCo_validate($form, &$sta) {
  global $mya;
  
  extract(u\normalizeCase(just('contact fullName', $sta['input'])));
  extract(just('zip phone email selling agentQid pass', $sta['input']));

  foreach (ray('contact fullName') as $field) {
    $$field = trim(str_replace('  ', ' ', u\normalizeCase(@$$field))); // @ for testing
    if ($err = u\badName($$field)) return say($err, compact('field'), $field);
  }

  if ($agentQid = trim(strtoupper(nn($agentQid)))) {
    if (!$a = r\acct($agentQid)) return say(t('That is not an account ID. Please check again (look in the lower left on your %PROJECT Card).'), 'agentQid');
    if ($mya) {
      if ($mya->id != $a->id) u\FAIL('signed in to wrong account in signupco');
    } else {
      if (w\flooding($a->id, $sta, $err)) return say($err, 'pass');
      if (!$a->passwordOkay(nn($pass), 'pass', $err)) return say('login failed', 'agentQid');
    }
  }    

  if ($err = u\badZip($zip)) return say($err, 'zip');
  if ($err = u\badPhone($phone)) return say($err, 'phone');
  if (!emailOkay($email, TRUE, NULL)) return;
  if ($err = zBadSelling($selling)) return say($err, 'selling');
  
  u\preray(@compact(ray('contact fullName agentQid zip phone email selling')), $sta['input']); // fix formatting and add name
}
  
function formSignupCo_submit($form, &$sta) {
  global $mya;
  
  $dbFields = 'coType contact source selling fullName legalName email flags phone federalId zip state helper';
  extract(just($dbFields . ' source2 ownPhone agentQid args', $sta['input']));
  extract(just('code relate', nn($args)));

  list ($helper, $legalName, $federalId) = [CGID, CGF_LEGALNAME, CGF_EIN];
  if ($agentQid) {
    signout(TRUE);
    $agentA = r\acct($agentQid);
    r\setAcct($agentA); // set agent
    if ($agentA->hasFid) list ($helper, $legalName, $federalId) = [$agentA->id, $agentA->legalName, $agentA->federalId];
  }

  
  list ($city, $state, $latitude, $longitude) = zipCity($zip);
  $preid = getPreId($source);
  if (@$source2) $source = trim(mb_strtoupper($source2) . "-$source");  
  $flags = u\bit(B_CO) | u\bit(B_DEPENDS) | u\bit(B_CONFIRMED);
//  $stepsDone = array_fill_keys(ray('signup discount verify'), FALSE);
  $coType = $coType = CO_CUSTODIAN;
  $info = @compact(ray($dbFields));
  
  if (!$newA = new r\Acct($info)) return say('cannot create account', 'zot');
  r\setAcct($newA->id, $sta); // this works for both an individual account (proSe) and a company account (with agent)
  
  foreach (ray(CO_DFTS) as $i) $newA->setCoBit(APP_CANS + $i);

  list ($myid, $name) = [$newA->id, $newA->name];  
  $qid = $newA->mainQid;
  tellStaffNewMember($info + compact(ray('qid name'))); // report to staff and user
  
  $pw1 = $newA->oneTimePass($name, TRUE);

  if ($agentQid) {
    $newA->newRelation(ray('other owner permission', $agentA->id, TRUE, r\perm(B_MANAGE)));
    $pass = t('None. Sign in to your personal account, then click your photo in the upper right corner to select the company account.');
  } else {
    $newA->update('pass', $pass = u\randPass());
  }

  r\notify($myid, 'verify', ray('name qid code pass', $newA->name, $qid, $pw1, $pass), TRUE); 

  if (nn($code)) w\invitation($code, $myid); // mark invitation used
  
  say('info saved');

  if ($ownPhone != 1) {
    be\invoice(r\acct(CGID), $mya, EQUIP_DEPOSIT, t('equipment deposit'), FOR_NONGOODS);
    say('refundable deposit');
  }

  return w\goNextStep('signup'); 
}
