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
function custStatementNew($args = '') {
  $purposeLenMax = 60; // maximum number of characters in purpose field, not including added "..."

  if (!$mya = r\acct()) return w\go('', t('You must first sign in.'), 'ERR');
  $cust = arrayGet($args, 'cust', null);
  $period = arrayGet($args, 'period', null);
  $way = arrayGet($args, 'way', 'OUT');

  $a = is_null($cust) ? null : r\acct($cust);
  if (is_null($a)) return w\softErr(t('No customer was specified.'));
//  if (!@$period) return w\softErr(t('No period was specified.'));

//  list ($start, $end, $period) = statementMonth($period, $mya->fyEnd ?: '12/31'); // not used yet
  $uids = is_null($a->jid) or $a->jid == 0 ? [ $a->id ] : [ $a->id, $a->jid ];
  $way == 'IN' ? be\getCustTxsNew($mya, $uids, 0, time()+1) : be\getCustTxsNew($a->id, $mya->id, 0, time()+1);$mya);
  if (!$q->rowCount()) return w\softErr(t('That is not a %thing of yours.', 'thing', $way == 'IN' ? t('customer') : t('vendor')));

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
  $fields = ray("Date//25/ /space/1/ /tid/20/C Description/purpose/83/ /space/1/ Invoiced//20/R /space/1/ Paid//20/R /space/10/ Balance//20/R");
  $pdf->colHeads($fields);
  $pdf->setFontSize(.6 * $size); // a bit of space below field headers
  $pdf->newLine();
  $pdf->setFontSize(.9 * $size);

  $bal = 0;
  $row0 = ['purpose' => t('Opening balance')];

  while ($row = @$row ? $q->fetchAssoc() : $row0) {
    if (!@$classes) $classes = array_keys($row);
//    extract(just('tid date name amount purpose reward', array_combine($classes, $row)));
    extract(just('tid created purpose invoiced paid', $row));
    $date = @$created ? u\fmtDate($created, TRUE) : '';
    $bal += (@$invoiced ?: 0) - (@$paid ?: 0);
    $balance = $bal;
    if (strlen(@$purpose) > $purposeLenMax + 3) $purpose = substr($purpose, 0, $purposeLenMax) . '...';

    foreach ($fields as $one) {
      list ($head, $fldName, $width, $align) = explode('/', $one);
      if (!$fldName) $fldName = strtolower($head);
      $debit = ($align == 'R' and is_numeric($n = @$$fldName) and $n < 0);
      if ($debit) $pdf->SetTextColor(128, 0, 0); else $pdf->SetTextColor();
      if (in($fldName, $numFields) and isset($$fldName)) {
        $$fldName = ($$fldName == '0' and $fldName != 'balance') ? '' : number_format($$fldName ?: 0, 2);
      }
      $pdf->Cell($width, 0, @$$fldName, '', $head == $lastHead ? 1 : 0, $align, '', '', 1);
    }
    if ($row == $row0) $pdf->newLine(); // space after opening balance
  }

  $pdf->newLine();
  
  $closing = @$classes
  ? ($bal ? t('Total due: $') . $balance : t('No payment is due.'))
  : t('There are no transactions this month.');

  $pdf->SetFillColorArray([255, 255, 0]); // yellow
  $pdf->Cell($pageWidth - 2 * $m, 0, "$closing", '', 1 , 'L', TRUE);
  $pdf->finish(t('Statement-%acct-%date.pdf', 'acct date', $a->mainQid, strftime('%m-%d-%Y', time()))); //Close and output PDF document
}

