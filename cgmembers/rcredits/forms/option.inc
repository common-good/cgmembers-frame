<?php
namespace CG\Web;
use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Web as w;
use CG\Vote as v;

include_once R_ROOT . '/rvote/rvote.inc';
include_once R_ROOT . '/rvote/rvote-ballot.inc';
include_once R_ROOT . '/rvote/rvote-results.inc';

/**
 * Add or edit a question.
 */
function formOption($form, &$sta, $args = '') {
  global $base_url, $mya;

  extract(just('qid oid', $args));
  
  if (!$add = empty($oid)) {
    $fields = 'e.id AS eid, e.ctty, q.id AS qid, q.type, o.text, o.detail, o.minimum as min, o.maximum as max';
    if (is_numeric($oid) and $option = db\get($fields, 'r_options o LEFT JOIN r_questions q ON q.id=o.question LEFT JOIN r_events e ON e.id=q.event', 'o.id=:oid', compact('oid'))) extract($option); else $add = FALSE;
    if ($ctty != $mya->community) return w\softErr(t('Wrong community.'));

    $id = hidFld($oid);
  } else {
    if (empty($qid)) return w\softErr(t('Missing question id.'));
    $type = db\get('type', 'r_questions', 'id=:qid', compact('qid'));
    $id = NULL;
  }

  $title = item($add ? t('Add Option') : t('Edit Option'));
  $question = hidFld($qid);
  $text = textFld((v\realOpts($type) ? REQ : '') . t('Option:'), [t('Option Text')], dft(nn($text)));
  $detail = areaFld(t('Detail:'), [t('Option Detail')], dft(nn($detail)));
  if ($type == 'R') {
    $minimum = numFld(t('Minimum:'), [t('Minimum Value')], required(nn($min)));
    $maximum = numFld(t('Maximum:'), [t('Maximum Value')], required(nn($max)));
  } else $minimum = $maximum = NULL;
  
  if ($mya->admManageVote) $submit = submit();
  
  return cgform(compact(ray('title id question text detail minimum maximum submit')));
}

function formOption_validate($form, &$sta) {
  extract($info = just('minimum maximum', $sta['input']));
  if ($info) foreach ($info as $k => $zot) if ($err = u\badAmount($info[$k])) return w\say($err, $k);
  u\preray($info, $sta['input']);
}

function formOption_submit($form, &$sta) {
  global $base_url;
  extract($info = just('id question text detail maximum minimum', $sta['input']));
  db\updateOrInsert('r_options', $info, 'id');
  w\go("community/events/question/qid=$question", 'info saved');
}
