<?php
/**
 * @file
 * The page that serves all page requests (slightly modified from Drupal).
 * All Drupal code is released under the GNU General Public License.
 * See COPYRIGHT.txt and LICENSE.txt.
 */

//if ($_SERVER['REMOTE_ADDR'] != '199.167.127.19')
//exit(file_get_contents('down.html')); // maintenance mode

define('DRUPAL_ROOT', getcwd());
require_once __DIR__ . '/rcredits/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
// rebuild when the route definitions change, not only when R_VERSION is bumped (a forgotten bump left routes missing)
$menuSig = md5(R_VERSION . md5_file(__DIR__ . '/rcredits/cg-menu.inc') . md5_file(__DIR__ . '/rcredits/rcredits.module'));
if ($menuSig != getv('menuSig', '')) {
  \menu_rebuild();
  setv('menuSig', $menuSig);
}
//\setV('up', 1);
menu_execute_active_handler();
