<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Cron as cr;

/*
  Give the nonmember a chance to donate to a sponsored (CG FBO) account.
*/
function formDonateFBO($form, &$sta, $args = NULL) {
  $memberUrl = '/cgpay?' . $_SERVER['QUERY_STRING'];
  global $mya; if ($mya) return go($memberUrl);
  $args = $args ? urlq2ray($args) : (u\_GET() ?: $_POST);
  $op = nni($args, 'op');
  if ($op == 'done') return completeCCGift($args);
  if ($op == 'cancel') return softErr('We apologize. Our donation form does not yet handle cancelations from PayPal. Use your browser\'s "Back" button, to return to the form with your data filled in.');
  if (!$ray = getCGPayArgs($args)) return; // error message already shown
  extract($ray); // coA, CGPAY_VARS, and CGPAY_VARS2 (prefixed by s_)
  
  jsx('fbo');
  $oftens = array_reverse(r\recurRay(TRUE));
  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of');
  
  $logo = item($coA->logo ? tr('<img src="%logo">', 'logo', $coA->logo) : '');
  $title = t('Donate to ') . $coA->bestName;
  $subtext = t('<p><small>%PROJECT members <%aMember>go here</a> instead</small></p><p>Make a tax-deductible contribution of any amount to support %coName (for donations of stock, <%aStock>contact us</a>).</p>', '_aMember _aStock coName', atag($memberUrl), atag(r\promoLink('about-us/contact-us')), $coA->id == CGID ? PROJECT : ($coA->bestName . t(' through ') . PROJECT));
  $amount = numFld(REQ . t('Donation:'), [t('Amount')], dft(nn($s_amount)));
  $options = item(btnOpt('', t('Repeat'), 'btn-repeat') . btnOpt('', t('In honor or memory'), 'btn-honor'));
  $period = hidFld('once'); // selectFld(REQ . t('When:'), t('If you choose a recurring donation, you can stop it at any time.'), dft('once'), $oftens);
  $honored = rendA('honored', textFld(BARE, [t('(optional)')]));
  $honor = selectFld(t('Honoring:'), '', suffix($honored), $honors);

  $fsFee = db\get('`portion`', '(SELECT * FROM tx_rules WHERE end IS NULL) x', ray('payee action amount to', $coA->id, ACT_SURTX, 0, CGID)) ?: .05;
  $coverFSFee = boxFld('coverFSFee', NBSP, t('Check here to cover the %fee% fiscal sponsorship fee also.', 'fee', u\fmtAmt($fsFee * 100, 's')), 0);
  
  $fullName = textFld(REQ . t('Name:'), [t('Your full name'), t('Your full name, properly capitalized')]);
  $phone = phoneFld(REQ . t('Phone:'), [t('Your phone number')]);
  $email = emailFld(REQ . t('Email:'), [t('Email')]);
  $country = selectFld(t('Country:'), t(''), dft(US_COUNTRY_ID), r\countryList()); // can't use required() here
  $zip = zipFld(nn($zip));
  
  $payHow = radiosFld(REQ . t('Pay By:'), '', dft(-1), [t('ACH (easier and saves us 3% in CC fees)'), t('Credit Card')]);
  $coverCCFee = boxFld('coverCCFee', NBSP, t('Check here to cover the credit card processing fees also.'), 0);
  
  $checkHint = item(tr('<img src="%src" />', 'src', BASE_URL . '/rcredits/images/check-hint.png'));
  $routingNumber = routingFld(FALSE);
  $bankAccount = intFld(t('Account:'), t('Type <b class="loud">carefully</b> your account number. <br>'), maxlen(17));
  $isSavings = radiosFld(t('Type:'), '', dft(-1), [t('Checking'), t('Savings')]);
  $ach = fieldSet('ach', compact(ray('checkHint routingNumber bankAccount isSavings')), BARE);
  $comment = areaFld(t('Comment:'));
  list ($cq, $ca) = w\captcha();
  $co = hidFld($coA->mainQid);
  $fsFee = hidFld($fsFee);

  $submit = submit(t('Donate'), NBSP, '', suffix(lnk('', t('Add comment'))));;

  return cgform(compact(ray('logo title subtext amount options period honor coverFSFee fullName phone email country zip payHow coverCCFee ach comment cq ca submit co fsFee')));
}

function formDonateFBO_validate($form, &$sta) {
  extract(just($flds = 'amount fullName phone email zip country', $sta['input'], NULL));

  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount'); // fix amount
  if ($err = u\badName($fullName)) return err($err, ['field' => 'fullName'], 'fullName');
  $fullName = u\normalizeCase($fullName);
  
  if ($err = u\badPhone($phone)) return err($err, 'phone');
  if (!emailOkay($email, FALSE)) return;
  if ($err = u\badZip($zip, $country)) return err($err, 'zip');
  if (!checkBankAccount($sta)) return;
  if (badCaptcha($sta)) return;

  u\preray(compact(ray($flds)), $sta['input']);
}

function formDonateFBO_submit($form, &$sta) {
  global $testReturnUrl;
  
  $dbFields = 'fullName email notices phone zip country city state notes source';
  extract(just($input = $dbFields . ' amount period honor honored payHow routingNumber bankAccount isSavings comment coverFSFee coverCCFee co fsFee', $sta['input'], NULL));

  $coA = r\acct($co);
  list ($city, $state) = r\zipCity($zip);
  $feeCovered = $amount * (($coverFSFee ? $fsFee : 0) + (($payHow > 0 and $coverCCFee) ? .03 : 0));

  $amount = round($amount + $feeCovered, 2);
  $notices = NOTICE_NONE;

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
    return thanks($pid, $amount, $period, $coA->id);
 
  } else { // credit card donation
    $code = u\cryRay(ray('pid amount period coId', $pid, number_format($amount, 2), $period, $coA->mainQid)); // info for thanks, etc. (standard format for amount helps tests)
    $return = $testReturnUrl = BASE_URL . "/donate-fbo/op=done&code=$code";
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
  
  r\tellAdmin(t('CC donation complete (FBO %coName)'), compact(ray('fullName amount period')) + ['coName' => $coA->bestName]);
  
  return thanks($pid, $amount, $period, $coId);
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

function thanks($pid, $amount, $period, $coId) {
  $coA = r\acct($coId);
  $gift = $subs0 = u\fmtAmt($amount) . ($period == 'once' ? '' : ' ' . r\recurDesc($period));
  $res = db\get('*', 'people', compact('pid'));
  extract(just('fullName email phone city state zip', $res));
  $st = r\realState($state);
  
  $subs0 = $res + ray('date gift', u\fmtDate(NULL, TRUE), $gift);
  $subs = $subs0 + ray('donor donorPhone donorAddress donorEmail', $fullName, u\fmtPhone($phone), "$city, $st $zip", $email);
  unset($subs['fullName']);
  $coA->tellIn('fbo-report', $subs);

  $subs = $subs0 + ray('noFrame coName coPostalAddr coPhone', 1, $coA->bestName, $coA->postalAddr, u\fmtPhone($coA->phone));
  r\rMail('fbo-thanks', [$email => $fullName], $subs);
  
  return sayEmpty('gift thanks');
}

/**
 * Add an "outer" record to txs2 for the donation.
 * @return the outer transaction record ID (xid)
 */
function addToTxs2($amount, $coA, $period, $pid, $honorId = NULL, $bankAccount = NULL, $isSavings = NULL) {
  $recursId = $period == 'once' ? NULL
  : db\insert('tx_timed', ray('action from to payer payerType amount period purpose duration', ACT_PAY, $pid, IUD_OUTER, REF_PERSON, $coA->id, $amount, $period, t('donation'), 'once'));
  $data = ray('gift recurs', 1, (bool) $recursId);
  list ($purpose, $explicit, $cat, $completed) = [t('donation'), TRUE, BCAT_GIFT, now()];
  $deposit = $bankAccount ? 0 : now();
  $info = compact(ray('purpose pid completed deposit explicit cat bankAccount isSavings recursId'));
  $xid = be\createUsdTransfer($coA, $amount, $info);
  if ($id = $honorId) db\update('r_honors', compact(ray('id xid')), 'id');
  return $xid;
}
