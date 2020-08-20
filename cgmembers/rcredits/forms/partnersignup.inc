<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once R_ROOT . '/forms/signup.inc'; // for validation
require_once R_ROOT . '/pdf.class';

define('SF_URL', 'https://webto.salesforce.com/servlet/servlet.WebToLead?encoding=UTF-8');
define('CRM_URL', 'SMT solar customer management URL');
define('BILL_DIR', R_PICTURE_DIR . 'electric-bills');
define('CONTRACT_DIR', R_PICTURE_DIR . 'contracts');

/**
 * Accept and process signup data from a partner organization.
 * 1) Create a stub account and assign a random contract URL, so we can send data to partner's Salesforce db just once.
 * 2) Store info securely for SMT, Send data to Salesforce, email the partner, email the customer with a link to the contract.
 * 3) Next step, when customer clicks link, is to choose a payment system and, if they choose CG, do two more steps
 * 4) Final step is a congratulations notice, mail final contract (and CG info if any) to customer, email contract to partner
 */
function formPartnerSignup($form, &$sta, $args = '') {
  global $mya;
  
  extract(just('signed end solar1', $args, FALSE));
  if ($solar1) return gotSolarOne($solar1);
  if ($end) return partnerEnd();
  if ($signed) return signedContract($signed, $sta);
  
  if (!$pA = pA($_POST['pid'])) return partnerErr(t('bad partner'), 'pid');
  return gotSignup($pA);
}

/**
 * Customer has signed the contract, thereby also verifying their email address. 
 * Create a PDF of the signed contract, based on the $code, and store it where we told the partner's Salesforce to expect it.
 * Next, see how the customer wants to pay.
 * If they choose Common Good, initiate an appropriate short multi-step signup sequence.
 * @param string $code: an encrypted array of all data needed for the contract, plus a permanent or temporary cgAccount.
 * @return a form to display to request payment information
 */
function signedContract($code, &$sta = '') {
  global $pagePartner, $mya;
  
  if (!$info = u\decryRay($code)) return softErr(t('That is not a valid link.'));
  signout(TRUE); // after decryRay, so tests work
  extract(just('cgAccount pid customer m_company co', $info, FALSE));
  if (!$pA = pA($pid)) return softErr(t('bad partner'));
  $partnerName = $pA->fullName;
  $pagePartner = $pA->name; // tell page.html to use the partner's branding
  if (!$cgAccount or !$mya = cgA($cgAccount)) return softErr(t('You have already completed your signup with ') . $partnerName);
  
  $myid = hidFld($mya->id);
  $mya->stepDone('verifyemail');
  
  if (!file_exists(contractFlnm($customer))) fillContract($pA->name, $info, [], TRUE); // only if first time clicking the link
  if ($mya->member) return partnerEnd(); // customer is already set up to pay with Common Good
  if ($mya->stepIsDone('fund')) return partnerContinue($mya, $sta);

  $title = t('Enter Your Payment Information');
  $subtext = t(<<< X
    <p>Thank you for signing the contract! The next step is to tell us how you will pay for your solar credits.</p>
    <p>You will get two bills for your subscription solar:</p>
    <ul><li>One bill from %m_company as usual that shows how much electricity you used and what %m_company is billing you for it.  That bill will also have a credit that shows how much money you received from your solar subscription.</li>
    <li>The other bill will be from %partnerName. We will bill you for the credits you received, minus your discount, plus any membership fee due.</li></ul>

    <p>Enter the bank account information for the <span class="loud">checking account</span> you will use to pay that bill.</p>
X
  , compact(ray('m_company partnerName')));

  $routingNumber = routingFld(REQ);
  //$bankAccount = textFld(t('Account:'), t('Type carefully your checking account number.'));
  $bankAccount = intFld(REQ . t('Account:'), t('Type <b>carefully</b> your <b>checking account</b> number.'), maxlen(17));
    
  $midtext = t(<<< X
    <p>Now choose which financial institution will process your payments. We recommend using %PROJECT because it provides a lot of other community benefits and costs us less. %PROJECT is to banking what %partnerName is to energy. Check out their website: <%a>CommonGood.earth</a>.</p>
X
  , 'partnerName _a', $partnerName, atag(PROMO_URL));
  
//  if ($co) $midtext .=  t('<p><b>NOTE:</b> If you choose %PROJECT, as a participating %PROJECT company you will also need to open an individual %PROJECT account and complete several additional steps. You probably will want to do that if you passionately support community economic democracy or despise credit card fees. %PROJECT staff are happy to talk to you about how it all works, by phone (%CGF_PHONE) or email (%CGF_EMAIL).</p>');

  $processor = radiosFld(t('Payment Processor:'), '', required(0), [PROJECT, t('Standard ACH')]);
  
  $data = hidFld(serialize(compact(ray('info pid'))));
  $submit = t('Submit');
  
  return cgForm(compact(ray('title subtext routingNumber bankAccount midtext processor submit data myid')));
}

/**
 * Handle submission of payment processor form (/partner/signed=whatever)
 */
function formPartnerSignup_validate($form, &$sta, $args = '') {
  global $mya;
  $accts = 'routingNumber bankAccount';
  extract(just($accts, $sta['input']));
  
  foreach (ray($accts) as $one) $$one = preg_replace('/[\.\- ]/', '', nn($$one));
  u\preray(compact(ray($accts)), $sta['input']);
  if (!u\routingOk($routingNumber)) return say('bad routing number', 'routingNumber');
  if (!u\bankAcctOk($bankAccount)) return say('bad account number', 'bankAccount');
  
  if (nn($sta['input']['city2'])) { // solar1, so extra fields
    extract(u\normalizeCase(just($normals = 'fullName address2 city2', $sta['input'])));
    extract(just('email phone state2 zip2 years months myid', $sta['input']));

    if ($err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');
    $mya = r\acct($myid); // shortname uses this
    $name = r\shortName($fullName);

    if ($err = u\badZip($zip2)) return say($err, 'zip');
    if ($err = u\badPhone($phone)) return say($err, 'phone');

    if (isset($years)) { // showing tenure field
      if (is_null($tenure = w\tenure($years, $months, $err))) return say($err, 'years');
      $extras = ' tenure';
    } else $extras = '';
    
    u\preray(compact(ray($normals . ' email phone name' . $extras)), $sta['input']); // fix formatting and add name
  }
}

/**
 * Handle submission of payment processor form (/partner/signed=whatever)
 */
function formPartnerSignup_submit($form, &$sta) {
  global $mya;
  
  extract(just('routingNumber bankAccount processor data myid', $sta['input']));
  extract(just('info pid', unserialize($data)));
  if (!$pA = r\acct($pid)) u\FAIL('no partner account');
  if (!$mya = r\acct($myid)) u\FAIL('no customer account');
  r\setAcct($mya, $sta); // part way through, so gotta sign back in
  
  $bankAccount = "USkk$routingNumber$bankAccount";
  $last4bank = substr($bankAccount, -4, 4);
  $bankOk = FALSE;
  $mya->update(compact(ray('bankAccount last4bank bankOk')));
  
  if (nn($sta['input']['city2'])) { // solar1, so extra fields
    extract(just('sorch fullName name address2 city2 email phone state2 zip2 tenure owns', $sta['input']));
    $state2 = r\realState($state2);
    $postalAddr = "$address2, $city2, $state2 $zip2";
    $isSavings = (bool) $sorch;
    $partnerInfo = compact(ray('fullName email phone')) + $info;
    $mya->update(compact(ray('isSavings fullName name email phone postalAddr tenure partnerInfo')));
    $mya->setRisk('rents', !$owns);
  }
  if ($processor == 1) return w\go('partner/end=1'); // Standard ACH
  $mya->stepDone('fund'); // otherwise, new CG member (old members get handled without submitting the funding form)

  return w\goNextStep('signup');
}

/**
 * Partner signup is complete. Send final emails and congratulate the customer.
 */
function partnerEnd() {
  global $mya, $pagePartner;

  if (!$mya) return softErr(t('no account in partner end'));
  if (!$info = $mya->partnerInfo) return softErr(t('no partner info in partner end'));
  extract(just('pid customer noContract', $info));
  if (!$pA = pA($pid)) return softErr(t('bad partner'));
  $pagePartner = $pA->name; // tell page.html to use the partner's branding

  $title = t('Congratulations!');
  $subtext = t('<p>You have completed your solar subscription sign-up. %contractAttached</p><p>We will let you know when your array is ready to start delivering credits to your electric account and when we will start billing you for those credits. Let us know if you have any questions!</p><p><b>Learn more</b> about the organizations working together on this community solar program:</p>', 'contractAttached', nn($noContract) ? '' : t('We emailed you the signed contract.')) . getLinks($mya->zip);

  emailContract($pA, $info, TRUE, nn($noContract) ? NULL : contractFlnm($customer));
  if ($mya->stepIsDone('fund')) { // CG member (new or old)
    $reid = $pA->newRelation(ray('other customer code autopay', $mya->id, TRUE, $customer, TRUE)); // autopay should maybe be FALSE outside Western MA
  }

  if ($mya->nextStep() == 'partnerend') { // new CG member finishing signing up through the partner
    $mya->stepDone('partnerend');
    $mya->update('legalName', $mya->fullName); // no longer just an asterisk
    $mya->setBit('nonudge', FALSE);
  } else { // old CG member or nonmember (AZ B&T)
// NOT YET (need to store for later upload to CRM)  if (!$mya->stepIsDone('fund')) db\del('users', 'uid', $mya->id); // nonmember
  }

  if (!isDEV) // stay signed in on DEV so we can see messages about what emails got sent
  signout(TRUE);
  return cgform(compact(ray('title subtext')));
}

/**
 * Analyze and handle joint signup data from signup form on partner website. Send a contract, display a view-only page, and wait.
 * @param acct $pA: the partner account
 * @param assoc $_POST: (implicit) the data submitted in the form
 */
function gotSignup($pA) {
  global $mya, $pagePartner;
  
  signout(TRUE); // current account is irrelevant and could be confusing, so sign out
  $partnerName = $pA->fullName;
  $pagePartner = $pA->name; // tell page.html to use the partner's branding
  
  $sfFlds = ray('first_name:first_name,last_name:last_name,orgName:company,address:street,city:city,state:state,zip:zip,m_address:Meter_Street_Address__c,m_city:Meter_City__c,m_state:Meter_State__c,m_zip:Meter_ZIP_Code__c,m_company:Electric_Company__c,m_number:Electric_Account_Number__c,m_amount:Electric_Bill_Monthly_Amount__c,m_person:Electric_Account_Holder__c,referrer:Referrer__c,source:lead_source,m_wholeAddress:Electric_Meter_Address__c,cgAccount:cg_account,partnerName:host,oid:oid,retURL:retURL'); // salesforce fields (mapped from POST fields)
  $keys = array_keys($sfFlds); // keys from form corresponding to keys expected in Salesforce
  $sfKeys = array_values($sfFlds); // keys of data to send to Salesforce

  extract($info = just(join(' ', $keys) . ' type fullName email phone address city state zip years owns pid', $_POST));

  // set Salesforce parameters
  $oid = '00DA0000000gGh5';
  $retURL = 'http://cooppower.coop'; // return from salesForce
  list ($first_name, $last_name) = u\parseName($fullName);
  if (empty($first_name)) $first_name = '-';
  $m_wholeAddress = "$m_address, $m_city, $m_state $m_zip";
  $customer = substr($m_company, 0, 1) . $m_number; // begin meter/customer number with the company letter (ENC)
  $upfront = FALSE; // default to not paying membership fee upfront (don't scare people by asking the question)

  // validate data
  $co = ($type == '1');
  
  foreach ($keys as $k) {
    if (!in($k, 'referrer orgName cgAccount m_person') and empty($$k)) return partnerErr('parameter must not be empty', $k);
    if ($co and empty($orgName)) return partnerErr('parameter must not be empty', 'orgName');
    if ($err = u\badName($$k)) return partnerErr($err, $k);
  }

  foreach (ray('fullName orgName city address m_city m_address') as $k) $info[$k] = $$k = u\normalizeCase($info[$k]);
  $info += compact(ray('co customer'));

  if ($err = u\badZip($zip)) return partnerErr($err, 'zip');
  if ($err = u\badZip($m_zip)) return partnerErr($err, 'm_zip');
  if (!emailOkay($email, TRUE, NULL, TRUE)) return partnerErr('', 'email');
  if ($err = u\badPhone($phone)) return partnerErr($err, 'phone');
  if (u\poAddress($m_address)) return partnerErr('po in location', 'm_address');
  if ($err = u\badAmount($years, '>=0')) return partnerErr(tr($err) . ' ' . t('The number of years you have been at that address must be a number.'), 'years');
  if (test() and is_numeric($state)) $state = r\realState($state);
  foreach (ray('state m_state') as $k) if (!${$k . 'Id'} = r\stateNum($$k)) return partnerErr('bad state', $k);
  if (r\relation(1, 'main=:main AND :IS_CUSTOMER AND code=:customer', ray('main customer', $pA->uid, $customer))) {
    return softErr(t('You are already signed up with %partnerName.', compact('partnerName')));
  }

  foreach ($sfFlds as $k => $v) $sfData[$v] = $$k; // save Salesforce data
// NO! bad security  $sfData['contractUrl'] = contractUrl($customer);

  // handle Common Good signup
  $postalAddr = "$address, $city, $state $zip";
  foreach (ray('address city state zip') as $k) {$$k = ${"m_$k"}; unset(${"m_$k"});}
  u\preRay(compact(ray('postalAddr address city state zip')), $info);

  $flnms = renameUploads($_FILES, $customer);
  
  if ($cgAccount) { // person already has an account, according to them (be careful not to trust that), so no need for this form
    if (!$mya = r\acct($cgAccount) or !$similar = u\similar($mya->fullName, $fullName, 5, .5) or !($mya->phone == u\fmtPhone(nn($phone), '+n') or $mya->email == $email)) {
      if ($mya and !$similar) w\say(t('The name on that account is different.'), 'alert');
      return w\softErr(t('That does not appear to be your correct %PROJECT member ID. Use your browser\'s Back button to go back to the previous page and be sure to use the same name, phone number, and email address that you use in your %PROJECT account. If you are not yet a %PROJECT member, leave that field blank!', '_a', atag('/signin')), 'cgAccount'); // member mistyped their qid (a royal pain in the butt)
    }
  } else { // no CG account, so create a stub

    $flags = u\bit(B_NONUDGE); // no way to sign in yet, so don't bug them
    if ($co) {
      $flags |= u\bit(B_CO);
      $contact = $fullName;
      $fullName = $orgName;
      u\preRay(compact(ray('contact fullName')), $info);
    }

    $legalName = '*';
    $tenure = $years * 12;
    if (nn($referrer)) $source = nn($source) . ': ' . $referrer;
    $idVerifiedBy = $pA->fullName;
    $helper = $pA->uid;
    $state = $m_stateId;
    $acctInfo = compact(ray('legalName tenure source idVerifiedBy helper flags state'))
    + just('fullName contact phone email address city zip postalAddr source', $info);
   
    $mya = new r\Acct($acctInfo);
    u\EXPECT($mya, 'cannot create account');
    if (isset($owns)) $mya->setRisk('rents', !$owns);
    $sfData['cg_account'] = $info['cgAccount'] = $mya->mainQid . '*'; // asterisk means account is tentative

/*    if ($co) {
      $mya->setTask('partnerco');
    } else { */
    $mya->setTask('partner');
    if ($co) $mya->stepDoneForNow('signup'); else $mya->stepDone('signup');
    $mya->stepDone('verifyid');
//    }
    $mya->stepDone('contact');

  }

  $mya->update('partnerInfo', $info); // store for final step and beyond
  $flnm = fillContract($pA->name, $info, [], FALSE); // get populated contract
  emailContract($pA, $info, FALSE, $flnm);
  unlink($flnm);
  sendToCRM($pA, $sfData);
  tellPartner($pA, $info, $flnms);

  $title = t('Sign the Contract');
  $subtext = t('<p>You have completed the first step of your solar subscription sign-up.</p><p>We emailed you a contract. Please click the button in that email within 3 days, to sign the contract.</p><p>If you do not see the email, look for it in your Spam folder.</p><p>You can also preview the standard contract <%a>here</a>.</p><p>If you have questions, please call us at %phone!', '_a phone', atag("/rcredits/images/contracts/$pA->name.pdf"), u\fmtPhone($pA->phone, '-'));
  
  return cgform(compact(ray('title subtext')));
}

/**
 * Handle joint signup data from SMT (uploaded by Solar One). Combine functionality of gotSignup() and signedContract().
 * @param assoc $info: encoded parameters passed in URL
 * NOTE: to remove specialized code for Solar One, search this file (only) for Solar One and solar1.
 */
function gotSolarOne($info) {
  global $mya, $pagePartner;

  /*debug(*/ if (isDEV or (!is_array($info) and $info == 1)) $info = bin2hex(http_build_query(ray('smtid co fullName orgName address city state zip email phone m_number source', '654321', 0, 'Jane Dough', '', '10 J St.', 'Jville', 'NY', '10101', 'wspademan+j@gmail.com', '(212) 222-1111', rand(1000, 9999), 'sunset')));
  
  signout(TRUE); // current account is irrelevant and could be confusing, so sign out
  if (!$info = hex2bin($info)) return solarOneErr('bad solar1 encoding');
  
  $keys = 'smtid co fullName orgName address city state zip email phone m_number source';
  extract($info = just($keys, $info));

  $noContract = TRUE;
  $m_company = 'Con Ed';
  $customer = substr($m_company, 0, 1) . $m_number; // begin meter/customer number with the company letter (ENC)
  $pA = pA($pid = (isDEV ? 'NEWAIL' : 'NEWBHS'));
  $partnerName = $pA->fullName;
  $pagePartner = $pA->name; // tell page.html to use the partner's branding

  // validate data
  
  foreach (ray($keys) as $k) {
    if (!in($k, 'orgName') and !isset($$k)) return solarOneErr('parameter must not be empty', $k);
    if ($co and empty($orgName)) return solarOneErr('co parameter must not be empty', 'orgName');
    if ($err = u\badName($$k)) return solarOneErr($err, $k);
  }

  foreach (ray('fullName orgName city address') as $k) $info[$k] = $$k = u\normalizeCase($info[$k]);

  if ($err = u\badZip($zip)) return solarOneErr($err, 'zip');
  if (!emailOkay($email, TRUE, NULL, TRUE)) return solarOneErr('', 'email');
  if ($err = u\badPhone($phone)) return solarOneErr($err, 'phone');
  if (u\poAddress($address)) return solarOneErr('po in location', 'address');
  if (isset($years) and $err = u\badAmount($years, '>=0')) return solarOneErr(tr($err) . ' ' . t('The number of years you have been at that address must be a number.'), 'years');
  if (test() and is_numeric($state)) $state = r\realState($state);
  if (!$stateId = r\stateNum($state)) return solarOneErr('bad state', 'state');

  if ($ta = r\acct($email) and $ta->hasBank and $ta->partnerInfo['customer'] == $customer) return partnerContinue($ta, $sta);

  $info = compact(ray($keys . ' m_company customer pid noContract')); // recreate info with possibly improved values

  $flags = u\bit(B_NONUDGE); // no way to sign in yet, so don't bug them
  if ($co) {
    $flags |= u\bit(B_CO);
    $contact = $fullName;
    $fullName = $orgName;
    u\preRay(compact(ray('contact fullName')), $info);
  }

  $legalName = '*';
  $idVerifiedBy = $pA->fullName;
  $helper = $pA->uid;
  $state = $stateId;
  $acctInfo = compact(ray('legalName idVerifiedBy helper flags state'))
  + just('fullName contact phone email address city zip source', $info);
 
  $mya = new r\Acct($acctInfo);
  u\EXPECT($mya, 'cannot create account');
  $info['cgAccount'] = $mya->mainQid . '*'; // asterisk means account is tentative

  $mya->setTask('partner');
  if ($co) $mya->stepDoneForNow('signup'); else $mya->stepDone('signup');
  $mya->stepDone('verifyid');
  $mya->stepDone('contact');

  $mya->update('partnerInfo', $info); // store for final step and beyond
// ----- end of gotSignup() functionality

  $mya->stepDone('verifyemail');
  
  if ($mya->stepIsDone('fund')) {
    r\setAcct($mya, $sta);
    $title = t('Next Step');
    $subtext = t('Click the button below to continue setting up your %PROJECT payment account, as your chosen method of paying for your community solar subscription.');
    $go = item(button(t('Continue'), $mya->nextStepUrl()));
    return cgform(compact(ray('title subtext go')));
  }

  $title = t('Enter Your Payment Information');
  $subtext = t(<<< X
    <p>The next step in your community solar sign-up is to tell us how you will pay for your solar credits.</p>
    <p>You will continue to get a bill from Con Ed as usual that shows how much electricity you used and what Con Ed is billing you for it. That bill will also have a credit that shows how much money you received from your solar subscription.</p>
    <p>You will also get a subscription solar bill from %partnerName. We will bill you for the credits you received, minus your discount, plus any membership fee due.</p>
    <p>For example, you could get a bill from Con Edison that says:<br>
    <blockquote>You used $110 worth of electricity.<br>
    You received a credit of $100 from your solar subscription.<br>
    You need to pay Con Edison $10.</blockquote></p>

    <p>The bill from the NYC Community Energy Co-op would then say:<br>
    <blockquote>You received a $100 subscription solar credit on your Con Edison bill.  <br>
    You owe the NYC Community Energy Co-op $85 plus your membership fee of $1.50, for a total due of $86.50.</blockquote></p>
X
  , compact(ray('m_company partnerName')));

  $processorPre = item(t('Now choose which payment option you want to use to process your payments. We have two options for you:'), t('Payment Option:'));
  
  $processor = radiosFld('', '', required(0), [t('<b>%PROJECT</b>, a secure payment processing system (recommended<sup>*</sup>)<br><br>OR'), t('<b>Standard ACH</b>')]);
  
  $midtext1 = t('<p><sup>*</sup> The Sunset Park Solar Team recommends using %PROJECT because it focuses on community reinvestment (like setting up a lending pool for local projects, for example). Check out their website for more information: <%a>CommonGood.earth</a>.</p>', 'partnerName _a', $partnerName, atag(PROMO_URL));  
  
  $checkHint = item(tr('<img src="%src" />', 'src', BASE_URL . '/rcredits/images/check-hint.png'));
  
  $midtext2 = t('<p>Enter the bank account information for the savings or checking account you will use to pay your solar subscription bill.</p>');

  $routingNumber = routingFld(REQ);
  //$bankAccount = textFld(t('Account:'), t('Type carefully your checking account number.'));
  $bankAccount = intFld(REQ . t('Account:'), t('Type your bank account number. (Please check to be sure it\'s correct!)'), maxlen(17));
  $sorch = radiosFld(t('Type:'), '', required(-1), [t('Checking'), t('Savings')]);

//  $bankInfo = fieldSet(BARE . 'bankInfo', compact(ray('midtext2 routingNumber bankAccount sorch')));
  
  $data = hidFld(serialize(compact(ray('info pid'))));
  $submit = t('Submit');

  // additions for Solar One
//  $fullName = textFld($co ? t('Company Name:') : t('Your Name:'), '', required($fullName));
  $electricName = item($fullName, t('Name on Electric Account:'));
  $fullName = textFld(t('Name on Bank Account:'), [t('Name'), t('Please correct, if needed')], required($fullName));
//  $contact = $co ? textFld(t('Your Name:'), '', required($contact)) : NULL;
  $contact = $co ? hidFld($contact) : NULL;
//  $phone = phoneFld(REQ . t('Phone:'), [t('Your phone number')], dft($phone));
  $phoneDpy = item(u\fmtPhone($phone, '-'), t('Phone:'));
  $phone = hidFld($phone);

  $emailDpy = item($email, t('Email:'));
  $email = hidFld($email);
  $mailingHdr = item(t('<h3>Billing Address</h3>'));
  $address2 = textFld(t('Street Address:'), [t('Postal street address')], required($address));
  $city2 = textFld(t('City:'), [t('Postal city')], required($city));
  $state2 = stateFld($state);
  $zip2 = textFld(t('Postal Code:'), [t('Postal code')], required($zip));
  $years = intFld(BARE, [t('Years')], suffix(t('&nbsp years + ')));
  $months = intFld(BARE, [t('Months')], suffix(t('&nbsp months')));
  $tenure = fieldSet('tenure2', compact('years', 'months'), t('How long:'), t('How long have you been at your current address (roughly)'));
  $owns = radiosFld('', '', required(-1), [t('Rent'), t('Own')]);
  $myid = hidFld($mya->id);

  jsx('contact');

  return cgForm(compact(ray('title subtext electricName fullName contact phoneDpy phone emailDpy mailingHdr address2 city2 state2 zip2 tenure owns processorPre processor midtext1 midtext2 checkHint routingNumber bankAccount sorch submit data email myid')));
}

/**
 * Rename the uploaded files ASAP so PHP doesn't delete them
 * @return an array of new pathnames (NULL if none)
 */
function renameUploads($files, $meter) {
  if (!is_dir(BILL_DIR)) mkdir(BILL_DIR, 0755) or u\FAIL('cannot create electric bill directory'); // required for tests
  if ($files) foreach ($files as $f => $one) {
    if (!$one['name']) {if (isDEV) break; else u\FAIL('missing file');}
    extract(just('name type tmp_name error size', $one));
    if ($size > 10 * MAX_ID_SIZE * 1024 * 1024) {
      w\say(t('<p>That file is too big. You will need to send the photos of your electric bill to %partner by postal mail or as attachments to an email (%partnerEmail).</p>', 'partner partnerEmail', $pA->fullName, $pA->email), 'err');
      $flnms = NULL;
      break;
    }
    $flnms[] = $flnm = BILL_DIR . "/$meter-$f." . pathinfo($name)['extension']; // like E12345-page1.png
    u\deleteFile($flnm);
    rename($tmp_name, $flnm);
  }
  return nn($flnms, NULL);
}

/**
 * Tell the partner about this signup and forward any attached files.
 */
function tellPartner($pA, $info, $flnms) {
  if (!$pA->website) r\tellAdmin(t('Partner %partner has no website. Fix it on the Company Info settings page.', 'partner', $pA->fullName)); // partner should always have a website
  extract(just('customer fullName cgAccount email', $info));
  
  $fields = ray('partnerName cgAccount fullName email customer noFrame', $pA->fullName, $cgAccount, $fullName, $email, $customer, TRUE);
  r\rMail('partner-signup', $pA, $fields, NULL, nn($flnms));
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

/**/ function solarOneErr($msg, $k) {die("$msg (field $k)");}
  
/**
 * Send data to the partner's CRM(s)
 */
function sendToCRM($pA, $data) {
  global $mya;

  if (test()) return;
  $data['created'] = date('Y-m-d');
  foreach (ray('created host upfront') as $k) unset($data[$k]);
//  $data['debugEmail'] = 'wspademan@gmail.com';
//  $data['debug'] = 1;
  $res = u\post(SF_URL, $data); // send data to partner's Salesforce (fails on DEV)
  // NOT YET u\post(CRM_URL, $data); // send data to the partner's CRM program
}

/**
 * Email a contract for Community Solar to the customer.
 * @param bool $sign: <sign the contract>
 */
function emailContract($pA, $info, $sign, $contractFlnm) {
  global $mya;

  extract(just('fullName email customer noContract', $info));

  $partnerName = $pA->fullName;
  $partnerReply = [$pA->email => $pA->fullName];
  $subs = ray('fullName customer email partnerName partnerAddress partnerPhone partnerEmail pseudo noFrame', $fullName, $customer, $email, $partnerName, $pA->postalAddr, u\fmtPhone($pA->phone, '-'), $pA->email, $partnerName, TRUE); // white label from partner name (but our email)

  if ($sign) {
    list ($extra, $cgAccount) = $mya->stepIsDone('fund') ? ['', $mya->mainQid]
    : [t('You will need to delete the %PROJECT account ID in your database.'), t('(none)')];
    $processor = $mya->stepIsDone('fund') ? PROJECT : t('Standard ACH');
    $tpl = nn($noContract)? 'partner-solar1report' : 'partner-report';
    r\rMail($tpl, $partnerReply, $subs + compact(ray('extra processor cgAccount')), NULL, $contractFlnm);
    $links = getLinks($mya->zip);

    if ($mya->nextStep() == 'partnerend') { // completing new member signup
      tellStaffNewMember($info + ray('acctType', $mya->co ? CO_CUSTODIAN : CO_PERSONAL)); // report to admin
      $mya->update('pass', $pass = u\randPass());
      $links = str_replace('</li></ul>', '', $links);
      $links .= t('&nbsp; <b>Account ID:</b> %qid &nbsp; <b>Password:</b> %pass</li></ul>', 'qid pass', strtolower($mya->mainQid), $pass);
    } else $extra = '';
    
    $tpl = nn($noContract)? 'partner-solar1end' : 'partner-end';
    r\rMail($tpl, [$email => $fullName], $subs + compact('extra', 'links'), $partnerReply, $contractFlnm);
  } else {
    r\tellAdmin(t('New %partnerName customer: %fullName', compact('partnerName', 'fullName')), $info);
    $code = u\cryRay($info);
    r\rMail('partner-contract', [$email => $fullName], $subs + compact('code'), $partnerReply, $contractFlnm);
  }
}

/**
 * Create a PDF of the specified contract, filled with the given data.
 * @param string $host: the name of the contract image (without path or extention) in the contracts folder
 * @param assoc $data: data to use to fill the contract
 * @param array $map: UNUSED an array of coordinates for filling the contract. Each entry is [field, page, x, y, attributes].
 * @param boot $sign: <include signature and date (current date if none given)> "signature" field is fullName in handwriting
 * @return the path to the contract
 * NOTE: use https://smallpdf.com/pdf-to-jpg to convert
 */
function fillContract($host, $data, $map, $sign = FALSE) {
  global $mya;
  $dir = R_ROOT . "/images/contracts/$host/";
  $pages = array_slice(scandir($dir), 2); // ignore . and ..
  
  extract(just('customer fullName contact postalAddr email phone m_company m_number m_address m_city m_state m_zip', $data));
  list ($address, $city, $state, $zip) = u\parseAddr($postalAddr);
  $phone = u\fmtPhone($phone, '-');
  $buyer = "$fullName<br><br>$address<br>$city, $state, $zip<br>$phone<br>$email<br>";
  $meter = "Electric Company: $m_company<br>Electric Meter Account #$m_number<br>$m_address<br>$m_city, $m_state $m_zip<br>";
  
  $pdf = new r\Pdf();
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(FALSE);
  $pdf->setPageUnit('in'); // measure everything in inches
  $pdf->SetAutoPageBreak(FALSE);
  
  list ($W, $H) = array($pdf->pageW, $pdf->pageH);
  $lineH = $pdf->lineH; // standard line height in inches
  $m = .71; // left/right margins

//  $pdf->Rect(.75, 3.75, 3.5, 1.75, 'D', [], [255,255,255]); // x, y, w, h, style, border, fill

  foreach ($pages as $pi => $page) {
    $pdf->AddPage();
    $pdf->Image($dir . $page, 0, 0, $W, $H, '', '', 'L', true); // file, x, y, w, h, type, link, align, resize
    
    list ($buyerX, $buyerY, $sigX, $sigY, $sigNameY, $dateX, $dateY, $sigPage, $font) = 
       $host == 'cooppower' ? [.47, 2.7, 1.2, 9.88, 9.23, 1.2, 8.6, 5, '10;DejaVuSans']
    : ($host == 'nyccec' ? [.71, 3.5, .71, 9.25, 0, 6.25, 9.25, 8, '11;Times']
    : NULL); // invalid partner should get caught way before this (NULL causes death)
    
    if ($pi == 0) $pdf->say("$buyer<br><br>$meter", $buyerX, $buyerY, 5, 1.75, $font, 'L'); // text, x, y, w, h, sizesEtc, align
    if ($sign and $pi == $sigPage) {
      list ($name, $signer) = nn($contact) ? [$fullName, $contact] : ['', $fullName];
      $pdf->say(u\fmtDate(now(), TRUE), $dateX, $dateY, 5, 1.75, $font, 'L');
      if ($host == 'cooppower') {
        $sellerX = 5.2;
        $pdf->say($name, $sigX, 8.3, 5, 1.75, $font, 'L');
        $pdf->say($signer, $sigX, $sigNameY, 5, 1.75, $font, 'L');
        $pdf->say('Mark Skinder, Dir. of Energy Services', $sellerX, $sigNameY, 5, 1.75, $font, 'L');
        $pdf->say(u\fmtDate(now(), TRUE), $sellerX, $dateY, 5, 1.75, $font, 'L');
        $pdf->setTextColor(0, 0, 128); // sign in color (so do this last)
        $pdf->say('Mark Skinder', $sellerX, $sigY, 5, 1.75, '22;B;DSGabriele', 'L');
      } elseif ($host == 'nyccec') {
        
      }
      $pdf->setTextColor(0, 0, 128); // sign in color (so do this last)
      $pdf->say($signer, $sigX, $sigY, 5, 1.75, '22;B;DSGabriele', 'L');
    }
  }
  
  $flnm = $sign ? contractFlnm($customer) : (sys_get_temp_dir() . '/contract-' . u\code(FALSE, 10) . '.pdf');
  u\deleteFile($flnm);
  $pdf->Output($flnm, 'F');
  return $flnm;
}

/**
 * Return links for more information, appropriate to the given zipcode.
 */
function getLinks($zip) {
  $links['uprose'] = 'UPROSE,https://uprose.org,' . t('sustainability and resiliency in Brooklyn\'s Sunset Park');
  $links['solar1'] = 'Solar One,https://solar1.org,' . t('education, training, and technical assistance for sustainability and resiliency');
  $links['nyccec'] = 'NYC Community Energy Co-op,https://cooppower.coop/nyccec,' . t('sustainable solutions in Manhattan and the borroughs');
  $links['hampshirecec'] = 'Hampshire Community Energy Co-op,http://hampshire-energy.coop,' . t('low-carbon energy for Hampshire County, MA');
  $links['hampdencec'] = 'Hampden Community Energy Co-op,https://www.cooppower.coop/hampden-county,' . t('building justice and sustainability in Hampden County');
  $links['franklincec'] = 'Franklin Community Energy Co-op,https://www.cooppower.coop/franklin,' . t('community owned renewable energy');
  $links['cooppower'] = 'Co-op Power,https://cooppower.coop,' . t('building locally-owned sustainable energy');
  $links['cg'] = PROJECT . ',' . PROMO_URL . ',' . t('the payment card for community power');

  $z2 = substr($zip, 0, 2);
  $list = in($z2, '10 11') ? 'uprose solar1 nyccec'
  : (in($z2, '01') ? 'hampshirecec hampdencec franklincec'
  : '');
  $list .= ' cooppower cg';
  foreach (just($list, $links) as $k => $one) {
    list ($org, $_url, $desc) = explode(',', $one);
    $res[] = tr('<a href="%url">%org</a>', compact(ray('org _url desc')));
  }
  return '<ul><li>' . join("</li><li>\n", $res) . '</li></ul>';
}

/**
 * Give the customer a way to continue signing up.
 */
function partnerContinue($a, &$sta) {
  r\setAcct($a, $sta); // part way through, so gotta sign back in
  $title = t('Next Step');
  $subtext = t('Click the button below to continue setting up your %PROJECT payment account, as your chosen method of paying for your community solar subscription.');
  $go = item(button(t('Continue'), $a->nextStepUrl()));
  return cgform(compact(ray('title subtext go')));
}  

function contractFlnm($customer) {
  if (!is_dir(CONTRACT_DIR)) mkdir(CONTRACT_DIR, 0755) or u\FAIL('cannot create contract directory'); // required for tests
  return CONTRACT_DIR . "/$customer.pdf"; // customer-specific flnm
}
//function contractUrl($customer) {return str_replace(ROOT, BASE_URL, contractFlnm($customer));}
function pA($pid) {return (nn($pid) and $pA = r\acct($pid) and in($pA->name, SIGNUP_PARTNERS)) ? $pA : FALSE;}
function cgA($qid) {return r\acct(str_replace('*', '', $qid));}
