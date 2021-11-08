<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

use Endroid\QrCode\Builder\Builder;
use Endroid\QrCode\Encoding\Encoding;
use Endroid\QrCode\ErrorCorrectionLevel\ErrorCorrectionLevelHigh;
//use Endroid\QrCode\Label\Alignment\LabelAlignmentCenter;
use Endroid\QrCode\Label\Font\NotoSans;
use Endroid\QrCode\RoundBlockSizeMode\RoundBlockSizeModeMargin;
use Endroid\QrCode\Writer\PngWriter;


/**
 * Show a QR code for the current account.
 */
function formShowQr($form, &$sta, $args = '') {
  global $mya, $testQr;
//  extract(just('nothing', $args));

  if (!$mya->ok) return softErr(t('You cannot display this account\'s QR code until you have completed the steps to set it up.'));
  if (!$mya->canBuy) return softErr(t('You do not have permission to use this account\'s QR code.'));

  $title = $mya->bestName;
  $subtext = t('<p>Businesses will scan this code to charge your account.</p>');

  if ($mya->co ? ($mya->ok and $mya->agentA->hasPhoto) : $mya->card) {
    $qr = item(tr('<img src="%data" />', '_data', genQr($testQr = qrUrl($mya))));
    u\EXPECT(r\Qo::qr2(str_replace('HTTP://', '', $testQr)) == [$mya->qid, $mya->cardCode()], 'generated QR differs from source');
  } else {
    $subtext .= t('<p>You must <%a>complete the steps</a> for getting a %PROJECT card before you can display your account\'s code.</p>', '_a', atag($mya->co ? '/dashboard' : '/scraps/card'));
    $qr = NULL;
  }
    
  return cgform(compact(ray('title subtext qr')));
}

function genQr($url) {
  global $mya;
  require_once DRUPAL_ROOT . '/vendor/autoload.php';

  $aA = $mya->superAdmin ? $mya : $mya->agentA;

  return Builder::create()
    ->writer(new PngWriter())
//    ->writerOptions([])
    ->data($url)
    ->encoding(new Encoding('ISO-8859-1'))
    ->errorCorrectionLevel(new ErrorCorrectionLevelHigh())
    ->size(300)
    ->margin(5)
    ->roundBlockSizeMode(new RoundBlockSizeModeMargin())
    ->logoPath($aA->photoSrc(FALSE, TRUE, TRUE))
    ->logoResizeToWidth(99)
    ->logoResizeToHeight(136)
    ->labelText($aA->bestName . ", $mya->city, $mya->st") 
    ->labelFont(new NotoSans(10))
//    ->labelAlignment(new LabelAlignmentCenter())
    ->build()
    ->getDataUri();
}

/**
 * Return a more tightly-encoded qid for use in a QR code or magnetic stripe (radix 36 instead of 26)
 * The first character tells the length of region, account in region, and agentCode, as follows:
 * 0-3: 1/2/n, n=c (where c is a radix-36 digit)
 * 4-7: 1/3/n, n=c-4
 * 8-B: 2/2/n, n=c-8
 * C-F: 2/3/n, n=c-C (meaning c minus twelve)
 * G-J: 3/2/n, n=c-G, etc.
 * K-N: 3/3/n
 * O-R: 3/4/n
 * S-V: 4/4/n
 * W-Z: 4/5/n
 * @return [fmt, region, tail, agentCode], where fmt is the format character
 */
function qrUrl($a) {
  $qo = $a->qo;
  if (!$a->cardCode()) $a->makeCardCode(); // first time printing company card
  $regLen = strlen($region = u\n2a(u\a2n($qo->region), -1, 36));
  $tailLen = strlen($tail = u\n2a(u\a2n($qo->tail), $regLen > 3 ? -4 : -2, 36));
  $codeLen = ($agentCode = $qo->agentCode()) ? strlen($agentCode = u\n2a(u\a2n($agentCode), -1, 36)) : 0;
  $i = array_search($regLen . $tailLen, ray('12 13 22 23 32 33 34 44 45'));
  u\EXPECT($i !== FALSE, 'bad piece lengths in qo->qr mainQid=' . $qo->mainQid);
  u\EXPECT($codeLen < 4, 'agentCode too long: ' . $qo->agentCode);
  $fmt = u\n2a($i * 4 + $codeLen, 1, 36);
  $qrDom = ray(QR_DOMS)[(bool) isPRODUCTION];
  return "HTTP://$region.$qrDom/$fmt$tail$agentCode" . $a->cardCode();
}

/**
 * Mask the image (to create a QR that looks like our logo, using very dark blue and green)
 */
function image_mask(&$src, &$mask) {
  imagesavealpha($src, true);
  imagealphablending($src, false);

  for ($x = 0; $x < imagesx($src); $x++) { // scan image pixels
    for ($y = 0; $y < imagesy($src); $y++) {
      $mask_pix = imagecolorat($mask,$x,$y);
      $mask_pix_color = imagecolorsforindex($mask, $mask_pix);
      if ($mask_pix_color['alpha'] < 127) {
        $src_pix = imagecolorat($src,$x,$y);
        $src_pix_array = imagecolorsforindex($src, $src_pix);
        imagesetpixel($src, $x, $y, imagecolorallocatealpha($src, $src_pix_array['red'], $src_pix_array['green'], $src_pix_array['blue'], 127 - $mask_pix_color['alpha']));
      }
    }
  }
}
