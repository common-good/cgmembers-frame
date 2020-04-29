<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Handle coupons for the account.
 */
 
const COUPON_FLDS = 'type code amount portion count minimum start end useMax forOnly automatic purpose'; // not db fields

function formCoupons($form, &$sta, $args = '') {
  global $mya;
  
  jsx('coupons', 'minText', t('on your purchase of $%min or more'));

  foreach (ray(COUPON_FLDS . ' buttons submit') as $k) $$k = NULL;
  extract(just('type amount', $args, ''));

  switch ($type) {
  case 'redeem':
    $title = t('Redeem a General Gift Certificate');
    $subtext = t('Find the code on your gift certificate. If the the certificate is just for one company, there is no code &mdash; just go buy something at that business.');
    $code = textFld(t('Certificate Code:'), [t('Code')], required());
    $submit = t('Redeem');
    break;
  case 'gift':
    $title = t('Create a Gift Certificate');
    $subtext = t('Each gift certificate is assigned a <b class="loud">unique code</b>. The "Count" is the number of gift certificates you want; for example if you want a gift certificate for just one person, type "1". Print the sheet of certificates only once and do not photocopy it.');
    $amount = numFld(t('Amount:'), [t('Gift amount')], required($amount ? u\fmtAmt($amount) : ''));
    $count = intFld(REQ . t('Count:'), [t('How many?'), t('Print how many coupons/recipients? (8 to a sheet, to be cut in half 3 times)')]);
    $submit = t('Create');
    break;

  case 'discount':
    $title = t('Create a Discount');
    $subtext = t(<<<X
      <p>How it works:</p>
        <ol><li>Fill out the form.</li>
          <li>You choose:
            <ul><li><b>Automatic. </b>%PROJECT members get an <i>automatic</i> rebate when they pay you. %PROJECT will promote your coupon to each %PROJECT member. You can also print and distribute (or copy and paste into a print ad or newsletter).</li>
              <li><b>NOT automatic. </b>You charge less when members present the coupon. No electronic version of the coupon is saved. You must print and distribute them yourself and/or ask %PROJECT to distribute them to members.</li>
            </ul>
          </li>
        </ol>
      <p>After you print a sheet of coupons, cut the sheet exactly in half three times.</p>
X
      );
  
    $amount = textFld(REQ . t('Discount:'), [t('Amount or Percentage'), t('For example $20 or 10%')]);
    $minimum = numFld(t('Minimum:'), [t('Minimum purchase'), t('Minimum purchase amount, to get the discount. If this amount equals the discount amount, your coupons will be labeled "Gift Certificate".')], required(0));
    $start = dateFld(t('Valid from:'), [t('Start date')], required(u\fmtDate(now(), TRUE)));
    $end = dateFld(t('Valid until:'), [t('End date'), t('An end date is strongly encouraged.')]);
    $useMax = intFld(t('Limit:'), t('Leave this blank for unlimited uses.'), suffix(t(' uses per member')));
    $automatic = w\boolFld(t('Automatic?'), t('If the discount is not automatic, you will need to do the math each time.'), TRUE);
    $purpose = textFld(t('Description:'), [t('Discount on what')], dft(t('on any purchase')));
//    if ($mya->admin) $forOnly = textFld(t('For only:'), [t('ID number of group'), t('If you want to restrict this discount to certain %PROJECT members, type the ID of their group.')]);
    $submit = t('Create');
              
    break;
  default:
    $title = t('Discounts and Gift Certificates');
    $subtext = t('<p>A <b>Discount</b> applies only at your business. If the minimum purchase amount equals the discount amount, it is equivalent to a traditional gift certificate.</p><p>A <b>General Gift Certificate</b> is redeemable at any %PROJECT business.</p>');
    $buttons = 
      w\btn('/community/coupons/type=discount', t('Create a Discount'))
      . w\btn('/community/coupons/type=gift', t('Create a General Gift Certificate'))
      . w\btn('/community/coupons/type=redeem', t('Redeem a General Gift Certificate'))
      . w\btn('/community/coupons/list', t('List Discounts You Offer'))
      . w\btn('/community/coupons/list/ALL', t('Show All Discounts Available to You'));
  }
  
  $type = hidFld($type);
  
  return cgform(compact(ray('title subtext buttons ' . COUPON_FLDS . ' submit')));
}

function formCoupons_validate($form, &$sta) {
  global $mya;

  extract(just(COUPON_FLDS, $sta['input'], NULL));
  $portion = 0;

  if (empty($amount) and in($mya->task, 'co co2')) return;

  switch ($type) {
  case 'gift':
    if ($err = u\badAmount($amount, '>0')) return w\sayFieldErr($err, 'amount');
    if ($err = u\badAmount($count, '>0')) return w\sayFieldErr($err, 'count');
    if ($mya->creditLine < $amount * $count) {
      return say(t('You cannot issue more Gift Certificates than your credit line allows: (%try > %max).',
                   'try max', u\fmtAmt($amount * $count), u\fmtAmt($mya->creditLine)), 'err');
    }
    $useMax = 1;
    $chgFlds = 'amount portion count useMax';
    break;

  case 'discount':
    $amt0 = trim($amount);
    $amount = str_replace('%', '', $amount);
    $nums = 'amount minimum' . ($useMax === '' ? '' : ' useMax');
    foreach (ray($nums) as $k) if ($err = u\badAmount($$k, $k == 'amount' ? '>0' : '>=0')) return w\sayFieldErr($err, $k);
    //    if ($amt0[0] == '$' and $amount < 1) return w\sayFieldErr(t('The discount amount cannot be less than $1.'), 'amount');
    //    if (substr($amt0, -1, 1) == '%' and $amount > 100) return w\sayFieldErr(t('The discount percentage must be no more than 100%.'), 'amount');
    if (strpos($amt0, '%')) {
      $portion = $amount * 0.01;  // Convert percentage to value
      $amount = 0;
    } else $portion = 0;
    
    if (!$start) $start = u\fmtDate(NOW);
    foreach (ray('start end') as $k) if ($$k and $err = u\badDate($$k, today())) return w\sayFieldErr($err, $k);
    if ($end and $end < $start) return w\sayFieldErr(t('Ending date cannot be before starting date.'), 'end');
    foreach (ray('useMax end') as $k) $$k = (int) $$k;
//    if ($forOnly and !db\exists('u_groups', ['id' => $forOnly])) return say(t('That is not a group Id.'), 'forOnly');
    $chgFlds = 'amount portion minimum useMax start end';
    break;

  case 'redeem':
    if (!$fromA = r\iCardAcct($code, $iCode)) return say('invalid gift card', 'code');
    if ($fromA->id == $mya->id) return say(t('You cannot redeem your own gift card.'), 'err');

    $ruleId = db\get('id', 'tx_rules', ray('from code', $fromA->id, $iCode));
    $payeeCheck = u\genRuleSQL('payee', ':payee');
    $sql = <<< EOF
      SELECT id AS ruleId, r.end FROM tx_rules r
      WHERE r.from=:issuer
      AND :iCode = r.code
      AND r.action=:ACT_REDEEM
      AND start<=:NOW
      AND $payeeCheck
EOF;
    
    $q = db\q($sql, ray('issuer iCode payee', $fromA->id, $iCode, $mya->id));
    if (!$res = $q->fetchAssoc()) return say('invalid gift card', 'err');
    extract($res);
    if ($end or db\exists('tx_entries', ['rule' => $ruleId])) return say(t('already redeemed'), 'err');
    $from = $fromA->id;
    $to = $mya->id;
    $chgFlds = 'ruleId from to';
    break;

  default:
    return say(t('bad coupon'), 'err');
  }
  
  u\preray(compact(ray($chgFlds)), $sta['input']); // remember fixed numeric values

}

/**
 * Finish creating the discount or coupon.
 * @param bool $trial: <called from discount.inc in trial company signup>
 */
function formCoupons_submit($form, &$sta, $trial = FALSE) {
  global $mya; $myid = $mya->id;
  extract($input = just(COUPON_FLDS . ' from to ruleId', $sta['input'], NULL));

  if (empty($amount) and in($mya->task, 'co co2')) {
    return $mya->task == 'co' ? w\go($mya->nextStepUrl()) : w\goNextStep('discount'); // new member pressed skip on discount form
  }

  switch ($type) {
  case 'redeem': 
    $fromA = r\acct($from);

    $amount = db\get('amount', 'tx_rules', ['id' => $ruleId]);
    list ($ok, $index, $subs) = be\transfer('payment', $fromA, r\acct($to), $amount, t('gift'), FOR_GOODS, ['force' => 1, 'ruleId' => $ruleId]);
    u\EXPECT($ok, t('failed forced gift redemption by %who for rule %ruleId', 'who ruleId', $mya->fullName, $ruleId));
    $mya->update('giftPot', $mya->giftPot + $amount);
    db\update('tx_rules', ray('id end', $ruleId, now()), 'id'); // mark this used
    return say(t('coupon redeemed', 'amount giver', u\fmtAmt($amount), $fromA->fullName));

  case 'gift':
    u\setDft($minimum, 0);
    u\setDft($purpose, t('on any purchase'));
    $start = $mya->giftCoupons + 0;
    $end = $start + $count - 1;
    $info = ray('start from to action payeeType', now(), $myid, MATCH_PAYER, ACT_REDEEM, REF_ANYBODY);
    $info += compact(ray('amount portion purpose minimum useMax'));

    $DBTX = \db_transaction();
    foreach (range($start, $end) as $code) db\insert('tx_rules', $info + compact('code'));
    unset($DBTX);

    return reportCoupon(t('gift certificate'), $input);

  case 'discount':
    if ($end) $end = strtotime('tomorrow', $end) - 1; // last second of the day
    // purpose needs a default when called from discount.inc
    u\setDft($purpose, $minimum ? t('on your purchase of %min or more', 'min', u\fmtAmt($minimum)) : t('on any purchase'));
    
    if ($automatic) {
      $info = ray('payee payeeType from to action', $myid, REF_ACCOUNT, $myid, MATCH_PAYER, ACT_SURTX);
      $info += compact(ray('amount portion purpose minimum useMax start end'));
      db\insert('tx_rules', $info);
    }
  
    return reportCoupon(t('discount'), compact(ray('end purpose')) + $input);
  }
}

/**
 * Report the successful creation of the coupon and go somewhere else.
 */
function reportCoupon($thing, $info) {
  global $mya;
  w\say(t('Your %thing was created successfully.', compact('thing')));
  r\tellCo(t('New Coupons!'), $info + ray('company', $mya->fullName));
  if ($mya->co) $mya->stepDone('discount');

  $args = u\urlify(http_build_query($info));
  if (test()) $args = u\deurlify($args);

  $printUrl = "/community/coupons/print/$args";
  $listUrl = '/community/coupons/list';

  return $mya->ok 
  ? (test() ? go($printUrl) : go2($listUrl, $printUrl))
  : goNextStep('discount');
}
