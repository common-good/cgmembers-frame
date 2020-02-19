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
function formCoupons($form, &$sta, $args = '') {
  global $mya;
  extract(just('type amount', $args));
  
  if (@$type == 'redeem') {
    $title = t('Redeem a General Gift Certificate');
    $subtext = t('Find the code on your gift certificate. If the the certificate is just for one company, there is no code &mdash; just go buy something at that business.');
    $code = textFld(t('Certificate Code:'), [t('Code')], required());
    $submit = t('Redeem');
  } elseif (@$type == 'gift') {
    $title = t('Create a Gift Certificate');
    $subtext = t('Each gift certificate is assigned a <b class="loud">unique code</b>. The "Count" is the number of gift certificates you want; for example if you want a gift certificate for just one person, type "1". Print the sheet of certificates only once and do not photocopy it.');
    $amount = textFld(t('Amount:'), [t('Gift amount')], required(@$amount ? u\fmtAmt($amount) : ''));
    $count = textFld(t('Count:'), [t('How many?'), t('Print how many coupons/recipients? (8 to a sheet, to be cut in half 3 times)')], required());
    $submit = t('Create');
  } elseif (@$type == 'discount') {
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
  
    $amount = textFld(t('Discount:'), [t('Amount or Percentage'), t('For example $20 or 10%')], required());
    $minimum = textFld(t('Minimum:'), [t('Minimum purchase'), t('Minimum purchase amount, to get the discount. If this amount equals the discount amount, your coupons will be labeled "Gift Certificate".')], required(0));
    $start = textFld(t('Valid from:'), [t('Start date')], required(u\fmtDate(now(), TRUE)));
    $end = textFld(t('Valid until:'), [t('End date'), t('An end date is strongly encouraged.')]);
    $ulimit = textFld(t('Limit:'), t('Leave this blank for unlimited uses.'), suffix(t(' uses per member')));
    $automatic = w\boolFld(t('Automatic?'), t('If the discount is not automatic, you will need to do the math each time.'), TRUE);
    $on = textFld(t('Description:'), [t('Discount on what')]);
    $forOnly = textFld(t('For only:'), [t('List of account IDs'), t('If you want to restrict this discount to certain %PROJECT members, type their account IDs here, separated by commas.')]);
    $submit = t('Create');
  } else {
    $title = t('Discounts and Gift Certificates');
    $subtext = t('<p>A <b>Discount</b> applies only at your business. If the minimum purchase amount equals the discount amount, it is equivalent to a traditional gift certificate.</p><p>A <b>General Gift Certificate</b> is redeemable at any %PROJECT business.</p>');
    $buttons = 
      w\btn('/community/coupons/type=discount', t('Create a Discount'))
    . w\btn('/community/coupons/type=gift', t('Create a General Gift Certificate'))
    . w\btn('/community/coupons/type=redeem', t('Redeem a General Gift Certificate'))
    . w\btn('/community/coupons/list', t('List Discounts You Offer'))
    . w\btn('/community/coupons/list/ALL', t('Show All Discounts Available to You'));
  }
  
  $type = hidFld(@$type);
  jsx('coupons');
  
  return cgform(@compact(ray('title buttons subtext code amount count minimum start end ulimit forOnly automatic on submit type')));
}

function formCoupons_validate($form, &$sta) {
  global $mya;
  $today = today();
  extract(just($fields = 'type code amount count minimum start end ulimit forOnly', $sta['input']));

  if (empty($amount) and in($mya->task, 'co co2')) return;
    
  if ($type == 'gift') {
    if ($err = u\badAmount($amount, '>0')) return w\sayFieldErr($err, 'amount');
    if ($err = u\badAmount($count, '>0')) return w\sayFieldErr($err, 'count');
    if ($mya->creditLine < $amount * $count) return say(t('You cannot issue more Gift Certificates than your credit line allows: (%try > %max).', 'try  max', u\fmtAmt($amount * $count), u\fmtAmt($mya->creditLine)), 'err');
    $ulimit = 1;
  } elseif ($type == 'discount') {
    $amt0 = trim($amount);
    $amount = str_replace('%', '', $amount);
    $nums = 'amount minimum' . ($ulimit === '' ? '' : ' ulimit');
    foreach (ray($nums) as $k) if ($err = u\badAmount($$k, $k == 'amount' ? '>0' : '>=0')) return w\sayFieldErr($err, $k);
//    if ($amt0[0] == '$' and $amount < 1) return w\sayFieldErr(t('The discount amount cannot be less than $1.'), 'amount');
//    if (substr($amt0, -1, 1) == '%' and $amount > 100) return w\sayFieldErr(t('The discount percentage must be no more than 100%.'), 'amount');
    if (strpos($amt0, '%')) $amount = -$amount; // we use negative to mean percentage
    
    if (!$start) $start = u\fmtDate(NOW);
    foreach (ray('start end') as $k) if ($$k and $err = u\badDate($$k, $today)) return w\sayFieldErr($err, $k);
    if ($end and $end < $start) return w\sayFieldErr(t('Ending date cannot be before starting date.'), 'end');
    foreach (ray('ulimit end') as $k) $$k = (int) $$k;
    if ($list = mb_strtoupper(str_replace(' ', '', nn($forOnly)))) {
      $forOnly = [];
      foreach (explode(',', $list) as $qid) if ($qid and $a = r\acct($qid) and $uid = $a->id) {
        $forOnly[] = $uid;
      } else return say(t('"%qid" is not an account ID.', compact('qid')), 'forOnly');
    }
  } elseif ($type == 'redeem') {
    if (!$a = r\iCardAcct($code, $iCode)) return say('invalid gift card', 'code');
    if ($a->id == $mya->id) return say(t('You cannot redeem your own gift card.'), 'err');
    
    $auxid = db\get('id', 'u_auxtxs a JOIN u_rules r ON a.rule = r.id', 'r.fromId=:uid AND :iCode = a.code',
                    ray('uid iCode', $a->id, $iCode)) ?: 0;
    if (db\get('id', 'tx_entries_all', 'auxtx = :auxid', compact(ray('auxid'))))
      return say('already redeemed', 'err');
    u\preray(compact(ray('auxid')), $sta['input']); // pass the results to submit function
  } else return say(t('bad coupon'), 'err');
  
  u\preray(@compact(ray($fields)), $sta['input']); // remember fixed numeric values
}

/**
 * Finish creating the discount or coupon.
 * @param bool $trial: <called from discount.inc in trial company signup>
 */
function formCoupons_submit($form, &$sta, $trial = FALSE) {
  global $mya; $myid = $mya->id;
  extract(just('type amount coupid iCode count minimum start end ulimit automatic on forOnly', $sta['input']));

  if (empty($amount) and in($mya->task, 'co co2')) {
    return $mya->task == 'co' ? w\go($mya->nextStepUrl()) : w\goNextStep('discount'); // new member pressed skip on discount form
  }
    
  if ($type == 'redeem') {
    $res = db\get('fromId, amount', 'r_coupons', 'coupid=:coupid', compact('coupid'));
    extract($res);
    $fromA = r\acct($fromId);
    list ($ok, $index, $subs) = be\transfer('payment', $fromA, $mya, $amount, t('gift'), FOR_GOODS, ray('force', 1));
    u\EXPECT($ok, t('failed forced gift redemption by %who for coupon %coupid', 'who coupid', $mya->fullName, $coupid));
    db\insert('r_coupated', ray('uid coupid uses when', $myid, $coupid, $iCode, now()));
    $mya->update('giftPot', $mya->giftPot + $amount);
    return say(t('coupon redeemed', 'amount giver', u\fmtAmt($amount), $fromA->fullName));
  }

  // gift or discount  
  $gift = ($type == 'gift');
  $flags = $gift ? u\bit(B_GIFT) : 0; // set gift flag

  $DBTX = \db_transaction();
  if ($gift) {
    list ($start, $end) = [$mya->giftCoupons + 0, $coups = $mya->giftCoupons + $count];
  } else {
		if (!@$on) $on = $minimum ? t('on your purchase of %min or more', 'min', u\fmtAmt($minimum)) : t('on any purchase');
		if ($end) $end = strtotime('tomorrow', $end) - 1; // last second of the day
	}

  if ($gift or $automatic) {
    if (@$forOnly) $flags |= u\bit(B_SOME);
    $coupid = db\insert('r_coupons', @compact(ray('amount on minimum start end ulimit flags')) + ray('fromId sponsor', $myid, $myid), 'coupid');
    if (!$gift and nn($forOnly)) foreach ($forOnly as $uid) db\insert('r_coupated', compact(ray('uid coupid')), 'id');
  }
  unset($DBTX);
  
  if ($gift) {unset($start); unset($end);} // don't pass these to printCoupons()

  $args = u\urlify(http_build_query($ray = @compact(ray('type amount minimum ulimit count on start end automatic'))));
	r\tellCo(t('New Coupons!'), $ray + ray('company gift forOnly', $mya->fullName, $gift, nn($forOnly)));
  if (test()) $args = u\deurlify($args);
	$url = "/community/coupons/print/$args";
  w\say(t('Your %thing was created successfully.', 'thing', $gift ? t('gift certificate') : t('discount')));

  if ($trial) {
    return w\goNextStep('discount');
  } elseif ($mya->co and !$gift) $mya->stepDone('discount');
  return test() ? w\go($url) : w\go2('/community/coupons/list', $url);
}
