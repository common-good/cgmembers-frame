<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const QR_PHOTO_BW = 1.05; // how much wider is image with white border
const QR_PHOTO_BH = QR_PHOTO_BW * 136 / 99; // shape is designed (with showqr.inc) to align with block boundaries

/**
 * Output an account photo or thumbnail if the caller is authorized.
 * @param string $page: account-photo or account-thumb
 * @param string $args: other parameters, including:
 *   $qid: account letter-code
 *   $orig: show the original photo before it was cropped
 *   $code: encrypted array of qid and session ID (for showing arbitrary photo securely when caller knows it has permission)
 *   $qr: show a photo with borders, suitable for use in a QR code
 * @return the photo or thumb if authorized and one exists. Otherwise a generic picture.
 */
function formAccountPhoto($form, &$sta, $page = '', $args = '') {
  global $mya;

  list($zot, $type) = explode('-', $page);
  $dftImgFlnm = ($mya and $mya->co) ? 'placeholder-co.png' : 'placeholder.png';

  extract(just('orig qid code qr', $args, NULL));
  if ($code and $ray = u\decryRay($code)) extract(just('qid sid qr', $ray, NULL));
  
  if ($code and nn($sid) != session_id() and !isDEV) { // allow insecure file_get_contents on dev
    $photo = NULL;
  } elseif ($a = $qid ? r\acct($qid) : $mya) {
    if ($qr) $a = $a->agentA; // use agent's photo, if embedding in QR
    $photo = $orig ? u\getFile($a->tempPhotoFilename) : $a->$type;
  } else { $photo = NULL; }
  
  if (strlen($photo) and $qr) { // add borders for embedding in QR code
    $im = imagecreatefromstring($photo);
    list ($w0, $h0) = [imagesx($im), imagesy($im)];
    $im = u\alterImg($im, $w0, $h0, 0, 0, $w0 * QR_PHOTO_BW, $h0 * QR_PHOTO_BW);
    $photo = u\img2string($im);
  }

  header("Content-type: image/jpeg");
  return exitJust($photo ?: \file_get_contents(R_ROOT . "/images/icons/$dftImgFlnm"));
}
