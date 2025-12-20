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
function formSeeChanges($form, &$sta, $args = '') {
  extract(just('qid table', $args, [NULL, 'users']));
  $a = r\acct($qid);
  $flds = ray('date:Date, rid:ID, field:Field, oldValue:Old Value, newValue:New Value, changerName:Changed By');
  if ($table == 'users') unset($flds['rid']);

  [$idCrit, $subs, $title] = $table == 'users'
  ? ['h.rid=:rid', ray('rid', $a->id), t('Account Changes for ') . $a->fullName]
  : ['1', [], t('Transaction Changes')];

  $list[] = '<tr><th>' . join('</th><th>', array_values($flds)) . '</th></tr>';

  $sql = "SELECT h.created, h.rid, h.field, h.oldValue, h.newValue, h.changedBy, u.name AS changerName FROM changes h LEFT JOIN users u ON u.uid=h.changedBy WHERE h.table='$table' AND $idCrit ORDER BY h.created DESC";
  $q = db\q($sql, $subs);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $date = fmtDt($created);
    foreach (ray('oldValue newValue') as $k) {
      $type = u\crypted($$k);
      $$k = w\formatField($field, $type ? decryChange($type, $$k) : $$k);
    }
    $changedBy = $changerName ?: "($changedBy)";
    if ($table == 'txs') $rid = lnk("/history/transaction/xid=$rid", $rid);
    
    $line = ''; foreach ($flds as $k => $v) $line .= "<td>{${$k}}</td>";
    $list[] = "<tr>$line</tr>\n";
  }

  if (count($list) > 1) {
    $list = join("\n", $list);
    $list = item("<table>$list</table>");
  } else $list = t('No changes for table ' . $table);

  $chx = ray('users txs'); // table choices
  $table = selectFld(t('Table:'), '', dft($table) + short(), array_combine($chx, $chx));
  
  jsx('seeChanges', 'qid', $a->mainQid);
  return cgform(compact(ray('title table list')), 'nolabs');
}

function decryChange($type, $v) {global $mya; return ($type == 'V' and !$mya->admV) ? t('(secret)') : u\decry($type, $v);}
