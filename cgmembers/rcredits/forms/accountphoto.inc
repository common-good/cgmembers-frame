<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Output an account photo or thumbnail if the caller is authorized.
 * @param string $page: account-photo or account-thumb
 * @param string $args: other parameters, including:
 *   $qid: account letter-code
 *   $orig: show the original photo before it was cropped
 *   $code: encrypted array of qid and session ID (for showing arbitrary photo securely when caller knows it has permission)
 * @return the photo or thumb if authorized and one exists. Otherwise a generic picture.
 */
function formAccountPhoto($form, &$sta, $page = '', $args = '') {
  global $mya;

  list($zot, $type) = explode('-', $page);
  $dftImgFlnm = ($mya and $mya->co) ? 'placeholder-co.png' : 'placeholder.png';

  extract(just('orig qid code', $args, NULL));
  if ($code and $ray = u\decryRay($code)) extract(just('qid sid', $ray, NULL));
  
  if ($code and nn($sid) != session_id()) {
    $photo = NULL;
  } elseif ($a = $qid ? r\acct($qid) : $mya) {
    $photo = $orig ? u\getFile($a->tempPhotoFilename) : $a->$type;
  } else { $photo = NULL; }

  header("Content-type: image/jpeg");
  return exitJust($photo ?: \file_get_contents(R_ROOT . "/images/icons/$dftImgFlnm"));
}

//function formIcon($orig = FALSE) {return memberPhoto(@r\acct()->mainQid, TRUE, $orig);} // Output a small version of the current account's photo.
