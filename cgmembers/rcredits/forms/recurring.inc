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

  $classes = ray('arrow:arrows, fWho:name, fAmount:amount, fPurpose:purpose, fPeriod:period, fCreated:date, fNext:date, fEnd:date, button:buttons');
  $fields = array_keys($classes);
  $url0 = BASE_URL . '/history/recurring';
  $to = "<img src=\"$rUrl/images/icons/arrow-right-red.png\" border=0 />";
  $from = "<img src=\"$rUrl/images/icons/arrow-left-green.png\" border=0 />";
  
  $html = <<<EOF
  <table>
    <th></th>
    <th>Who</th>
    <th>Amount</th>
    <th>Purpose</th>
    <th>How often?</th>
    <th>Starting</th>
    <th>Next</th>
    <th>Ending</th>
    <th></th>
EOF;

  $sql = <<< X
    SELECT id as recId, IF(:myid=t.from,t.to,t.from) AS fWho, t.from=:myid AS paying, 
      amount, purpose AS fPurpose, period, periods, start, t.end 
    FROM tx_templates t 
    WHERE action IN (:ACT_PAY, :ACT_CHARGE) AND :myid IN (t.from,t.to) 
    ORDER BY t.to=:myid
X;
  $q = db\q($sql, compact('myid'));
  
  foreach ($q as $row) {
    extract((array)$row);
    $fAmount = u\fmtAmt($amount, '$');
    $fPeriod = ucfirst(r\recurDesc($period, $periods));
    $fCreated = u\fmtDate($start, TRUE);
    $finished = ($end and $end < now());
    $fNext = $finished ? '' : u\fmtDate(nextRecur($start, $period, $periods), TRUE);
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

  $transactions = item('<div>' . $html . '</div>');
  $title = t('Recurring Payments'); // . $mya->bestName;

  return cgform(compact(ray('title transactions')));
}

function addLine($info, $classes, $url) {
  global $rUrl;

  extract($info);
  $fWho = r\acct($fWho)->bestName;
  $button = $button ? "<a href=\"$url&do=stop\" title=\"Stop recurring transaction\"><img src=\"$rUrl/images/icons/close-blue.png\" border=0 /></a>" : '';
  $res = "<tr>\n";
  
  foreach (compact(array_keys($info)) as $k => $v) {
    $class = $classes[$k];
    $res .= "  <td class=\"$class\">$v</td>\n";
  }
  return $res . "</tr>\n";
}

function nextRecur($starting, $period, $periods = 1) {
  if (array_key_exists($period, OFTENS_SECS)) {
    $perSecs = OFTENS_SECS[$period] * $periods;
    $nPer = ceil((now() - $starting) / $perSecs);
    return round($starting + ($nPer * $perSecs));
  } else {
    return FALSE;
  }
}

function formRecurring_validate($form, &$sta) {}

function formRecurring_submit($form, &$sta) {}

function stopRecur($id) {
  global $mya;
  
  $row = db\get('t.from, t.to, end', 'tx_templates t', compact('id'));
  if (!$row) return w\err('invalid recur id');

  extract($row);
  if ($end) return w\err('recur already ended');
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