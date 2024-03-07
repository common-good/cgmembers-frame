<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

include_once R_ROOT . '/admin/admin.inc';
include_once R_ROOT . '/admin/admin-web.inc';

/**
 * Offer to print checks or a deposit slip for Dollar Pool transfers to or from member bank accounts.
 */
function formDeposits($form, &$sta, $args = NULL) {
  global $mya, $base_url;
  
  extract(just('reverse', $args, NULL));
  if ($reverse) {
    if ($reverse < today()) {
      err(t('That transfer batch is too old to reverse (before today).'));
    } else {
      $q = db\q('UPDATE tx_hdrs h JOIN txs2 x2 USING (xid) SET h.flags=(h.flags & ~(1<<:B_QBOK)), x2.deposit=0, x2.bankTxId=NULL WHERE x2.deposit=:reverse', compact('reverse'));
      $cnt = $q->rowCount();
      say(t('%cnt transfers canceled.', compact('cnt')), $cnt ? NULL : 'err');
      return go('/sadmin/deposits'); // make sure we don't accidentally reverse twice
    }
  }
  
  $title = t('Bank Transfers');

  $fields = <<<EOF
    SUM(IF(amount<0, amount, 0)) AS amtOut,
    SUM(IF(amount>0, amount, 0)) AS amtIn,
    SUM(IF(amount<0, 1, 0)) AS countOut,
    SUM(IF(amount>0, 1, 0)) AS countIn
EOF;
  $res = db\rows($fields, 'txs2', "deposit=0 AND amount<>0 AND created<=:NOW"); // fails for db\get
  //  if ($res[0]) {
  extract((array) $res[0]);
  foreach (ray('amtIn amtOut') as $k) $$k = u\fmtAmt($$k);
  $subtext = t('<b>New IN:</b> &nbsp;%countIn (%amtIn)<br><b>New OUT:</b> %countOut (%amtOut)', compact(ray('countIn amtIn countOut amtOut')));
  
  if ($mya->admAch) {
    $btns = btn("$base_url/sadmin/achs/date=0&mark=1", t('Download ACH File'));
    $last = db\max('deposit', 'txs2', 'deposit<>0');
    if ($last >= today()) $btns .= ' ' . lnk('/sadmin/deposits/reverse=' . $last, t('UNDO', 'dt', fmtDt($last)), clas('undo'));
    $go = item($btns);
  } else $go = NULL;
  
  $deposits = item(deposits(), ' ');
  w\jsx('deposits', 'msg', t('Are you sure you want to undo the most recent ACH batch (from today)? If the bank already accepted it, DON\'T DO IT!!!'));

  return cgform(compact(ray('title subtext go deposits')));
}

/**
 * Return a table of deposits.
 * for example a\deposits(0, TRUE); (to help with reconciliation)
 */
function deposits($starting = 0, $showTxIds = FALSE) {
  global $base_url;
  $sql = <<<EOF
    SELECT SUM(IF(amount>0 AND txid>0, amount, 0)) AS amtIn, SUM(IF(amount>0 AND txid>0, 1, 0)) AS countIn,
      SUM(IF(amount<0 AND txid>0, amount, 0)) AS amtOut, SUM(IF(amount<0 AND txid>0, 1, 0)) AS countOut,
      SUM(IF(txid<0, amount, 0)) AS amtBad, SUM(IF(txid<0, 1, 0)) AS countBad,
      SUM(amount) AS amtNet, SUM(1) AS countNet,
      deposit, IF(txid<0, 'bad', IF(amount>0, 'in', 'out')) AS type
    FROM txs2 WHERE amount<>0 AND created<=:NOW AND created>=:starting
    GROUP BY deposit ORDER BY deposit<>0, deposit DESC, amount<0
EOF;

  $ways = ray('Net In Out Bad', t('Net'), t('IN'), t('OUT'), t('BAD')); // translate, don't use strtoupper
  $q = db\q($sql, compact('starting')); // include today's potential deposit at the top

  while ($row = $q->fetchAssoc()) {
    extract($row);
    $firstline = TRUE;
    foreach ($ways as $way => $wayDesc) {
      list ($total, $count) = [u\fmtAmt(${"amt$way"}, ''), number_format(${"count$way"})];
      if (!$count) continue;
      if ($firstline) {
        if ($way == 'Net' and in($amtNet, [$amtIn, $amtOut, $amtBad])) continue; // use first line for Net only if there's more than one way
        $dateDpy = $deposit == 1 ? t('<b>ANCIENT</b>') : ($deposit ? fmtDt($deposit) : t('<b>TODAY</b>'));
        $countSum = $way == 'Bad' ? $count : ($countIn + $countOut);
        $details = w\btn("/sadmin/deposit-details/date=$deposit&total=$total&count=$countSum", t('details'));
        if ($deposit) { // don't risk printing checks or sending ACHes to bank for unprocessed transfers
          $details .= ' ' . w\btn("/sadmin/achs/date=$deposit&mark=0", t('ACH'));
          $details .= ' ' . w\btn("/sadmin/checks/way=In&date=$deposit&mark=0&limit=$count", t('checks'));
          $xref = $deposit;
        } else $xref = '';

      } else $dateDpy = $details = $xref = '';

      if ($showTxIds) {
        $where = $type == 'bad' ? 'txid<0' : ('txid>0 AND amount' . ($type == 'in' ? '>0' : '<0'));
        $baids = db\col('bankTxId', 'txs2_bank', "deposit=:deposit AND $where", compact('deposit'));
        foreach ($baids as $i => $baid) $baids[$i] = btxid($baid);
        $bankTxIds = '<td>' . join(', ', $baids) . '</td>';
      } else {$bankTxIds = '';}

      $deposits[] =<<<EOF
<tr>
  <td>$dateDpy</td>
  <td>$wayDesc</td>
  <td>$count</td>
  <td>$total</td>
  <td>$details</td>
  <td>$xref</td>
  $bankTxIds
</tr>
EOF;
      $firstline = FALSE;
    }
  }
  $deposits = join("\n", nn($deposits) ?: []);
  $header = t('<th>Transfers</th><th></th><th>Count</th><th>Total</th><th></th><th>cgFund#</th>');
  $header .= $showTxIds ? t('<th>Bank Tx IDs</th>') : '';
  return <<<EOF
<table id="deposits">
<tr>$header</tr>
$deposits
</table>
EOF;

}
