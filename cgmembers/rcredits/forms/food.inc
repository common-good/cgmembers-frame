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
  $mya = w\eLinkAcct($sta, $args, 'any');

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

  $welcomeText = t(<<< X
  <p>Probably you came to this page because you see the wealth of healthy local food we have in the Pioneer Valley (or wherever you live) and it troubles you that 1 out of every 8 families can't afford to buy that good food. %PROJECT offers you an easy way to support families that need help the most.</p>
X
  );

  $title = t('%PROJECT Food Fund');
  $food = $amtChoice = $amount = $period = $recurId = $or = $submit = $recurs = NULL;

  if (isset($welcome) and !$mya) { // came here signed out (or non-member)
    $title = '<small>' . t('Welcome to the') . '<br></small>' . $title;
    $subtext = $welcomeText . t(<<< X
      <p>Pay a little extra every time you pay for food or meals with %PROJECT, to help provide reliable access to healthy local food to people who can't afford it otherwise. OR donate to the %PROJECT Food Fund directly — weekly, monthly, quarterly, or yearly. Your contributions are tax-deductible.

      <p>Sign in to your %PROJECT account and click <%aFood>Food Fund</a> or <%aSignup>click here</a> to open your free %PROJECT account now.</p>
      
      <h4>Financially Challenged?</h4>
      <p>If you are eligible for SNAP, please consider participating in our Food Fund pilot and get $20 off each month on healthy local food at Simple Gifts Farm in Amherst. Participation is limited to 10 families. There are still a few slots available. <%aSignup>Click here</a> to open a free %PROJECT account.</p>
      <p>The discount happens automatically when you pay with your %PROJECT card.</p>
X
    , '_aSignup _aFood', atag('/signup/food=1'), atagB('', 'warning'));
  } elseif ($mya and $mya->canManage) {
    $foodId = db\get('uid', 'users', "name='foodfund'");
    if ($recurs = db\get('id,period,amount', 'r_recurs', 'payer=:payer AND payee=:foodId AND ended=0', ray('payer foodId', $mya->id, $foodId))) {
      extract($recurs);
      $recurs = hidFld(serialize($recurs));
      $replaces = t('donation replaces', 'period amt', $oftens[$period], u\fmtAmt($amount));
    } else $replaces = '';
    
    $whenActive = $mya->ok ? '' : t('when your account is activated');

    $subtext = isset($welcome) ? $welcomeText : '';
    $subtext .= t('<p>Make a tax-deductible gift of any size to the Food Fund.</p><p>Pay a little extra every time you pay for food or meals with %PROJECT, to help provide reliable access to healthy local food to people who can\'t afford it otherwise. For example, 5% here means when you buy a $10 pizza, you contribute 50 cents to the fund.</p>
    <p><b>OR</b> donate to the %PROJECT Food Fund directly &mdash; weekly, monthly, quarterly, or yearly.</p>
    <p>Percentage or recurring donations are better than one-time contributions, so the people we are helping can count on it. (If you prefer to make a one-time contribution, %whenActive use the "Pay" button and type "Food Fund".) %replaces</p>', compact('replaces', 'whenActive'));

    $accountId = $pass = NULL;
    $food0 = $mya->food * 100;
    $food = textFld(t('Contribution Percentage:'), [t('Percentage')], dft($food0 ?: '') + suffix(' %'));
    $or = item(t('AND / OR'));
      
//    $amtChoice = selectFld(t('Donation:'), '', required($mya->co ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT), $giftLevels);
    $amtChoice = selectFld(t('Donation:'), '', required($mya->food), $giftLevels);
    $amount = textFld(t('Other amount $'), '');
    $period = selectFld(t('How often:'), '', required('M'), $oftens);
    $submit = submit();
  } else {$subtext = t('You do not have permission to use this page.');}
  
  jsx('amtChoice');
  return cgform(@compact(ray('title subtext food or amtChoice amount period accountId pass submit recurs')));
}

function formFood_validate($form, &$sta) {
  if (!w\checkAmtChoice($sta)) return;
  extract(just('food', $sta['input']));
  $food = @$food ?: 0;
  if ($err = u\badAmount($food, '>=0', 1, 100)) return say($err, 'food');
  u\preray(compact(ray('food')), $sta['input']); // update trimmed percentage
}

function formFood_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta); 

  extract($info = just('food amount period recurs', $sta['input']));
  
  $food /= 100;
  $mya->update(compact('food'));
  
  if (@$recurs) {
    extract(just('id period amount', unserialize($recurs)), EXTR_PREFIX_ALL, 'x');
    if ($x_period != $period or $x_amount != $amount) {
      db\update('r_recurs', ray('id ended', $x_id, NOW), 'id');
      say(t('Your previous recurring contribution has been canceled.'));
    }
	}

  if ($amount > 0) {
    $payee = db\get('uid', 'users', "name='foodfund'");
    db\insert('r_recurs', ray('payer payee amount period', $mya->id, $payee, $amount, $period));
    list ($desc, $data) = r\setupGift($period);
    $msg = 'gift successful';
    if ($mya->payApproved($payee, $amount, $desc, FOR_USD, $data)) {
      r\notify($mya->id, 'recur pay', ray('amount to purpose', u\fmtAmt($amount), t('the Food Fund'), $desc));
    } else $msg .= '|gift transfer later';
    say($msg);
  }
  if ($mya->ok) r\tellCo(t('Food Fund gift'), $info);

  return r\acct() ? w\goNextStep('food', '') : w\doSay('');
}
