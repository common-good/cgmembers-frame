<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/invest.inc';

/**
 * Let Investment Club administrators accommodate member requests to reclaim all or part of their investment.
 */
function formInvestCashout($form, &$sta, $args = '') {
	global $mya; $clubid = $mya->id;
	
  $clubVal = investmentsVal($clubid, $investments, $liquid, $reserve);
	$avail = $liquid - $reserve;
	$sql = 'SELECT SUM(request) AS total, COUNT(request) AS count FROM r_stakes WHERE clubid=:clubid AND request<0';
	extract(db\q($sql, compact('clubid'))->fetchAssoc());

  $title = t('Handle Requests to Cash Out Investments');
  $available = item(u\fmtAmt($avail), t('Available Funds:'));
	$requests = item(tr('%total (%count)', 'total count', u\fmtAmt(-$total), number_format($count)), t('Total Requests:'));
	$method = $avail < $total ? w\radiosFld(t('Method:'), '', ray('inline', TRUE), [t('Oldest Request First'), t('Partial Cashout for All')]) : NULL;
	$submit = t('Cash Them Out');
  $back = backButton();
	$hid = hidFld(serialize(compact(ray('clubid avail total'))));
  
  return cgform(compact(ray('back title available requests method submit hid')));
}

function formInvestCashout_submit($form, &$sta) {
	global $mya;
	
	extract(just('method hid', $sta['input'], NULL));
	extract(just('clubid avail total', unserialize($hid)));
	$icA = r\acct($clubid);
	$pct = ($partial = $method) ? min(1, $avail / $total) : 1;
	$count = $sum = 0;
	
	$q = db\q('SELECT stakeid,stake,uid,request FROM r_stakes WHERE clubid=:clubid AND request<0 ORDER BY requestedOut', compact('clubid'));
  while ($row = $q->fetchAssoc()) {
		extract($row);
		$amount = round($pct * -$request, 2);
		if ($amount > 0 and ($partial or $sum + $amount <= $avail)) {
      $a = r\acct($uid);
      $stake = round($stake - $amount, 2);
      u\EXPECT($stake >= 0, 'negative stake');
			$DBTX = db_transaction();
			list ($ok, $index, $subs) = be\transfer('payment', $icA, $a, $amount, t('redeem investment'), FOR_USD, ray('stake', 1));
      if ($ok) {
        $request = round($request + $amount, 2);
			  db\update('r_stakes', compact(ray('stakeid stake request')), 'stakeid');
			  $count++;
			  $sum += $amount;
      } else say(t('Payment to %whom failed, redeeming investment', 'whom', "$a->fullName ($a->mainQid)"), 'err');
		  unset($DBTX);
		}
	}
	say(t('The club paid %count members a total of %sum.', 'count sum', number_format($count), u\fmtAmt($sum)));
}