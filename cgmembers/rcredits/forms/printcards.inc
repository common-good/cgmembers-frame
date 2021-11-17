<?php
namespace CG\Web;
use CG\Admin as a;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once R_ROOT . '/admin/admin.inc';

/**
 * List cards to be printed.
 */
function formPrintCards($form, &$sta, $args = '') {
  global $base_url;
  extract(just('ids', $args, NULL));
  if (!$ids) return softErr(t('No cards to print.'));
  
  $title = t('Cards to Print');

  foreach (ray($ids) as $uid) {
    $a = r\acct($uid);
    $ph = u\fmtPhone($a->phone, '-');
    $nm = lnk("$base_url/change-account/acct=$uid", $a->fullName);
    $addr = $a->postalAddr;
//    if ($i = strpos($addr, ',')) $addr = substr_replace($addr, '<br>', $i, 1);
    $gift = $a->giftsDesc() ?: '-';
    $btn = $a->co ? a\showRelations($a) : btn("/print-rcard/$a->id", 'Card');
    $line = ''; foreach (ray('ph nm addr gift btn') as $k) $line .= "<div class=\"card-$k\">${$k}</div>";
    $list[] = $line;
  }
  $list = join("\n", $list); 
  
  return cgform(compact(ray('title list')), 'wide nolabs');
}
