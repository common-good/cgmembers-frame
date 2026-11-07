<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

/**
 * Tell whether this user may use the PHP window: anyone on a local machine, else an admin with the code permission.
 * @param bool $local: whether this is a local machine (a parameter so a test can try the online case)
 * The superAdmin (uid 1) passes every permission check, so it is refused here explicitly.
 */
function phpWindowOk($local = isLOCAL) {
  global $mya;
  return $local or ($mya and $mya->agentId != UID_SUPER and $mya->admCode);
}

/**
 * Generates the execute form.
 */
function formPhp($form, &$sta, $res = '') {
  if (!phpWindowOk()) return softErr(t('The PHP window is available only on a local development machine or to an admin with the code permission.'));
  $res = item($res, BARE);
  $code = areaFld(BARE, '', dft(getV('code')));
///  $code = str_replace('href', 'hruf', $code); // needed occasionally if we type something that looks like a hack
  $submit = t('Execute Code');
  return cgform(compact('res', 'code', 'submit'), 'wide nolabs');
}

function formPhp_validate($form, &$sta) {
  global $mya;
  if (!phpWindowOk()) return w\err(t('The PHP window is not available to this account.'));
  extract(just('code', $sta['input']));

  if (!!isPRODUCTION) { // safety precaution
    $code = str_replace("\r", '', $code);
    if (strstr($code, "\n", TRUE) != $mya->agentA->name) return w\err(t('You are on the PRODUCTION server, so you can\'t do that.'));
    $sta['input']['code'] = substr(strstr($code, "\n"), 1);
  }
}

/**
 * Process PHP execute form submissions.
 * WARNING: ALL ERRORS are suppressed on this page
 */
function formPhp_submit($form, &$sta) {
  extract(just('code', $sta['input']));
  \setV('code', $code); // remember for next time

  $code = str_replace('<.', '<', $code); // fudge around drupal's sanitizing of all HTML

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
