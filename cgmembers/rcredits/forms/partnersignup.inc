<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once R_ROOT . '/forms/signup.inc'; // for validation

/**
 * Accept and process signup data from a partner organization.
 */
function formPartnerSignup($form, &$sta, $args = '') {
  signout(TRUE); // current account is irrelevant and could be confusing, so sign out
  
  extract(just('pid', $_POST));
  $badp = t('That is not a %PROJECT signup-partner organization.');

  if (!@$pid or !$pA = r\acct($pid)) return partnerErr($badp, 'pid');
  
  if ($pA->name == 'cooppower') return doCoopPower($pA);
    
  return partnerErr($badp, 'pid');
}

function doCoopPower($pA) {
  global $mya; // currently NULL
  $partnerName = $pA->fullName;
  
  $sfFlds = ray('first_name:first_name,last_name:last_name,orgName:company,m_address:Meter_Street_Address__c,m_city:Meter_City__c,m_state:Meter_State__c,m_zip:Meter_ZIP_Code__c,m_company:Electric_Company__c,m_number:Electric_Account_Number__c,m_amount:Electric_Bill_Monthly_Amount__c,m_person:Electric_Account_Holder__c,referrer:Referrer__c,source:lead_source,m_wholeAddress:Electric_Meter_Address__c,cgAccount:cg_account'); // salesforce fields (mapped from POST fields)
  $keys = array_keys($sfFlds); // keys from form corresponding to keys expected in Salesforce
  $sfKeys = array_values($sfFlds); // keys of data to send to Salesforce
  $sfUrl = 'https://webto.salesforce.com/servlet/servlet.WebToLead?encoding=UTF-8';

  extract($info = just(join(' ', $keys) . ' type fullName email phone address city state zip years owns', $_POST));

  // set Salesforce parameters
  $debug = 1;
  $debugEmail = 'lynn@cooppower.coop';
  $oid = '00DA0000000gGh5';
  $retURL = ''; // return from salesForce -- maybe report start of CG signup to CP
  list ($first_name, $last_name) = u\parseName($fullName);
  $m_wholeAddress = "$m_address\n$m_city, $m_state $m_zip";

  // validate data
  $co = ($type == '1');
  
  foreach ($keys as $k) {
    if (!in($k, 'referrer orgName cgAccount') and empty($$k)) return partnerErr('parameter must not be empty', $k);
    if ($co and empty($orgName)) return partnerErr('parameter must not be empty', 'orgName');
    if ($err = u\badName($$k)) return partnerErr($err, $k);
  }

  extract(u\normalizeCase(just('fullName orgName city address m_city m_address', $info)));

  if ($err = u\badZip($zip)) return partnerErr($err, 'zip');
  if ($err = u\badZip($m_zip)) return partnerErr($err, 'm_zip');
  if (!emailOkay($email, TRUE, NULL, TRUE)) return partnerErr('', 'email');
  if ($err = u\badPhone($phone)) return partnerErr($err, 'phone');
  if (u\poAddress($m_address)) return partnerErr('po in location', 'm_address');
  if ($err = u\badAmount($years, '>=0')) return partnerErr(tr($err) . ' ' . t('The number of years you have been at that address must be a number.'), 'years');
  if (test() and is_numeric($state)) $state = r\realState($state);
  foreach (ray('state m_state') as $k) if (!${$k . 'Id'} = r\stateNum($$k)) return partnerErr('bad state', $k);

  foreach ($sfFlds as $k => $v) $sfData[$v] = $$k; // save Salesforce data for later

  // handle Common Good signup
  $postalAddr = "$address, $city, $state $zip";
  foreach (ray('address city state zip') as $k) {$$k = ${"m_$k"}; unset(${"m_$k"});}
  $state = $m_stateId;
  $customer = @$m_number;

  if (r\relation(1, 'main=:main AND :IS_CUSTOMER AND code=:customer', ray('main customer', $pA->id, $customer))) {
    return partnerErr(t('You are already signed up with %partnerName', compact('partnerName')), 'pid');
  }
  
  if ($cgAccount) { // person already has an account, according to them (be careful not to trust that)
    if (!$mya = r\acct($cgAccount) or !$similar = u\similar($mya->fullName, $fullName, 5, .5) or !($mya->phone == u\fmtPhone(@$phone, '+n') or $mya->email == $email)) {
      if ($mya and !$similar) w\say(t('The name on that account is different.'), 'alert');
      return w\partnerErr(t('That does not appear to be your correct %PROJECT member ID. Use your browswer\'s Back button to go back to the previous page and be sure to use the same name, phone number, and email address that you use in your %PROJECT account. If you are not yet a %PROJECT member, leave that field blank!").', 'partnerName _a', $partnerName, atag('/signin')), 'cgAccount'); // member mistyped their qid (a royal pain in the butt)
    }
    r\tellAdmin(t('new customer of %partnerName: %fullName', compact('partnerName', 'fullName')), $info);
    r\acct($pA->id, $mya->id)->setRel('customer code', TRUE, $customer); // mark old member as a customer of pA (NOT autopay)
    tellPartner($pA, $mya, $customer);
    
    sendToCRM($pA, $sfUrl, $sfData);

    return w\go('settings/partnerend');
//    $signinQid = ($co and !$mya->trialco) ? '' : $cgAccount;
//    return w\go("signin?name=$signinQid", t('Sign in to finish setting up your %PROJECT account for use with %partnerName.', compact('partnerName')));
  }

  // new member
  $tenure = $years * 12;
  if (@$referrer) $source = @$source . ': ' . $referrer;
  $autopay = TRUE;
  $idVerifiedBy = $partnerName;
  $helper = $pA->id;
  $info = compact(ray('fullName phone email address city state zip postalAddr source tenure idVerifiedBy helper'));

  if ($co) {
    $acctType = CO_CUSTODIAN;
    $flags = u\bit(B_CO) | u\bit(B_DEPENDS);
    $contact = $fullName;
    $fullName = $orgName;
    u\preRay(compact(ray('flags contact fullName')), $info);
  } else $acctType = CO_PERSONAL;

  $mya = new r\Acct($info);
  u\EXPECT($mya, 'cannot create account');
  if (isset($owns)) $mya->setRisk('rents', !$owns);

  w\setAcct($mya->id);
  $sta['uid'] = $mya->id; // tell Drupal we exist (essential so that messages show up on next page shown)
  
  if (!$co) {
    $mya->stepDone('signup');
    if ($mya->idVerifiedBy) $mya->stepDone('verifyid');
  }
  $mya->stepDone('contact');

  $reid = $pA->newRelation(ray('other customer code autopay', $mya->id, TRUE, $customer, $autopay));
  w\say(t('partner welcome', ['partner' => $pA->fullName])); // this should appear on the Agreement page
  tellPartner($pA, $mya, $customer);

  $sfData['cgAccount'] = $mya->mainQid; // we waited until now, to be able to report the qid
  //u\post($sfUrl, $sfData); // send data to Co-op Power's Salesforce

  return $co ? w\go('signup-co') : finishSignup($info + compact('acctType'), TRUE);
}

/**
 * Tell the partner about this signup and forward any attached files.
 * @return error message, if any.
 */
function tellPartner($pA, $a, $customer) {
  if (!$pA->website) r\tellAdmin(t('Partner %partner has no website. Fix it on the Company Info settings page.', 'partner', $pA->fullName)); // partner should always have a website
  
  foreach ($_FILES as $f => $one) {
    extract(just('name type tmp_name error size', $one));
    if ($size > 10 * MAX_ID_SIZE * 1024 * 1024) {
      w\say(t('<p>That file is too big. You will need to send the photos of your electric bill to Coop Power by postal mail or as attachments to an email (info@cooppower.coop).</p>'), 'err');
      $flnms = NULL;
      break;
    }
    $flnms[] = $flnm = dirname($tmp_name) . '/' . $name;
    rename($tmp_name, $flnm);
  }
  
  $fields = ray('accountId name email customer noFrame', $a->mainQid, $a->fullName, $a->email, $customer, TRUE);

  $pName = $pA->name;
  r\rMail("$pName-signup", $pA, $fields, NULL, @$flnms);
}

//  $fields = ray('op partnerCode qid name email customer', $op, $pA->emailCode, $a->mainQid, $a->fullName, $a->email, $customer)
//  if ($a->member) w\say('new customer done', ray('partnerName', $pA->fullName));
//  u\post($pA->api(), $fields); // tell partner we're starting or done

/**
 * Cancel the signup and alert an admin (should never happen).
 */
function partnerErr($msg, $k) {
  $subs = compact('msg', 'k');
  r\tellAdmin(t('partner signup: %msg (field %k)', $subs), $_POST);
  return w\softErr(t('<h2>Drat.</h2><p>There was a problem with the data you submitted (field %k) -- %msg</p><p>%PROJECT is looking into the problem and we will contact you with instructions for continuing.</p><p>We apologize for the delay.</p>', $subs)); // fix double spaces and leading or trailing
}

function setStepsDone($a) {
  if ($a->idVerifiedBy) $a->stepDone('verifyid');
  $a->stepDone('contact');
}

function sendToCRM($pA, $url, $data) {
  //u\post($url, $data); // send data to the partner's CRM program

  $data['created'] = date('Y-m-d');
  $fileName = LOG_DIR . $pA->name . '.csv'; // temporary log file for staging data to new CRM
  if (!file_exists($fileName)) file_put_contents($fileName, join("\t", array_keys($data)) . "\n", FILE_APPEND);
  file_put_contents($fileName, join("\t", array_values($data)) . "\n", FILE_APPEND);
}
