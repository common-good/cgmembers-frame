<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once R_ROOT . '/forms/ccpay.inc';

/*
  Give the member or nonmember a chance to donate to CG or a sponsored (CG FBO) account or a ccOk account.
  @params: donation form sometimes redirects to signin, which passes amount, period, honor, and honored here in $_GET
*/
function formDonate($form, &$sta, $args = NULL) {
  if (!$args) $args = u\_GET(); // can't use u\setDft here
  $args = $args ? urlq2ray($args) : $_POST;
  extract(just('amount period honor honored', $args, NULL));
  $mya = w\eLinkAcct($sta, $args, 'any');
  
  switch ($op = nni($args, 'op')) {
    case 'done': return completeCCPayment($args); // in ccpay.inc
    case 'cancel': return softErr(tr('no paypal cancel'));
    case 'thermometer': return therm($args);
    default: // fall through
  }

  if ($code = nni($args, 'code')) { // donating to a sponsored org
    if (!$ray = getCGPayArgs($args)) return; // webmaster error message already shown
    extract(just('coA s_amount', $ray, NULL));
    if (!nn($coA)) return; // softErr
    if ($mya and $mya->id == $coA->id) return softErr(t('Donating to yourself is not permitted.'));
  } else $coA = r\acct(CGID); // vanilla CG donation page

  u\setDft($s_amount, $amount); // get the default one way or the other (or not at all)
  
  js('https://js.stripe.com/v3/');
  jsx('donate', ray('ccRate', CC_FEE / 100));
  
  $oftens = array_reverse(r\recurRay(TRUE));
  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of');
  $recursId = $amtChoice = NULL;
  $isCG = ($coA->id == CGID);
  $fbo = $coA->sponsored;
  $ccOk = ($fbo or $coA->ccOk);
  $target = $coA->target + 0;
  $options = btnOpt('', t('repeat'), 'btn-repeat') . btnOpt('', t('in honor or memory'), 'btn-honor');
// to support our planned merger with <%a>The Truth Telling Project</a>
  $special = ($isCG and $target) ? t('<p>We currently need to raise %amt over the next few months.</p>', '_a amt', atag('https://thetruthtellingproject.org/'), u\fmtAmt($target)) : '';
  
  if ($isCG and $mya) {
    if ($recurs = db\get('id,period AS xPeriod,amount AS xAmount', 'tx_timed tm', 'action=:ACT_PAY AND tm.from=:payer AND tm.to=:CGID AND end IS NULL', ray('payer', $mya->id))) {
      extract($recurs);
      $recursId = hidFld($id);
      $replaces = t('donation replaces', 'period amt', $oftens[$xPeriod], u\fmtAmt($xAmount));
    } else $recursId = NULL;
    
    if (!$mya->activated or !$recurs) {
      unset($oftens[PER_1]); // no one-time donation unless the account is activated and has a recurring donation
      unset($oftens[0]);
      $recurOnly = t(': monthly, quarterly, or yearly');
      $noOneTime = t('<p><small><%aWhy>Why no option for a one-time donation?</a></small></p>', '_aWhy', atag('/help/why-no-one-time-option'));
    } else $recurOnly = $noOneTime = '';

    $amountTip = $mya->co ? '' : t('The most common amounts chosen are $50 and $25.');
    
    $subtext = t('<p>Hello, %name. Thank you for considering partnering with us to create a %PROJECT Economy, by making a tax-deductible donation.</p><p>Most members choose to donate%recurOnly. %replaces %amountTip</p>%special<p>To donate an amount not listed (including ZERO), choose "Water" (and fill in the amount).</p><p><b class="loud">NOTE: This is a donation, not a deposit.</b> Thank you for your support!</p>', 'name recurOnly replaces amountTip _special', $mya->bestName, $recurOnly, nn($replaces), $amountTip, $special) . $noOneTime;
    $amtChoice = selectFld(REQ . t('Donation:'), '', dft($mya->co ? COMPANY_GIFT : INDIVIDUAL_GIFT), GIFT_LEVELS);
    jsx('amtChoice');

  } else { // sponsored (or other member company accepting donations) or not signed in
    $coName = $isCG ? PROJECT : ($coA->bestName . (strhas($coA->bestName, PROJECT) ? '' : t(' through ') . PROJECT));
    $subtext = t('<p>Make a tax-deductible contribution of any amount to support %coName.</p>', compact('coName')) . $special;
    if ($isCG or $fbo) $subtext .= ' ' . t('<p>For donations of stock, <%aStock>contact us</a>.</p>', '_aStock', atag($mya ? '/help' : r\promoLink('about-us/contact-us')));
  }

  if ($target) { // set up thermometer, if there's a target
    $fsFeeCat = r\nick2cat('FS-FEE');
    $where = "(uid2=:uid OR (:uid=:CGID AND uid2 IN :BUCKET_UIDS)) AND cat2<>$fsFeeCat AND :IS_GIFT AND created>=:dt";
    extract(db\get('SUM(1) AS count, SUM(amt) AS value', 'txs_noreverse', $where, ray('uid dt', $coA->id, $coA->targetStart)) ?: ray('count value', 0, 0));
    $therm = img(BASE_URL . "/community/donate/op=thermometer&max=$target&value=$value&count=$count");
  } else $therm = '';
  
  $logo = logoFld($coA, $therm);
  $title = t('Donate to ') . $coA->bestName;

  $period = selectFld(REQ . t('When:'), t('If you choose a recurring donation, you can stop it at any time.'), dft($period ?: ($isCG ? nn($xPeriod, PER_YEAR) : PER_1)), $oftens);
  $honored = rendA('honored', textFld(BARE, [t('whom or what')], dft($honored)));
  $honor = selectFld(t('Honoring:'), '', suffix($honored) + dft($honor), $honors);

  $fsFee = db\get('`portion`', '(SELECT * FROM tx_rules WHERE end IS NULL) x', ray('payee action amount to', $coA->id, ACT_SURTX, 0, CGID)) ?: .05;
  $coverFSFee = $fbo ? boxFld('coverFSFee', NBSP, t('Check here to cover the %fee% fiscal sponsorship fee also.', 'fee', u\fmtAmt($fsFee * 100, 's')), 0) : hidFld(0);

  $std = payFlds('', $mya, $coA, '', '', 0);
  if (!nni($std, 'amount')) return $std; else extract($std); // no amount means error from payFlds()
  $amount = numFld(REQ . t('Donation:'), [t('Amount')], dft($s_amount) + clas('short') + suffix(NBSP . NBSP . NBSP . $options));

  if ($mya) { // member: no need to ask for payment method
    $paySet = $nonMember = $stay = $stayLabel = NULL;
    $options = item($options);
  } else {
    $stayLabel = item('<br>' . t('Are you a %PROJECT member? (yet 🙂)'));
    $stay = boolFld();
    $coverCCFee = boxFld('coverCCFee', NBSP, t('Check here to cover the transaction processing fees also.'), 0);
    $next = btnFld('', '', t('Next'));
    $nonMember = fieldSet('nonMember', compact(ray('coverCCFee fullName phone email country zip next')), BARE);
    $options = NULL;
    
    $payment = item('', t('Pay by:'));
    $paymentErr = item('', NBSP);
    $paySet = fieldSet('paySet', compact(ray('payment paymentErr')), BARE);
  }
  
  $fsFee = hidFld($fsFee);

  $form = compact(ray('logo title subtext amtChoice amount amtDpy options period honor coverFSFee stayLabel stay nonMember paySet note coId fsFee recursId'));
  
  $noteSuffix = suffix(lnk('', t('Add note'))); // set to [] to disable
  $submit = submit(t('Donate'), '', 'md', $noteSuffix);
  $form += ($mya and $isCG) ? w\setupFoot(t('Donate'), $noteSuffix) : compact('submit');

  return cgForm($form);
}

function formDonate_validate($form, &$sta) {return formCCPay_validate($form, $sta);}

function formDonate_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta); 
  if ($mya) return submitCG($sta);

  extract(just('amount fsFee payByCard coverCCFee coverFSFee coId', $sta['input'], NULL));
  $feeCovered = $amount * (($coverFSFee ? $fsFee : 0) + (($payByCard and $coverCCFee) ? CC_FEE / 100 : 0));
  $sta['input']['amount'] = round($amount + $feeCovered, 2);

  return submitNonMemberPayment($sta, r\ccGiftType($coId), compact('feeCovered'));
}

/**
 * Handle submission of donation by a CG member, signed in.
 */
function submitCG($sta) {
  $mya = w\eLinkAcct($sta); 

  extract(just('amount period honor honored roundup recursId coId note', $sta['input']));
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
      } else $mya->update('notes', r\stampNote(u\honorDesc($honor) . $honored) . $mya->notes);
    }      

    $purpose = t('donation');
    if ($note) $purpose .= ' ("' . $note . '")';

    $recursId = $period == PER_1 ? NULL
    : db\insert('tx_timed', ray('action from to amount period purpose duration', ACT_PAY, $mya->id, $coId, $amount, $period, $purpose, DUR_1));
    $data = ray('gift', TRUE);
    if ($mya->ok and $z = $mya->payApproved($coId, $amount, $purpose, FOR_NONGOODS, $data, $recursId) and nni($z->subs, 'xid')) {
      $mya->tellOut('gift sent', ray('amount', u\fmtAmt($amount)));
    } else say('gift transfer later');

    $nm = $mya->fullName;
    if ($mya->ok) r\tellCAdmin('gift', $info + compact(ray('nm was honored')));
    be\thanks(r\ccGiftType($coId), TRUE, $mya->id, $purpose, $amount, $period, 1, $coId, !$mya->stepCount());
  }
  
  return r\acct() ? w\goNextStep('donate', '') : w\doSay();
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
function therm($args) {
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
