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
  
  $agtViaBoxHelp = t('||Agent is who did the transaction on your behalf.|Channel (Via) is where the transaction originated -- what type of device: Web, mobile, SMS, or Automatic.|Box is where the transaction originated -- which specific device.');

  if (is_string($args)) $args = urlq2ray($args);
  if ($mya->id == CGID) u\setDft($args['hideFees'], 1);
  extract(just('period undo download hideFees', $args, NULL)); 
  
  global $testPeriod; $testPeriod = $period ?: TX_DEFAULT_PERIOD; // remember for tests
  if ($undo) if ($result = reverseTx($undo, $sta, $args)) return $result;
  extract(just('dateRange downloadMsg url starting ending', dateRange($args, 'history/transactions')));

  $uids = $mya->jid ? [$mya->id, $mya->jid] : [$mya->id];
  if (!in($download, ' CSV QBO Combo')) return err(t('That is not an allowable download format.')); // note that $download can be empty
  require_once R_ROOT . "/classes/txhistory$download.class"; // choose the right TxHistory class
  $class = "CG\\Web\\TxHistory$download";
  $processor = new $class($uids, $url, $starting, $ending);
  be\processTxs($processor, $uids, $starting, $ending, ray('descending omitAux hideFees', !$download, TRUE, $hideFees));

  if (test() and $download) return;
  if ($download) exit();
  
  $summary = fld('markup', '', '', array('markup' => txSummary($starting, $ending)));
  $pgFactor = MIN_TX_ROWS / NORMAL_TX_ROWS;

  $transactions = $processor->getBody();

  $divider = item('<hr>');
  $title = t('Transaction History');

  [$text, $href] = strhas($url, 'hideFees=0')
  ? [str_replace('hideFees=0', 'hideFees=1', $url), t('Hide Fees')]
  : [str_replace('hideFees=1', 'hideFees=0', $url), t('Show Fees')];
  if ($mya->id == CGID) $title = item($title, '', '', suffix(buttino($text, $href)));

  extract(just('simple advanced downloadMsg', $dateRange));
  $dateRange = w\fieldSet('dateRange', compact(ray('simple advanced downloadMsg')), BARE);
  $form = compact(ray('title dateRange summary transactions'));
  
  jsx('paginate', compact('pgFactor'));
  jsx('txs', 'msg title uids', t('Reverse this transaction?'), t('Reverse'), join(',', $uids));
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
  if (is_array($args)) $args = http_build_query($args);

  if (!$xid = (int) nn($xid)) return FALSE;
  if (!$mya->lastTx(compact('xid'))) return err($mya->admRegion ? t('That is not a transaction for this account.') : t('That transaction does not exist.'));

  $url = strstr("history/transactions/$args", '&undo=', TRUE); // don't loop when we redirect

  if ($tx = db\get('*', 'txs2', compact('xid'))) { // reversing bank transfer or outer tx
    foreach (just('txid amount', $tx) as $k => $v) $tx[$k] = -$v; // reverse transfer info
    extract(just('txid amount deposit completed pid', $tx));
    if (!$mya->admReverseBankTx) {
      if ($pid) {
//        if ($deposit) return err(
      } else return err(t('To reverse a bank transfer, go to your <%a>Dashboard</a>, click "Bank", and either cancel a pending transfer or request a transfer back the other way.', '_a', atag('/dashboard')));
    }

    $DBTX = \db_transaction();
    $z = be\reverse($xid, $mya); // create an offsetting tx
    if (!$z->ok) return go($url, $msg, 'err');

    $deposit = $tx['deposit'] = !$deposit ? now() // list reversals distinctly, for easy reconciliation
    : ($deposit + 1 + db\count('txs2', 'txid<0 AND deposit BETWEEN :deposit+1 AND :deposit+:DAY_SECS/4', compact('deposit')));
    if (!$completed) $completed = $tx['completed'] = $deposit;
    $tx['xid'] = $z->subs['xid'];
    $tx['created'] = $deposit; // not now() (must agree with creation date of txs record -- see below)
    // bankTxId (effective date) can be the same as originally (0 if the original never went to bank)
    db\insert('txs2', $tx, 'txid'); // bypass acct::bank() -- nothing fancy wanted
    if (!$deposit) db\update('txs2', ray('deposit completed txid', $tx['deposit'], $completed, -$txid), 'txid'); // mark original as done too (from bank's view)
    db\update('tx_hdrs', ray('created xid', $deposit, $tx['xid']), 'xid'); // created must match deposit date, so QBO export includes it
  
    if ($pid) {
      say(t('IMPORTANT: You will need to reverse the credit card transaction also (for example in Stripe).'));
    } else $mya->update('lastBounce floor', now(), max(0, $mya->floor));
    unset($DBTX);
    
    return w\go($url, t('reversed bank tx', 'amount who', u\fmtAmt(-$amount), $mya->fullName));

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

  $starting = fmtDt($starting, 'dmY');
  $ending = fmtDt($ending, 'dmY');
  
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
