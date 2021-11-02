<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Display an encrypted document.
 */
function formShowDoc($form, &$sta, $args = '') {
  global $mya;
  extract(just('code', $args, NULL));

  if (!$code or !$ray = u\decryRay($code)) return softErr('That is not a proper link to an encrypted document.');
  extract(just('path expires admin', $ray, NULL));
  if ($admin and !($mya and $mya->$admin)) return softErr('You do not have the proper administrative permission.');
  list ($s, $mimeType) = u\decryFile($path);
  header("Content-type: $mimeType");
  return exitJust($s);
}
