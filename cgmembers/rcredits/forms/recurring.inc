<?php
namespace CG\Web;
use CG\Web as w;
use CG\Backend as be;
use CG\Util as u;
use CG as r;
use CG\DB as db;
use CG\Txs as x;


/**
 * List Recurring Transactions for the current account (both in and out).
 * Include automatically paid invoices.
 */
function formRecurring($form, &$sta, $args = '') {
  global $rUrl, $mya;
  $myid = $mya->id;

  extract(just('recId reid do', $args)); 
  if (nn($do) == 'stop') {
    if (isset($recId)) $result = stopRecur($recId);
    if (isset($reid)) $result = stopAutoPay($reid);
  }

  $hdrs = ray(t(',Who,Amount,Purpose,How often?,Starting,Next,Ending,'));
  $classes = ray('arrow:arrows, fWho:name, fAmount:amount, fPurpose:purpose, fPeriod:period, fCreated:date, fNext:date, fEnd:date, button:buttons');
  $fields = array_keys($classes);
  $url0 = BASE_URL . '/history/recurring';
  $to = "<img src=\"$rUrl/images/icons/arrow-right-red.png\" border=0 />";
  $from = "<img src=\"$rUrl/images/icons/arrow-left-green.png\" border=0 />";
  
  $html = "<table>\n";
  foreach ($hdrs as $k) $html .= "<th>$k</th>\n";

  $sql = <<< X
    SELECT id as recId, IF(:myid=tp.from,tp.to,tp.from) AS fWho, tp.from=:myid AS paying, 
      tp.amount, tp.purpose AS fPurpose, tp.period, tp.periods, tp.start, tp.end,
      MAX(
        IF(tp.action=:ACT_PAY, GREATEST(IFNULL(x.created,0), IFNULL(i.created,0)),
        i.created)
      ) AS maxStart
    FROM tx_templates tp
    LEFT JOIN txs x ON x.recursId=tp.id
    LEFT JOIN r_invoices i ON i.recursId=tp.id
    WHERE action IN (:ACT_PAY, :ACT_CHARGE) AND :myid IN (tp.from,tp.to) 
    GROUP BY tp.id
    ORDER BY tp.end IS NOT NULL, tp.to=:myid, tp.start DESC, tp.id DESC
X;
  $q = db\q($sql, compact('myid'));
  
  foreach ($q as $row) {
    extract((array)$row);
    $fAmount = u\fmtAmt($amount, '$');
    $fPeriod = ucfirst(r\recurDesc($period, $periods));
    $fCreated = u\fmtDate($start, TRUE);
    $finished = ($end and $end < now());
    $fNext = $finished ? '' : u\fmtDate(max($start, now(), u\dateIncr($maxStart, $periods, $period)), TRUE);
    $fEnd = $end ? u\fmtDate($end, TRUE) : '';
    $button = $finished ? '' : t('Stop recurring transaction');
    $arrow = $paying ? $to : $from;
    $html .= addLine(compact($fields), $classes, "$url0/recId=$recId");
  }
  $html .= '<tr><td class="gap" colspan="7"></td></tr>' . "\n";
  
  $sql = 'SELECT reid, IF(other=:myid, main, other) AS fWho, other=:myid AS paying FROM r_relations WHERE :myid IN (main, other) AND :IS_AUTOPAY ORDER BY main=:myid';
  $q = db\q($sql, compact('myid'));
  
  foreach ($q as $row) {
    extract((array)$row);
    list ($fPeriod, $fPurpose) = ray(t('AutoPay Invoice'));
    $fAmount = $fCreated = $fNext = $fEnd = '';
    $button = t('Stop automatic payment');
    $arrow = $paying ? $to : $from;
    $html .= addLine(compact($fields), $classes, "$url0/reid=$reid");
  }  
  
  $html .= "</table>\n";

  $transactions = item('<div>' . $html . '</div>', BARE);
  $title = t('Automated Payments'); // . $mya->bestName;

  return cgform(compact(ray('title transactions')));
}

function addLine($info, $classes, $url) {
  global $rUrl, $mya;

  extract($info);
  $a = r\acct($fWho);
  $fWho = $a->bestName;
  $button = $button ? "<a href=\"$url&do=stop\" title=\"Stop automatic transaction\"><img src=\"$rUrl/images/icons/close-blue.png\" border=0 /></a>" : '';
  if ($mya->admin) foreach (ray('ok:O, closed:-, hasBank:b') as $k => $c) if ($a->$k) $button .= $c;
  $res = "<tr>\n";
  
  foreach (compact(array_keys($info)) as $k => $v) {
    $class = $classes[$k];
    $res .= "  <td class=\"$class\">$v</td>\n";
  }
  return $res . "</tr>\n";
}

function formRecurring_validate($form, &$sta) {}

function formRecurring_submit($form, &$sta) {}

function stopRecur($id) {
  global $mya;
  
  $row = db\get('t.from, t.to, end', 'tx_templates t', compact('id'));
  if (!$row) return w\err('invalid recur id');

  extract($row);
  if ($end and $end <= now()) return w\err('recur already ended');
  if (!in_array($mya->id, [$from, $to])) return w\err('recur not yours');

  db\update('tx_templates', ray('id end', $id, now()), 'id');
  return w\say('recur stopped');
}

function stopAutoPay($reid) {
  global $mya;
  
  $row = db\get('main, other, flags', 'r_relations', 'reid=:reid AND :IS_AUTOPAY', compact('reid'));
  if (!$row) return w\err('invalid recur id');
  extract($row);
  
  if ($other != $mya->id and $main != $mya->id) return w\err('recur not yours');
  u\setBit($flags, B_AUTOPAY, FALSE);

  db\update('r_relations', compact(ray('flags reid')), 'reid');
  return w\say('recur stopped');  
}