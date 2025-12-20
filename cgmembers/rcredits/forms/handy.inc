<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

function formHandy($form, &$sta) {
  global $base_path, $base_url, $base_root;

  $links = [
    'sadmin/check-sanity|Check database integrity|Sanity Check|ok',
    'sadmin/util/reset-menu|Rebuild menus based on cg-menu.inc.|Redo MENUS|menu-hamburger',
    'sadmin/ofac|Upload the latest OFAC files to prevent use by known criminals.|Upload OFAC Files|upload',
    'sadmin/util/remove-test-data|Remove test data|REMOVE test data|fast-backward',
    'sadmin/util/clear|Empty the cache and sessions tables|Clear Cache & Sessions|unchecked',
    'sadmin/variables|Edit and delete site variables.|Variables|copy',
    'rcredits/misc/mirror.php|Mirror a Gherkin array|Mirror|refresh',
  ];

  foreach ($links as $link) {
    list ($url, $detail, $label, $icon) = u\padRay(explode('|', $link), 4);
    if (!$label) $label = $detail;
    $handy[$url] = ['', $label, '', ANY, '', $detail, $icon];
  }
  return w\subMenu(t('<h2>Handy Links</h2>'), $handy, 'handy');
}
