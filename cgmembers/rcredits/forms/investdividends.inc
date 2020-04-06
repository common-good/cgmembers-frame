<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/invest.inc';

/**
 * Investment Club administrators issue dividends to club members.
 */
function formInvestDividends($form, &$sta, $args = '') {
	global $mya; $clubid = $mya->id;
	
  $clubVal = investmentsVal($clubid, $investments, $liquid, $reserve);
  $stakes = db\sum('stake', 'r_stakes', compact('clubid'));  
  $amt = u\fmtAmt($avail0 = $clubVal - $stakes);
  
  $title = t('Issue Dividends');
  $avail0 = hidFld($avail0);
  $avail = item($amt, t('Available for Dividends:'), t('(%pct is reserved for %PROJECT)', 'pct', u\fmtAmt(ICLUB_TITHE, '%')));
  if ($avail0 <= 0) {
    $amount = item(t('You cannot issue dividends at this time.'));
    $submit = NULL;
  } else {
    $amount = textFld(t('Total Dividends to Issue:'), '', required($amt));
    $submit = t('Issue Dividends');
  }
  $back = backButton();
  return cgform(compact(ray('back title avail0 avail amount submit')));
}

function formInvestDividends_validate($form, &$sta) {
  extract(just('avail0 amount', $sta['input']));
  if ($err = u\badAmount($sta['input']['amount'], '>0', 2, $avail0)) return say($err, 'amount');
}

function formInvestDividends_submit($form, &$sta) {
	global $mya; $clubid = $mya->id;
  extract(just('amount', $sta['input']));
	
	$icA = r\acct($clubid);
  $cgA = r\acct(CGID);
  $pool = $amount * (1 - ICLUB_TITHE);
	$count = $sum = $got = $reCount = $reSum = 0;

  $DBTX = \db_transaction();

  list ($ok, $index, $subs) = be\transfer('payment', $icA, $cgA, $amount - $pool, t('community dividend'), FOR_USD);
  u\EXPECT($ok, 'community dividend error');

	$q = db\q('SELECT uid FROM r_stakes', compact('clubid'));
  while ($row = $q->fetchAssoc()) {
		extract($row);
    $a = r\acct($uid);
    $pct = myShareOfClub($uid, $clubid);
		$amt = round($pct * $pool, 2);
		if ($amt > 0) {
			list ($ok, $index, $subs) = be\transfer('payment', $icA, $a, $amt, t('dividend'), FOR_USD);
      u\EXPECT($ok, 'dividend error');
      
      if ($a->reinvest) {
			  list ($ok, $index, $subs) = be\transfer('payment', $a, $icA, $amt, t('re-investment of dividends'), FOR_USD, ray('stake', 1));
        u\EXPECT($ok, 'dividend reinvestment error');
        db\q('UPDATE r_stakes SET stake=GREATEST(0, stake+request+:amt), request=LEAST(0, request+:amt) WHERE clubid=:clubid AND uid=:uid', compact(ray('clubid uid amt')));
        $reCount++;
        $reSum += $amt;
      }
      $got++;
			$sum += $amt;
		}
    $count++;
	}

  unset($DBTX);
  foreach (ray('got count reCount') as $k) $$k = number_format($$k);
  foreach (ray('sum reSum') as $k) $$k = u\fmtAmt($$k);
	say(t('dividends paid', compact(ray('got count sum reCount reSum'))));
}