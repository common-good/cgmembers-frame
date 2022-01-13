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

  if ($mya->admV) {
    $vsecure = $a->vsecure;
    foreach (ray(VSECURE_FLDS) as $k) if (isset($vsecure[$k])) {
      $vnew[$k] = $vsecure[$k]; // reorder
      $show[$k] = $a->$k; // some need decrypting
    }
    $a->update('vsecure', nn($vnew));
  }

  say(u\rayTable($show, FALSE));
}

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

