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
  $optHelp = $agtViaBoxHelp . t('||Exchanges are 1-to-1 trades of US Dollars for %RCREDITS (usually automatic).|PExchanges are daily automatic payment exchanges of %RCREDITS between company and employee. The company pays the employee in US Dollars, then the employee trades back some US Dollars 1-to-1 for %RCREDITS. The net effect is that the employee gets paid partly in US Dollars, partly in %RCREDITS.');

  extract(just('options period starting ending do xid download downloaded', $args)); 
  
  global $testPeriod; $testPeriod = nn($period) ?: TX_DEFAULT_PERIOD; // remember for tests
  if (nn($do)) if ($result = handleTx($do, $xid, $sta, $args)) return $result;
  extract(just('dateRange downloadMsg url starting ending', dateRange($args, 'history/transactions')));

//  $descending = TRUE;
//  foreach (ray('download agents channels boxes descending') as $k) if (isset($$k)) $optionSet[$k] = $$k;

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

/* /\** */
/*  * Return a form to edit the given transaction. */
/*  * @param int $xid: the transaction id */
/*  * @return the form (FALSE on error) */
/*  *\/ */
/* function txEdit($xid) { */
/*   global $mya; */
/*   $tx = $mya->lastTx('xid=:xid', compact('xid')); */
/*   if (empty($tx)) return FALSE; // bail on hack attempt */
/*   extract(just('taking toMe payer payee amount goods payerFor payeeFor created', (array)$tx)); */

/*   if (!$mya->can($taking ? B_SELL : B_BUY)) return FALSE; */

/*   $form = array(     */
/*     '#validate' => array('CG\\Web\\txEdit_validate'), */
/*     '#submit' => array('CG\\Web\\txEdit_submit'), */
/*   ); */

/*   $purpose = $toMe ? $payeeFor : $payerFor; */
/*   $orig = hidFld(serialize((array)$tx)); */
/*   $title = t('Edit Transaction'); */
/*   $to = item(r\acct($toMe ? $payer : $payee)->fullName, $toMe ? t('From:') : t('To:')); */
/*   $amount = numFld(t('Amount: $'), '', dft(number_format($amount,2))); */
/*   $goods = $mya->cttyRewardy ? radiosFld('&nbsp;', '', dft((int) $goods), ray(R_WHYS)) : hidFld($goods); */
/*   $purpose = textFld(t('For:'), '', dft(u\nonish(@$purpose) ? '' : $purpose)); */
/*   $created0 = hidFld($created); */
/*   if ($mya->cAdmin) $created = dateFld(t('Date:'), '', dft($created)); else unset($created); */
/*   $submit = t('Update'); */

/*   return cgform($form + compact(ray('title to amount goods purpose created0 created submit orig'))); */
/* } */

/* function txEdit_validate($form, &$sta) { */
/*   global $base_url; */
/*   global $mya; */
/*   extract(just('created0 created amount goods purpose orig', $sta['input'])); */
/*   extract($orig = unserialize($orig), EXTR_PREFIX_ALL, 'x'); */
/*   $dif = abs($amount - $x_amount); */
  
/*   if ($goods == FOR_GOODS and trim($purpose) == '') return say('missing purpose', 'purpose'); */
/*   if ($dif and $err = u\badAmount($amount, $mya->cAdmin ? '' : '>0')) return say($err, 'amount'); */
/*   if (isset($created) and $err = u\badDate($created, $created0 - 180 * DAY_SECS, now() + 7 * DAY_SECS)) return say($err, 'created'); */
/*   if (!$mya->cAdmin and ($x_toMe ? ($amount > $x_amount) : ($amount < $x_amount))) { */
/*     $who = r\acct($x_toMe ? $x_payer : $x_payee)->name; */
/*     $action = $x_toMe ? t('increase') : t('decrease'); */
/*     $date = u\fmtDate($x_created); */
/*     $amount = u\fmtAmt($amount); */
/*     $purpose = urlencode($purpose); */
/*     $_a = atag("/charge/who=$who&amount=$dif&goods=$goods&purpose=$purpose ($action amount of $date payment to $amount)"); */
/*     $amount = u\fmtAmt($x_amount); */
/*     return say(t('illegal amount change', compact(ray('amount action _a'))), 'amount'); */
/*   } */
/*   u\preray(compact('amount', 'created'), $sta['input']); */
/* } */

/* function txEdit_submit($form, &$sta) { */
/*   extract(just('created amount goods purpose orig', $sta['input'])); */
/*   extract($orig = unserialize(unplain($orig)), EXTR_PREFIX_ALL, 'x'); */
/*   $a = r\acct(); $id = $a->id; */
/*   $tx = r\Tx::read($x_xid); */
/*   u\EXPECT($tx, 'submitted non-tx!'); */
/*   u\EXPECT($a->cAdmin or $tx->payerId == $id or $tx->payeeId == $id, "not our xid:$x_xid id=$id"); */

/*   $forField = $x_toMe ? 'payeeFor' : 'payerFor'; */
/*   if ($purpose != $orig[$forField]) $changes[$forField] = $purpose; // $x->setField($forField, $purpose); */
/*   if ($goodsChange = ($goods != $x_goods)) $changes['goods'] = $goods; // $x->setField('goods', $goods); */
/*   if ($amtChange = ($amount != $x_amount)) $changes['amount'] = $amount; // $x->setField('amount', $amount); */
/*   if (isset($created) and $created != $x_created) $changes['created'] = $created; // $x->setField('created', $created); */
/*   if (@$changes) $x->setFields($changes); */
/*   if ($amtChange or $goodsChange) { */
/*     $agent = r\agent(); */
/*     $msg = 'tx edited' . ($goodsChange ? '|new tx goods' : '') . ($amtChange ? '|new tx amount' : ''); */
/*     $what = ray(R_WHYS)[$goods]; */
/*     $amount = u\fmtAmt($amount); */

/*     foreach ([$x_payer => $x_payerTid, $x_payee => $x_payeeTid] as $uid => $tid) { */
/*       $who = $uid == $agent->id ? t('you') : $agent->fullName; */
/*       r\notify($uid, $msg, compact(ray('who tid amount what'))); */
/*     } */
/*   } */
/*   return w\go('history', 'info saved'); */
/* } */

/**
 * Handle deletion of the given transaction, when the user clicks the delete button by it.
 */
function handleTx($do, $xid, &$sta, $args) {
  global $mya;
  $url = str_replace('&do=', '&did=', "history/transactions/$args"); // don't do it again

  if ($mya->superAdmin and $tx = db\get('*', 'r_usd', compact('xid'))) { // superAdmin reversing bank transfer
    u\EXPECT($do == 'no', 'trying to edit bank transfer');
    foreach (just('txid amount tid', $tx) as $k => $v) $tx[$k] = -$v; // reverse transfer info
    extract(just('txid amount deposit completed', $tx));
    u\EXPECT($completed, 'incomplete bank tx should not be on list: ' . $xid);

    $DBTX = \db_transaction();
    list ($ok, $msg, $info) = be\reverse($xid); // create an offsetting tx
    $tx['deposit'] = now();
    $tx['xid'] = $info['xid'];
    db\insert('r_usd', $tx, 'txid'); // bypass acct::bank() -- nothing fancy wanted
    if (!$deposit) db\update('r_usd', ray('deposit txid', now(), $txid), 'txid'); // mark original as done too (from bank's view)
    unset($DBTX);
    
    return w\go($url, t('Reversed %amount bank transfer for %who.', 'amount who', u\fmtAmt(-$amount), $mya->fullName));
  }
  
  if (!$xid = (int) nn($xid)) return FALSE;
  if ($do == 'edit') return txEdit($xid);
  if ($do == 'ok' or $do == 'no') {
    txOKorNO($do, $xid);
    return w\go($url); // back to the normal form
  }
  return FALSE;
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
 */
function txOKorNO($task, $xid) {
  $a = r\acct();
  $uids = empty($a->jid) ? [$a->uid] : [$a->uid, $a->jid];
  $result = r\Tx::lastTx($uids, 'xid=:xid', compact('xid'));
  if (empty($result)) return FALSE;
  list ($ok, $message, $args) = be\undoTx($xid); // X (reverse it)
  say($message, $args);
}
