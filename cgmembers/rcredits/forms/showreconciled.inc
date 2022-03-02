<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

/**
 * Show a list of reconciled Dollar Pool transfers and other transfers in or out of that (or those) account(s).
 */
function formShowReconciled($form, &$sta) {
  $neg = 'amount<0 XOR txid<0';
  $desc = "IF(txid<0, 'BAD', IF(amount>0, 'IN', 'OUT'))";
  
  $sql = <<< X
    SELECT * FROM

    (SELECT '' as extra, deposit AS dt, $desc AS what, SUM(amount) AS amt, COUNT(*) AS cnt,
     SUM(IF($neg, -amount, '')) AS outt,
     SUM(IF($neg, '', amount)) AS inn
     FROM txs2 WHERE amount<>0 AND bankTxId<>0 GROUP BY deposit,$desc DESC
     UNION ALL SELECT 1 AS extra, completed AS dt, IF(memo='', type, memo) AS what, amount AS amt, '' AS cnt,
     IF(amount<0, -amount, '') AS outt,
     IF(amount<0, '', amount) AS inn
     FROM r_usd2) t
    
    ORDER BY dt DESC    
X;

  $bal = db\sum('amount', 'txs2', 'bankTxId<>0') + db\sum('amount', 'r_usd2');
  $q = db\q($sql);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $dt = fmtDt($dt);
    if ($cnt) $what .= ": $cnt";
    if ($what == 'T') $what = t('transfer');
    if ($what == 'S') $what = $amt < 0 ? t('fee') : t('interest');
    $balance = $bal;
    foreach (ray('inn outt balance') as $k) if ($$k) $$k = number_format($$k, 2);
    foreach (ray('dt what outt inn balance') as $i => $k) $line[$i] = "<td class='$k'>" . $$k . '</td>';
    $class = $extra ? ' class="hiRow"' : '';
    $lines[] = "<tr$class>\n" . join("\n", $line) . '</tr>';
    $bal -= $amt;
  }
  $deposits = join("\n", nn($lines) ?: []);
  foreach (ray('Date Memo Payment Deposit Balance') as $i => $k) $line[$i] = "<th class='$k'>$k</th>";
  $header = "<tr>\n" . join("\n", $line) . '</tr>';
  $list = <<< X
    <table id="deposits">
    $header
    $deposits
    </table>
X;
  $title = t('Reconciled Dollar Pool Transactions');
  $txid = db\min('txid', 'txs2', 'bankTxId=0 AND amount<>0') ?: 999999999;
  $asof = fmtDt(db\max('deposit', 'txs2', 'txid<:txid', compact('txid')) ?: now());
  $subtext = t('These Dollar Pool transactions have been matched one-to-one with the bank\'s records, for all transfers dated on or before %asof.', compact('asof'));
  $list = item($list);
  return cgForm(compact(ray('title subtext list')));
}
