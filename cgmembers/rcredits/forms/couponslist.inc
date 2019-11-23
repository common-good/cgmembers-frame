<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Print coupons for the account.
 */
function formCouponsList($form, &$sta, $args = '') {
  global $mya;

	if ($args == 'ALL') return showDiscounts();
	
  $title = t('Your Discounts and Gift Certificates');

	$fields = ray('type amount on start end minimum ulimit action');
  $fieldCount = count($fields);
  $hdrs = u\tableRow(ray('Type,Amount,On,Starting,Ending,Min&nbsp;Purchase,Max&nbsp;Uses,'));
	$automatic = TRUE;

  $sql = <<< X
    SELECT :IS_SOME AS some,c.* FROM r_coupons c
		WHERE fromId=:uid
		ORDER BY coupid DESC
X;
  $q = db\q($sql, ray('uid', $mya->id));
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
		$gift = u\getbit($flags, B_GIFT);
		$type = $gift ? t('gift') : t('discount');
		
		if ($gift) {
			$args = "type=gift&amount=$amount";
			$count = $end - $start;
		} else {
      $args = http_build_query(@compact($fields, 'automatic'));
      if (!test()) $args = u\urlify($args);
      $args = 'print/' . $args;
  		$start = u\fmtDate($start, TRUE);
  		$end = $end ? u\fmtDate($end, TRUE) : t('indefinite');
//  		$on = $on ? w\popHelp(t(' on'), $on) : '';
		}

		$action = w\btn("/community/coupons/$args", t('reprint'), 'primary', 'xs', $gift ? NULL: w\away());

		$amount = $amount < 0 ? u\fmtAmt(-$amount/100, '%') : u\fmtAmt($amount);
		$minimum = $minimum ? u\fmtAmt($minimum) : t('none');
		$ulimit = ($ulimit + 0) ? number_format($ulimit) : t('no limit');

    $rows[] = u\tableRow(compact($fields));
    if ($some) {
      $uids = db\col('uid', 'r_coupated', compact('coupid'));
      $qids = [];
      foreach ($uids as $uid) $qids[] = $mya->cAdmin ? r\acct($uid)->fullName : r\qid($uid);
      $rows[] = "<tr><td class=\"account-list\" colspan=\"$fieldCount\">" . t('<b>for only: </b>') . join(', ', $qids) . "</td></tr>\n";
    }
  }

  if (@$rows) {
    $rows = join($rows, "\n");
    $list = <<< X
    <table id="coupons">
    $hdrs
    $rows
    </table>
X;
  } else $list = t('You do not have any discounts yet.');
  
  return cgform(compact(ray('title list')));
}

/**
 * Show automatic discounts available to the current account.
 */
function showDiscounts() {
	global $mya; $myid = $mya->id;

	$title = t('Automatic Discounts in Your Region');
	$subtext = t('as of %today', 'today', u\fmtDate(NOW));
	$fields = ray('company amount on end for left');
  $hdrs = u\tableRow(ray('Company,Discount,On,Ending,For,Uses&nbsp;Left'));
	
  $sql = <<< X
  	SELECT DISTINCT u.uid,amount,`on`,end,ulimit,c.:IS_SOME AS some,
      (SELECT SUM(1) FROM txs t WHERE t.relType='D' AND t.rel=c.coupid AND t.uid2=:myid) AS used
    FROM r_coupons c JOIN users u ON u.uid=c.fromId 
    LEFT JOIN r_coupated d USING(coupid)
		WHERE (u.community=:ctty OR :DISTANCE_BETWEEN<:NEIGHBOR_MAX) 
    AND NOT c.:IS_GIFT AND (NOT c.:IS_SOME OR d.uid=:myid) AND (end=0 OR end>:NOW) AND start<=:NOW
		ORDER BY end=0, end
X;
  $q = db\q($sql, $subs = ray('ctty state lat lon myid', $mya->community, $mya->state, $mya->latitude, $mya->longitude, $myid));

  while ($row = $q->fetchAssoc()) {
    extract($row);
		$a = r\acct($uid);
		$company = $a->bestName;
 		$end = $end ? u\fmtDate($end, TRUE) : t('--');
    $left = $ulimit > 0 ? t('%left of %ulimit', 'left ulimit', $ulimit - $used, $ulimit) : t('no limit');
    if ($some and $amount > 0 and !$ulimit) $left = u\fmtAmt(be\handleCoupon($zot, $mya, $a, $amount));
    $for = $some ? t('you+') : t('anyone');
  	$amount = $amount < 0 ? u\fmtAmt(-$amount/100, '%') : u\fmtAmt($amount);

    $rows[] = u\tableRow(compact($fields));
  }

  if (@$rows) {
    $rows = join($rows, "\n");
    $list = <<< X
    <table id="coupons">
    $hdrs
    $rows
    </table>
X;
  } else $list = t('There are no discounts in your community at this time.');
  
  return cgform(compact(ray('title subtext list')));	
}