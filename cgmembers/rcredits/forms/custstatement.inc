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

  if (!$mya = r\acct()) return w\softErr(t('You must first sign in.'));
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
  $fields = ray(t("Date/date/25/ /space/1/ /tid/20/C Description/description/83/ /space/1/ Invoiced/invoiced/20/R /space/1/ Credits/paid/20/R /space/10/ Balance/balance/20/R"));
  $pdf->colHeads($fields);
  $pdf->setFontSize(.6 * $size); // a bit of space below field headers
  $pdf->newLine();
  $pdf->setFontSize(.9 * $size);

  $bal = 0;
  $row0 = ['description' => t('Opening balance')];
  $space = '';

  foreach ($rows as $row) {
    if (!nn($classes)) $classes = array_keys($row);
    extract(just('tid created description invoiced paid', $row, NULL));
    $date = nn($created) ? u\fmtDate($created, TRUE) : '';
    if ($row != $row0) $bal += (float) $invoiced - (float) $paid; // nn($invoiced, 0) + nn($paid, 0) fails (PHP bug)
    $balance = $bal;
    $x = $pdf->GetX();

    foreach ($fields as $i => $one) {
      list ($head, $fldName, $width, $align) = explode('/', $one);
      if (!$fldName) $fldName = mb_strtolower($head);
      $debit = ($align == 'R' and is_numeric($n = nn($$fldName)) and $n < 0);
      if ($debit) $pdf->SetTextColor(128, 0, 0); else $pdf->SetTextColor();
      if (in($fldName, $numFields) and isset($$fldName)) {
        $$fldName = ($$fldName == 0 and $fldName != 'balance') ? '' : number_format($$fldName ?: 0, 2);
      }
      if ($fldName == 'description') { // handle this field separately, last (see below)
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
    $pdf->MultiCell($purposeWidth, 0, $description, 0, 'L', '', 1);
    
//    $pdf->SetY($nextY);

    if ($row == $row0) $pdf->newLine(); // space after opening balance
  }

  $pdf->newLine();
  
  $closing = nn($classes)
  ? ($bal ? t('Account Balance: $') . $balance : t('No payment is due.'))
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
  $reverseText = t('reverses tx #');
  $special = be\specialTxFlds();
  
  $sql = <<< X
    SELECT DISTINCT CONCAT('tx #', xid) AS tid,
      IF(uid1 IN (:aUids), -amt, amt) AS paid, $special, :E_PRIME AS type,
      '' AS invoiced, t.created, (d.status=:DS_OPEN) AS disputed,
      IF(t.reversesXid, CONCAT('($reverseText', t.reversesXid, ')'), IF(uid1 IN (:aUids), for1, for2)) AS description
    FROM txs_prime t LEFT JOIN tx_disputes d USING(xid)
    LEFT JOIN r_invoices i ON i.nvid=t.rel
    WHERE (uid1 IN (:aUids) AND uid2 IN (:bUids))
      OR (uid2 IN (:aUids) AND uid1 IN (:bUids))
    UNION ALL
    SELECT CONCAT('inv #', nvid) AS tid, '' AS paid, NULL AS nvid, NULL AS part, 'inv' AS type,
      IF (payee IN (:aUids), amount, -amount) AS invoiced,
      inv.created AS created, status=:TX_DENIED AS disputed, inv.`purpose` AS description
    FROM r_invoices inv
    WHERE ( (payer IN (:aUids) AND payee IN (:bUids)) OR (payee IN (:aUids) AND payer IN (:bUids)) )
    AND status<>:TX_CANCELED
    ORDER BY created, paid<>0
X;

  $q = db\q($sql, ['aUids' => $aUids, 'bUids' => $bUids]);
  u\EXPECT($q, "Failure reading transactions between users " . pr($aUids) . " and " . pr($bUids));

  $myBalance = 0.00; //balanceAsOf($aUids[0], $starting);
  $entries[] = ray('description balance invoiced paid created tid', t('Opening balance'), $myBalance, '', '', '', '');
  foreach ($q as $row) {
    $row = (array) $row;
    extract(just('paid disputed', $row));
    $myBalance -= $paid ?: 0;
    $row['balance'] = $myBalance;
    $disputed = $disputed ? t('(DISPUTED) ') : '';
    $row['description'] = $disputed . be\txDesc($row);
    $entries[] = $row;
  }

  return $entries;
}
