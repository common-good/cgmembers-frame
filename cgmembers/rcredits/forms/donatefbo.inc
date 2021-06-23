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
  if ($op == 'done') return ccThanks($args);
  if ($op == 'cancel') return softErr('We apologize. Our donation form does not yet handle cancelations from PayPal. Use your browser\'s "Back" button, to return to the form with your data filled in.');
  extract(getCGPayArgs($args)); // co, CGPAY_VARS, and CGPAY_VARS2 (prefixed by s_)
  
  jsx('fbo');
  $oftens = array_reverse(r\recurRay(TRUE));
  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of');
  
  $logo = item($co->photoHtml());
  $title = t('Donate to ') . str_replace(PROJECT . t(' FBO '), '', $co->fullName);
  $subtext = t('<p><small>%PROJECT members <%aMember>go here</a> instead</small></p><p>Make a tax-deductible contribution of any amount to support %coName (for donations of stock, <%aStock>contact us</a>).</p>', '_aMember _aStock coName', atag($memberUrl), atag(r\promoLink('about-us/contact-us')), $co->id == CGID ? PROJECT : ($co->fullName . t(' through ') . PROJECT));
  $amount = numFld(REQ . t('Donation:'), [t('Amount')], dft(nn($s_amount)));
  $options = item(btnOpt('', t('Repeat'), 'btn-repeat') . btnOpt('', t('In honor or memory'), 'btn-honor'));
  $period = selectFld(REQ . t('When:'), t('If you choose a recurring donation, you can stop it at any time.'), dft('once'), $oftens);
  $honored = rendA('honored', textFld(BARE, [t('(optional)')]));
  $honor = selectFld(t('Honoring:'), '', suffix($honored), $honors);
  $coverFSFee = boxFld('coverFSFee', NBSP, t('Check here to cover the 3% fiscal sponsorship fee also.'), 0);
  
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
  $co = hidFld($co->mainQid);

  $submit = submit(t('Donate'), NBSP, '', suffix(lnk('', t('Add comment'))));;

  return cgform(compact(ray('logo title subtext amount options period honor coverFSFee fullName phone email country zip payHow coverCCFee ach comment cq ca submit co')));
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
  $dbFields = 'fullName email flags phone zip country source city state notes';
  extract(just($input = $dbFields . ' amount period honor honored payHow routingNumber bankAccount isSavings comment coverFSFee coverCCFee co', $sta['input'], NULL));

  $co = r\acct($co);
  list ($city, $state) = r\zipCity($zip);
  $source = t('Donation FBO %nm (by %how)', 'nm how', $co->fullName, $payHow ? t('cc') : t('ach'));
  $notes = $comment ? r\stampNote($comment) : '';
  $feeCovered = $amount * (($coverFSFee ? .03 : 0) + (($payHow > 0 and $coverCCFee) ? .03 : 0));

  r\tellAdmin($source, compact(ray($input . ' feeCovered')));
  $amount = round($amount + $feeCovered, 2);

  $flags = u\bit(B_NONUDGE);
  $info = []; foreach (ray($dbFields) as $k) if (isset($$k)) $info[$k] = $$k;
  $a = new r\Acct($info);
  u\EXPECT($a, t('cannot create account'));
  if ($honored) db\insert('r_honors', $honored = ray('uid honor honored', $a->id, $honor, $honored));
  
  if ($payHow == 0) { // bank transfer (the default)
    $bankAccount = "USkk$routingNumber$bankAccount";
    $last4bank = substr($bankAccount, -4, 4);
    $bankOk = FALSE;
    $isSavings = (bool) $isSavings;
    $a->update(compact(ray('bankAccount last4bank bankOk isSavings')));
    $a->stepDone('fund');

    return handleRecurs($a, $amount, $period, $co->id, ACT_PAY);
 
  } else { // credit card donation
    $code = u\cryRay(ray('qid amount period coId', $a->mainQid, $amount, $period, $co->id)); // info for thanks, etc.
    $return = BASE_URL . "/donate-fbo/op=done&code=$code";
    u\EXPECT(strlen($return) <= 1024, 'cc return url too long');
    $cancel_return = str_replace('=done', '=cancel', $return);

    $cmd = '_donations';
    list ($first_name, $last_name) = u\parseName($fullName);
    $no_shipping = 1;
    $rm = 0; // 0 for GET, 2 for POST
    $business = 'info@CommonGood.earth'; // CGF_EMAIL
    $item_name = t('for %what', 'what', $co->id == CGID ? t('general support') : $co->fullName);
    $data = compact(ray('cmd business amount first_name last_name email zip item_name return cancel_return no_shipping rm'));
  //  if ($permo) $data += ray('a3 t3 p3 src', $amount, 'M', 1, 1); // t3=DWMY, p3=periods, src=1 for recur
  //  if ($permo) unset($data['amount']);
    $url = 'https://www.paypal.com/donate';
    return u\postnGo($url, $data); // does not return (until redirected from PayPal)
  }
}

/**
 * Thank the CC donor
 */
function ccThanks($args) {
  extract(just('code', $args, NULL));
  if (!$code or !$ray = u\decryRay($code)) return softErr(t('That code is flawed.'));
  
  extract(just('qid amount period coId', $ray));
  $a = r\acct($qid);
    
  $source = t('cc donation');

  $fullName = $a->fullName;
  r\tellAdmin(t('CC donation complete'), compact(ray('fullName qid amount period')));
  $msg = 'gift thanks';
  $a->update('notes', r\stampNote(t('cc donation: $') . "$amount ($period)") . $a->notes);
  return handleRecurs($a, $amount, $period, $coId, ACT_CHARGE);
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
  $ray['co'] = $co = r\acct($account);
  u\EXPECT($co, 'missing account in code!');
  if (isset($s_amount) and $err = u\badAmount($s_amount, '>0')) return exitErr(t('Parameter "amount" (suggested amount): %err', compact('err')));
  return $ray;
}

/**
 * If donor specified a period other than "once", create the template.
 * @return int: the template's record ID (0 if no recur)
 */
function handleRecurs($a, $amount, $period, $coId, $action) {
  $recursId = $period == '1' ? 0
  : db\insert('tx_templates', ray('action from to amount period purpose duration', $action, $a->id, $coId, $amount, $period, t('donation'), 'once'));
  $data = ray('gift recurs', 1, (bool) $recursId);
  $a->payApproved($coId, $amount, t('donation'), $coId == CGID ? FOR_NONGOODS : FOR_GOODS, $data, $recursId);
  $gift = u\fmtAmt($amount) . ($recursId ? ' ' . r\recurDesc($period) : '');
  $subs = ray('fullName date noFrame gift code', $a->fullName, u\fmtDate(NULL, TRUE), 1, $gift, $a->oneTimePass());
  $co = r\acct($coId);
  foreach (ray('fullName postalAddr') as $k) $subs['co' . ucfirst($k)] = $co->$k;
  $subs['coPhone'] = u\fmtPhone($co->phone);
  r\rMail('fbo-thanks', $a, $subs);
  return sayEmpty('gift thanks');
}
