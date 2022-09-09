<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

/**
 * Generates the execute form.
 */
function formPhp($form, &$sta, $res = '') {
  $res = item($res, BARE);
  $code = areaFld(BARE, '', dft(getV('code')));
  $submit = t('Execute Code');
  return cgform(compact('res', 'code', 'submit'), 'wide nolabs');
}

/**
 * Process PHP execute form submissions.
 * WARNING: ALL ERRORS are suppressed on this page
 */
function formPhp_submit($form, &$sta) {
  extract(just('code', $sta['input']));
  $code = str_replace('<.', '<', $code); // fudge around drupal's sanitizing of all HTML
  \setV('code', $code); // remember for next time

  $u = 'r:, db:\DB, qr:\QR, be:\Backend, u:\Util, a:\Admin, w:\Web, cr:\Cron, s:\Smart, v:\Vote, k:\Risk';
  if (!isPRODUCTION) $u .= ', t:\Testing';
  foreach (ray($u) as $k => $v) $uses[] = "use CG$v as $k;";

  $code = join('', $uses) . $code;
  
  ob_start();
  try {
    eval($code);
  } catch (\CompileError $e) {
    say(t('<b>COMPILE:</b> %msg - line %line', 'msg line', $e->getMessage(), $e->getLine()), 'err');
  }
  say(pr(ob_get_clean()));
  //  w\go('sadmin/php/');
}
