<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once R_ROOT . '/forms/ccpay.inc';

/*
  Give the member or nonmember a chance to donate to CG or a sponsored (CG FBO) account or a CCOK account.
*/
function formDonate($form, &$sta, $args = NULL) {
  if (!$args) $args = u\_GET(); // can't use u\setDft here
  $args = $args ? urlq2ray($args) : $_POST;
  $mya = w\eLinkAcct($sta, $args, 'any');
  
  $op = nni($args, 'op');

  if ($op == 'done') return completeCCPayment($args);
  if ($op == 'cancel') return softErr(tr('no paypal cancel'));

  if ($code = nni($args, 'code')) { // donating to a sponsored org
    if (!$ray = getCGPayArgs($args)) return; // webmaster error message already shown
    extract(just('coA s_amount', $ray, NULL));
    if (!nn($coA)) return; // softErr
  } else $coA = r\acct(CGID); // vanilla CG donation page
  
  jsx('donate');
  
  $oftens = array_reverse(r\recurRay(TRUE));
  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of');
  $recursId = $amtChoice = NULL;
  $isCG = ($coA->id == CGID);
  $fbo = $coA->sponsored;
  $ccok = ($fbo or $coA->ccok);
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
    $subtext = t('<p>By opening a %PROJECT account, you are partnering with us to create a %PROJECT Economy. Most members choose to make a tax-deductible donation%recurOnly. %replaces %amountTip</p><p>To donate an amount not listed (including ZERO), choose "Water" (and fill in the amount).</p><p><b class="loud">NOTE: This is a donation, not a deposit.</b> Thank you for your support!</p>', 'recurOnly replaces amountTip', $recurOnly, nn($replaces), $amountTip) . $noOneTime;
    $amtChoice = selectFld(REQ . t('Donation:'), '', dft($mya->co ? COMPANY_GIFT : INDIVIDUAL_GIFT), GIFT_LEVELS);
    jsx('amtChoice');

  } else { // sponsored (or other member company accepting donations) or not signed in
    $options = btnOpt('', t('repeat'), 'btn-repeat') . $options;
    $coName = $isCG ? PROJECT : ($coA->bestName . (strhas($coA->bestName, PROJECT) ? '' : t(' through ') . PROJECT));
    $subtext = t('Make a tax-deductible contribution of any amount to support %coName.', compact('coName'));
    if ($isCG or $fbo) $subtext .= ' ' . t('For donations of stock, <%aStock>contact us</a>.</p>', '_aStock', atag($mya ? '/help' : r\promoLink('about-us/contact-us')));
  }

  $options = item($options);
    
  $logo = logoFld($coA);
  $title = t('Donate to ') . $coA->bestName;

  $period = selectFld(REQ . t('When:'), t('If you choose a recurring donation, you can stop it at any time.'), dft($isCG ? PER_YEAR : PER_1), $oftens);
  $honored = rendA('honored', textFld(BARE, [t('(optional)')]));
  $honor = selectFld(t('Honoring:'), '', suffix($honored), $honors);

  $fsFee = db\get('`portion`', '(SELECT * FROM tx_rules WHERE end IS NULL) x', ray('payee action amount to', $coA->id, ACT_SURTX, 0, CGID)) ?: .05;
  $coverFSFee = $fbo ? boxFld('coverFSFee', NBSP, t('Check here to cover the %fee% fiscal sponsorship fee also.', 'fee', u\fmtAmt($fsFee * 100, 's')), 0) : hidFld(0);
  $coverCCFee = boxFld('coverCCFee', NBSP, t('Check here to cover the credit card processing fees also.'), 0);
  
  if ($mya) { // member: no need to ask for payment method
    $payHow = $coverCCFee = $ach = NULL;
  } elseif ($fbo) { // nonmember & sponsored: guest can choose ACH or CC
    $payHow = radiosFld(REQ . t('Pay By:'), '', dft(-1), [t('ACH (easier and saves us 3% in CC fees)'), t('Credit Card <span>(cannot automatically repeat)</span>')]);
    $checkHint = item(tr('<img src="%src" />', 'src', BASE_URL . '/rcredits/images/check-hint.png'));
    $routingNumber = routingFld(FALSE);
    $bankAccount = intFld(t('Account:'), t('Type <b class="loud">carefully</b> your account number. <br>'), maxlen(17));
    $isSavings = radiosFld(t('Type:'), '', dft(-1), [t('Checking'), t('Savings')]);
    $ach = fieldSet('ach', compact(ray('checkHint routingNumber bankAccount isSavings')), BARE);
  } else { // nonmember & not sponsored: CC only
    $payHow = hidFld(1);
    $ach = NULL;
  }

  $fsFee = hidFld($fsFee);

  $cgPayLink = '/' . first('signin');
  $form = compact(ray('logo title subtext amtChoice options period honor coverFSFee payHow coverCCFee ach fsFee recursId cgPayLink'));
  $form = payFlds(t('Donation'), $mya, $coA, t('donation'), '', nn($s_amount), $form);
  if (!nni($form, 'amount')) return $form; // error
  
  $noteSuffix = []; // suffix(lnk('', t('Add comment')));
  $form += ($mya and $isCG) ? w\setupFoot(t('Donate'), $noteSuffix) : ray('submit', submit(t('Donate'), '', '', $noteSuffix));

  return cgForm($form);
}

function formDonate_validate($form, &$sta) {return formCCPay_validate($form, $sta);}

function formDonate_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta); 
  if ($mya) return submitCG($sta);

  extract(just('amount fsFee payHow coverCCFee coverFSFee coId', $sta['input'], NULL));
  $feeCovered = $amount * (($coverFSFee ? $fsFee : 0) + (($payHow > 0 and $coverCCFee) ? CC_FEE / 100 : 0));
  $sta['input']['amount'] = round($amount + $feeCovered, 2);

  return submitNonMemberPayment($sta, w\ccGiftType($coId), compact('feeCovered'));
}

/**
 * Handle submission of donation by a CG member, signed in.
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
    be\thanks(w\ccGiftType($coId), TRUE, $mya->id, t('donation'), $amount, $period, 1, $coId, !$mya->stepCount());
  }
  
  return r\acct() ? w\goNextStep('donate', '') : w\doSay();
}
