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

	$fields = ray('type amount purpose start end minimum useMax action');
  $fieldCount = count($fields);
  $hdrs = u\tableRow(ray('Type,Amount,On,Starting,Ending,Min&nbsp;Purchase,Max&nbsp;Uses,'));
	$automatic = TRUE;

  $sql = <<< X
    SELECT * FROM tx_rules
		WHERE `from`=:uid
		ORDER BY id DESC
X;
  $q = db\q($sql, ray('uid', $mya->id));
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
		$gift = $action == 'gift';
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
		$useMax = ($useMax + 0) ? number_format($useMax) : t('no limit');

    $rows[] = u\tableRow(compact($fields));
    /* if ($some) { */
    /*   $uids = db\col('uid', 'r_coupated', compact('coupid')); */
    /*   $qids = []; */
    /*   foreach ($uids as $uid) $qids[] = $mya->cAdmin ? r\acct($uid)->fullName : r\qid($uid); */
    /*   $rows[] = "<tr><td class=\"account-list\" colspan=\"$fieldCount\">" . t('<b>for only: </b>') . join(', ', $qids) . "</td></tr>\n"; */
    /* } */
    // NOT CURRENTLY HANDLED
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
	global $mya;
  $myid = $mya->id;

	$title = t('Automatic Discounts in Your Region');
	$subtext = t('as of %today', 'today', u\fmtDate(NOW));
	$fields = ray('company amount on end for left amtLeft');
  $hdrs = u\tableRow(ray('Company,Discount,On,Ending,For,Uses&nbsp;Left,Amount&nbsp;Left'));
  $nearby = tr('%DISTANCE_BETWEEN<%NEIGHBOR_MAX');
	
  $sql = <<< X
    SELECT `from` AS sponsor, `amount`, `portion`, `purpose` AS `on`, `end`, payerType, payer, useMax, uid, extraMax,
    (SELECT IFNULL(SUM(e.amount), 0) FROM tx_entries e WHERE e.uid=:myid AND e.rule IS NOT NULL AND e.rule=r.id) AS `amtUsed`,
    (SELECT IFNULL(SUM(SIGN(e.amount)), 0) FROM tx_entries e WHERE e.uid=:myid AND e.rule IS NOT NULL AND e.rule=r.id) AS `used`
    FROM tx_rules r JOIN users u ON r.`from`=u.uid
    WHERE
      (CASE payerType
       WHEN 'anybody' THEN TRUE
       WHEN 'account' THEN payer = :myid
       WHEN 'industry' THEN EXISTS (SELECT 1 FROM ancestors WHERE ancestor = :myid)
       WHEN 'group' THEN EXISTS (SELECT 1 FROM u_groups g JOIN u_groupies u ON (u.grpId = g.id)
                                    WHERE u.uid = :myid AND u.start <= :NOW
                                    AND IFNULL(:NOW < u.end, TRUE))
       ELSE FALSE
       END)
    AND (u.community=:ctty OR $nearby)
    AND (r.action = 'pay')
    AND (r.start <= :NOW AND IFNULL(:NOW < r.end, TRUE))
		ORDER BY end IS NOT NULL, end
X;
  $q = db\q($sql, $subs = ray('ctty state lat lon myid', $mya->community, $mya->state, $mya->latitude, $mya->longitude, $myid));

  while ($row = $q->fetchAssoc()) {
    extract($row);
		$a = r\acct($uid);
		$company = $a->bestName;
 		$end = $end ? u\fmtDate($end, TRUE) : t('--');
    $left = $useMax > 0 ? t('%left of %useMax', 'left useMax', $useMax - $used, $useMax) : t('no limit');
    $for = /* $some ? t('you+') : */ t('anyone');
    if ($amount != 0) {
      if($portion != 0) {
        $amount = u\fmtAmt($amount) . ' + ' . u\fmtAmt($portion*100, '%');
      } else { // portion is 0
        $amount =  u\fmtAmt($amount);
      }
    } else {
      $amount = u\fmtAmt($portion, '%');
    }
    $amtLeft = is_null($extraMax) ? 'no limit' : u\fmtAmt($extraMax - $amtUsed, '$');

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