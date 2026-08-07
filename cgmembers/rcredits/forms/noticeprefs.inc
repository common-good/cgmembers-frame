<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const POST_TYPES = 'offer need tip';

/**
 * Set preferences for notice frequency, etc.
 */
function formNoticePrefs($form, &$sta, $args = '') {
  global $mya;
  extract(just('code', $args, ''));
  
  $idRay = just('pid uid', test() ? $code : u\decryRay($code));
  if (!$idRay) {
    if (!$mya) return w\softErr(t('That is not a valid link for changing notice preferences.'));
    $idRay = ray('uid', $mya->id);
    $code = u\cryRay($idRay);
  }
  
  $title = t('Notice Preferences');
  $subtext = t('Choose how often you want to receive each type of notice (changes are immediate).');

  $hdrs = ray(t('Type,How often?'));
  $oftens = ray(NOTICE_OFTENS);
  $postOftens = just('d w m n', $oftens);
  $types = ray(NOTICE_TYPES);
  $isPid = (bool) nni($idRay, 'pid');
  $prefs = ray(db\get('notices', $isPid ? 'people' : 'users', $idRay));
  
  $lines[] = u\tableRow($hdrs);
  
  foreach ($types as $k => $type) {
    $isPost = in($k, POST_TYPES);
    if ($isPid and !$isPost) break;
    $freq = dropdown("freq", nni($prefs, $k), $isPost ? $postOftens : $oftens, $k);
    $lines[] = u\tableRow(compact(ray('type freq')));
  }
  $list = "<table>\n" . join('', $lines) . "</table>\n";
  
  jsx('notice-prefs', compact('code'));

  // Company accounts can also designate which authorized relations receive
  // transaction notifications. Show a link to that form (which lives at
  // /settings/notice-recipients but no longer has its own menu entry).
  $posttext = ($mya and $mya->co)
    ? t('<p><%a>Extra Notification Recipients</a> - choose which authorized users also receive email or text notifications when money moves in or out of this account.</p>', '_a', atag('/settings/notice-recipients'))
    : NULL;

  return cgform(compact(ray('title subtext list posttext')));
}
