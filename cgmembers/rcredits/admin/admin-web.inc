<?php
/**
 * @file
 * CG admin
 */
namespace CG\Web;

use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Admin as a;
use CG\Web as w;

function seeSecure($a) {
  global $mya;
  $secure = $a->secure;
  foreach (ray(SECURE_FLDS) as $k) if (isset($secure[$k])) {
    $new[$k] = $secure[$k]; // reorder
    $show[$k] = $a->$k; // some need decrypting
  }
  $a->update('secure', nn($new, []));

  if ($mya->superAdmin) {
    $vsecure = $a->vsecure;
    foreach (ray(VSECURE_FLDS) as $k) if (isset($vsecure[$k])) {
      $vnew[$k] = $vsecure[$k]; // reorder
      $show[$k] = $a->$k; // some need decrypting
    }
    $a->update('vsecure', nn($vnew));
  }

  say(u\rayTable($show, FALSE));
}

/*
function seeHidden($a) {
  foreach (ray(R_SECRET_FLDS) as $k) if ($k != 'ssnData') $ray[$k] = $a->$k;
  say(u\rayTable($ray, FALSE));
}
*/

/**
 * Change a transaction amount (only if the transaction happened today and only for a non-rewardable type).
 * @param int $xid: tx record ID
 * @param float $oldAmt: the old amount (for a consistency check)
 * @param float $newAmt: what the amount should be
 */ /*
function changeTxAmt($xid, $oldAmt, $newAmt) {
  if (!$rec = db\get('payer,payee,amount,created,goods', 'r_txs', 'xid=:xid', compact('xid'))) return say(t('No such transaction.'), 'ERR');
  extract($rec);
  if ($amount != $oldAmt) return say(t('The existing amount of that transaction is different.'), 'ERR');
  if (u\fmtDate($created) != u\fmtDate(now())) return say(t('That transaction is not from today.'), 'ERR');
  if ($goods) return say(t('That transaction has rewards.'), 'ERR');
  if (!$amount = round($newAmt, 2)) return say(t('Bad new amount.'), 'ERR');

  $DBTX = \db_transaction();
  db\update('r_txs', compact('xid', 'amount'), 'xid');
  $change = $newAmt - $oldAmt;
  if ($payee > 0) db\q('UPDATE users SET r=r+:change WHERE uid=:payee', compact('change', 'payee'));
  if ($payer > 0) db\q('UPDATE users SET r=r-:change WHERE uid=:payer', compact('change', 'payer'));
  say(t('done'));
  unset($DBTX);
}*/


/**
 * Download the member list for the current community
 */
define('EXPORT_PARAMS', 'do allCttys status types chimp anonymous');

function export($params) {
  extract(just(EXPORT_PARAMS, $params, NULL));
  global $mya;
  $cttyA = r\acct($mya->community);
  $solarUid = db\get('uid', 'users', "name='cooppower'");

  $filename = ($allCttys ? PROJECT : str_replace('.', '', $cttyA->name))
  . ($status == 0 ? t('-active') : ($status == 1 ? t('-notyet') : ''))
  . ($types == 0 ? t('-members') : ($types == 1 ? t('-orgs') : t('-accounts')))
  . '.csv';
  u\csvStart($filename);
  $delimiter = $chimp ? "\t" : ',';
  
  $fields = $anonymous ?
    'uid city state zip type'
      . ' created activated signed helper iCode balance rewards savingsAdd minimum share risk trust'
      . ' community co gross employees achMin pinThreshold lastiCode tenure moves mediaConx jid'
      . ' owns bankName slave closed notices secret'
  : ($chimp ? 'id name phone email city state zip community type code'
  : 'id fullName manages lastTx solar snap phone email address city state zip community type code modified created');
  u\csv($fields = ray($fields), $delimiter);

  // add actual accounts
  $where = $allCttys ? '1' : 'community=:cttyId';
  if ($status == 0) $where .= ' AND :IS_OK';
  if ($status == 1) $where .= ' AND NOT :IS_OK';

  if ($types == 0) $where .= ' AND NOT :IS_CO';
  if ($types == 1) $where .= ' AND :IS_CO';

  $sql = "SELECT uid AS id, fullName FROM users WHERE uid>:UID_CANON9 AND (NOT activated OR :IS_OK) AND $where ORDER BY :IS_OK, -signed, -created";
  
  $q = db\q($sql, ray('cttyId', $cttyA->id));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($id);
    $type = $a->ok ? 'member' : ($a->activated ? 'deactivated' : 'notyet');
    if ($a->co) $type .= ' company';
    
    if ($anonymous) {
      foreach ($fields as $k) $$k = $a->$k; // do this first. some will be null.
      $zip = $a->zip;
      foreach (ray('weekly monthly secret') as $k) $$k = $a->can(u\consta('b', $k));
      $notices = $monthly ? t('monthly') : ($weekly ? t('weekly') : t('daily'));
      $state = r\realstate($state);
      if ($routing = substr($a->bankAccount, 4, 9)) {
        $bankName = db\get('name', 'r_banks', 'route=:routing', compact('routing'));
      } else $bankName = '';
    } else {
      list ($firstname, $lastname, $mid) = $a->co ? [$a->fullName, '', ''] : u\parseName($a->fullName, TRUE);
      if (strlen($firstname) < 2 and strlen($mid) > 2) $firstname = $mid;
      $name = $firstname ?: $a->fullName; // Company name or person's first name
      $email = $a->email;
      $phone = $a->phone;
      list ($address, $city, $state, $zip) = $a->postalAddr
      ? u\parseAddr($a->postalAddr)
      : [$a->address, $a->city, $a->st, $a->zip];
      $phone = u\fmtPhone($phone);
      $id = $a->mainQid;
      $code = $a->doCode(); 
      $asof = $modified = u\fmtDate(now());
      $manages = db\get('fullName', 'r_relations r LEFT JOIN users u ON u.uid=r.main', 'r.permission>=:B_MANAGE-:B_RELATED AND u.:IS_CO AND r.other=:id', ['id' => $a->id]);
      $tx = $a->lastTx;
      $lastTx = empty($tx) ? '' : u\fmtDate($tx->created, TRUE);
      $solar = $solarUid ? r\relation(t("'solar'"), $solarUid, $a->id, ':IS_CUSTOMER') : '';
      $snap = $a->snap ? t('SNAP') : '';
      $special = '';
// (BROKEN)      $special = $a->special;
    }
    $community = r\cttyName($a->community, TRUE);
    $created = u\fmtDate($a->created);
//    if (is_numeric($zip) and !@$chimp) $zip .= " "; // prevent interpretation as a number (fails for MailChimp, so don't)
//    if (!@$chimp) foreach ($fields as $k) $$k = '"' . $$k . '"'; // keeps some imports from getting confused
    u\csv(array_values(compact($fields)), $delimiter);
  }
  
  exit();
}
