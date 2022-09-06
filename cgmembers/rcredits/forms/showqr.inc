<?php
namespace CG\Web;
use CG\Web as w;
use CG\QR as qr;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once R_ROOT . '/cg-qr.inc';

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
    $aA = $mya->agentId == UID_SUPER ? $mya : $mya->agentA;
    $logo = $aA->photoSrc(FALSE, TRUE, TRUE);
    $text = $aA->bestName . ", $mya->city, $mya->st";
    $testQr = qr\url($mya);
    $qr = item(qr\html($testQr, $logo, $text));
    u\EXPECT(r\Qo::qr2(str_replace('HTTP://', '', $testQr)) == [$mya->qid, $mya->cardCode()], 'generated QR differs from source');
  } else {
    $subtext .= t('<p>You must <%a>complete the steps</a> for getting a %PROJECT card before you can display your account\'s code.</p>', '_a', atag($mya->co ? '/dashboard' : '/scraps/card'));
    $qr = NULL;
  }
    
  return cgform(compact(ray('title subtext qr')));
}
