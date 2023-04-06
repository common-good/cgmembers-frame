<?php
/**
 * @file
 * QR Code functions.
 */

namespace CG\QR; // typically abbreviated as "qr"
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;

use Endroid\QrCode\Builder\Builder;
use Endroid\QrCode\Encoding\Encoding;
use Endroid\QrCode\ErrorCorrectionLevel\ErrorCorrectionLevelHigh;
//use Endroid\QrCode\Label\Alignment\LabelAlignmentCenter;
use Endroid\QrCode\Label\Font\NotoSans;
use Endroid\QrCode\RoundBlockSizeMode\RoundBlockSizeModeMargin;
use Endroid\QrCode\Writer\PngWriter;

function html($data, $logo, $text) {return tr('<img src="%data" />', '_data', generate($data, $logo, $text));}

/**
 * Return a QR code for the given data, with the logo in the middle, and text at the bottom.
 * @param string $data: the URL or other data for which to create a QR code
 * @param string $logo: a fully-qualified URL for an image to place in the center
 * @param string $text: text to append too the bottom of the QR image
 * @return a QR image data URI
 */
function generate($data, $logo, $text) {
  require_once DRUPAL_ROOT . '/../vendor/autoload.php';

  $qr = Builder::create()
    ->writer(new PngWriter())
//    ->writerOptions([])
    ->data($data)
    ->encoding(new Encoding('ISO-8859-1'))
    ->errorCorrectionLevel(new ErrorCorrectionLevelHigh())
    ->size(300)
    ->margin(5)
    ->roundBlockSizeMode(new RoundBlockSizeModeMargin())
    ->labelFont(new NotoSans(10));
//    ->labelAlignment(new LabelAlignmentCenter())
    
  if ($logo) $qr
    ->logoPath($logo)
    ->logoResizeToWidth(99)
    ->logoResizeToHeight(136);
    
  if ($text) $qr->labelText($text);

  return $qr->build()->getDataUri();
}

/**
 * Return a tightly-encoded qid for use in a QR code or magnetic stripe (radix 36 instead of 26)
 * The first character (fmt) tells the length of region, account in region (tail), and agentCode, as follows:
 * 0-3: 1/2/n, n=c (where c is a radix-36 digit)
 * 4-7: 1/3/n, n=c-4
 * 8-B: 2/2/n, n=c-8
 * C-F: 2/3/n, n=c-C (meaning c minus twelve)
 * G-J: 3/2/n, n=c-G, etc.
 * K-N: 3/3/n
 * O-R: 3/4/n
 * S-V: 4/4/n
 * W-Z: 4/5/n
 * @param acct $a: the account for which to return a QR url
 * @param bool $type: 
 *   'card':  standard for printing a Common Good card (HTTP://<region>.<domain>/<fmt><tail><agentCode><cardCode>)
 *            where <domain> is the appropriate domain for production cards if we're running in a production environment, otherwise for test cards
 *   'short' (DEPRECATED): compact code for displaying on PWA (<fmt><region><tail><agentCode><cardCode> ending with "." for a test card)
 *   'main':  like short but omit cardCode (and .) (used in PWA api: <fmt><region><tail><agentCode>)
 * @return the code as appropriate for the type
 */
function url($a, $type = 'card') {
  $qo = $a->qo;
  $cardCode = $a->cardCode() ?: $a->makeCardCode(); // first time printing company card
  $regLen = strlen($region = u\n2a(u\a2n($qo->region), -1, 36));
  $tailLen = strlen($tail = u\n2a(u\a2n($qo->tail), $regLen > 3 ? -4 : -2, 36));
  $agentCode = $qo->agentCode();
  $codeLen = $agentCode ? strlen($agentCode = u\n2a(u\a2n($agentCode), -1, 36)) : 0;
  $i = array_search($regLen . $tailLen, ray('12 13 22 23 32 33 34 44 45'));
  u\EXPECT($i !== FALSE, 'bad piece lengths in qo->qr mainQid=' . $qo->mainQid);
  u\EXPECT($codeLen < 4, 'agentCode too long: ' . $qo->agentCode);
  $fmt = u\n2a($i * 4 + $codeLen, 1, 36);
  $qrDom = ray(QR_DOMS)[(bool) isPRODUCTION];
//  $testFlag = isPRODUCTION ? '-A' : '.'; // test mode indicator for short version (-A only while transitioning from old app (3/2023))
  if ($type == 'short') u\FAIL('short QR url is deprecated'); // return "$fmt$region$tail$agentCode$cardCode$testFlag";
  if ($type == 'main') return "$fmt$region$tail$agentCode";
  if ($type == 'card') return "HTTP://$region.$qrDom/$fmt$tail$agentCode$cardCode";
  u\FAIL(t('invalid qr type: ') . $type);
}

/**
 * Interpret a CG card QR code (from an actual card or an abbreviated one from our app).
 * @param string $qr: the QR code to interpret
 * @param bool $requireCode: require card code
 * @return NULL on error, else the account.
 */
function acct($qr, $requireCode) {
  $parts = preg_split('[/\\.-]', $qr);
  $count = count($parts);
  
  if ($count <= 2) { // abbreviated format (for QRs displayed by app) - <fmt><region><acct><agent><cardCode><type><counter>
    if (!preg_match('/^[0-9A-Z]{3,}/', $qr)) return NULL;
    $fmt = $qr[0]; // one radix 36 digit representing format (field lengths)
    $i = (int) u\a2n($fmt, 36) / 4;
    if (!$regionLen = (int) substr(REGION_LENS, $i, 1)) return NULL;

    $region = substr($qr, 1, $regionLen);
    $isTestCard = strhas($qr, '.'); // ignore -A on temporary QRs generated in spring 2023
//    $counter = $count < 2 ? 0 : $parts[1]; //phased out 3/2023
    $unfmt = qrUnfmt($region, $fmt . substr($parts[0], 1 + $regionLen), $isTestCard); // pretend it was the long format to get the rest
  } else if ($count == 6) { // normal format - HTTP://<region>.<server>/<fmt><acct><agent><cardCode>
    $region = $parts[2];
    $isTestCard = (strtoupper($parts[3]) == 'RC4');
    $unfmt = qrUnfmt($region, $parts[5], $isTestCard);
  } else if ($count == 7 || $count == 9){ // old formats
    u\log('old QR format: ' . $qr);
    return NULL;
    /*
    $region = $parts[2];
    $isTestCard = (strtoupper($parts[3]) == 'RC4');
      $code = $parts[$count - 1];
      $account = $parts[$count - 2];
      int markPos = $qr.length() - $cardCode.length() - ($count == 9 ? $account.length() + 2 : 1);
      qid = region . $account;
      if (isAgent = A.substr($qr, markPos, 1).equals('-')) {
          int i = oldAgentQids.indexOf(qid . '/');
          if (i < 0) return NULL;
          oldAgent(region . ':' . $account, A.substr(oldAgentQids, i + 7, 8), $isTestCard);
      } else co = qid;
      if (!qid.matches('^[A-Z]{6}(-[A-Z])?')) return NULL;
      */
  } else return NULL;

  if ($unfmt) extract($unfmt); else return NULL;

  if (!$a = r\acct($qid)) return NULL;
  if ($requireCode and $cardCode != $a->cardCode()) return NULL;
  return r\acct($qid);
}

function qrUnfmt($region, $tail, $isTestCard) {
  $fmt = $tail[0]; // one radix 36 digit representing format (field lengths)
  if (!preg_match('/[0-9A-Z]/', $fmt)) return NULL;
  $i = (int) u\a2n($fmt, 36);
  $agentLen = $i % 4;
  $acctLen = substr(ACCT_LENS, $i / 4, 1);

  if ($acctLen == 6 or strlen($tail) < 1 + $acctLen + $agentLen) return NULL;

  $account = substr($tail, 1, $acctLen);
  $agent = substr($tail, 1 + $acctLen, $agentLen);
  $cardCode = substr($tail, 1 + $acctLen + $agentLen);
  $isAgent = ($agentLen > 0);

  $region = u\n2a(u\a2n($region, 36), 3);
  $account = u\n2a(u\a2n($account, 36), 3);
  $agent = $isAgent ? ('-' . u\n2a(u\a2n($agent, 36), -1)) : '';

  $co = $region . $account;
  $qid = $co . $agent;
  if (!preg_match('/^[A-Z]{3,4}[A-Z]{3,5}(-[A-Z]{1,5})?/', $qid)) return NULL;

  return compact(ray('qid co cardCode isAgent'));
}

/**
 * Mask the image (to create a QR that looks like our logo, using very dark blue and green)
 * Not Yet USED
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
