<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/*
  Give a nonmember a chance to pay a member by credit card.
*/
function formCCPay($form, &$sta, $args = NULL) {
  if (!$args) $args = u\_GET(); // can't use u\setDft here
  $args = $args ? urlq2ray($args) : $_POST;
  $mya = w\eLinkAcct($sta, $args, 'any');

  jsx('cc');
  $op = nni($args, 'op');

  if ($op == 'done') return completeCCPayment($args);
  if ($op == 'cancel') return softErr(tr('no paypal cancel'));

  $cgPayLink = '/' . str_replace('ccpay/', 'cgpay/', $_SERVER['REQUEST_URI']) . '?' . $_SERVER['QUERY_STRING'];
  if ($mya) return go($cgPayLink);

  if ($code = nni($args, 'code')) {
    if (!$ray = getCGPayArgs($args)) return; // webmaster error message already shown
    if (!nni($ray, 'coA')) return $ray; // softErr
    extract(just('coA amount item s_amount', $ray, NULL));
  } else return softErr(t('That is not a valid payment link.'));

  $logo = logoFld($coA);
  $title = t('Pay ') . $coA->bestName;
  $subtext = '';
  $payHow = hidFld(1); // always by CC
  $form = payFlds(t('Pay'), $mya, $coA, $item, $amount, $s_amount, compact(ray('logo title subtext payHow cgPayLink')));
  if (!nni($form, 'amount')) return $form; // error
  
  $submit = submit(t('Pay'), '', '', suffix(lnk('', t('Add comment'))));
  
  return cgForm($form + compact('submit'));
}

function formCCPay_validate($form, &$sta) {
  $mya = w\eLinkAcct($sta); 
  extract(just($flds = 'amount fullName phone email zip country coId', $sta['input'], NULL));

  if ($mya and $coId == CGID and array_key_exists('amtChoice', $sta['input'])) return w\checkAmtChoice($sta);

  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount'); // fix amount
  if (!$mya) {
    if ($err = u\badName($fullName)) return err($err, ['field' => 'fullName'], 'fullName');
    $fullName = u\normalizeCase($fullName);
    
    if ($err = u\badPhone($phone)) return err($err, 'phone');
    if (!emailOkay($email, TRUE)) return; // allow duplicate emails so a member can donate with CC
    if ($err = u\badZip($zip, $country)) return err($err, 'zip');
    if (!checkBankAccount($sta)) return;
    if (badCaptcha($sta)) return;
  }

  u\preray(compact(ray($flds)), $sta['input']);
}

function formCCPay_submit($form, &$sta) {
  return submitNonMemberPayment($sta, 'purchase');
}

/**
 * Submit non-member payment from formCCPay_submit() and formDonate_submit().
 * @param assoc $sta: the status hash from the form submit function
 * @param string $type: fbo, gift, cggift, or purchase
 * @param assoc $reportInfo: extra information to report to admin
 */
function submitNonMemberPayment($sta, $type, $reportInfo = []) {
  $dbFields = 'fullName email notices phone zip country city state notes source';
  extract(just($inputFlds = $dbFields . ' item amount period recursId honor honored payHow routingNumber bankAccount isSavings comment coverFSFee coverCCFee coId fsFee', $sta['input'], NULL));
  
  $coA = r\acct($coId);
  list ($city, $state) = r\zipCity($zip);
  $notices = NOTICE_NONE;
  $notes = $comment ? r\stampNote($comment) : '';

  $source = t('Payment to %nm (by %how)', 'nm how', $coA->bestName, $payHow ? t('cc') : t('ach'));
  $reportInfo += compact(ray($inputFlds));

  $info = []; foreach (ray($dbFields) as $k) if (isset($$k)) $info[$k] = $$k;
  
  $DBTX = \db_transaction();
  $pid = db\insert('people', $info, 'pid');
  
  if ($payHow == 0) { // bank transfer (the default for cggift and fbo, else not allowed)
    u\EXPECT(in($type, 'cggift fbo'), 'wrong type for ACH donation');
    $bankAccount = u\cry('V', "USkk$routingNumber$bankAccount");
    $honorId = NULL; //$honored ? db\insert('r_honors', $honored = ray('xid honor honored tnm', 0, $honor, $honored)) : NULL;
    addToTxs2($amount, t('donation'), $coA, $period, $pid, $honorId, TRUE, $bankAccount, $isSavings);
    r\tellAdmin($source, $reportInfo);
    unset($DBTX);
    return be\thanks($type, FALSE, $pid, $item, $amount, $period, 1, $coA->id, TRUE);
 
  } else { // credit card donation or purchase
    $code = u\cryRay(compact(ray('type item pid period')) + ray('amount coId', number_format($amount, 2, '.', ''), $coA->mainQid)); // info for thanks, etc. (standard format for amount helps tests, but omit commas so it's still a number)
    $what = $type == 'purchase' ? t('your purchase of %item from %who', 'item who', $item, $coA->bestName)
    : ($coA->id == CGID ? t('general support') : $coA->bestName);
    return paypal($type != 'purchase', $what, $reportInfo, $code);
    // commit $DBTX when function ends
  }
}

/**
 * Post the charge request to PayPal, with this page as the callback (with parameters).
 * @param bool $donating: the payer is making a donation to a member company
 * @param string $what: description of item or service
 * @param assoc $reportInfo: a collection of parameters to report to an admin
 * @param string $code: an encrypted collection of parameters to return from PayPal
 * @return (does not return)
 */
function paypal($donating, $what, $reportInfo, $code) {
  global $testReturnUrl;
  
  extract(just('amount fullName email zip source', $reportInfo));
  list ($cmd, $paypalUri, $returnTo) = ray($donating? '_donations donate community/donate' : '_xclick cgi-bin/webscr ccpay');
  $return = $testReturnUrl = BASE_URL . "/$returnTo/op=done&code=$code";
  u\EXPECT(strlen($return) <= 1024, 'cc return url too long');
  $cancel_return = str_replace('=done', '=cancel', $return);

  list ($first_name, $last_name) = u\parseName($fullName);
  $no_shipping = 1;
  $rm = 0; // 0 for GET, 2 for POST
  $business = 'info@CommonGood.earth'; // CGF_EMAIL
  $item_name = t('for %what', compact('what'));
  $data = compact(ray('cmd business amount first_name last_name email zip item_name return cancel_return no_shipping rm'));
//  if ($permo) $data += ray('a3 t3 p3 src', $amount, 'M', 1, 1); // t3=DWMY, p3=periods, src=1 for recur
//  if ($permo) unset($data['amount']);
  r\tellAdmin($source, $reportInfo + compact('return')); // include clickable return parameter in case the donation goes through but PayPal doesn't return
  return u\postnGo("https://www.paypal.com/$paypalUri", $data); // does not return (until redirected from PayPal)
}

/**
 * Complete the payment accounting, upon return from PayPal.
 * @
 */
function completeCCPayment($args) {
  extract(just('code', $args, NULL));

  if (!$code or !$ray = u\decryRay($code)) return softErr(t('That code is flawed.'));
  
  extract(just('type pid item amount period coId honorId', $ray, NULL));
  if (!($type and $pid and $amount and $coId)) return softErr(t('That code is missing a parameter.'));
  $coA = r\acct($coId);
    
  $fullName = db\get('fullName', 'people', compact('pid'));
  
  $DBTX = \db_transaction();
  $xid = addToTxs2($amount, $item, $coA, $period, $pid, $honorId, $type != 'purchase');
  be\txEntryPair($txes, round($amount * CC_FEE / 100, 2), [$coA->id, CGID], tr('cc fee'), NULL, E_USD_FEE);
  be\insertTxes($txes, $xid);
  unset($DBTX);
  
  r\tellAdmin(t('CC payment complete (to %coName)', 'coName', $coA->bestName), compact(ray('fullName amount period')));
  
  return be\thanks($type, FALSE, $pid, $item, $amount, $period, 1, $coId, TRUE);
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
 * Add an "outer" record to txs2 for the credit card payment.
 * @param string $bankAccount: the bankAccount, V-encrypted
 * @return the outer transaction record ID (xid)
 */
function addToTxs2($amount, $desc, $coA, $period, $pid, $honorId = NULL, $gift = FALSE, $bankAccount = NULL, $isSavings = NULL) {
  $recursId = NULL;
  if ($period != PER_1) {
    if ($bankAccount) { // allow recurring payment only by ACH (not CC)
      $recursId = db\insert('tx_timed', ray('action from to payerType payer amount period purpose duration', ACT_PAY, MATCH_PAYER, $coA->id, REF_PERSON, $pid, $amount, $period, $desc, PER_1));
    } else w\say('no cc recur');
  }
  list ($purpose, $explicit, $cat, $completed, $gift) = [$desc, TRUE, BCAT_GIFT, now(), TRUE];
  $deposit = $bankAccount ? 0 : now();
  $info = compact(ray('purpose pid completed deposit explicit cat bankAccount isSavings recursId gift'));
  $xid = be\createUsdTransfer($coA, $amount, $info);
  if ($id = $honorId) db\update('r_honors', compact(ray('id xid')), 'id');
  return $xid;
}

/**
 * Return most of the form fields for a CC or ACH payment.
 * @param acct $a: payer account, if any (not $mya, so this can be called from a no-signin link)
 * @param assoc $extra: additional fields to include
 */
function payFlds($amtLabel, $a, $coA, $item, $amount, $amtDft, $extra = []) {

  $comment = areaFld(t('Comment:'));
  list ($cq, $ca) = $a ? [NULL, NULL] : w\captcha();
  $coId = hidFld($coA->id);
  u\setDft($period, hidFld(PER_1));

  if ($amount) {
    $amount = preg_replace('/[\\$\\,]/', '', $amount); // no $ or ,
    $amtDpy = item(t('%amt for %item', 'amt item', u\fmtAmt($amount), $item), t('Amount:'));
    $amount = hidFld($amount);
  } else {
    $amount = numFld(REQ . "$amtLabel:", [t('Amount:')], dft($amtDft) + ($item ? suffix(t('For: ') . $item) : []));
    $amtDpy = NULL;
  }
  $item = $item ? hidFld($item) : textFld(REQ . t('For:'), [t('What your payment is for')]);

  if (!$a) {
    $fullName = textFld(REQ . t('Name:'), [t('Your full name'), t('Your full name, properly capitalized')]);
    $phone = phoneFld(REQ . t('Phone:'), [t('Your phone number')]);
    $email = emailFld(REQ . t('Email:'), [t('Email')]);
    $country = selectFld(t('Country:'), t(''), dft(US_COUNTRY_ID), r\countryList()); // can't use required() here
    $zip = zipFld(nn($zip));
  }
  extract($extra);
  
  // Stuff to do AFTER extracting $extra
  if (!$a) $subtext .= t('<p>%PROJECT members save time: <%aMember>go here instead</a>.</p>', '_aMember', atag($cgPayLink));  
  
  foreach (ray('logo title subtext amtChoice amount amtDpy item options period honor coverFSFee fullName phone email country zip payHow coverCCFee ach comment cq ca coId fsFee recursId for') as $k) if (isset($$k)) $form[$k] = $$k;
  return $form;
}
