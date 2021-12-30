<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Toggle default to this account or personal account.
 */
function formDftAcct($form, &$sta, $args = '') {
  global $mya;

  if (r\cook('dftAcct') == $mya->mainQid) {
    r\deleteCook('dftAcct');
    say(t('Done. Your personal account is now the default on this device.'));
  } else {
    r\setCook('dftAcct', $mya->mainQid);
    say(t('Done. This account is now the default on this device.'));
  }
  return go('dashboard');
}
