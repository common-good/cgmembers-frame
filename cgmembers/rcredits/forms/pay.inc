<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/*
  Give the member or nonmember a chance to donate to CG or a sponsored (CG FBO) account or a ccOk account.
  @params: donation form sometimes redirects to signin, which passes amount, period, honor, and honored here in $_GET
*/
function formPay($form, &$sta, $args = NULL) {
  if (!$args) $args = u\_GET(); // can't use u\setDft here
  $args = $args ? urlq2ray($args) : $_POST;
  $mya = w\eLinkAcct($sta, $args, 'any');
  
  if (nni($args, 'op') == 'thermometer') return thermometer($args);
  $codeFlds = 'coId coA for isGift amount item credit period honor honored return s_amount s_item s_id s_request';

  if ($code = nni($args, 'code')) { // customer clicked a coded button
    if (!$ray = getCGPayArgs($args, $mya) or !$coA = nni($ray, 'coA')) return; // error message already shown
    extract(just($codeFlds, $ray, NULL));
  } else { // vanilla CG donation page
    foreach (ray($codeFlds) as $k) $$k = NULL;
    $coA = r\acct($coId = CGID);
    $for = 'donate'; $isGift = TRUE;
  }

  $button = !empty($code);
  $oftens = array_reverse(r\recurRay(TRUE));
  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of');
  $isCG = ($coId == CGID);
  $fbo = $coA->sponsored;
  $ccOk = ($fbo or $coA->ccOk);
  $target = $coA->target + 0;

  if ($isGift) u\setDft($s_item, t('donation'));
  $therm = therm($isGift ? $target : 0); // set up thermometer, if there's a target
  $logo = logoFld($coA, $therm);
  $title = title($for, $coA);

  if ($isCG and $mya and $isGift) { // member gift to CG
    extract(just('recursId oftens subtext amtChoice', memberGift($mya, $oftens)));
  } else { // other member company accepting payments) or not signed in
    $recursId = $amtChoice = NULL;
    $subtext = $isGift ? subtext($coA, $mya) : NULL;
  }

  [$amount, $amtDpy] = amount($amount, $s_amount, $isGift); 
  [$itemDpy, $item, $forDpy] = itemDpy($for, $item, $s_item, $credit);
  $fsPct = db\get('`portion`', '(SELECT * FROM tx_rules WHERE end IS NULL) x', ray('payee action amount to', $coA->id, ACT_SURTX, 0, CGID)) ?: .05;

  if ($isGift) {
    $coverFSFee = $fbo ? boxFld('coverFSFee', NBSP, tr('cover FS fee', 'fee', u\fmtAmt($fsPct * 100, 's')), 0) : hidFld(0);
    $period = selectFld(REQ . t('When:'), t('If you choose a recurring donation, you can stop it at any time.'), dft(nn($period) ?: ($isCG ? nn($xPeriod, PER_YEAR) : PER_1)), $oftens);
    $honored = rendA('honored', textFld(BARE, [t('whom or what')], dft(nn($honored))));
    $honor = selectFld(t('Honoring:'), '', suffix($honored) + dft(nn($honor)), $honors);
  } else [$coverFSFee, $period, $honor] = [NULL, hidFld(PER_1), NULL]; // no recurring or honoring payments that aren't gifts


  if ($mya) { // member: no need to ask for payment method
    $qid = $pass = $paySet = $nonMember = $stay = $stayLabel = NULL;
  } else {
    [$qid, $pass] = w\signinFlds($s_id);
    [$paySet, $nonMember, $stay, $stayLabel] = nonMember($isGift, $s_id);
  }
  
  $note = textFld(t('Note:'), [t('About the payment')]);

  $form = compact(ray('logo title subtext amtChoice amount amtDpy period honor itemDpy item forDpy coverFSFee stayLabel stay qid pass nonMember paySet note'));
  foreach (ray('isGift coId credit for return fsPct recursId button') as $k) $form[$k] = hidfld(nn($$k, NULL));
  
  $noteSuffix = suffix(lnk('', t('Add note'))); // set to [] to disable
  $submitLabel = $isGift ? t('Donate') : t('Pay');
  $submit = submit($submitLabel, '', 'md', $noteSuffix);
  $form += ($mya and $isCG) ? w\setupFoot($submitLabel, $noteSuffix) : compact('submit');

  js('https://js.stripe.com/v3/');
  jsx('pay', ray('stripePublicKey ccPct ccPlus', STRIPE_PUBLIC, CC_PCT, CC_PLUS));

  return cgForm($form);
}

function formPay_validate($form, &$sta) {
  include_once __DIR__ . '/signin.inc';
  $mya = w\eLinkAcct($sta); 
  if (!$mya and !formSignin_validate($form, $sta)) return payRet();

  extract(just($flds = 'qid for forDpy item amount amtChoice fullName phone email zip country coId', $sta['input'], NULL));
  $isCG = ($coId == CGID);

  if (($mya and $isCG and isset($amtChoice) and !w\checkAmtChoice($sta))) return payRet();

  if ($err = u\badAmount($amount, ($isCG and r\acct()) ? '>=0' : '>0')) return  payRet($err, 'amount'); // fix amount
  if (!$mya and !$qid) { // nonmember (not even trying to sign in)
    if ($err = u\badName($fullName)) return payRet($err, ['field' => 'fullName'], 'fullName');
    $fullName = u\normalizeCase($fullName);
    
    if ($err = u\badPhone($phone)) return payRet($err, 'phone');
    if (!emailOkay($email, TRUE)) return payRet(); // allow duplicate emails so a member can donate with CC
    if ($err = u\badZip($zip, $country)) return payRet($err, 'zip');
  }

  if ($for == FOR_GIVE_CREDIT) {
    $forDpy = trim(strhas($forDpy, '@') ? mb_strtolower($forDpy) : mb_strtoupper($forDpy));
    if (!$a = r\acct($forDpy)) return payRet(t('That %PROJECT account does not exist. You can currently give only to another %PROJECT member.'), 'forDpy');
    $item = nn($item) . t(' (to %who)', 'who', $a->fullName);
  }
  
  u\preray(compact(ray($flds)), $sta['input']);
}

function payRet($erMsg = NULL) {
  $url = $_SERVER['REQUEST_URI'];
  if ($erMsg) err($erMsg);
  return go($url);
}

function formPay_submit($form, &$sta) {
  extract(just('isGift uid amount period honor honored roundup recursId coId note credit for forDpy qid item request return button', $sta['input'], NULL));
  if (!$mya = w\eLinkAcct($sta)) $mya = r\acct($uid); // uid is from signin_validate() 
  $isCG = ($coId == CGID);

  $info = ray('payer payee amount period created lastTx invoiced', $mya->id, $coId, $amount, $period, now(), 0, 0);

  if ($period != PER_1 and nn($recursId)) {
    $was = (object) db\get('amount, period, periods', 'tx_timed', ray('id', $recursId));
    $was = u\fmtAmt($was->amount) . ' ' . r\recurDesc($was->period, $was->periods);
    r\endRule('tm', $recursId);
    say('prev gift canned');
  } else $was = t('no previous recurring donation');

  if ($amount == 0) { // must cme after if($period...)
    u\EXPECT($isCG and r\acct(), 'zero tx not to CG signed in');
    return w\goNextStep('donate', ''); // a zero donation is fine
  }
  
  $recursId = $period == PER_1 ? NULL // new recursId
  : db\insert('tx_timed', ray('action from to amount period purpose duration', ACT_PAY, $mya->id, $coId, $amount, $period, $item, DUR_1));

  if ($honored and $isCG) db\insert('r_honors', $honored = ray('uid honor honored', $mya->id, $honor, $honored));
    //else $mya->update('notes', r\stampNote(u\honorDesc($honor) . $honored) . $mya->notes);

  $data = compact(ray('for button'));
  if ($for == FOR_GIVE_CREDIT) $data['creditWhom'] = $forDpy;
  if ($credit) $data += compact('credit');
  if ($isGift) $data['gift'] = TRUE;
  $z = $mya->payApproved($coId, $amount, $item, FOR_NONGOODS, $data, $recursId);
  if ($z->ok) {
    if ($mya->ok and nni($z->subs, 'xid')) {
      $mya->tellOut('gift sent', ray('amount', u\fmtAmt($amount)));
      $nm = $mya->fullName;
      if ($mya->ok) r\tellCAdmin('gift', $info + compact(ray('nm was honored')));
      if ($note) $item .= ' ("' . $note . '")';
      $type = $isGift ? r\ccGiftType($coId) : 'purchase';
      be\thanks($xid, $type, TRUE, $mya->id, $item, $amount, $period, 1, $coId, !$mya->stepCount());
    } else say('gift transfer later');
  }
  
  if ($return) {
    $res = compact(ray('ok msg request'));
    $return = tr($return, $res);
    return u\postnGo($return, $res);
  } else { // show standard CG success / failure page
    if (!r\acct()) return w\doSay("success title|$z->index", $z->subs);
    return $z->ok ? w\goNextStep('donate', '') : softErr($z->index, $z->subs);
  }

}

/**
 * Set up for purchase or donation by a member.
 */
function memberGift($mya, $oftens) {
  if ($recurs = db\get('id AS recursId,period AS xPeriod,amount AS xAmount', 'tx_timed tm', 'action=:ACT_PAY AND tm.from=:payer AND tm.to=:CGID AND end IS NULL', ray('payer', $mya->id))) {
    extract($recurs);
    $replaces = tr('donation replaces', 'period amt', $oftens[$xPeriod], u\fmtAmt($xAmount));
  } else $recursId = NULL;

  if (!$mya->activated or !$recurs) {
    unset($oftens[PER_1]); // no one-time donation unless the account is activated and has a recurring donation
    unset($oftens[0]);
    $recurOnly = t(': monthly, quarterly, or yearly -- think of it as an optional membership fee while we are scaling up');
    $noOneTime = t('<p><small><%aWhy>Why no option for a one-time donation?</a></small></p>', '_aWhy', atag('/help/why-no-one-time-option'));
  } else $recurOnly = $noOneTime = '';

  $amountTip = $mya->co ? '' : t('The most common amounts chosen are $50 and $25.');
  
  $subtext = t('<p>Thank you for partnering with us to create a %PROJECT Economy and thank you for considering a tax-deductible donation.</p><p>Most members choose to donate%recurOnly. %replaces %amountTip</p><p>To donate an amount not listed (including ZERO), choose "Water" (and fill in the amount).</p><p><b class="loud">NOTE: This is a donation, not a deposit.</b> Thank you for your support!</p>', 'name recurOnly replaces amountTip', $mya->bestName, $recurOnly, nn($replaces), $amountTip) . $noOneTime;
  $amtChoice = selectFld(REQ . t('Donation:'), '', dft($mya->co ? COMPANY_GIFT : INDIVIDUAL_GIFT), GIFT_LEVELS);
  jsx('amtChoice');
  return compact(ray('recursId oftens subtext amtChoice'));
}

const IMG_W = 350;
const IMG_H = 220;
const BULB_X0 = 24;
const BULB_Y0 = 5;
const BULB_Y9 = 170;
const BULB_H = BULB_Y9 - BULB_Y0;

const TXT_X = 56; // left edge of text
const FONT_SIZE = 16; // font-size in pixels
const MAX_FONT_SIZE = .85 * FONT_SIZE; // smaller max
const MAX_Y = 1.4 * MAX_FONT_SIZE;
define('MAX_TEXT', 'target: ');
define('VAL_TEXT', ' so far');
  
/**
 * Return a thermometer image.
 */
function thermometer($args) {
  /*
       Fundraising Thermometer Generator v1.1
       Sairam Suresh sai1138@yahoo.com / www.entropyfarm.org (more than 50% modified by CG)
  */
  $thermDir = R_ROOT . '/images/thermometer';
  $font = "$thermDir/Poppins Medium 500.ttf";
  foreach (ray('max value count') as $k) $$k = nni($args, $k) ?: 0; // convert empty string to 0

  $unit = ' $';

  $img = imagecreateTrueColor(IMG_W, IMG_H);

  $white = imagecolorallocate ($img, 255, 255, 255);
  $black = imagecolorallocate ($img, 0, 0, 0);
  $red = imagecolorallocate ($img, 255, 0, 0);
  $blue = imagecolorallocate ($img, 0, 0, 255);

  imagefill($img, 0, 0, $white);
  ImageAlphaBlending($img, true); 

  $thermImage = imagecreatefromjpeg("$thermDir/therm.jpg");
  $tix = ImageSX($thermImage);
  $tiy = ImageSY($thermImage);
  ImageCopy($img, $thermImage, 0, 0, 0, 0, $tix, $tiy);
  Imagedestroy($thermImage);

  $bar = ImageCreateFromjpeg("$thermDir/thermbar.jpg"); // thermbar pic courtesy http://www.rosiehardman.com/
  $barW = ImageSX($bar); 
  $barH = ImageSY($bar); 

  // Draw the filled bar
  $newH = (is_numeric($max) and $max > 0) ? min($max, round(BULB_H * ($value / $max))) : 0;
  $y = BULB_Y9 - $newH;
  imagecopyresampled($img, $bar, BULB_X0, $y, 0, 0, $barW, $newH, $barW, $barH); 
  Imagedestroy($bar);
  $count = t(' (%cnt gifts)', 'cnt', number_format($count));

  if ($y > MAX_Y + MAX_FONT_SIZE/2) imagettftext ($img, MAX_FONT_SIZE, 0, round(TXT_X+4), round(MAX_Y-4), $black, $font, MAX_TEXT . $unit . number_format($max)); // the max
  if ($y <= MAX_Y + FONT_SIZE/2) {
      imagettftext ($img, FONT_SIZE, 0, TXT_X, round(MAX_Y), $blue, $font, $unit . number_format($value) . $count . '!!'); // Current > Max
  } elseif ($value > 0) {
    imagettftext ($img, FONT_SIZE, 0, TXT_X, round($y+FONT_SIZE/2), $blue, $font, $unit. number_format($value) . VAL_TEXT . $count);  // Current < Max
  }

  if ($value > $max) {
      $burstImg = ImageCreateFromjpeg("$thermDir/burst.jpg");
      $burstW = ImageSX($burstImg);
      $burstH = ImageSY($burstImg);
      ImageCopy($img, $burstImg, 0, 0, 0, 0, $burstW, $burstH);
  }

  Header("Content-Type: image/jpeg"); 
  Imagejpeg($img);
  Imagedestroy($img);
  exit();
}

/**
 * Return a thermometer image.
 */
function therm($target) {
  if (!$target) return '';

  $fsFeeCat = r\nick2cat('FS-FEE');
  $where = "(uid2=:uid OR (:uid=:CGID AND uid2 IN :BUCKET_UIDS)) AND cat2<>$fsFeeCat AND :IS_GIFT AND created>=:dt";
  extract(db\get('SUM(1) AS count, SUM(amt) AS value', 'txs_noreverse', $where, ray('uid dt', $coA->id, $coA->targetStart)) ?: ray('count value', 0, 0));
  return img(BASE_URL . "/ccpay/op=thermometer&max=$target&value=$value&count=$count");
}

/**
 * Return the parameters in a CGPay link
 */
function getCGPayArgs($args, $mya) {
  global $testCode;

  foreach ($args as $k => $v) if (!in($k, 'q code')) {
    if (strlen($v) > CGPAY_MAXLEN) return exitErr(tr('field too long', 'field max', $k, CGPAY_MAXLEN));
    if (!in($k, CGPAY_VARS2 . ' _gl')) return exitErr(t('"%k" is not a parameter. Check your spelling?', compact('k'))); // _gl is added by google
  }

  if ($err = missingArgs($args, 'code')) return exitErr($err);
  $code = $args['code'];
  if (test() and $code == 'TESTCODE') $code = $testCode;
  if (!$ray = u\decryRay($code)) return exitErr(t('That is not a valid link for payment or donation.'));
  if ($dups = just($args, $ray)) return exitErr(t('Parameter "%k" attempts to override that parameter as encoded in "code".', 'k', $dups[0]));

  foreach (ray(CGPAY_VARS) as $k) u\setDft($ray[$k], NULL); // make sure all possible parameters get extracted from return, even if not set
  foreach (ray(CGPAY_VARS2) as $k) $ray["s_$k"] = nni($args, $k, NULL); // optional "suggested" parameters (amount, id, item, name) except for request

  u\setDft($ray['for'], FOR_PURCHASE);
  extract(just('expires amount s_amount account for', $ray, NULL));

  if ($expires and $expires < now()) return softErr(tr('button expired'));
  if (!$account or !$coA = r\acct($account)) return exitErr(t('That account does not exist.'));
  $coId = $coA->id;
  if ($mya and $mya->id == $coId) return softErr(t('Making a payment to yourself is not permitted.'));
  if (isset($s_amount) and $err = u\badAmount($s_amount, '>0')) return exitErr(t('Parameter "amount" (suggested amount): %err', compact('err')));
  $isGift = ($for == FOR_DONATE);
  if (in($for, [FOR_CREDIT, FOR_GIVE_CREDIT])) u\setDft($ray['credit'], $s_amount ?: $amount);

  return $ray + compact(ray('isGift coId coA'));
}

function title($for, $coA) {
  $nm = $coA->bestName;
  return ($for == 'donate' ? t('Donate to ') : t('Pay ')) . $nm;
}

function subtext($coA, $mya) {
  $isCG = ($coA->id == CGID);
  $coName = $isCG ? PROJECT : ($coA->bestName . (strhas($coA->bestName, PROJECT) ? '' : t(' through ') . PROJECT));
  $subtext = t('<p>Make a tax-deductible contribution of any amount to support %coName.</p>', compact('coName'));

  if ($isCG or $coA->sponsored) $subtext .= ' ' . t('<p>For donations of stock, <%aStock>contact us</a>.</p>', '_aStock', atag($mya ? '/help' : r\promoLink('about-us/contact-us')));
  return $subtext;
}

function amount($amount, $s_amount, $isGift) {
  $amtLabel = $isGift ? t('Donation:') : t('Pay:');
  $suffix = $isGift ? (NBSP . NBSP . NBSP . btnOpt('', t('repeat'), 'btn-repeat') . btnOpt('', t('in honor or memory'), 'btn-honor')) : '';
  
  if ($amount) {
    $amtDpy = item(u\fmtAmt($amount, ''), $amtLabel, '', suffix($suffix));
    $amount = hidFld($amount);
  } else {
    $amtDpy = NULL;
    $dft = $s_amount ? u\fmtAmt($s_amount, '') : '';
    $amount = numFld(REQ . $amtLabel, [t('Amount')], dft($dft) + suffix($suffix) + clas('short middle'));
  }

  return [$amount, $amtDpy];
}

function itemDpy($for, $item, $s_item, $credit) {
  $forDpy = NULL;
  
  if ($for == FOR_GIVE_CREDIT) {
    $forDpy = textFld(REQ . t('As a Gift to:'), [t('Email or account ID')]);
    $item = t('gift of store credit');
    $itemDpy = item(t('store credit'), t('For:'));
  } elseif ($for == FOR_CREDIT) {
    $item = ($credit ? u\fmtAmt($credit) . ' ' : '') . t('store credit');
    $itemDpy = item($item, t('For:'));
  } elseif ($for == FOR_CREDIT50) {
    $item = t('store credit');
    $itemDpy = item($item, t('For:'));
  } elseif ($for == FOR_DONATE) {
    $for = NULL;
    u\setDft($item, t('donation'));
    $itemDpy = NULL;
  } elseif (!$for or $for == FOR_PURCHASE) { // for is other or empty or nonsense
    $for = FOR_PURCHASE;
    if ($item) {
      $itemDpy = item($item, t('For:'));
    } else {
      $item = textFld(REQ . t('For:'), [t('What to pay for')], dft($s_item));
      $itemDpy = NULL;
    }
  } else return u\FAIL(t('The "for" parameter must be credit, gift, donate, or other -- not "%for"', compact('for')));

  if (!is_array($item)) $item = hidFld($item);
  
  return [$itemDpy, $item, $forDpy];
}

function nonMember($isGift, $s_id) {
  $payment = item('', t('Pay by:'));
  $paymentErr = item('', NBSP);
  $paySet = fieldSet('paySet', compact(ray('payment paymentErr')), BARE);

  $coverCCFee = $isGift ? boxFld('coverCCFee', NBSP, tr('cover cc fee'), 0) : NULL;
  $next = btnFld('', '', t('Next'));
  $fullName = textFld(t('Name:'), [t('Your full name, properly capitalized')]);
  $phone = phoneFld(t('Phone:'), [t('Your phone number')]);
  $email = emailFld(t('Email:'), [t('Email')], dft(strhas($s_id, '@') ? $s_id : ''));
  $country = selectFld(t('Country:'), t(''), dft(US_COUNTRY_ID), r\countryList()); // can't use required() here
  $zip = zipFld(nn($zip), FALSE);
  $nonMemberInfo = compact(ray('fullName phone email country zip coverCCFee next'));
  $nonMember = fieldSet('nonMember', $nonMemberInfo, BARE);
  
  $signinMsg = t('%PROJECT members save time and card fees! ') . buttino('/signup', t('<big>Sign up here</big>')) . qBtn('signup-why');

  $stay = boolFld('', $signinMsg);
  $stayLabel = item('<br>' . t('Are you a %PROJECT member? (yet 🙂)'));
  
  return [$paySet, $nonMember, $stay, $stayLabel];
}
