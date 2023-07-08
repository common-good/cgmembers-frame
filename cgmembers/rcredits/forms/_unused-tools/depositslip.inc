<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

define('BANK_NAME', 'Citizens Bank');
define('BANK_ADDR', 'Greenfield Branch<br>89 French King Highway<br>Greenfield, MA 01301<br>+1 413 774 2135');

/**
 * Show a PDF of a (populated) deposit slip for a particular Dollar Pool transfer group ("deposit") in and/or out.
 */
function depositDetails($args) {
  extract(just('way date total count only', $args, NULL));
  include_once R_ROOT . '/classes/pdf.class';
  $pdf = new r\Pdf();

  if (!$in = (!isset($way) or strtoupper($way) == 'IN')) $total = str_replace('-', '', $total);

  $m = .5; // left/right margin size
  $colW = 1.25; // amount column width
  $pdf->setPageUnit('in'); // measure everything in inches
  $pdf->SetMargins($m, $m, $m);  // left/top/right margin, needed for auto page breaks
  $pdf->SetAutoPageBreak($count > ($in ? 47 : 9), $m+.1); // leave space at bottom (count compensates for TCPDF bug)
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(TRUE);
  //  $pdf->setLanguageArray($l); // (eg "page" in footer)

  //  list ($activity, $where) = $in ? [t('DEPOSIT'), '(NOT (amount<0 XOR txid<0))'] : [t('CASHOUT'), '(amount<0 XOR txid<0)'];
  $activity = t('Bank Transfer');
  $where = 1;
  $today = $date == 1 ? t('(Historical)') : fmtDt($date ?: now());
  $b = r\bankData();
  $escrow = $in ? $b->acctIn : $b->acctOut;
  $bankName = $b->name;
  // Citizens Bank wants "A5990D1155A" in micr font at bottom of deposit slip (currently omitted)
  $pdf->setupFooter('', BANK_DBA . " $activity $today", ['........ '.$escrow.'C', 'GnuMICR;12'], 'Page @PAGE of @PAGES');
  //  $pdf->setupFooter('', BANK_DBA . " $activity $today", ['ACHes', 'GnuMICR;12'], 'Page @PAGE of @PAGES');
  $pdf->AddPage();

  $left = t(<<<EOF
            <h1><b>%activity Details</b><br>
            %today</h1><br>
            <br>
            <br>
            <b>%CGF_LEGALNAME<br>
            dba %BANK_DBA</b><br>
            %CGF_POSTALADDR<br>
            %CGF_PHONE (messages)<br>
            <br>
            <br>
            <b>%bankName</b><br>
            Account #%escrow<br>
            %BANK_ADDR<br>
            <br>
            <br>
            <b>TOTAL: %total</b><br>
            %count checks<br>
EOF
            , compact(ray('activity today bankName total count escrow')));

  $pdf->say($left, $m, 0);
  //    SELECT ABS(amount) AS amount, txid, uid, u.fullName AS name FROM txs2_bank t LEFT JOIN users u ON u.uid=t.payee 

  $sql = <<<EOF
    SELECT amount, xid, uid, u.fullName AS name, pid FROM txs2 t LEFT JOIN users u ON u.uid=t.payee 
    WHERE $where AND deposit=$date AND amount<>0 AND t.created<=:NOW ORDER BY xid
EOF;
  $q = db\q($sql);
  $x = $pdf->pageW / 2 - 1;
  $pdf->setY($m);
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($pid) $name = db\get('fullName', 'people', compact('pid')) . ($uid == CGID ? t(' FBO ') . PROJECT : str_replace(PROJECT, '', $name));
    $pdf->setX($x);
    if ($only and ($amount > 0 xor $in)) continue;
    if ($amount < 0) $pdf->SetTextColor(255, 0, 0);
    $pdf->Cell(1, 0, u\fmtAmt($amount, ''), '', 0, 'R');
    $pdf->SetTextColor(0);
    $pdf->setX($x + $colW);
    $pdf->Cell($x - $colW, 0, "(# $xid) $name", '', 1);
    if (FALSE) { // !$in) { // for outgoing checks, show bank address and member's legal name
      $pdf->setX($x + $colW);
      if ($bankInfo = r\acct($uid)->bankInfo) {
        extract($bankInfo, EXTR_PREFIX_ALL, 'b');
        $b_phone = u\fmtPhone($b_phone);
        $pdf->Cell($x, 0, $b_name, '', 1);
        $pdf->setX($x + $colW);
        $pdf->Cell($x, 0, $b_address, '', 1);
        $pdf->setX($x + $colW);
        $pdf->Cell($x, 0, "$b_city, $b_state $b_zip ($b_phone)", '', 1);
      } else $pdf->Cell($x, 0, 'NO BANK ADDRESS', '', 1);
      $pdf->newLine(); // a blank line between
    }
  }
  $pdf->setX($x);
  $pdf->Cell(1, 0, '----------------', '', 1, 'R');
  $pdf->setX($x);
  $pdf->Cell(1, 0, $total, '', 0, 'R');
  $pdf->setX($x + $colW);
  $pdf->Cell(($pdf->pageW - $x) - $colW, 0, t('TOTAL'), '', 1);

  $date = fmtDt($date, 'Ymd');
  if (test()) $pdf->Close(); else 
    //  $pdf->Output(PROJECT . "-deposit-$way-$date.pdf", 'I'); //Close and output
    $pdf->Output(PROJECT . "-deposit-$date.pdf", 'I'); //Close and output
}
