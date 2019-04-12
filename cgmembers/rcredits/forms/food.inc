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
  extract(just('welcome', $args));
  if (!$mya = w\eLinkAcct($sta, $args, 'acct')) return;

  $oftens = array_reverse(ray(OFTENS));
  unset($oftens['X']); // no one-time donation here
  
  $giftLevels = [
    1000 => t('$1,000'),
     500 => t('$500'),
     250 => t('$250'),
     100 => t('$100'),
      50 => t('$50'),
      25 => t('$25'),
       0 => t('$0'),
      -1 => t('Other'),
  ];

  if (isset($welcome)) {
    $title = t('Welcome to the<br>%PROJECT Food Fund!');
    $subtext = t(<<< X
      <p>Probably you came to this page because you see the wealth of healthy local food we have in the Pioneer Valley (or wherever you live) and it troubles you that 1 out of every 8 families can't afford to buy that good food. %PROJECT offers you an easy way to support families that need help the most.</p>
      
      <p>Pay a little extra every time you pay for food or meals with Common Good, to help provide reliable access to healthy local food to people who can't afford it otherwise. OR donate to the Common Good Food Fund directly — weekly, monthly, quarterly, or yearly. Your contributions are tax-deductible.

      <p>Sign in below to your %PROJECT account or <a>click here</a> to open your free %PROJECT account now.</p>  
      
      <h4>Financially Challenged?</h4>
      <p>If you are eligible for SNAP, please consider participating in our Food Fund pilot and get $20 off each month on healthy local food at Simple Gifts Farm in Amherst. Participation is limited to 10 families. There are still a few slots available. <a>Click here</a> to open a free %PROJECT account.</p>
      <p>The discount happens automatically when you pay with your %PROJECT card.</p>
X
  );
    $accountId = textFld(t('Account ID:'));
    $pass = textFld(t('Password:'));
    $food = $amtChoice = $amount = $period = $recurId = $or = NULL;
  } else {
    $foodId = db\get('uid', 'users', "name='foodfund'");
    if ($recurs = db\get('id,period,amount', 'r_recurs', 'payer=:payer AND payee=:foodId AND ended=0', ray('payer foodId', $mya->id, $foodId))) {
      extract($recurs);
      $recurId = hidFld($id);
      $replaces = t('donation replaces', 'period amt', $oftens[$period], u\fmtAmt($amount));
    } else $replaces = '';

    $title = t('%PROJECT Food Fund');
    $subtext = t('<p>Make a tax-deductible gift of any size to the Food Fund.</p><p>Pay a little extra every time you pay for food or meals with %PROJECT, to help provide reliable access to healthy local food to people who can\'t afford it otherwise. For example, 5% here means when you buy a $10 pizza, you contribute 50 cents to the fund.</p><p><b>OR</b> donate to the %PROJECT Food Fund directly &mdash; weekly, monthly, quarterly, or yearly. Recurring donations are best, so the people we are helping can count on it. (If you prefer to make a one-time contribution, use the "Pay" button and type "Food Fund".) %replaces</p>', compact('replaces'));

    $accountId = $pass = NULL;
    $food0 = $mya->food * 100;
    $food = textFld(t('Contribution Percentage:'), [t('Percentage')], dft($food0 ?: '') + suffix(' %'));
    $or = item(t('OR'));
      
    $amtChoice = selectFld(t('Donation:'), '', required($mya->co ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT), $giftLevels);
    $amount = textFld(t('Other amount $'), '');
    $period = selectFld(t('How often:'), '', required('M'), $oftens);
  }
  
  $submit = submit();

  jsx('amtChoice');
  return cgform(@compact(ray('title subtext food or amtChoice amount period accountId pass submit recurId')));
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
