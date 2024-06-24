<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Admin as a;

include_once R_ROOT . '/admin/admin.inc';
include_once R_ROOT . '/admin/admin-web.inc';

/**
 * Merge one account into another.
 */
function formMergeAccts($form, &$sta, $args = '') {
  global $mya;

  $title = t('Merge Accounts');
  $from = textFld(REQ . t('Merge this account:'));
  $into = textFld(REQ . t('Into this account:'));
  $submit = submit(t('Submit'));

  return cgform(compact(ray('title from into submit')));
}

function formMergeAccts_validate($form, &$sta) {
  extract($input = just('from into', $sta['input']));
  
  foreach ($input as $k=>$v) {
    if (!$a = a($v)) return err('"%v" is not an account ID.', compact('v'), $k);
    $sta['input'][$k] = $a->id;
  }
}

function formMergeAccts_submit($form, &$sta) {
  extract(just('from into', $sta['input']));
  
  a\changeUid($from, $into, 'merge');
}
