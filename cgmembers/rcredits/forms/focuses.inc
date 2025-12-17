<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const FOCUSES_FLDS = 'uid project percent'; // just the fields we handle here

/**
 * Display work focus percentages for each staff member.
 * NOTE: not currently used (we use an audit spreadsheet tab instead)
 */
function formFocuses($form, &$sta, $args = '') {
  global $mya;

  extract(crud('u_focuses', $args, 'id', FOCUSES_FLDS, 'uid, project')); // get ret or [title, orig, id, and FOCUSES_FLDS]
  if (nn($ret)) return $ret;

  $who = textFld(REQ . t('Staff Member:'), [t('Full name')], dft($uid ? r\acct($uid)->mainQid : NULL));
  $uid = NULL;
  $project = textFld(REQ . t('Project:'), '', dft($project));
  $percent = numFld(REQ . t('Percent:'), '', dft($percent) + step(.001) + vrange(0, 100) + suffix(' %'));
  $submit = submit();

  w\whoFldSubmit('who', t('%name?'), '', ':IS_ADMIN', ray('allowNonmember', FALSE));

  $orig = hidFlds($orig);
  $form = compact(ray('title who ' . FOCUSES_FLDS . ' submit orig'));

  return cgform($form);
}

function formFocuses_validate($form, &$sta) {
}

function formFocuses_submit($form, &$sta) {
  extract($info = just(FOCUSES_FLDS, $sta['input'], NULL));
  extract(just('id', hidFlds($sta, 'orig'), NULL));

  $info['uid'] = whoId($sta);
  if ($id or $id = db\get('id', 'u_focuses', just('uid project', $info))) $info += compact('id');
  db\updateOrInsert('u_focuses', $info, 'id');

  say('info saved');
  return go('sadmin/focuses');
}
