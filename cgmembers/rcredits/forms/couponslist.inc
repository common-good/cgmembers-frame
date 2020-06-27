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
	
  $title = t('Discounts and Gift Certificates From You');

	$fields = ray('type amount purpose start end minimum useMax action');
  $fieldCount = count($fields);
  $hdrs = u\tableRow(ray('Type,Amount,On,Starting,Ending,Min&nbsp;Purchase,Max&nbsp;Uses,'));
	$automatic = TRUE;

  $sql = <<< X
    SELECT * FROM tx_rules
		WHERE `from`=:uid AND (action=:ACT_REDEEM OR `to`=:MATCH_PAYER)
		ORDER BY id DESC
X;
  $q = db\q($sql, ray('uid', $mya->id));
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
		$gift = $action == 'redeem';
		$type = $gift ? t('gift') : t('discount');
		
		if ($gift) {
			$args = "type=gift&amount=$amount";
  		$start = u\fmtDate($start, TRUE);
  		$end = empty($end) ? t('indefinite') : u\fmtDate($end, TRUE);
		} else {
      $args = http_build_query(compact($fields, 'automatic'));
      if (!test()) $args = u\urlify($args);
      $args = 'print/' . $args;
  		$start = u\fmtDate($start, TRUE);
  		$end = empty($end) ? t('indefinite') : u\fmtDate($end, TRUE);
//  		$on = $on ? w\popHelp(t(' on'), $on) : '';
		}

		$action = w\btn("/community/coupons/$args", t('reprint'), 'primary', 'xs', $gift ? NULL: w\away());

		$amount = $portion > 0 ? u\fmtAmt($portion, '%') : u\fmtAmt($amount);
		$minimum = $minimum ? u\fmtAmt($minimum) : t('none');
		$useMax = ($useMax + 0) ? number_format($useMax) : t('no limit');
    $rows[] = u\tableRow(compact($fields));
  }

  if (nn($rows)) {
    $rows = join($rows, "\n");
    $list = <<< X
    <table id="coupons">
    $hdrs
    $rows
    </table>
X;
  } else {$list = t('You have not yet issued any automatic discounts or gift certificates.');}
  
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

  $payerCheck = u\genRuleSQL('payer', ':myid');
  $sql = <<< X
    SELECT `from` AS sponsor, `amount`, `portion`, `purpose` AS `on`, `end`, payerType, payer, useMax, uid, amtMax,
    (SELECT IFNULL(SUM(e.amount), 0) FROM tx_entries e WHERE e.uid=:myid AND e.rule IS NOT NULL AND e.rule=ru.id) AS `amtUsed`,
    (SELECT IFNULL(SUM(SIGN(e.amount)), 0) FROM tx_entries e WHERE e.uid=:myid AND e.rule IS NOT NULL AND e.rule=ru.id) AS `used`
    FROM tx_rules ru JOIN users u ON ru.from=u.uid
    WHERE $payerCheck
    AND (u.community=:ctty OR $nearby)
    AND (ru.action=:ACT_SURTX AND ru.to=:MATCH_PAYER)
    AND (ru.start <= :NOW AND IFNULL(:NOW < ru.end, TRUE))
		ORDER BY end IS NOT NULL, end
X;
  $q = db\q($sql, $subs = ray('ctty state lat lon myid', $mya->community, $mya->state, $mya->latitude, $mya->longitude, $myid));

  while ($row = $q->fetchAssoc()) {
    extract($row);
		$a = r\acct($uid);
		$company = $a->bestName;
 		$end = $end ? u\fmtDate($end, TRUE) : '--';
    $left = $useMax > 0 ? t('%left of %useMax', 'left useMax', $useMax - $used, $useMax) : t('no limit');
    $for = $payerType == REF_GROUP ? t('you+') : t('anyone');
    if ($amount != 0) {
      if($portion != 0) {
        $amount = u\fmtAmt($amount) . ' + ' . u\fmtAmt($portion*100, '%');
      } else { // portion is 0
        $amount =  u\fmtAmt($amount);
      }
    } else {
      $amount = u\fmtAmt($portion, '%');
    }
    $amtLeft = is_null($amtMax) ? 'no limit' : u\fmtAmt($amtMax - $amtUsed, '$');

    $rows[] = u\tableRow(compact($fields));
  }

  if (nn($rows)) {
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