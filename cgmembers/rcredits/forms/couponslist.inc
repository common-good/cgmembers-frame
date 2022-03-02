<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const COUP_GRP_FLDS = 'action from to amount portion purpose payerType payeeType payer payee minimum useMax';

/**
 * Show coupons for the account.
 */
function formCouponsList($form, &$sta, $args = '') {
  global $mya;

  if ($args == 'ALL') return showDiscounts();
  
  $title = t('Discounts/Gift Certificates From You');

  $fields = ray('type amount purpose end minimum max action');
  $fieldCount = count($fields);
  $hdrs = u\tableRow(ray('Type,Amount,For,Ending,Min&nbsp;Purchase,Max,'));
  $automatic = TRUE;
  
  $grpFlds = u\sqlList(COUP_GRP_FLDS, 'ru');

  $sql = <<< X
    SELECT $grpFlds, payer, end, useMax, SUM(amtMax) AS amtMax, fullName, COUNT(*) AS cnt
    FROM tx_rules ru
    LEFT JOIN users u ON u.uid=payer AND payerType=:REF_ACCOUNT
    WHERE IF(`from`=:MATCH_PAYEE, payee, `from`)=:uid AND (action=:ACT_REDEEM OR `to`=:MATCH_PAYER)
    GROUP BY $grpFlds 
    ORDER BY end, fullName, start
X;
  $q = db\q($sql, ray('uid', $mya->id));
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $gift = $action == ACT_REDEEM;
    $credit = r\isCreditRule($row);
    $type = $gift ? t('gift (%cnt)', compact('cnt')): ($credit ? t('store credit') : t('discount'));

    $action = $credit ? '' : w\btn("/community/coupons/$args", t('reprint'), 'primary', 'xs', $gift ? NULL: w\away());

    $amount = $portion > 0 ? u\fmtAmt($portion, '%') : u\fmtAmt($amount);
    $minimum = $minimum ? u\fmtAmt($minimum) : t('none');
    $useMax = ($useMax + 0) ? number_format($useMax) . 'x' : '-';
    $amtMax = ($amtMax + 0) ? u\fmtAmt($amtMax) : '-';
    $max = strtr("$useMax/$amtMax", ray('-/ /-', '', ''));

    if ($gift) {
      $args = "type=gift&amount=$amount";
      $end = empty($end) ? t('indefinite') : fmtDt($end);
      
    } else {
      if ($credit) {
        $purpose = $fullName;
        $amount = $amtMax;
        $minimum = $max = '';
      } elseif (!$purpose) $purpose = t('any purchase');
      $args = http_build_query(compact($fields, 'automatic'));
      if (!test()) $args = u\urlify($args);
      $args = 'print/' . $args;
      $end = empty($end) ? t('indefinite') : fmtDt($end);
//      $on = $on ? w\popHelp(t(' on'), $on) : '';
    }
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
  
  return cgform(compact(ray('title list')), 'nolabs');
}

/**
 * Show automatic discounts available to the current account.
 */
function showDiscounts() {
  global $mya;
  $myid = $mya->id;

  $title = t('Discounts in Your Region');
  $subtext = t('as of %today', 'today', fmtDt(NOW));
  $fields = ray('company amount on end for left amtLeft');
  $hdrs = u\tableRow(ray('Company,Discount,On,Ending,For,Uses&nbsp;Left,Amount&nbsp;Left'));
  $nearby = tr('%DISTANCE_BETWEEN<%NEIGHBOR_MAX');
  $payerCheck = u\genRuleSQL('payer', ':myid');
  $anything = t('any purchase');
  
  $sql = <<< X
    SELECT `from` AS sponsor, `amount`, `portion`, IF(`purpose`='', '$anything', `purpose`) AS `on`, `end`, payerType, payer, useMax, uid, amtMax,

    (SELECT IFNULL(SUM(e.amount), 0) FROM tx_entries e WHERE e.uid=:myid AND e.rule IS NOT NULL AND e.rule=ru.id AND e.entryType=:E_REBATE) AS `amtUsed`,
    (SELECT IFNULL(SUM(SIGN(e.amount)), 0) FROM tx_entries e WHERE e.uid=:myid AND e.rule IS NOT NULL AND e.rule=ru.id AND e.entryType=:E_REBATE) AS `used`

    FROM tx_rules ru JOIN users u ON IF(ru.from=:MATCH_PAYEE, ru.payee, ru.from) = u.uid
    WHERE $payerCheck
    AND (u.community=:ctty OR $nearby)
    AND (ru.action=:ACT_SURTX AND ru.to IN(:MATCH_PAYER, :myid))
    AND (ru.start <= :NOW AND IFNULL(:NOW < ru.end, TRUE))
    ORDER BY end IS NOT NULL, end
X;
  $q = db\q($sql, ray('ctty state lat lon myid', $mya->community, $mya->state, $mya->latitude, $mya->longitude, $myid));

  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($uid);
    $company = $a->bestName;
     $end = $end ? fmtDt($end) : '--';
    $left = $useMax > 0 ? t('%left of %useMax', 'left useMax', $useMax - $used, $useMax) : t('no limit');
    $for = $payerType == REF_GROUP ? t('you+') : ($payerType == REF_ACCOUNT ? t('you') : t('anyone'));
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
    if (r\isCreditRule($row)) $on = t('store credit');

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
