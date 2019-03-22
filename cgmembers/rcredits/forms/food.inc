<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Cron as cr;

/*
  Invite the member to contribute to the Food Fund.
*/
function formFood($form, &$sta, $args = '') {
  if (!$mya = w\eLinkAcct($sta, $args, 'acct')) return;

  $oftens = [0 => t('How often?')] + array_reverse(ray(OFTENS));
  $giftLevels = [
    1000 => t('$1,000'),
     500 => t('$500'),
     250 => t('$250'),
     100 => t('$100'),
      50 => t('$50'),
      25 => t('$25'),
       0 => t('$0'),
      -1 => t('Other)'),
  ];

  $foodId = db\get('uid', 'users', "name='foodfund'");
	if ($recurs = db\get('id,period,amount', 'r_recurs', 'payer=:payer AND payee=:foodId AND ended=0', ray('payer foodId', $mya->id, $foodId))) {
		extract($recurs);
		$recurId = hidFld($id);
    $replaces = t('donation replaces', 'period amt', $oftens[$period], u\fmtAmt($amount));
	}
  if (!$mya->activated or !$recurs or !r\acct()) {
		unset($oftens['X']); // no one-time donation unless the account also has a recurring donation and is signed in
		$recurOnly = t(': monthly, quarterly, or yearly');
	}
   
//  $inSetup = $mya->ok ? '' : t(' The amount you choose will come from your account once it is activated and has enough funds.</p>');
  $title = t('Donate to %PROJECT');
  
  $food0 = $mya->food * 100;
  $food = textFld(t('Food Percentage:'), [t('Contribution Percentage'), t('Pay a little extra every time you pay for food or meals with %PROJECT, to lower the cost of healthy local food for people who can\'t afford it otherwise. For example, 5% here means when you buy a $10 pizza, you contribute 50 cents to the %PROJECT Food Fund. To donate to the %PROJECT Food Fund directly, use the "Pay" button and type "Food Fund".')], dft($food0 ?: '') + suffix(' %'));
    
  $subtext = t('<p>(optional) Make a tax-deductible gift of any size to the Food Fund%recurOnly. %replaces</p>', 'recurOnly replaces', @$recurOnly, @$replaces);
  $amtChoice = selectFld(t('Donation:'), '', required($mya->co ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT), $giftLevels);
  $amount = textFld(t('Other amount $'), '');
  $period = selectFld(t('How often:'), '', required(0), $oftens);

  jsx('amtChoice');
  return cgform(@compact(ray('title subtext food amtChoice amount period submit recurId')));
}

function formFood_validate($form, &$sta) {
  if (!w\checkAmtChoice($sta)) return;
  $food = @$food ?: 0;
  if ($err = u\badAmount($food, '>=0', 1, 100)) return say($err, 'food');
  u\preray(compact(ray('food')), $sta['input']); // update trimmed percentage
}

function formFood_submit($form, &$sta) {
  include_once R_ROOT . '/rcron/rcron-subs.inc';
  $mya = w\eLinkAcct($sta); 

  extract(just('amount period honor honored roundup recurId', $sta['input']));
  $info = ray('payer payee amount period created lastTx invoiced', $mya->id, CGID, $amount, $period, NOW, 0, 0);
  if ($period != 'X' and @$recurId) {
		db\update('r_recurs', ray('id ended', $recurId, NOW), 'id');
		say(t('Your previous recurring gift has been canceled.'));
	}

  if ($amount > 0) {
    if ($honored) db\insert('r_honors', ray('uid honor honored', $mya->id, $honor, $honored));
    if ($period != 'X') db\insert('r_recurs', ray('payer payee amount period', $mya->id, CGID, $amount, $period));
//    $count = db\count('r_txs', 'payer=:payer AND payee=:payee AND :IS_RECURS AND created>=:created', compact(ray('payer payee created')));
    list ($desc, $data) = r\setupGift($period);
    $msg = 'gift successful';
    if ($mya->payApproved(CGID, $amount, $desc, $data)) {
      r\notify($mya->id, 'gift sent', ray('amount', u\fmtAmt($amount)));
    } else $msg .= '|gift transfer later';
    say($msg);
  }
  if ($mya->ok) r\tellCo('gift', $info);

  return r\acct() ? w\goNextStep('donate', '') : w\doSay('');
}
