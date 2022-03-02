<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

/**
 * Create a PDF of all relevant checks.
 * @param string $way: IN (from member bank account to CG) or OUT (from CG to member bank account)
 * @param int $date: deposit/check date
 * @param bool $reprint: UNUSED whether to include a reprint of the previous deposit's checks
 * @param int $previous: UNUSED date of previous deposit
 * @param bool $mark: whether to set the deposit date for each relevant transfer, in txs2_bank
 * @param int $limit: maximum number of checks to print (defaults to 42, the most we can mail for a 3oz stamp)
 */
 define('CHECK_LIMIT', 42); // default maximum numbere of checks to print at a time
 
function printChecks($args) {
  extract(just('way date mark limit', $args, NULL));
  if (!$way or is_null($date)) return w\softErr('You cannot print checks without specifying the "deposit" date and way (In, Out, or Bad).');
  
  include_once R_ROOT . '/pdf.class';
  $ck = new r\Pdf();
  $ck->setPrintHeader(FALSE);
  $ck->setPrintFooter(FALSE);
  $ck->setPageUnit('in'); // measure everything in inches
  //  $ck->SetMargins(0, 0, 0); // left/top/right margin
  $ck->SetAutoPageBreak(FALSE);

  $ways = ['IN'=>'(amount>0 AND txid>0)', 'OUT'=>'(amount<0 AND txid>0)', 'BAD'=>'txid<0', 'VERIFY'=>'amount=0'];
  $where = $ways[strtoupper($way)];
  $sql = "SELECT xid,txid,created,payee,amount,deposit,bankAccount FROM txs2_bank WHERE deposit=:date AND created<=:NOW AND $where ORDER BY xid";
  $subs = compact('date');
  u\setDft($limit, CHECK_LIMIT);
  if (is_numeric($limit)) $sql .= " LIMIT $limit"; // ' LIMIT :limit' fails
  $q = db\q($sql, $subs);
  $pos = 3; // before first position of 3, on 3-up check paper
  $count = 0;

  while ($tx = $q->fetchAssoc()) {
    if ($pos == 3) {$pos = 1; $ck->AddPage();} else $pos++;
    printCheck($ck, $pos, $tx, $mark);
    $count++;
  }
  $date = fmtDt($date, 'YYYY-MM-dd');

  if (test()) $ck->Close(); else $ck->Output(PROJECT . "-deposit-checks-$way-$date.pdf", 'I'); //Close and output 
  //  say($count . t(' checks printed'));
}

/**
 * Print a check
 * @param Pdf $ck: the Check object to place the check in.
 * @param int $pos: what position on the current page (1, 2, or 3)
 * @param assoc $tx: relevant fields from the txs2_bank record
 * @param bool $mark: whether to set the deposit date for each relevant transfer, in txs2_bank
 */
function printCheck(&$ck, $pos, $tx, $mark) {
  extract(just('xid txid created payee amount deposit bankAccount', $tx));
  $b = r\bankData();
  $escrow = 'USkk' . $b->routing . ($amount > 0 ? $b->acctIn : $b->acctOut);

  $a = r\acct($payee);
  $bankAccount = u\decry('V', $bankAccount);
  if ($bankAccount != $a->bankAccount) {
    r\tellAdmin(t('bank account misMatch in deposit (xid=%xid): %b1 (now), %b2 (deposit)', 'xid b1 b2', $xid, $a->bankAccount, $bankAccount));
    if ($a->bankAccount) {
      list ($b2, $bankAccount) = [$bankAccount, $a->bankAccount];
      if (!$b2) db\update('txs2_bank', compact(ray('txid bankAccount')), 'txid'); // fix it (else save for analysis)
    }
  } elseif (!$bankAccount) return r\tellAdmin(t('Empty bank account in deposit xid=%xid', compact('xid')));
  
  list ($fromName, $addr, $phone, $fromAcct, $toName, $toAcct) = $amount > 0 // which way
    //    ? [$a->legalNameDpy, $a->postalAddr, $a->phone, $bankAccount, PROJECT, $escrow] // from bank to escrow
    ? [$a->legalNameDpy, $a->postalAddr, $a->phone, $bankAccount, BANK_DBA, $escrow] // from bank to escrow
    : [PROJECT . ' (' . CGF_LEGALNAME . ')', CGF_POSTALADDR, CGF_PHONE, $escrow, $a->legalNameDpy, $bankAccount]; //e2b
  $memo = $amount > 0 ? t('for %RCREDITS') : t('for DEPOSIT to account #') . substr($toAcct, 4 + 9);

  list ($signature, $belowLine) = $amount > 0 
    ? [t('Pre-authorized Check'), t('No Signature Required')] 
    : ['', SIGNATORY];
  
  list ($W, $H) = array($ck->pageW, $ck->pageH);
  $lineH = $ck->lineH; // standard line height in inches
  $m = .25; // left/right margins
  $mt = ($pos - 1) * CHECK_HEIGHT + $m; // top margin

  $phone = u\fmtPhone($phone);
  $routing = substr($fromAcct, 4, 9);
  $account = substr($fromAcct, 4 + 9);

  $bankInfo = db\get('*', 'r_banks', 'route=:routing', compact('routing'));
  if (!$bankInfo) return r\tellAdmin(t('Skipped transaction #%xid for %nm (no bank info)', 'xid nm', $xid, $a->fullName));

  extract($bankInfo, EXTR_PREFIX_ALL, 'b');
  if (strlen($b_zip) > 5) $b_zip = substr($b_zip, 0, 5) . '-' . substr($b_zip, 5);
  if (!$prefix = db\get('id', 'r_transit', 'location=:location', ray('location', "$b_city, $b_state"))) {
    $prefix = db\get('id', 'r_transit', 'location=:b_state', compact('b_state'));
    u\EXPECT((bool) $prefix, 'no transit prefix');
  }
  $transit = $prefix . '-' . substr($routing, 4, 4) . '/' . substr($routing, 0, 4);
  
  $amount = abs($amount);
  $dollars = floor($amount);
  $cents = round(100 * ($amount - $dollars)) ?: t('NO');

  $ck->say("<b>$fromName</b><br><small>$addr<br>$phone</small>", $m, $y = $mt, $W - $m);
  $ck->say("$transit<br>$account", $W / 2, '', $W / 4, '', '8', 'C');
  $ck->say("<b>$xid</b>", -$W / 4, '', -$W / 4 - $m, '', '', 'R');

  $ck->say('DATE:', 6.7, $y += .4, '', .25, '8');
  $ck->say(fmtDt($deposit ?: now()), 7.1, '', '', .25);

  $ck->say('Pay to the<br>Order of', $m, $y += .4, '', 2 * $lineH, '8;CAPS');
  $ck->say("  $toName", $m + .7, $y + $lineH, 5.9, $lineH, '', '', 'BR');
  $ck->say('$ ' . number_format($amount, 2), 7, '');
  
  $ck->say(u\n2english($dollars) . " and $cents/100", $m, $y += .65, 6.9, $lineH, '', '', 'B');
  $ck->say('DOLLARS', $m + 6.9, '', '', $lineH, '8');
  //  $ck->Image(R_ROOT . '/images/icons/padlock.gif', $W - $m - .5, $y, $lineH, $lineH);
  
  $ck->say("<b>$b_name</b><br><small>$b_address<br>$b_city, $b_state $b_zip</small>", $m + 1, $y += .4, '', '', '9');
  
  $ck->say($signature, $W - $W / 3, $y + .35, $W / 3 - $m, $lineH, 'CAPS', 'C', 'B'); // $y+ not +=
  $ck->say($belowLine, '', $y + .35 + $lineH, $W / 3 - $m, $lineH, '8;CAPS', 'C');
  //  if (!$signature) $ck->Image(R_ROOT . '/images/checksig.png', $W - $W / 3, $y, $W / 3 - $m, 5 * $lineH);
  
  $ck->say('MEMO:', $m, $y += .5, '', $lineH, '8');
  $ck->say($memo, $m + .5, '');

  // clear band must be at least .625" high, at least 0.1875" from bottom (so leave more). Micr should be 12pt
  //  if ($a->id != 26742000000042) {
  //    $ck->say("A{$routing}A {$account}C $xid", $m, $mt - $m + CHECK_HEIGHT - .25 - $lineH * (12 / PDF_FONT_SIZE), '', '', 'GnuMICR 12');
  //  } else 
  $account = str_replace('-', 'D', $account); // show dashes properly
  $ck->say("C000{$xid}C A{$routing}A {$account}C", $m, $mt - $m + CHECK_HEIGHT - .25 - $lineH * (12 / PDF_FONT_SIZE), '', '', 'GnuMICR;12');
  //  $ck->say('', 0, $mt - $m + CHECK_HEIGHT, $W, 1, '', '', 'T'); // comment this out
  
  if (!$deposit and $mark) db\update('txs2_bank', ray('deposit txid', today(), $txid), 'txid');
}
