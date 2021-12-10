<?php
namespace CG\Web;
use CG\Web as w;
use CG\Backend as be;
use CG\Util as u;
use CG as r;
use CG\DB as db;
use CG\Txs as x;

include_once __DIR__ . '/../cg-txs.inc';

/**
 * List Transactions
 */
function formTxs($form, &$sta, $args = '') {
  global $mya;
  global $testPeriod;
  global $agtViaBoxHelp, $base_url;

  w\downloadCheck('history/transactions');
  
  $agtViaBoxHelp = t('||Agent is who did the transaction on your behalf.|Channel (Via) is where the transaction originated -- what type of device: Web, Smartphone, SMS, or Automatic.|Box is where the transaction originated -- which specific device.');

  extract(just('options period starting ending undo download downloaded', $args)); 
  
  global $testPeriod; $testPeriod = nn($period) ?: TX_DEFAULT_PERIOD; // remember for tests
  if (nn($undo)) if ($result = reverseTx($undo, $sta, $args)) return $result;
  extract(just('dateRange downloadMsg url starting ending', dateRange($args, 'history/transactions')));

  $summary = fld('markup', '', '', array('markup' => txSummary($starting, $ending, $run)));
  
  $uids = ($mya->jid > 0) ? [ $mya->id, $mya->jid ] : [ $mya->id ];
  if (nn($download)) {
    global $mya;
    $dt1 = strftime('%Y%m%d', $starting);
    $dt2 = strftime('%Y%m%d', $ending);
    u\csvStart(($mya->cAdmin ? $mya->mainQid : PROJECT_ID) . "$dt1-$dt2.csv");
    require_once __DIR__ . '/txhistorydownload.class';
    $processor = new TxHistoryDownload($uids, "$url");
  } else {
    require_once __DIR__ . '/txhistory.class';
    $processor = new TxHistory($uids, "$url");
  }

  be\processTxs($processor, $uids, $starting, $ending, TRUE);

  if (test() and nn($download)) return;
  if (nn($download)) exit();
  
  $pgFactor = MIN_TX_ROWS / NORMAL_TX_ROWS;

  $links = <<<EOF
<div id="txs-links" class="row">
  <div class="showMore col-xs-3">
    <a title="Show more transactions per page"><span class="glyphicon glyphicon-plus"></span>Show more</a>
  </div>
  <div class="dates col-xs-2">
    <a title="Select dates to show"><span class="glyphicon glyphicon-calendar"></span>Dates</a>
  </div>
  <div class="download col-xs-3">
    <a title="Select dates and download as CSV file"><span class="glyphicon glyphicon-download-alt"></span>Download</a>
  </div>
  <div class="totals col-xs-2">
    <a data-toggle="modal" data-target="#txs-totals" title="Show totals for the selected period"><span class="glyphicon glyphicon-usd"></span>Totals</a>
  </div>
  <div class="nav col-xs-1">
    <a class="prevPage" title="Previous Page"><span class="glyphicon glyphicon-triangle-left"></span></a>
    <a class="nextPage" title="Next Page"><span class="glyphicon glyphicon-triangle-right"></span></a>
  </div>
</div>
EOF;

  $transactions = $processor->getBody();

  $divider = item('<hr>');
  $title = t('Transaction History');
  extract(just('simple advanced downloadMsg', $dateRange));
  $dateRange = w\fieldSet('dateRange', compact(ray('simple advanced downloadMsg')), BARE);
  $form = compact(ray('title dateRange summary transactions'));
  
  jsx('paginate', compact('pgFactor'));
  jsx('reverse-tx', 'msg title', t('Reverse this transaction?'), t('Reverse'));
  /* jsx('advanced-dates', compact('pgFactor')); */
  return cgform($form);
}

function formTxs_validate($form, &$sta) {w\dateRangeValidate($sta);}

function formTxs_submit($form, &$sta) {
//  if (confirming_s($sta)) return;
  return w\dateRangeRedirect('history/transactions', $sta);
}

/**
 * Reverse the given transaction, when the user clicks the X button by it.
 */
function reverseTx($xid, &$sta, $args) {
  global $mya;

  if (!$xid = (int) nn($xid)) return FALSE;
  if (!$mya->lastTx(compact('xid'))) return err($mya->admin ? t('That is not a transaction for this account.') : t('That transaction does not exist.'));

  $url = strstr("history/transactions/$args", '&undo=', TRUE); // don't loop when we redirect

  if ($tx = db\get('*', 'txs2', compact('xid'))) { // reversing bank transfer or outer tx
    foreach (just('txid amount', $tx) as $k => $v) $tx[$k] = -$v; // reverse transfer info
    extract(just('txid amount deposit completed pid', $tx));
    if (!$mya->superAdmin) {
      if ($pid) {
//        if ($deposit) return err(
      } else return err(t('To reverse a bank transfer, go to your <%a>Dashboard</a>, click "Bank", and either cancel a pending transfer or request a transfer back the other way.', '_a', atag('/dashboard')));
    }

    $DBTX = \db_transaction();
    $z = be\reverse($xid, $mya); // create an offsetting tx
    if (!$z->ok) return go($url, $msg, 'err');
    $tx['deposit'] = now();
    $tx['xid'] = $z->subs['xid'];
    db\insert('txs2', $tx, 'txid'); // bypass acct::bank() -- nothing fancy wanted
    if (!$deposit) db\update('txs2', ray('deposit txid', now(), $txid), 'txid'); // mark original as done too (from bank's view)
    $mya->update('lastBounce floor', now(), max(0, $mya->floor));
    unset($DBTX);
    
    return w\go($url, t('Reversed %amount bank transfer for %who.', 'amount who', u\fmtAmt(-$amount), $mya->fullName));
  } else { // reversing normal transaction
    $z = be\undoTx($mya, $xid); // X (reverse it)
    return w\go($url, $z->msg, $z->ok ? '' : 'err'); // back to the normal form
  }
}

/**
 * Return the transaction summary table
 * @param int $starting: starting time (the start of some day)
 * @param int $ending: ending time (the end of some day)
 * @param numeric $endBalance: (RETURNED) closing balance at end of period
 * @return html for summary
 */
function txSummary($starting, $ending, &$endBalance = '') {
  global $mya;
  $myid = $mya->id;
  $jid = $mya->jid;
  $uids = empty($jid) ? [$myid] : [$myid, $jid];

  $endingNow = ($ending > now());
  $previous_ending = $starting - 1;
  $nowInfo = be\creditInfo($uids, $ending);
  extract($nowInfo, EXTR_PREFIX_ALL, 'n');
  $endBalance = $n_balance;
  
  $oldInfo = be\creditInfo($uids, $starting);
  extract($oldInfo, EXTR_PREFIX_ALL, 'o');
  $startBalance = round($o_balance, 2);

  $dif = [];
  foreach (ray('in out fromBank toBank') as $key) $dif[$key] = $nowInfo[$key] - $oldInfo[$key];
  extract($dif, EXTR_PREFIX_ALL, 'd');
  /* $p_committed = $mya->committed; // this will include all pending commitments */
  
  /* $fields = 'old bank in out new'; */
  /* $values = array($startBalance, $d_bank, $d_in, $d_out, $endBalance); // both */
  /* $values = array_combine(ray($fields), $values); */

  /* $p_in = $p_out = 0; // temporary until we include unpaid invoice data here */
  /* $p_balance = $p_bank - $p_committed; */
  /* $values += compact(ray('p_in p_bank p_out p_committed p_balance d_roundups')); */

  /* foreach ($values as $key => $value) $$key = number_format($value, 2); */

  $starting = u\fmtDate($starting);
  $ending = u\fmtDate($ending);
  
  $classes = u\prefixValues('txsum-', ray('label sign1 amount sign2 savings other'));
  
  $PRIMARY = w\popHelp(t('Balance'), t('Your balance is what you use for spending and for receiving payments.'));
  $STARTING = w\popHelp(t('Starting Balance'), t('Your %PROJECT Account balance at the start of this time period'));
  $BANKIN = w\popHelp(t('From Bank'), t('How much was transferred from your bank account to your %PROJECT Account'));
  $BANKOUT = w\popHelp(t('To Bank'), t('How much was transferred from your %PROJECT Account to your bank account'));
  $IN = w\popHelp(t('Funds Received'), t('How much %RCREDITS you received, in this period'));
  $OUT = w\popHelp(t('Funds Paid Out*'), t('How much %RCREDITS you paid out, in this period'));
  $CREDIT_LINE = w\popHelp(t('Credit Line+'), (t('Credit line increases you received in this period')));
  $ENDING = w\popHelp(t('Ending Balance'), t('Your %PROJECT Account balance at the end of this time period'));
  $SAVINGS = w\popHelp(t('Credit Reserve'), t('Amount in your Credit Reserve, to spend in a pinch (once your %PROJECT community declares itself stable, this additional amount is yours to keep)'));
  $PENDING = w\popHelp(t('pending'), t('Pending transfers from your bank account to your %PROJECT Account'));
  /* if ($p_bank) $p_bank = str_replace('+ -', '- ', $endingNow ? "+ $p_bank $PENDING" : ''); */
  
  /* $p_balance = u\starts($p_balance, '-') ? '- ' . substr($p_balance, 1) : "+ $p_balance"; */
  foreach (ray('o_balance d_fromBank d_toBank d_in d_out n_balance') as $k) $$k = u\fmtAmt($$k, '$');

  $n_pendingBank = $endingNow ? (($n_pendingBank < 0 ? '- ' : '+ ') . u\fmtAmt($n_pendingBank, '') . " $PENDING") : '';

/*  if (!$endingNow) {
    $n_pendingBank = '';
  } elseif ($n_pendingBank > 0) {
    $n_pendingBank = '+ ' . u\fmtAmt($n_pendingBank, '') . " $PENDING";
  } elseif ($n_pendingBank == 0) {
    $n_pendingBank = '';
  } else {  // $n_pendingBank < 0
    $n_pendingBank = '- ' . u\fmtAmt(-$n_pendingBank, '') . " $PENDING";
  }
  */
  
  $rows = [
           ray("||$PRIMARY|"),
           ray("$STARTING||$o_balance|$starting"),
           ray("$BANKIN|+|$d_fromBank|$n_pendingBank"),
           ray("$BANKOUT|-|$d_toBank|"),
           ray("$IN|+|$d_in|"),
           ray("$OUT|-|$d_out|"),
           ray("$ENDING||$n_balance|$ending"),
  ];  
  
  $attributes = ['id' => 'txsum'];
  $header = $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = [];
  $caption = $sticky = $empty = '';
  $txsum = \theme_table(compact(ray('header rows attributes caption colgroups sticky empty')));
  return modal('txs-totals', t('Period Totals'), '', $txsum, 'X');
}

/**
 * Handle click 'OK' or 'NO' (approving or undoing a transaction)
 * Currently only NO.
 */
function txOKorNO($task, $xid) {
  global $mya;
}
