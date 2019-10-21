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

/**
 * Accept and process signup data from a partner organization.
 */
function formPartnerSignup($form, &$sta, $args = '') {
  global $mya;
  signout(TRUE); // current account is irrelevant and could be confusing, so sign out
  
  extract(just('pid', $_POST));
  if (!@$pid or !$pA = r\acct($pid) or !in($pA->name, 'cooppower nyccec')) return partnerErr(t('bad partner'), 'pid');
  
  $partnerName = $pA->fullName;
  svar('partner', $pA->name);
  
  $sfFlds = ray('first_name:first_name,last_name:last_name,orgName:company,m_address:Meter_Street_Address__c,m_city:Meter_City__c,m_state:Meter_State__c,m_zip:Meter_ZIP_Code__c,m_company:Electric_Company__c,m_number:Electric_Account_Number__c,m_amount:Electric_Bill_Monthly_Amount__c,m_person:Electric_Account_Holder__c,referrer:Referrer__c,source:lead_source,m_wholeAddress:Electric_Meter_Address__c,cgAccount:cg_account'); // salesforce fields (mapped from POST fields)
  $keys = array_keys($sfFlds); // keys from form corresponding to keys expected in Salesforce
  $sfKeys = array_values($sfFlds); // keys of data to send to Salesforce

  extract($info = just(join(' ', $keys) . ' type fullName email phone address city state zip years owns pid', $_POST));

  // set Salesforce parameters
  $debug = 1;
  $debugEmail = 'lynn@cooppower.coop';
  $oid = '00DA0000000gGh5';
  $retURL = ''; // return from salesForce -- maybe report start of CG signup to CP
  list ($first_name, $last_name) = u\parseName($fullName);
  $m_wholeAddress = "$m_address\n$m_city, $m_state $m_zip";
  $customer = @$m_number;

  // validate data
  $co = ($type == '1');
  
  foreach ($keys as $k) {
    if (!in($k, 'referrer orgName cgAccount') and empty($$k)) return partnerErr('parameter must not be empty', $k);
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

  foreach ($sfFlds as $k => $v) $sfData[$v] = $$k; // save Salesforce data for later

  // handle Common Good signup
  $postalAddr = "$address, $city, $state $zip";
  foreach (ray('address city state zip') as $k) {$$k = ${"m_$k"}; unset(${"m_$k"});}
  $state = $m_stateId;

  if (r\relation(1, 'main=:main AND :IS_CUSTOMER AND code=:customer', ray('main customer', $pA->id, $customer))) {
    return softErr(t('You are already signed up with %partnerName', compact('partnerName')));
  }
  
  if ($cgAccount) { // person already has an account, according to them (be careful not to trust that), so no need for this form
    global $mya; // set this for emailContract()
    if (!$mya = r\acct($cgAccount) or !$similar = u\similar($mya->fullName, $fullName, 5, .5) or !($mya->phone == u\fmtPhone(@$phone, '+n') or $mya->email == $email)) {
      if ($mya and !$similar) w\say(t('The name on that account is different.'), 'alert');
      return w\softErr(t('That does not appear to be your correct %PROJECT member ID. Use your browser\'s Back button to go back to the previous page and be sure to use the same name, phone number, and email address that you use in your %PROJECT account. If you are not yet a %PROJECT member, leave that field blank!").', '_a', atag('/signin')), 'cgAccount'); // member mistyped their qid (a royal pain in the butt)
    }
    r\acct($pA->id, $mya->id)->setRel('customer code', TRUE, $customer); // mark old member as a customer of pA (NOT autopay)
    tellPartner($pA, $mya, $customer, $_FILES);
    
    sendToCRM($pA, $sfData);
    emailContract($pA, 'oldMember', $info);

    return w\go('settings/partnerend');
//    $signinQid = ($co and !$mya->trialco) ? '' : $cgAccount;
//    return w\go("signin?name=$signinQid", t('Sign in to finish setting up your %PROJECT account for use with %partnerName.', compact('partnerName')));
  }

  $title = t('Source of Funding');
  $subtext = t(<<< X
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

  $processor = radiosFld(t('Payment Processor:'), '', required(0), [PROJECT, t('Arizona Bank & Trust')]);
  
  $data = hidFld(serialize(compact(ray('info sfData _FILES pid'))));
  $submit = t('Submit');
  
  return cgForm(compact(ray('title subtext routingNumber bankAccount midtext processor submit data')));

//  return w\go('settings/partner-fund'); // move stuff from here down into a subroutine in connect
}

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

function formPartnerSignup_submit($form, &$sta) {
  global $mya;
  
  extract(just('routingNumber bankAccount processor data', $sta['input']));
  extract(just('info sfData _FILES pid', unserialize($data)));
  extract(just('years referrer fullName orgName owns co customer', $info));
  $pA = r\acct($pid);
  
  if ($processor == 1) { // AZ B&T
    sendToCRM($pA, $sfData);
    emailContract($pA, 'noCG', $info);
    svar('partnerData', $info); // needed, since there is no CG account
    return w\go('settings/partnerend');
  }
  
  $tenure = $years * 12;
  if (@$referrer) $source = @$source . ': ' . $referrer;
  $autopay = TRUE;
  $idVerifiedBy = $pA->fullName;
  $helper = $pA->id;
  $acctInfo = just('fullName phone email address city state zip postalAddr source tenure idVerifiedBy helper', $info);
 
  if ($co) {
    $flags = u\bit(B_CO) | u\bit(B_DEPENDS);
    $contact = $fullName;
    $fullName = $orgName;
    u\preRay(compact(ray('flags contact fullName')), $acctInfo);
  }

  $mya = new r\Acct($acctInfo);
  u\EXPECT($mya, 'cannot create account');
  if (isset($owns)) $mya->setRisk('rents', !$owns);

  w\setAcct($mya->id);
  $sta['uid'] = $mya->id; // tell Drupal we exist (essential so that messages show up on next page shown)
  
  if ($co) {
    $mya->setTask('partnerco');
  } else {
    $mya->setTask('partner');
    $mya->stepDone('signup');
    $mya->stepDone('verifyid');
  }
  $mya->stepDone('contact');
  $mya->stepDone('fund');

  $reid = $pA->newRelation(ray('other customer code autopay', $mya->id, TRUE, $customer, $autopay));
//  w\say(t('partner welcome', ['partner' => $pA->fullName])); // this should appear on the Agreement page
  tellPartner($pA, $mya, $customer, $_FILES);

  $sfData['cgAccount'] = $info['cgAccount'] = $mya->mainQid; // we waited until now, to be able to report the qid
  sendToCRM($pA, $sfData);
  emailContract($pA, 'newMember', $info);

  return $co ? w\go('signup-co') : w\goNextStep('signup');
}

/**
 * Tell the partner about this signup and forward any attached files.
 * @return error message, if any.
 */
function tellPartner($pA, $a, $customer, $files) {
  if (!$pA->website) r\tellAdmin(t('Partner %partner has no website. Fix it on the Company Info settings page.', 'partner', $pA->fullName)); // partner should always have a website
  
  if ($files) foreach ($files as $f => $one) {
  if (!$one['name']) {if (isDEV) break; else u\FAIL('missing file');}
    extract(just('name type tmp_name error size', $one));
    if ($size > 10 * MAX_ID_SIZE * 1024 * 1024) {
      w\say(t('<p>That file is too big. You will need to send the photos of your electric bill to Coop Power by postal mail or as attachments to an email (info@cooppower.coop).</p>'), 'err');
      $flnms = NULL;
      break;
    }
    $flnms[] = $flnm = dirname($tmp_name) . '/' . u\code(FALSE, 5) . '-' . $name;
    rename($tmp_name, $flnm);
  }
  
  $fields = ray('partnerName accountId name email customer noFrame', $pA->fullName, $a->mainQid, $a->fullName, $a->email, $customer, TRUE);

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

function setStepsDone($a) {
  if ($a->idVerifiedBy) $a->stepDone('verifyid');
  $a->stepDone('contact');
}

function sendToCRM($pA, $data) {
  global $mya;

  $data['created'] = date('Y-m-d');
  if ($mya) $data['cgAccount'] = $mya->mainQid;

  //u\post(SF_URL, $data); // send data to partner's Salesforce
  //u\post(CMR_URL, $data); // send data to the partner's CRM program

  $fileName = LOG_DIR . $pA->name . '.csv'; // temporary log file for staging data to new CRM
  if (!file_exists($fileName)) file_put_contents($fileName, join("\t", array_keys($data)) . "\n", FILE_APPEND);
  file_put_contents($fileName, join("\t", array_values($data)) . "\n", FILE_APPEND);
}

/**
 * Email a contract for Community Solar to the customer.
 * @param string $type: oldMember, newMember, noCG, or signed.
 */
function emailContract($pA, $type, $info) {
  global $mya;

//  extract(just('fullName address city state zip m_address m_city m_state m_zip', $info));

  extract(just('fullName email', $info));
  $partnerName = $pA->fullName;
  $partnerReply = [$pA->email, $pA->fullName];
  $subs = ray('fullName partnerName partnerAddress partnerPhone partnerEmail noFrame', $fullName, $partnerName, $pA->postalAddr, u\fmtPhone($pA->phone, '-'), $pA->email, TRUE);
  $contractFlnm = fillContract($pA->name, $info, [], $type == 'signed'); // get populated contract
  $qid = $mya ? $mya->mainQid : '';
  $extraMsg = '';

  if ($type == 'signed') {
    r\rMail('partner-end', [$email => $fullName], $subs, $partnerReply, $contractFlnm);
    return;
  }

  if ($type == 'noCG') {
    r\tellAdmin(t('New nonCG customer of ') . $pA->name);
  } elseif ($type == 'oldMember') {
    r\tellAdmin(t('new customer of %partnerName: %fullName', compact('partnerName', 'fullName')), $info);
  } elseif ($type == 'newMember') {
    tellStaffNewMember($info + ray('acctType', $mya->co ? CO_CUSTODIAN : CO_PERSONAL)); // report to admin

    $mya->update('pass', $pass = u\randPass());

    $extraMsg = t('<p>If you were interrupted while setting up your %PROJECT account, you will also have a chance to complete that process when you click the button.</p><p>You can also sign in directly to your %PROJECT account with your account ID and password:</p><p><b>Account ID:</b> %qid<br><b>Password:</b> %pass</p>', 'qid pass', $mya->mainQid, $pass);
  } else u\FAIL('bad partner customer type');

  $code = r\makeDoCode($info);
  r\rMail('verify-partner', [$email => $fullName], $subs + compact(ray('code extraMsg qid')), $partnerReply, $contractFlnm);
}

/**
 * Create a PDF of the specified contract, filled with the given data.
 * @param string $name: the name of the contract image (without path or extention) in the contracts folder
 * @param assoc $data: data to use to fill the contract
 * @param array $map: an array of coordinates for filling the contract. Each entry is [field, page, x, y, attributes].
 * @param boot $sign: <include signature and date (current date if none given)> "signature" field is fullName in handwriting
 * @return a temporary filename with the filled contract
 * NOTE: use https://smallpdf.com/pdf-to-jpg to convert
 */
function fillContract($name, $data, $map, $sign = FALSE) {
  global $mya;
  $dir = R_ROOT . "/images/contracts/$name/";
  $pages = array_slice(scandir($dir), 2); // ignore . and ..
  
  extract(just('fullName address city state zip email phone m_company m_number m_address m_city m_state m_zip date', $data));
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
      $pdf->say(u\fmtDate($date, TRUE), 6.25, 9.25, 5, 1.75, '11;Times', 'L');
      $pdf->setTextColor(0, 0, 128);
      $pdf->say($fullName, .71, 9.25, 5, 1.75, '22;B;DSGabriele', 'L');
    }
  }
  
  $code = $mya ? $mya->mainQid : u\code(FALSE, 10);
  $flnm = sys_get_temp_dir() . "/contract-$code.pdf";
  $pdf->Output($flnm, 'F');
  return $flnm;
}
