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
 * Generate an account statement for the given month or year.
 * @param string $period: mmmyyyy (month) or CYyyyy (calendar year) or FYyyyy (fiscal year)
 */
function statement($period = '') {
  $purposeLenMax = 60; // maximum number of characters in purpose field, not including added "..."

  if (!$mya = r\acct()) return w\go('', t('You must first sign in.'), 'ERR');

  list ($start, $end, $period) = statementMonth($period, $mya->fyEnd ?: '12/31');

  $uids = ($mya->jid != 0) ? [ $mya->id, $mya->jid ] : [ $mya->id ];
  /* list ($header, $classes, $rows, $tot) = x\getTxRows($start, $end, ray('descending', FALSE)); // get the transaction data */
  $startBals = be\creditInfo($uids, $start);
  /* print_r($startBals); */
  $endBals = be\creditInfo($uids, $end);
  /* print_r($endBals); */
  debug($startBals);
  debug($endBals);
  /* extract(just('frombank fromyou toyou', $tot)); */
  $bal0 = $startBals['balance']; // balance BEFORE start of this period
  $bal9 = $endBals['balance'];
  $in0 = $startBals['in'];
  $in9 = $endBals['in'];
  $out0 = $startBals['out'];
  $out9 = $endBals['out'];
  $fromyou = $out9 - $out0;
  $toyou = $in9 - $in0;
  $frombank = $endBals['fromBank'] - $startBals['fromBank'];
  $tobank = $endBals['toBank'] - $startBals['toBank'];
  $bank = $frombank - $tobank;

  debug($fromyou);
  $fromyou -= $tobank;
  debug($fromyou);
  debug($toyou);
  /* $toyou -= $frombank; */
  debug($toyou);

  debug("$in0, $in9, $out0, $out9, $frombank, $tobank, $bank, $fromyou, $toyou");
  
  $committed = $mya->committed;
  list ($avg) = r\averageBalance($uids, $mya->created, $end, TRUE);
    
  $pdf = new r\Statement($mya, $period);

  $size = $pdf->getFontSizePt();
  $m = 5; // left/right margin size
  $pdf->SetMargins($m, PDF_MARGIN_TOP, $m);
  $pdf->SetAutoPageBreak(TRUE, 15); // leave space at bottom
  $pdf->setLanguageArray([]); // (eg "page" in footer)
  $pdf->AddPage();
  $pageWidth = $pdf->getPageWidth();

  list ($address, $city, $state, $zip) = u\parseAddr($mya->postalAddr);
  $pdf->Cell(0, 0, $mya->j_fullName, '', 2);
  $pdf->Cell(0, 0, $address, '', 2);
  $pdf->Cell(0, 0, "$city, $state $zip", '', 2);
  
  $pdf->setX($m);
  $pdf->setY($pdf->getY() + 10);

  $balTitle = t('Balance:');
  $savingsTitle = t('Credit line:');

  $pdf->barHead('SUMMARY');
  $fields[1] = explode('; ', "/balTitle/15/; Starting/bal0/28/R; /plus/15/R; From Bank/bank/23/R; /minus/15/C; Paid*/fromyou/23/R; /plus/15/R; Received/toyou/23/R; /equals/15/R; Ending/bal9/28/R");
  $pdf->colHeads($fields[1]);

  list ($plus, $minus, $equals, $space) = array('+', '-', '=', ' ');
  list ($total0, $underline9, $total9) = [$bal0, str_repeat('-', 17), $bal9];
  $underline0 = $underline9;

  $numFields = ray('total0 bal0 amount frombank fromyou toyou bal9 committed in out bank');
  foreach ($numFields as $one) $$one = number_format(@$$one ?: 0, 2); // ?: is needed because $$one might be null or ''
  
  $pdf->setFontSize(.9 * $size);
  for ($i = 1; $i <= 1; $i++) {
    foreach ($fields[$i] as $one) {
      list ($head, $fldName, $width, $align) = explode('/', "$one/");
      foreach (ray('fromyou', 1) as $k => $v) {
        if ($fldName == $k) $$k .= str_repeat(' ', $v); // footnote alignment
      }
      $pdf->Cell($width, 0, @$fldName ? @$$fldName : '', '', @$fldName[strlen(@$fldName) - 1] == '9' ? 1 : 0, $align);
    }
  }
  
  $pdf->newLine();
  list ($labelWidth, $numWidth) = array(45, 25);

  $pdf->setFontSize(.8 * $size);
  $pdf->newLine();

  $pdf->setFontSize($size);
  $pdf->barHead('DETAILS');
  $lastHead = 'Amount';
  $fields = ray("Tx#/tid/15/C /space/1/ Date//17/ Name//55/ /space/1/ Purpose/description/90/ /space/1/ Amount//20/R");
  $pdf->colHeads($fields);
  $pdf->setFontSize(.9 * $size);
  $none = strip_tags(R_NONE);

  $txs = be\getTxs($uids, $start, $end);

  $cats = [];
  $dups = FALSE;
  foreach ($txs as $entries) {
    foreach ($entries as $entry) {
      if (in_array($entry['uid'], $uids)) continue;
      extract(just('tid created name description amount', $entry));
      $date = strftime('%b %d', $created); // reformat for single month
      if (strlen($description) > $purposeLenMax + 3) $description = substr($description, 0, $purposeLenMax) . '...';
      $amount = -$amount;  // since we're looking at it from the other party's view
      
      foreach ($fields as $field) {
        list ($head, $fldName, $width, $align) = explode('/', $field);
        if (!$fldName) $fldName = strtolower($head);
        $n = str_replace(',', '', $$fldName);
        $debit = ($align == 'R' and is_numeric($n) and $n < 0);
        if ($debit) { $pdf->SetTextColor(128, 0, 0); }
        else { $pdf->SetTextColor(); }
        if (in($fldName, $numFields) and $$fldName != $none) $$fldName = number_format($$fldName, 2);
        $pdf->Cell($width, 0, $$fldName, '', $head == $lastHead ? 1 : 0, $align, '', '', 1);
      }
      foreach (['amount', 'reward'] as $k) {
        $$k = u\cleanAmount(@$$k) ?: 0; // convert from string
      }
      if ($amount) x\addCat($cats, $description, $amount, $dups);
    }
  }
  
  if ($txs = []) {
    $pdf->newLine();
    $pdf->Cell($pageWidth, 0, t('There are no transactions this month.'), '', 1 , 'L');
  }
  
  if ($dups) { // don't show categorization unless it's worthwhile
    $pdf->setFontSize($size);
    $pdf->newLine();
    $pdf->barHead('CATEGORY TOTALS');
    $fields = ray('Category/cat/75/ Total/total/25/R Count/count/17/R');
    $pdf->colHeads($fields);
    $pdf->setFontSize(.9 * $size);
    x\finishCats($cats);
    
    foreach ($cats as $cat => $ray) {
      list ($count, $amount) = $ray;
      $total = number_format($amount, 2);
      foreach ($fields as $field) {
        list ($head, $fldName, $width, $align) = explode('/', $field);
        $pdf->Cell($width, 0, $$fldName, '', $head == 'Count' ? 1 : 0, $align, '', '', 1);
      }
    }
  }
  
  $pdf->finish(str_replace(' ', '', PROJECT) . '-' . strftime('%Y-%m', $start) . '.pdf'); //Close and output PDF document
}

/**
 * Return the start and end dates and human-readable description of a given month, quarter, or year.
 * @param string $period: mmmyyyy (month) or CYyyyy (calendar year) or FYyyyy (fiscal year)
 * @return [$start, $end, $periodDesc] -
 *   start is the start of the current period
 *   end is one second beyond the end of the current period, so use should be exclusive not inclusive
 */
function statementMonth($period, $fiscalYearEnd = '12/31') {
  $type = substr($period, 0, 2); // CY, FY, Q<n>, or other

  $year = substr($period, 2);
  if (is_numeric($year)) {  
    if (substr($type, 0, 1) == 'Q') {
      list ($endMo, $endDay) = [3 * substr($type, 1), in($type, 'Q1 Q4') ? 31 : 30];
      $lastDay = "$endMo/$endDay/$year";
    } else {
      $lastDay = $type == 'CY' ? "12/31/$year" : "$fiscalYearEnd/$year";
    }
    $end = strtotime($lastDay) + DAY_SECS; // fails if FY ends on clock change day
    list ($start, $periodDesc) = [u\plusMonths(-12, $end), "$type $year"];
  } else {
    $start = strtotime("1$period");
    if (!$start) { return w\softErr(t('That is not a recognizable month.')); }
    $start = u\monthDay1($start);
    list ($end, $periodDesc) = [u\plusMonths(1, $start), strftime('%B %Y', $start)];
  }
  return [$start, $end, $periodDesc];
}