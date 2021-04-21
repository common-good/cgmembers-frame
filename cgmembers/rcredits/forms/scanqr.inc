<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Scan a QR to identify a customer.
 */
function formScanQr($form, &$sta, $args = '') {
  global $mya;
  extract(just('nothing', $args));

  js('scanqr');
  
  $title = t('Scan a %PROJECT Card');
  
  $result = item(t('No camera is available (yet) on this device.'), NBSP);
  $qr = item(w\tags('video', '', ray('id', 'scanqr')), BARE);

  return cgform(compact(ray('title result qr')));
}
