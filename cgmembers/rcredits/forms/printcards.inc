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
  $addrs = "ID\tline1\tline2\tline3\n"; // text for envelope label mailmerge
  $addrI = 0;
  
  $title = t('Cards to Print');

  foreach (ray($ids) as $uid) {
    $a = r\acct($uid);
    $qid = $a->mainQid;
    $ph = u\fmtPhone($a->phone, '-');
    list ($addr, $city, $st, $zip) = u\parseAddr($a->postalAddr);
    $addr .= "<br>$city, $st $zip";
    $nm = lnk("$base_url/change-account/acct=$uid", $a->fullName);
    $gift = str_replace('+', ' +', $a->giftsDesc() ?: '-');
    $btn = $a->co ? a\showRelations($a) : btn("/print-rcard/$a->id", 'Card');

    $ph = "$qid<br>$ph";
    $addr = "$nm<br>$addr";
    $addrs .= ++$addrI . "\t" . str_replace('<br>', "\t", $addr) . "\n"; // addrI is just for LibreOffice Base import

    $line = ''; foreach (ray('ph addr gift btn') as $k) $line .= "<div class=\"card-$k\">${$k}</div>";
    $list[] = "<div>$line</div>";
  }
  $list = join("\n", $list) . "<br><br><br><pre>$addrs</pre>";
  
  return cgform(compact(ray('title list')), 'wide nolabs');
}
