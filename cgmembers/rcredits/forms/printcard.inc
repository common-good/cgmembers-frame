<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\QR as qr;
use CG\Admin as a;
use CG\Web as w;

require_once R_ROOT . '/classes/pdf.class';
require_once R_ROOT . '/forms/showqr.inc'; // for qrUrl()

/**
 * Print an ID card
 * @param mixed $id: qid or uid of account to make a card for
 * @param mixed $agent: qid or uid of agent
 * @param bool $retroactive: create the id regardless of permissions (used only in testing)
 */
function printCard($id, $agent = '', $retroactive = FALSE) {
  global $mya;
  
  $a = r\acct($id, $agent);
  $cttyErr = (!$mya or (!$mya->admRegion and $a->community != $mya->community));
  if (!$a or ($cttyErr and !$retroactive)) u\FAIL('No such account ID: ' . $id . ':' . $agent);
  
  $a->setAgentNum(); // assign an agentCode, if appropriate (and not yet done)
  $qid = $a->qid; // get this before setting $a to agent (below)
  
  $acctName = $a->bestName;
  $nameFont = mb_strlen($acctName) > 38 ? 'font-stretch:condensed;'
    : (mb_strlen($acctName) > 33 ? 'font-stretch:semi-condensed;' : '');
  
  list ($aa, $proSe) = [$a->agentA, $a->proSe];

  list ($role, $ptQid, $yName, $bg) = $proSe 
    ? [t('Member'), 10, 1.70, 'bg'] 
    : [t('Company'), '9.5;font-stretch:semi-condensed', 1.64, 'bgCo'];

  $permitted = ($proSe or B_RELATED + r\relation('permission', $a->id, $aa->id) >= B_BUY);
  $photo = ($aa->hasPhoto and $permitted) ? "@$aa->photo" : R_NOT_VALID; // '@' tells tcpdf it's an image, not a file; $a->canBuy fails because admin has permission

  list ($wCard, $hCard, $m, $xText) = [3.375, 2.125, 0, .06]; // card dimensions, negative page margin, text indent
  //  list ($wPhoto, $mPhoto, $xCardNo) = [1.09, .1, 2.4];
  list ($xProj, $yProj) = [$m+1.3, $m+.04];
  list ($xSite, $ySite, $wPhoto, $mxPhoto, $myPhoto, $xCardNo) = [$xProj+.035, $yProj+.29, 1.1, .085, .09, 2.4];
  list ($cCG, $cSite, $cName) = ['#000065', 'black', '#004000'];
  list ($xTest, $xPhoto, $yPhoto, $hPhoto) = [$m+$wCard-.74, $m+$mxPhoto, $m+$myPhoto, $wPhoto*4/3];

  $style = array( // style for barcode
                 'border' => 0,
                 'vpadding' => '0',
                 'hpadding' => '0',
                 'fgcolor' => array(0,0,0),
                 'bgcolor' => false, //array(255,255,255)
                 'module_width' => 1, // width of a single module in points
                 'module_height' => 1, // height of a single module in points
                  );

  $pdf = new r\Pdf('L', 'in', 'CREDIT_CARD');
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(FALSE);
  $pdf->SetAutoPageBreak(FALSE);

  $pdf->AddPage();
  $pdf->StartTransform();
  $pdf->ScaleXY(100 * 3.375 / $wCard); // widen to page width (for extra resolution)

  $pdf->Image(R_ROOT . "/images/idcard/$bg.png", $m, $m, $wCard, $hCard, '', '', 'L', true); // file, x, y, w, h, type, link, align, resize
  //  $pdf->Image($photo, $m+$mPhoto, $m+$mPhoto, $wPhoto, $wPhoto*4/3, '', '', 'L', true);
  $pdf->Image($photo, $xPhoto, $yPhoto, $wPhoto, $hPhoto, '', '', 'L', true);
  $pdf->Rect($xPhoto, $yPhoto, $wPhoto, $hPhoto, 'D'); // x, y, w, h, style, border, fill
  $pdf->say(PROJECT, $xProj, $yProj, '', '', "18;Arial MT Medium;color:white;", 'L');
  
  $pdf->say('&reg;', $m+3.01, $m+.08, 1, '', "4;color:white", 'L');
  $sp = '<span style="font-size:50%;"> </span>';
  $url = str_replace('.', $sp . '<b style="display:inline; font-stretch:expanded; font-size:120%;">.</b>' . $sp, CG_DOMAIN);
  $pdf->say($url,  $xSite, $ySite, '', '', "7;color:white;font-stretch:normal;", 'L');
  $pdf->write2DBarcode(qr\url($a, 'card'), 'QRCODE,Q', $m+1.445, $m+0.645, .8, .8, $style, 'N'); // L,M,Q,H are low-high error-correction

  $pdf->say($acctName, $m+$xText, $m+$yName, '', '', "12;B;$nameFont", 'L');
  if ($proSe) $pdf->say($qid, $m+$xText, $m+$hCard-.19, '', '', "6.5;color:#666666;letter-spacing:.7px;", 'L');
  if (!$proSe) $pdf->say($aa->fullName, $m+$xText, $m+$yName+0.22, '', '', 8, 'L');

  if (NOT_PRODUCTION) $pdf->say(t('TEST'), $xTest, $m+$hCard-.19, .5, '', '6.5;color:darkred', 'C');

  $pdf->StopTransform();
/*  if (now() < strtotime('6/20/2017')) {
    $pdf->Output(strtolower("$qid.cgcard.pd"), 'D'); // Close and output PDF document ('I')
  } else */
  if ($mya and $mya->admPrintCards) $a->setBit(B_CARDED);
  if (test()) $pdf->Close(); else $pdf->Output(strtolower("$qid.cgcard.pd"), 'I'); // Close and output PDF document ('I')
//  if (now() < strtotime('6/20/2017') and $next = nextMember($id)) w\go("print-rcard/$next");
}
