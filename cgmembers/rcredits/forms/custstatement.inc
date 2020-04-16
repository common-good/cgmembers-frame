<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Txs as x;

include_once __DIR__ . '/../cg-txs.inc';
include_once __DIR__ . '/../statement.class';

/**
 * Generate an account statement for the given customer or vendor.
 * Call with /history/customer-statement/cust=QID?way=WAY
 * where WAY is either IN or OUT
 */
function custStatement($args = []) {
  $purposeLenMax = 60; // maximum number of characters in purpose field, not including added "..."

  if (!$mya = r\acct()) return w\go('', t('You must first sign in.'), 'ERR');
  extract(just('cust period way', $args, [NULL, NULL, 'OUT']));

  $a = is_null($cust) ? null : r\acct($cust);
  if (is_null($a)) return w\softErr(t('No customer was specified.'));
//  if (!@$period) return w\softErr(t('No period was specified.'));

//  list ($start, $end, $period) = statementMonth($period, $mya->fyEnd ?: '12/31'); // not used yet
  $custUids = empty($a->jid) ? [ $a->id ] : [ $a->id, $a->jid ];
  $rows = ($way == 'IN') ? getCustTxs([$mya->id], $custUids, 0, now())
                      : getCustTxs($custUids, [$mya->id], 0, now());
  if (count($rows) == 0) return w\softErr(t('That is not a %thing of yours.', 'thing', $way == 'IN' ? t('customer') : t('vendor')));

  $pdf = new r\Statement($mya);

  $size = $pdf->getFontSizePt();
  $m = 5; // left/right margin size
  $pdf->SetMargins($m, PDF_MARGIN_TOP, $m);
  $pdf->SetAutoPageBreak(TRUE, 15); // leave space at bottom
  $pdf->setLanguageArray([]); // (eg "page" in footer)
  $pdf->AddPage();
  $pageWidth = $pdf->getPageWidth();

  //$pdf->writeHTMLCell(40, 10, 160, 84.5, '<div style="font-size:44px;">CommonGood.earth</div>');
  list ($address, $city, $state, $zip) = u\parseAddr($a->postalAddr);
  $pdf->Cell(0, 0, $a->j_fullName . t(': Account ') . $a->mainQid, '', 2);
  $pdf->Cell(0, 0, $address, '', 2);
  $pdf->Cell(0, 0, "$city, $state $zip", '', 2);
  $pdf->newLine();

  $numFields = ray('invoiced paid balance');

  $pdf->setFontSize($size);
  $pdf->barHead('STATEMENT');
  $lastHead = 'Balance';
  $fields = ray("Date//25/ /space/1/ /tid/20/C Description/purpose/83/ /space/1/ Invoiced//20/R /space/1/ Paid//20/R /space/10/ Balance/balance/20/R");
  $pdf->colHeads($fields);
  $pdf->setFontSize(.6 * $size); // a bit of space below field headers
  $pdf->newLine();
  $pdf->setFontSize(.9 * $size);

  $bal = 0;
  $row0 = ['purpose' => t('Opening balance')];
  $space = '';

  foreach ($rows as $row) {
    if (!@$classes) $classes = array_keys($row);
//    extract(just('tid date name amount purpose reward', array_combine($classes, $row)));
    extract(just('tid created purpose invoiced paid', $row));
    $date = nn($created) ? u\fmtDate($created, TRUE) : '';
    if ($row != $row0) $bal += (float) $invoiced - (float) $paid; // nn($invoiced, 0) + nn($paid, 0) fails (PHP bug)
    $balance = $bal;
    $x = $pdf->GetX();

    foreach ($fields as $i => $one) {
      list ($head, $fldName, $width, $align) = explode('/', $one);
      if (!$fldName) $fldName = mb_strtolower($head);
      $debit = ($align == 'R' and is_numeric($n = @$$fldName) and $n < 0);
      if ($debit) $pdf->SetTextColor(128, 0, 0); else $pdf->SetTextColor();
      if (in($fldName, $numFields) and isset($$fldName)) {
        $$fldName = ($$fldName == 0 and $fldName != 'balance') ? '' : number_format($$fldName ?: 0, 2);
      }
      if ($fldName == 'purpose') { // handle this field separately, last (see below)
        $purposeX = $pdf->GetX();
        $purposeWidth = $width;
      } else {
        $pdf->setX($x);
        $pdf->Cell($width, 0, nn($$fldName), '', 0, $align, '', '', 1);
      }
      $x += $width;
    }

    // do the (potentially multiline) purpose last, so we don't mess up the pagination
    $pdf->SetTextColor();
    $pdf->SetX($purposeX);
    $pdf->MultiCell($purposeWidth, 0, $purpose, 0, 'L', '', 1);
    
//    $pdf->SetY($nextY);

    if ($row == $row0) $pdf->newLine(); // space after opening balance
  }

  $pdf->newLine();
  
  $closing = @$classes
  ? ($bal ? t('Total due: $') . $balance : t('No payment is due.'))
  : t('There are no transactions this month.');

  $pdf->SetFillColorArray([255, 255, 0]); // yellow
  $pdf->Cell($pageWidth - 2 * $m, 0, "$closing", '', 1 , 'L', TRUE);
  $pdf->finish(t('Statement-%acct-%date.pdf', 'acct date', $a->mainQid, strftime('%m-%d-%Y', now()))); //Close and output PDF document
}

/**
 * Generates an array of transactions between two accounts, with added data, for the given period.
 * @param array $aUids: the uids for member a (normally a business)
 * @param array $bUids: the uids for member b
 * @param unixtime $starting: the starting date/time for the period
 * @param unixtime $ending: the ending date/time for the period, e.g., $starting plus one day
 */
function getCustTxs($aUids, $bUids, $starting, $ending) {
  u\EXPECT('array array int int', $aUids, $bUids, $starting, $ending);
  // Note that we show the company's tx description, not the customer's  
  
  $sql = <<< X
    SELECT DISTINCT CONCAT('tx #', xid) AS tid,
      IF(uid1 IN (:aUids), -amt, amt) AS paid,
      '' AS invoiced, created, 
      (d.status=:DS_OPEN) AS disputed, IF(uid1 IN (:aUids), for1, for2) AS `purpose`
    FROM txs_prime LEFT JOIN tx_disputes d USING(xid)
    WHERE (uid1 IN (:aUids) AND uid2 IN (:bUids))
      OR (uid2 IN (:aUids) AND uid1 IN (:bUids))
    UNION ALL
    SELECT CONCAT('inv #', nvid) AS tid, '' AS paid, IF (payee IN (:aUids), amount, -amount) AS invoiced,
      inv.created AS created, :IS_DISPUTED AS disputed, inv.`purpose` AS `purpose`
    FROM r_invoices inv
    WHERE (
      (payer IN (:aUids) AND payee IN (:bUids)) 
        OR (payee IN (:aUids) AND payer IN (:bUids))
       ) AND status>:TX_DENIED
    ORDER BY created, paid<>0
X;

  $results = db\q($sql, ['aUids' => $aUids, 'bUids' => $bUids]);
  u\EXPECT($results, "Failure reading transactions between users " . pr($aUids) . " and " . pr($bUids));

  $txs = [];  // an array of arrays of entries
  $entries = [];
  $myBalance = 0.00; //balanceAsOf($aUids[0], $starting);
  $entries[] = ['created'=>'', 'tid'=>'', 'purpose'=>t('Opening balance'), 'invoiced'=>'', 'paid'=>'', 'balance'=>$myBalance];
  foreach ($results as $result) {
    $entry = (array)$result;
    if (!empty($entry['paid'])) { $myBalance -= $entry['paid']; }
    $entry['balance'] = $myBalance;
    $entries[] = $entry;
  }

  return $entries;
}
