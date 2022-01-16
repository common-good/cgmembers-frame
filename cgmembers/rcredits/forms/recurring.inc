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
  $extraCrit = $mya->admin ? '1' : '(tm.end IS NULL OR tm.end>:NOW)';
  
  $html = "<table>\n";
  foreach ($hdrs as $k) $html .= "<th>$k</th>\n";

  $sql = <<< X
    SELECT id as recId, IF(:myid=tm.from, tm.to, IF(tm.from=:MATCH_PAYER, tm.payer, tm.from)) AS fWho, tm.from=:myid AS paying, tm.payerType, 
      tm.amount, tm.purpose AS fPurpose, tm.period, tm.periods, tm.start, tm.end,
      MAX(
        IF(tm.action=:ACT_PAY, GREATEST(IFNULL(x.created,0), IFNULL(i.created,0)),
        i.created)
      ) AS maxStart
    FROM tx_timed tm
    LEFT JOIN txs x ON x.recursId=tm.id
    LEFT JOIN tx_requests i ON i.recursId=tm.id
    WHERE action IN (:ACT_PAY, :ACT_CHARGE) AND :myid IN (tm.from,tm.to) 
    AND $extraCrit
    GROUP BY tm.id
    ORDER BY tm.end IS NOT NULL, tm.to=:myid, tm.start DESC, tm.id DESC
X;
  $q = db\q($sql, compact('myid'));
  
  foreach ($q as $row) {
    extract((array) $row);
    $fAmount = u\fmtAmt($amount, '$');
    $fPeriod = ucfirst(r\recurDesc($period, $periods));
    $fCreated = u\fmtDate($start, TRUE);
    $finished = ($end and $end < now());
    $fNext = $finished ? '' : u\fmtDate(max($start, now(), u\dateIncr($maxStart, $periods, $period)), TRUE);
    $fEnd = $end ? u\fmtDate($end, TRUE) : '';
    $button = $finished ? '' : t('Stop recurring transaction');
    $arrow = $paying ? $to : $from;
    $html .= addLine(compact($fields), $classes, "$url0/recId=$recId", $payerType == REF_PERSON);
  }
  $html .= '<tr><td class="gap" colspan="7"></td></tr>' . "\n";
  
  $sql = 'SELECT reid, IF(other=:myid, main, other) AS fWho, other=:myid AS paying FROM u_relations WHERE :myid IN (main, other) AND :IS_AUTOPAY ORDER BY main=:myid';
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

function addLine($info, $classes, $url, $person = FALSE) {
  global $rUrl, $mya;

  extract($info);
  $button = $button ? "<a href=\"$url&do=stop\" title=\"Stop automatic transaction\"><img src=\"$rUrl/images/icons/close-blue.png\" border=0 /></a>" : '';

  if ($person) {
    $fWho = db\get('fullName', 'people', ray('pid', $fWho));
  } else {
    $a = r\acct($fWho);
    $fWho = $a->bestName;
    if ($mya->admRegion) {
      foreach (ray('ok:k, closed:., hasBank:b') as $k => $c) if ($a->$k) $button .= $c;
      $button .= strstr($url, '=');
    }
  }
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
  
  $row = db\get('tm.from, tm.to, end', 'tx_timed tm', compact('id'));
  if (!$row) return w\err('invalid recur id');

  extract($row);
  if ($end and $end <= now()) return w\err('recur already ended');
  if (!in_array($mya->id, [$from, $to])) return w\err('recur not yours');

  db\update('tx_timed', ray('id end', $id, now()), 'id');
  return w\say('recur stopped');
}

function stopAutoPay($reid) {
  global $mya;
  
  $row = db\get('main, other, flags', 'u_relations', 'reid=:reid AND :IS_AUTOPAY', compact('reid'));
  if (!$row) return w\err('invalid recur id');
  extract($row);
  
  if ($other != $mya->id and $main != $mya->id) return w\err('recur not yours');
  u\setBit($flags, B_AUTOPAY, FALSE);

  db\update('u_relations', compact(ray('flags reid')), 'reid');
  return w\say('recur stopped');  
}