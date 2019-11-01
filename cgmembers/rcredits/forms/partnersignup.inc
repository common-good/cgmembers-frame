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
define('CRM_URL', 'SMT solar customere management URL');
define('OTHER_ACH', 'Arizona Bank & Trust');

/**
 * Accept and process signup data from a partner organization.
 * 1) Create a stub account and assign a random contract URL, so we can send data to partner's Salesforce db just once.
 * 2) Store info securely for SMT, Send data to Salesforce, email the partner, email the customer with a link to the contract.
 * 3) Next step, when customer clicks link, is to choose a payment system and, if they choose CG, do two more steps
 * 4) Final step is a congratulations notice, mail final contract (and CG info if any) to customer, email contract to partner
 */
function formPartnerSignup($form, &$sta, $args = '') {
  global $mya;
  
  extract(just('signed end', $args, FALSE));
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
 * @param string $code: a doCode of an assoc of all data needed for the contract, plus a permanent or temporary cgAccount.
 * @return a form to display to request payment information
 */
function signedContract($code, &$sta = '') {
  global $pagePartner, $mya;
  
  if (!$info = r\doCode($code)) return softErr(t('That is not a valid link.'));
  signout(TRUE); // after doCode, so tests work
  extract(just('cgAccount pid customer m_company co', $info, FALSE));
  if (!$pA = pA($pid)) return softErr(t('bad partner'));
  $partnerName = $pA->fullName;
  $pagePartner = $pA->name; // tell page.html to use the partner's branding
  if (!$cgAccount or !$mya = cgA($cgAccount)) return softErr(t('You have already completed your signup with ') . $partnerName);
  $mya->stepDone('verifyemail');
  
  if (!file_exists(contractFlnm($customer))) fillContract($pA->name, $info, [], TRUE); // only if first time clicking the link
  if ($mya->member) return partnerEnd(); // customer is already set up to pay with Common Good
  if ($mya->stepIsDone('fund')) return w\goNextStep('signup');

  $title = t('Source of Funding');
  $subtext = t(<<< X
    <p>Thank you for signing the contract! The next step is to tell us how you will pay.</p>
    <p>You will get two bills for your subscription solar:</p>
    <ul><li>One bill from %m_company as usual that shows how much electricity you used and what %m_company is billing you for it.  That bill will also have a credit that shows how much money you received from your solar subscription.</li>
    <li>The other bill will be from %partnerName. We will bill you for the credits you received, minus your discount, plus any membership fee due.</li></ul>

    <p>Enter the bank account information for the <span class="loud">checking account</span> you will use to pay that bill.</p>
X
  , compact(ray('m_company partnerName')));

  $routingNumber = textFld(t('Routing:'), t('Type carefully your bank\'s routing number.'), required());
  //$bankAccount = textFld(t('Account:'), t('Type carefully your checking account number.'));
  $bankAccount = textFld(t('Account:'), t('Type <b>carefully</b> your <b>checking account</b> number.'), required());
    
  $midtext = t(<<< X
    <p>Now choose which financial institution will process your payments. We recommend using %PROJECT because it provides a lot of other community benefits and costs us less. %PROJECT is to banking what %partnerName is to energy. Check out their website: <%a>CommonGood.earth</a>.</p>
X
  , 'partnerName _a', $partnerName, atag(PROMO_URL));
  
  if ($co) $midtext .=  t('<p><b>NOTE:</b> If you choose %PROJECT, as a participating %PROJECT company you will also need to open an individual %PROJECT account and complete several additional steps. You probably will want to do that if you passionately support community economic democracy or despise credit card fees. %PROJECT staff are happy to talk to you about how it all works, by phone (%CGF_PHONE) or email (%CGF_EMAIL).</p>');

  $processor = radiosFld(t('Payment Processor:'), '', required(0), [PROJECT, OTHER_ACH]);
  
  $data = hidFld(serialize(compact(ray('info pid'))));
  $submit = t('Submit');
  
  return cgForm(compact(ray('title subtext routingNumber bankAccount midtext processor submit data')));
}

/**
 * Handle submission of payment processor form (/partner/signed=whatever)
 */
function formPartnerSignup_validate($form, &$sta, $args = '') {
  $accts = 'routingNumber bankAccount';
  extract(just($accts, $sta['input']));
  
  foreach (ray($accts) as $one) $$one = preg_replace('/[\.\- ]/', '', @$$one);
  u\preray(compact(ray($accts)), $sta['input']);
  if (!preg_match('/^(|[0-9]{9})$/', $routingNumber)) return say('bad routing number', 'routingNumber');
  if (!db\exists('r_banks', 'route=:routingNumber', compact('routingNumber'))) return say('bad routing number', 'routingNumber');
  if (!preg_match('/^[0-9]{3,17}$/', $bankAccount)) say('bad account number', 'bankAccount'); // {3,20} ? Dwolla says 3-17
//    if ($bankAccount != $bankAccount2) say('mismatch', ['thing' => t('account number')], 'bankAccount2');  
}

/**
 * Handle submission of payment processor form (/partner/signed=whatever)
 */
function formPartnerSignup_submit($form, &$sta) {
  global $mya;
  
  extract(just('routingNumber bankAccount processor data', $sta['input']));
  extract(just('info pid', unserialize($data)));
  extract(just('cgAccount years referrer fullName orgName owns co customer', $info));
  if (!$pA = r\acct($pid)) u\FAIL('no partner account');
  if (!$cgAccount or !$mya = cgA($cgAccount)) u\FAIL('no account in partner signup'); // shoulda been created in partner signup
  w\setAcct($mya->id);
  $sta['uid'] = $mya->id; // tell Drupal we exist (essential so that messages show up on next page shown)
  
  if ($processor == 1) return w\go('partner/end=1'); // AZ B&T
  $mya->stepDone('fund'); // otherwise, new CG member (old members get handled without submitting the funding form)

  return $co ? w\go('signup-co') : w\goNextStep('signup');
}

/**
 * Partner signup is complete. Send final emails and congratulate the customer.
 */
function partnerEnd() {
  global $mya, $pagePartner;

  if (!$mya) return softErr(t('no account in partner end'));
  if (!$info = $mya->partnerInfo) return softErr(t('no partner info in partner end'));
  extract(just('pid customer', $info));
  if (!$pA = pA($pid)) return softErr(t('bad partner'));
  $pagePartner = $pA->name; // tell page.html to use the partner's branding
  emailContract($pA, $info, TRUE);
  if ($mya->stepIsDone('fund')) { // CG member (new or old)
    $reid = $pA->newRelation(ray('other customer code autopay', $mya->id, TRUE, $customer, FALSE));
  }

  if ($mya->nextStep() == 'partnerend') { // new CG member finishing signing up through the partner
    $mya->stepDone('partnerend');
    $midtext = t('<p>Want to go further with %PROJECT? Click below to apply for your free %PROJECT Card or participate in the %PROJECT Democracy in your community.</p>');
    $continue = submit(t('Continue to %PROJECT'));
  } else { // old CG member or nonmember (AZ B&T)
// NOT YET (storing for upload to CRM)    if (!$mya->stepIsDone('fund')) db\del('users', 'uid', $mya->id); // nonmember
    signout(TRUE);
    $midtext = $continue = NULL;
  }

  $title = t('Congratulations!');
  $subtext = t('<p>You have completed your solar subscription sign-up. We have emailed you a copy of the signed contract.</p><p>We will let you know when your array is ready to start delivering credits to your electric account and when we will start billing you for those credits. Let us know if you have any questions!</p>');
   
  return cgform(compact(ray('title subtext midtext continue')));
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
  
  $sfFlds = ray('first_name:first_name,last_name:last_name,orgName:company,m_address:Meter_Street_Address__c,m_city:Meter_City__c,m_state:Meter_State__c,m_zip:Meter_ZIP_Code__c,m_company:Electric_Company__c,m_number:Electric_Account_Number__c,m_amount:Electric_Bill_Monthly_Amount__c,m_person:Electric_Account_Holder__c,referrer:Referrer__c,source:lead_source,m_wholeAddress:Electric_Meter_Address__c,cgAccount:cg_account,partnerName:host,upfront:upfront,oid:oid,retURL:retURL'); // salesforce fields (mapped from POST fields)
  $keys = array_keys($sfFlds); // keys from form corresponding to keys expected in Salesforce
  $sfKeys = array_values($sfFlds); // keys of data to send to Salesforce

  extract($info = just(join(' ', $keys) . ' type fullName email phone address city state zip years owns pid', $_POST));

  // set Salesforce parameters
  $debug = 1;
  $debugEmail = 'lynn@cooppower.coop';
  $oid = '00DA0000000gGh5';
  $retURL = 'http://cooppower.coop'; // return from salesForce -- maybe report start of CG signup to CP
  list ($first_name, $last_name) = u\parseName($fullName);
  if (empty($first_name)) $first_name = '-';
  $m_wholeAddress = "$m_address\n$m_city, $m_state $m_zip";
  $customer = $m_number = substr($m_company, 0, 1) . $m_number; // begin meter/customer number with the company letter (ENC)
  $upfront = FALSE; // default to not paying membership fee upfront (don't scare people by asking the question)

  // validate data
  $co = ($type == '1');
  
  foreach ($keys as $k) {
    if (!in($k, 'referrer orgName cgAccount upfront') and empty($$k)) return partnerErr('parameter must not be empty', $k);
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
  if (r\relation(1, 'main=:main AND :IS_CUSTOMER AND code=:customer', ray('main customer', $pA->id, $customer))) {
    return softErr(t('You are already signed up with %partnerName.', compact('partnerName')));
  }

  foreach ($sfFlds as $k => $v) $sfData[$v] = $$k; // save Salesforce data
  $sfData['contractUrl'] = contractUrl($customer);

  // handle Common Good signup
  $postalAddr = "$address, $city, $state $zip";
  foreach (ray('address city state zip') as $k) {$$k = ${"m_$k"}; unset(${"m_$k"});}
  $state = $m_stateId;
  foreach (ray('state m_state') as $k) unset(${$k . 'Id'});
  u\preRay(compact(ray('postalAddr address city state zip')), $info);

  $flnms = renameUploads($_FILES, $customer);
  
  if ($cgAccount) { // person already has an account, according to them (be careful not to trust that), so no need for this form
    if (!$mya = r\acct($cgAccount) or !$similar = u\similar($mya->fullName, $fullName, 5, .5) or !($mya->phone == u\fmtPhone(@$phone, '+n') or $mya->email == $email)) {
      if ($mya and !$similar) w\say(t('The name on that account is different.'), 'alert');
      return w\softErr(t('That does not appear to be your correct %PROJECT member ID. Use your browser\'s Back button to go back to the previous page and be sure to use the same name, phone number, and email address that you use in your %PROJECT account. If you are not yet a %PROJECT member, leave that field blank!', '_a', atag('/signin')), 'cgAccount'); // member mistyped their qid (a royal pain in the butt)
    }
// NO, do this when contract is signed    r\acct($pA->id, $mya->id)->setRel('customer code', TRUE, $customer); // mark old member as a customer of pA (NOT autopay)
  } else { // no CG account, so create a stub

    $flags = u\bit(B_NONUDGE); // no way to sign in, so don't bug them
    if ($co) {
      $flags |= u\bit(B_CO) | u\bit(B_DEPENDS);
      $contact = $fullName;
      $fullName = $orgName;
      u\preRay(compact(ray('contact fullName')), $info);
    }

    $acctInfo = just('fullName contact phone email address city state zip postalAddr source', $info);

    $legalName = '*';
    $tenure = $years * 12;
    if (@$referrer) $source = @$source . ': ' . $referrer;
    $idVerifiedBy = $pA->fullName;
    $helper = $pA->id;
    u\preRay(compact(ray('legalName tenure source idVerifiedBy helper flags')), $acctInfo);
   
    $mya = new r\Acct($acctInfo);
    u\EXPECT($mya, 'cannot create account');
    if (isset($owns)) $mya->setRisk('rents', !$owns);
    $sfData['cg_account'] = $info['cgAccount'] = $mya->mainQid . '*'; // asterisk means account is tentative

    if ($co) {
      $mya->setTask('partnerco');
    } else {
      $mya->setTask('partner');
      $mya->stepDone('signup');
      $mya->stepDone('verifyid');
    }
    $mya->stepDone('contact');

  }

  $mya->update('partnerInfo', $info); // store for final step
  emailContract($pA, $info, FALSE);
  sendToCRM($pA, $sfData);
  tellPartner($pA, $info, $flnms);

  $title = t('Sign the Contract');
  $subtext = t('<p>You have completed the first step of your solar subscription sign-up.</p><p>We emailed you a contract. Please click the button in that email within 3 days, to sign the contract.</p><p>If you do not see the email, look for it in your Spam folder.</p><p>You can also preview the standard contract <%a>here</a>.</p><p>If you have questions, please call us at %phone!', '_a phone', atag("/rcredits/images/contracts/$pA->name.pdf"), u\fmtPhone($pA->phone, '-'));
  
  return cgform(compact(ray('title subtext')));
}

/**
 * Rename the uploaded files ASAP so PHP doesn't delete them
 * @return an array of new pathnames (NULL if none)
 */
function renameUploads($files, $meter) {
  if ($files) foreach ($files as $f => $one) {
    if (!$one['name']) {if (isDEV) break; else u\FAIL('missing file');}
    extract(just('name type tmp_name error size', $one));
    if ($size > 10 * MAX_ID_SIZE * 1024 * 1024) {
      w\say(t('<p>That file is too big. You will need to send the photos of your electric bill to %partner by postal mail or as attachments to an email (%partnerEmail).</p>', 'partner partnerEmail', $pA->fullName, $pA->email), 'err');
      $flnms = NULL;
      break;
    }
    $flnms[] = $flnm = dirname($tmp_name) . "/$meter-$f." . pathinfo($name)['extension']; // like E12345-page1.png
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
  r\rMail('partner-signup', $pA, $fields, NULL, @$flnms);
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

/*
function setStepsDone($a) {
  if ($a->idVerifiedBy) $a->stepDone('verifyid');
  $a->stepDone('contact');
}
*/

function sendToCRM($pA, $data) {
  global $mya;

  if (test()) return;
  $data['created'] = date('Y-m-d');
  if (isDEV) foreach (ray('created host upfront contractUrl') as $k) unset($data[$k]);

  $res = u\post(SF_URL, $data); // send data to partner's Salesforce (fails on DEV)
  /* NOT YET
  $data['routing'] = $mya->routing;
  $data['bankAccount'] = $mya->bankAccount;
  u\post(CMR_URL, $data); // send data to the partner's CRM program
*/
}

/**
 * Email a contract for Community Solar to the customer.
 * @param bool $sign: <sign the contract>
 */
function emailContract($pA, $info, $sign = FALSE) {
  global $mya;

//  extract(just('fullName address city state zip m_address m_city m_state m_zip', $info));
  $contractFlnm = fillContract($pA->name, $info, [], $sign); // get populated contract

  extract(just('fullName email customer', $info));
  $partnerName = $pA->fullName;
  $partnerReply = [$pA->email => $pA->fullName];
  $subs = ray('fullName customer email partnerName partnerAddress partnerPhone partnerEmail pseudo noFrame', $fullName, $customer, $email, $partnerName, $pA->postalAddr, u\fmtPhone($pA->phone, '-'), $pA->email, $partnerName, TRUE); // white label from partner name (but our email)

  if ($sign) {
    list ($extra, $cgAccount) = $mya->stepIsDone('fund') ? ['', $mya->mainQid]
    : [t('You will need to delete the %PROJECT account ID in your database.'), t('(none)')];
    $processor = $mya->stepIsDone('fund') ? PROJECT : OTHER_ACH;
    r\rMail('partner-report', $partnerReply, $subs + compact(ray('extra processor cgAccount')), NULL, $contractFlnm);
    if ($mya->nextStep() == 'partnerend') { // completing new member signup
      tellStaffNewMember($info + ray('acctType', $mya->co ? CO_CUSTODIAN : CO_PERSONAL)); // report to admin
      $mya->update('pass', $pass = u\randPass());
      $extra = t('<p>Sign in any time to your %PROJECT account at %CG_DOMAIN with your account ID and password:</p><p><b>Account ID:</b> %qid<br><b>Password:</b> %pass</p>', 'qid pass', $mya->mainQid, $pass);
    } else $extra = '';
    r\rMail('partner-end', [$email => $fullName], $subs + compact('extra'), $partnerReply, $contractFlnm);
  } else {
    r\tellAdmin(t('New %partnerName customer: %fullName', compact('partnerName', 'fullName')), $info);
    $code = r\makeDoCode($info);
    r\rMail('partner-contract', [$email => $fullName], $subs + compact('code'), $partnerReply, $contractFlnm);
  }
}

/**
 * Create a PDF of the specified contract, filled with the given data.
 * @param string $name: the name of the contract image (without path or extention) in the contracts folder
 * @param assoc $data: data to use to fill the contract
 * @param array $map: UNUSED an array of coordinates for filling the contract. Each entry is [field, page, x, y, attributes].
 * @param boot $sign: <include signature and date (current date if none given)> "signature" field is fullName in handwriting
 * @return the path to the contract
 * NOTE: use https://smallpdf.com/pdf-to-jpg to convert
 */
function fillContract($name, $data, $map, $sign = FALSE) {
  global $mya;
  $dir = R_ROOT . "/images/contracts/$name/";
  $pages = array_slice(scandir($dir), 2); // ignore . and ..
  
  extract(just('fullName address city state zip email phone m_company m_number m_address m_city m_state m_zip', $data));
  $phone = u\fmtPhone($phone, '-');
  $buyer = "$fullName<br>$address<br>$city, $state, $zip<br>$phone<br>$email<br>";
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
    if ($pi == 0) $pdf->say("$buyer<br><br><br><br><br>$meter", .71, 3.5, 5, 1.75, '11;Times', 'L'); // text, x, y, w, h, sizesEtc, align
    if ($sign and $pi == 9) {
      $pdf->say(u\fmtDate(time(), TRUE), 6.25, 9.25, 5, 1.75, '11;Times', 'L');
      $pdf->setTextColor(0, 0, 128);
      $pdf->say($fullName, .71, 9.25, 5, 1.75, '22;B;DSGabriele', 'L');
    }
  }
  
  $code = $mya ? $mya->mainQid : u\code(FALSE, 10);
  $flnm = $sign ? contractFlnm($m_number) : sys_get_temp_dir() . "/$m_number-contract.pdf";;
  u\deleteFile($flnm);
  $pdf->Output($flnm, 'F');
  return $flnm;
}

function contractFlnm($customer) {return ROOT . "/contracts/$customer.pdf";} // permanent, customer-specific flnm
function contractUrl($customer) {return str_replace(ROOT, BASE_URL, contractFlnm($customer));}
function pA($pid) {return (@$pid and $pA = r\acct($pid) and in($pA->name, SIGNUP_PARTNERS)) ? $pA : FALSE;}
function cgA($qid) {return r\acct(str_replace('*', '', $qid));}