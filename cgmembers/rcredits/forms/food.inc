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
  <p>Probably you came to this page because you see the wealth of healthy local food we have in the Pioneer Valley (or wherever you live) and it troubles you that 1 out of every 8 people is still at risk of hunger &mdash; and many families struggle to afford good food. %PROJECT offers you an easy way to support families that need help the most.</p>
X
  );

  $title = t('%PROJECT Food Fund');
  $food = $amtChoice = $amount = $period = $recurId = $or = $submit = $recurs = NULL;
  $_payExtra = t('Pay a little extra every time you pay for food or meals with %PROJECT, to help provide reliable access to healthy local food to people who can\'t afford it otherwise.');
  $_orRegular = t('<b>OR</b> donate to the %PROJECT Food Fund directly &mdash; weekly, monthly, quarterly, or yearly.');
  $img = NULL;

  if (isset($welcome) and !$mya) { // came here signed out (or non-member)
    $title = '<small>' . t('Welcome to the') . '<br></small>' . $title;
    $subtext = $welcomeText . t(<<< X
      <p>%payExtra %orRegular Your contributions are tax-deductible.

      <p><%aSignin>Sign in</a> to your %PROJECT account and look for the <%aFood>Food Fund</a> button or click the button below to open your free %PROJECT account now.</p>
      
      <br><p><center><%aSignup>Sign Up</a></center></p>
      
      <h4>Financially Challenged?</h4>
      <p>If you struggle to buy healthy food, please consider participating in our Food Fund pilot and get $20 off each month on healthy local food at Simple Gifts Farm in Amherst (just one location during this pilot). Participation is limited to 10 families. There are still a few slots available. Click the button above to open a free %PROJECT account.</p>
      <p>The discount happens automatically when you pay with your %PROJECT card.</p>
X
    , '_payExtra _orRegular _aSignin _aSignup _aFood', $_payExtra, $_orRegular, atag('/signin'), atagB('/signup/source2=food', 'primary', 'md'), atagB('', 'warning'));
    $img = item(tr('<img src="%BASE_URL/rcredits/images/pages/simple-gifts-purchase.jpg" />'));
  } elseif ($mya and $mya->canManage) {
    $foodId = db\get('uid', 'users', "name='foodfund'");
    if ($recurs = db\get('id,period,amount', 'r_recurs', 'payer=:payer AND payee=:foodId AND ended=0', ray('payer foodId', $mya->id, $foodId))) {
      extract($recurs);
      $recurs = hidFld(serialize($recurs));
      $replaces = t('donation replaces', 'period amt', $oftens[$period], u\fmtAmt($amount));
      $period0 = $period;
      $amount += 0;
      $amtChoice0 = isset($giftLevels[$amount]) ? $amount : -1;
      $amount0 = $amtChoice0 == -1 ? $amount : '';
    } else {
      $replaces = '';
      $period0 = 'M';
      $amtChoice0 = '0';
      $amount0 = '';
    }
    
    $whenActive = $mya->ok 
    ? (r\acct() ? '' : t('sign in at %CG_DOMAIN to')) 
    : t('when your account is activated');

    $subtext = isset($welcome) ? $welcomeText : '';
    $subtext .= t('<p>Make a tax-deductible gift of any size to the Food Fund.</p>
    <p>%payExtra For example, 5% here means when you buy a $10 pizza, you contribute 50 cents to the fund.</p>
    
    <p>%orRegular %replaces</p>
    
    <p>Percentage or recurring donations are better than one-time contributions, so the people we are helping can count on it. (If you prefer to make a one-time contribution, %whenActive use the "Pay" button to pay "Food Fund".)</p>', compact(ray('_payExtra _orRegular replaces whenActive')));

    $accountId = $pass = NULL;
    $food0 = $mya->food * 100;
    $food = textFld(t('Contribution Percentage:'), [t('Percentage')], dft($food0 ?: '') + suffix(' %'));
    $or = item(t('AND / OR'));
      
//    $amtChoice = selectFld(t('Donation:'), '', required($mya->co ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT), $giftLevels);
    $amtChoice = selectFld(t('Donation:'), '', dft($amtChoice0), $giftLevels);
    $amount = textFld(t('Other amount $'), '', dft($amount0));
    $period = selectFld(t('How often:'), '', dft($period0), $oftens);
    $submit = submit();
  } else {$subtext = t('You do not have permission to use this page.');}
  
  jsx('amtChoice');
  return cgform(@compact(ray('title img subtext food or amtChoice amount period accountId pass submit recurs')));
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
  $pctGift = t('Your percentage contribution to the Food Fund');
  if ($food - $mya->food) {
    $mya->update(compact('food'));
    say(t('%pctGift is now %food.', 'pctGift food', $pctGift, u\fmtAmt($food, '%')));
  } elseif ($food) {say(t('%pctGift is unchanged.', compact('pctGift')));}
  
  if (@$recurs) {
    extract(just('id period amount', unserialize($recurs)), EXTR_PREFIX_ALL, 'x');
    if ($x_period != $period or $x_amount != $amount) {
      db\update('r_recurs', ray('id ended', $x_id, now()), 'id');
      say(t('Your previous recurring contribution has been canceled.'));
    } else {
      $amount = 0;
      say(t('Your recurring contribution is unchanged.'));
    }
	}

  if ($amount > 0) {
    $payee = db\get('uid', 'users', "name='foodfund'");
    $purpose = t('food donation');
    $recursId = db\insert('r_recurs', ray('payer payee amount period purpose', $mya->id, $payee, $amount, $period, $purpose));
    $msg = 'gift successful';
    $period = r\periodDesc($period);
    if ($mya->payApproved($payee, $amount, "$purpose ($period)", FOR_GOODS, ['recurs' => 1], $recursId)) {
      r\notify($mya->id, 'recur pay', ray('amount period to purpose', u\fmtAmt($amount), $period, t('the Food Fund'), $purpose));
    } else $msg .= '|gift transfer later';
    say($msg);
  }
  if ($mya->ok) r\tellCo(t('Food Fund gift'), $info);

  say(t('Thank you for supporting the Food Fund.'));
  return r\acct() ? ($mya->member ? w\go('') : w\goNextStep('food', '')) : w\doSay();
}
