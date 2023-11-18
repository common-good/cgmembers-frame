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

//  if ($mya->co and !$mya->relFld()) return go('dashboard', t('You cannot show a company QR without an agent. There is no way for an admin to do that yet.'), 'err'); // make sure admin is actually related to the company
  if (!$mya->ok) return go('dashboard', t('You cannot display this account\'s QR code until you have completed the steps to set it up.'), 'err');
  if (!$mya->canBuy) return go('dashboard', t('You do not have permission to use this account\'s QR code.'), 'err');

  $title = $mya->bestName;
  $subtext = $mya->co ? t('<p>Customers can scan this code to pay you. Right click to save it or copy it to a printable document. No one can charge you using this QR.</p>') : t('<p>Others will scan this code to charge your account.</p>');

  if ($mya->co ? $mya->ok : $mya->card) {
    $logo = $mya->co ? '' : $mya->photoSrc(FALSE, TRUE, TRUE);
    $text = "$mya->bestName, $mya->city, $mya->st"; // not enough room to show agent also
    $testQr = $qr0 = $mya->co ? qr\url($mya->mainA, 'payCo') : qr\url($mya, 'card');
    $qr = item(qr\html($qr0, $logo, $text));
  } else {
    $subtext .= t('<p>You must <%a>complete your account setup</a> before you can display your QR code.</p>', '_a', atag($mya->co ? '/dashboard' : '/scraps/card'));
    $qr = NULL;
  }
    
  return cgform(compact(ray('title subtext qr')));
}
