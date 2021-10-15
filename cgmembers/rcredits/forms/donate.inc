<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/*
  Give the member or nonmember a chance to donate to CG or a sponsored (CG FBO) account.
*/
function formDonate($form, &$sta, $args = NULL) {
  if (!$args) $args = u\_GET(); // can't use u\setDft here
  $args = $args ? urlq2ray($args) : $_POST;
  $mya = w\eLinkAcct($sta, $args, 'any');
  
  $op = nni($args, 'op');

  if ($op == 'done') return completeCCGift($args);
  if ($op == 'cancel') return softErr('We apologize. Our donation form does not yet handle cancelations from PayPal. Use your browser\'s "Back" button, to return to the form with your data filled in.');

  if ($code = nni($args, 'code')) { // donating to a sponsored org
    if (!$ray = getCGPayArgs($args)) return; // error message already shown
    extract(just('coA s_amount', $ray));
  } else $coA = r\acct(CGID); // vanilla CG donation page
  
  jsx('donate');
  $oftens = array_reverse(r\recurRay(TRUE));
  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of');
  $recursId = $amtChoice = NULL;
  $isCG = ($coA->id == CGID);
  $fbo = strhas($coA->fullName, t('%PROJECT FBO '));
  $options = btnOpt('', t('in honor or memory'), 'btn-honor');
  
  if ($isCG and $mya) {
    if ($recurs = db\get('id,period,amount', 'tx_timed tm', 'action=:ACT_PAY AND tm.from=:payer AND tm.to=:CGID AND end IS NULL', ray('payer', $mya->id))) {
      extract($recurs);
      $recursId = hidFld($id);
      $replaces = t('donation replaces', 'period amt', $oftens[$period], u\fmtAmt($amount));
    } else $recursId = NULL;
    
    if (!$mya->activated or !$recurs) {
      unset($oftens[PER_1]); // no one-time donation unless the account is activated and has a recurring donation
      unset($oftens[0]);
      $recurOnly = t(': monthly, quarterly, or yearly');
      $noOneTime = t('<p><small><%aWhy>Why no option for a one-time donation?</a></small></p>', '_aWhy', atag('/help/why-no-one-time-option'));
    } else $recurOnly = $noOneTime = '';

    $amountTip = $mya->co ? '' : t('The most common amounts chosen are $50 and $25.');
    $subtext = t('<p>Make a tax-deductible gift of ZERO OR MORE%recurOnly. %replaces %amountTip</p><p>To donate an amount not listed (including ZERO), choose "Water" (and fill in the amount).</p><p><b class="loud">NOTE: This is a donation, not a deposit.</b> Thank you for your support!</p>', 'recurOnly replaces amountTip', $recurOnly, nn($replaces), $amountTip) . $noOneTime;
    $amtChoice = selectFld(REQ . t('Donation:'), '', dft($mya->co ? COMPANY_GIFT : INDIVIDUAL_GIFT), GIFT_LEVELS);
    jsx('amtChoice');

  } else { // sponsored or not signed in
    $options = btnOpt('', t('repeat'), 'btn-repeat') . $options;
    $subtext = t('<p>Make a tax-deductible contribution of any amount to support %coName. For donations of stock, <%aStock>contact us</a>.</p>', '_aStock coName', atag($mya ? '/help' : r\promoLink('about-us/contact-us')), $isCG ? PROJECT : ($coA->bestName . (strhas($coA->bestName, PROJECT) ? '' : t(' through ') . PROJECT)));
  }

  $options = item($options);
  if (!$mya) $subtext .= t('<p>%PROJECT members <%aMember>sign in here</a> first.</p>', '_aMember', atag(first('/signin', '/donate?' . $_SERVER['QUERY_STRING'])));
    
  $logo = logoFld($coA);
  $title = t('Donate to ') . $coA->bestName;

  $amount = numFld(REQ . t('Donation:'), [t('Amount')], dft(nn($s_amount)));
  $period = selectFld(REQ . t('When:'), t('If you choose a recurring donation, you can stop it at any time.'), dft($isCG ? PER_YEAR : PER_1), $oftens);
  $honored = rendA('honored', textFld(BARE, [t('(optional)')]));
  $honor = selectFld(t('Honoring:'), '', suffix($honored), $honors);

  $fsFee = db\get('`portion`', '(SELECT * FROM tx_rules WHERE end IS NULL) x', ray('payee action amount to', $coA->id, ACT_SURTX, 0, CGID)) ?: .05;
  $coverFSFee = $fbo ? boxFld('coverFSFee', NBSP, t('Check here to cover the %fee% fiscal sponsorship fee also.', 'fee', u\fmtAmt($fsFee * 100, 's')), 0) : hidFld(0);
  
  if (!$mya) {
    $fullName = textFld(REQ . t('Name:'), [t('Your full name'), t('Your full name, properly capitalized')]);
    $phone = phoneFld(REQ . t('Phone:'), [t('Your phone number')]);
    $email = emailFld(REQ . t('Email:'), [t('Email')]);
    $country = selectFld(t('Country:'), t(''), dft(US_COUNTRY_ID), r\countryList()); // can't use required() here
    $zip = zipFld(nn($zip));
    
    $payHow = radiosFld(REQ . t('Pay By:'), '', dft(-1), [t('ACH (easier and saves us 3% in CC fees)'), t('Credit Card <span>(cannot automatically repeat)</span>')]);
    $coverCCFee = boxFld('coverCCFee', NBSP, t('Check here to cover the credit card processing fees also.'), 0);
    
    $checkHint = item(tr('<img src="%src" />', 'src', BASE_URL . '/rcredits/images/check-hint.png'));
    $routingNumber = routingFld(FALSE);
    $bankAccount = intFld(t('Account:'), t('Type <b class="loud">carefully</b> your account number. <br>'), maxlen(17));
    $isSavings = radiosFld(t('Type:'), '', dft(-1), [t('Checking'), t('Savings')]);
    $ach = fieldSet('ach', compact(ray('checkHint routingNumber bankAccount isSavings')), BARE);
  } else $fullName = $phone = $email = $country = $zip = $payHow = $coverCCFee = $ach = NULL;
  
  $comment = areaFld(t('Comment:'));
  list ($cq, $ca) = $mya ? [NULL, NULL] : w\captcha();
  $coId = hidFld($coA->id);
  $fsFee = hidFld($fsFee);
  $noteSuffix = suffix(lnk('', t('Add comment')));

  $form = compact(ray('logo title subtext amtChoice amount options period honor coverFSFee fullName phone email country zip payHow coverCCFee ach comment cq ca coId fsFee recursId'));
  $form += ($mya and $isCG) ? w\setupFoot(t('Donate'), $noteSuffix) : ray('submit', submit(t('Donate'), '', '', $noteSuffix));
  return cgForm($form);
}

function formDonate_validate($form, &$sta) {
  $mya = w\eLinkAcct($sta); 
  extract(just($flds = 'amount fullName phone email zip country coId', $sta['input'], NULL));
  
  if ($coId == CGID) return w\checkAmtChoice($sta);

  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount'); // fix amount
  if (!$mya) {
    if ($err = u\badName($fullName)) return err($err, ['field' => 'fullName'], 'fullName');
    $fullName = u\normalizeCase($fullName);
    
    if ($err = u\badPhone($phone)) return err($err, 'phone');
    if (!emailOkay($email, FALSE)) return;
    if ($err = u\badZip($zip, $country)) return err($err, 'zip');
    if (!checkBankAccount($sta)) return;
    if (badCaptcha($sta)) return;
  }

  u\preray(compact(ray($flds)), $sta['input']);
}

function formDonate_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta); 
  global $testReturnUrl;

  if ($mya) return submitCG($sta);
  
  $dbFields = 'fullName email notices phone zip country city state notes source';
  extract(just($input = $dbFields . ' amount period recursId honor honored payHow routingNumber bankAccount isSavings comment coverFSFee coverCCFee coId fsFee', $sta['input'], NULL));
  
  $coA = r\acct($coId);
  list ($city, $state) = r\zipCity($zip);
  $feeCovered = $amount * (($coverFSFee ? $fsFee : 0) + (($payHow > 0 and $coverCCFee) ? .03 : 0));

  $amount = round($amount + $feeCovered, 2);
  $notices = NOTICE_NONE;

  $comment = u\honorDesc($honor) . u\unite($honored, $comment, '/');
  $notes = $comment ? r\stampNote($comment) : '';

  $source = t('Donation FBO %nm (by %how)', 'nm how', $coA->bestName, $payHow ? t('cc') : t('ach'));
  $reportInfo = compact(ray($input . ' feeCovered')); // include all fields in what we tell admin

  $info = []; foreach (ray($dbFields) as $k) if (isset($$k)) $info[$k] = $$k;
  
  $DBTX = \db_transaction();
  $pid = db\insert('people', $info, 'pid');
  
  $honorId = NULL; //$honored ? db\insert('r_honors', $honored = ray('xid honor honored tnm', 0, $honor, $honored)) : NULL;
  
  if ($payHow == 0) { // bank transfer (the default)
    $bankAccount = "USkk$routingNumber$bankAccount";
    addToTxs2($amount, $coA, $period, $pid, $honorId, $bankAccount, $isSavings);
    r\tellAdmin($source, $reportInfo);
    unset($DBTX);
    return be\thanks(FALSE, $pid, $amount, $period, 1, $coA->id, TRUE);
 
  } else { // credit card donation
    $code = u\cryRay(ray('pid amount period coId', $pid, number_format($amount, 2), $period, $coA->mainQid)); // info for thanks, etc. (standard format for amount helps tests)
    $return = $testReturnUrl = BASE_URL . "/community/donate/op=done&code=$code";
    u\EXPECT(strlen($return) <= 1024, 'cc return url too long');
    $cancel_return = str_replace('=done', '=cancel', $return);

    $cmd = '_donations';
    list ($first_name, $last_name) = u\parseName($fullName);
    $no_shipping = 1;
    $rm = 0; // 0 for GET, 2 for POST
    $business = 'info@CommonGood.earth'; // CGF_EMAIL
    $item_name = t('for %what', 'what', $coA->id == CGID ? t('general support') : $coA->bestName);
    $data = compact(ray('cmd business amount first_name last_name email zip item_name return cancel_return no_shipping rm'));
  //  if ($permo) $data += ray('a3 t3 p3 src', $amount, 'M', 1, 1); // t3=DWMY, p3=periods, src=1 for recur
  //  if ($permo) unset($data['amount']);
    $url = 'https://www.paypal.com/donate';
    r\tellAdmin($source, $reportInfo + compact('return')); // include clickable return parameter in case the donation goes through but PayPal doesn't return
    unset($DBTX);
    return u\postnGo($url, $data); // does not return (until redirected from PayPal)
  }
}

/**
 * Complete the CC donation accounting, upon return from PayPal.
 */
function completeCCGift($args) {
  extract(just('code', $args, NULL));

  if (!$code or !$ray = u\decryRay($code)) return softErr(t('That code is flawed.'));
  
  extract(just('pid amount period coId honorId', $ray, NULL));
  if (!($pid and $amount and $period and $coId)) return softErr(t('That code is missing a parameter.'));
  $coA = r\acct($coId);
    
  $fullName = db\get('fullName', 'people', compact('pid'));
  
  $DBTX = \db_transaction();
  $xid = addToTxs2($amount, $coA, $period, $pid, $honorId);
  be\txEntryPair($txes, round($amount * FBO_CC_FEE / 100, 2), [$coA->id, CGID], tr('fbo cc fee'), NULL, E_USD_FEE);
  be\insertTxes($txes, $xid);
  unset($DBTX);
  
  r\tellAdmin(t('CC donation complete (FBO %coName)', 'coName', $coA->bestName), compact(ray('fullName amount period')));
  
  return be\thanks(FALSE, $pid, $amount, $period, 1, $coId, TRUE);
}


/**
 * Return the parameters in a CGPay link
 */
function getCGPayArgs($args) {
  global $testCode;

  foreach ($args as $k => $v) if (!in($k, 'q code')) {
    if (strlen($v) > CGPAY_MAXLEN) return exitErr(tr('field too long', 'field max', $k, CGPAY_MAXLEN));
    if (!in($k, CGPAY_VARS2)) return exitErr(t('"%k" is not a parameter. Check your spelling?', compact('k')));
  }

  if ($err = missingArgs($args, 'code')) return exitErr($err);
  $code = $args['code'];
  if (test() and $code == 'TESTCODE') $code = $testCode;
  if (!$ray = u\decryRay($code)) return exitErr(t('The "code" parameter is not valid.'));
  if ($dups = just($args, $ray)) return exitErr(t('Parameter "%k" attempts to override that parameter as encoded in "code".', 'k', $dups[0]));

  foreach (ray(CGPAY_VARS) as $k) u\setDft($ray[$k], NULL); // make sure all possible parameters get extracted from return, even if not set
  foreach (ray(CGPAY_VARS2) as $k) $ray["s_$k"] = nni($args, $k, NULL); // optional "suggested" parameters (amount, id, item, name) except for request
  extract(just('expires account s_amount', $ray));
  if ($expires and $expires < now()) return softErr(tr('button expired'));
  $ray['coA'] = $coA = r\acct($account);
  u\EXPECT($coA, 'missing account in code!');
  if (isset($s_amount) and $err = u\badAmount($s_amount, '>0')) return exitErr(t('Parameter "amount" (suggested amount): %err', compact('err')));
  return $ray;
}

/**
 * Add an "outer" record to txs2 for the donation.
 * @return the outer transaction record ID (xid)
 */
function addToTxs2($amount, $coA, $period, $pid, $honorId = NULL, $bankAccount = NULL, $isSavings = NULL) {
  $recursId = NULL;
  if ($period != PER_1) {
    if ($bankAccount) { // allow recurring donation only by ACH (not CC)
      $recursId = db\insert('tx_timed', ray('action from to payerType payer amount period purpose duration', ACT_PAY, MATCH_PAYER, $coA->id, REF_PERSON, $pid, $amount, $period, t('donation'), PER_1));
    } else w\say('no cc recur');
  }
  list ($purpose, $explicit, $cat, $completed, $gift) = [t('donation'), TRUE, BCAT_GIFT, now(), TRUE];
  $deposit = $bankAccount ? 0 : now();
  $info = compact(ray('purpose pid completed deposit explicit cat bankAccount isSavings recursId'));
  $xid = be\createUsdTransfer($coA, $amount, $info);
  if ($id = $honorId) db\update('r_honors', compact(ray('id xid')), 'id');
  return $xid;
}

/**
 * Handle submission of donation to CG itself.
 */
function submitCG($sta) {
  $mya = w\eLinkAcct($sta); 

  extract(just('amount period honor honored roundup recursId coId comment', $sta['input']));
  $info = ray('payer payee amount period created lastTx invoiced', $mya->id, $coId, $amount, $period, now(), 0, 0);
  if ($period != PER_1 and nn($recursId)) {
    $was = (object) db\get('amount, period, periods', 'tx_timed', ray('id', $recursId));
    $was = u\fmtAmt($was->amount) . ' ' . r\recurDesc($was->period, $was->periods);
    r\endRule('tm', $recursId);
    say('prev gift canned');
  } else $was = t('no previous recurring donation');

  if ($amount > 0) {
    if ($honored) {
      if ($coId == CGID) {
        db\insert('r_honors', $honored = ray('uid honor honored', $mya->id, $honor, $honored));
      } else $comment = u\honorDesc($honor) . u\unite($honored, $comment, '/');
    }      
    if ($notes = $comment ? r\stampNote($comment) : '') $mya->update($notes . $mya->notes);

    $purpose = t('donation');
    $recursId = $period == PER_1 ? NULL
    : db\insert('tx_timed', ray('action from to amount period purpose duration', ACT_PAY, $mya->id, $coId, $amount, $period, $purpose, DUR_1));
    $data = ray('gift', TRUE);
    if ($mya->ok and $z = $mya->payApproved($coId, $amount, $purpose, FOR_NONGOODS, $data, $recursId) and nni($z->subs, 'xid')) {
      $mya->tellOut('gift sent', ray('amount', u\fmtAmt($amount)));
    } else say('gift transfer later');

    $nm = $mya->fullName;
    if ($mya->ok) r\tellCAdmin('gift', $info + compact(ray('nm was honored')));
    be\thanks(TRUE, $mya->id, $amount, $period, 1, $coId, !$mya->stepCount());
  }
  
  return r\acct() ? w\goNextStep('donate', '') : w\doSay();
}
