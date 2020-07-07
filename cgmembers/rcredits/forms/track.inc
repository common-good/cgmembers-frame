<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const PIXEL = 'R0lGODlhAQABAJAAAP8AAAAAACH5BAUQAAAALAAAAAABAAEAAAICBAEAOw=='; // base64-encoded transparent pixel

/**
 * Track email opens.
 * Call with <img src="%BASE_URL/track/%code">
 * Output a one-pixel transparent GIF.
 */
function formTrack($form, &$sta, $code = '') {
  global $mya;

/**/ if (!$code or !$ray = u\decryRay($code)) die(t('bad tracking'));
  extract(just('qid type', $ray, NULL));
  if (!$type) die(t('bad tracking: ') . pr($ray));
  $uid = ($qid and $a = r\acct($qid)) ? $a->id : NULL;
  
  $info = compact(ray('uid type'));
  if ($id = db\get('id', 'u_track', $info)) $info += compact('id');
  $info['seen'] == now();
  db\updateOrInsert('u_track', $info);
  
  header('Content-Type: image/gif');
  echo base64_decode(PIXEL);  
}
