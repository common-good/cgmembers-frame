<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

include_once R_ROOT . '/admin/admin.inc';

/**
 * Display dated changes made to an account.
 * @param string $qid: the account QID (default to current account)
 */
function formSeeChanges($form, &$sta, $qid = '') {
  $a = r\acct($qid);
  $title = t('Account Changes for ') . $a->fullName;
  $list[] = '<tr><th>Date</th><th>Field</th><th>Old Value</th><th>New Value</th><th>Changed by</th></tr>';
  // $rec = []; // track historical field values

  $sql = 'SELECT h.created, field, oldValue, newValue, changedBy, u.name AS changerName FROM r_changes h LEFT JOIN users u ON h.changedBy=u.uid WHERE h.uid=:id ORDER BY created DESC';
  $q = db\q($sql, ray('id', $a->id));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $date = fmtDt($created);
    foreach (ray('oldValue newValue') as $k) {
      $type = u\crypted($$k);
      $$k = a\formatField($field, $type ? decryChange($type, $$k) : $$k);
    }
    $changedBy = $changerName ?: "($changedBy)";
    $list[] = "<tr><td>$date</td><td>$field</td><td>$oldValue</td><td>$newValue</td><td>$changedBy</td></tr>";
  }
  
  if (count($list) > 1) {
    $list = join("\n", $list);
    $list = item("<table>$list</table>");
  } else $list = t('No changes');
  
  return cgform(compact(ray('title list')), 'nolabs');
}

function decryChange($type, $v) {global $mya; return ($type == 'V' and !$mya->admV) ? t('(secret)') : u\decry($type, $v);}
