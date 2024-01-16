<?php
namespace CG\Sanity; // typically abbreviated as "san"
use CG as r;
use CG\Web as w;
use CG\DB as db;
use CG\Util as u;

const SAN_FUNCS = 'txHdrs txEntries txDisputes txs2 r_usd2 accounts relations';

/**
 * @file
 * Check the sanity of the database.
 */

function checkSanity() {
  foreach (ray(SAN_FUNCS) as $func) {$func = "CG\\Sanity\\$func"; $func();}
}

function txHdrs() {
  $xids = db\rows('xid', 'tx_hdrs');
  foreach ($xids as $xidO) {
    $xid = $xidO->xid;
    $tx = r\Tx::read($xid);
    foreignKeyCheck('Tx', $xid, 'actorId', 'users', 'uid', $tx);
    foreignKeyCheck('Tx', $xid, 'actorAgentId', 'users', 'uid', $tx);
    foreignKeyCheck('Tx', $xid, 'boxId', 'r_boxes', 'id', $tx);
    foreignKeyCheck('Tx', $xid, 'reversesXid', 'tx_hdrs', 'xid', $tx);
    if (($tx->flags & ~u\bit(ray(TX_FLAGS))) != 0) { sanityErr("Tx $xid flags has extraneous bits set"); }
    if (!in_array($tx->goods, [FOR_GOODS, FOR_USD, FOR_NONGOODS])) { sanityErr("Tx $xid goods is bad: $tx->goods"); }

    if ($tx->entries) {
      $total = 0;
      foreach ($tx->entries as $entry) {
        $total += $entry->amount;
      }
      if ($total != 0) { sanityErr("Tx $xid entries do not sum to 0"); }
    } else sanityErr("Tx $xid has no entries");
  }
}

function txEntries() {
  if ($eids = db\lst('id', 'tx_hdrs LEFT JOIN tx_entries_all e USING(xid)', 'e.deleted')) sanityErr(t('These entries are deleted from non-deleted headers and must be deleted lest the txs table include them!: ') . $eids);
  
  if ($eids = db\lst('id', 'tx_entries JOIN tx_hdrs_all h USING(xid)', 'h.deleted OR h.created IS NULL')) sanityErr(t('These entries are orphaned from their deleted or missing headers and should be deleted: ') . $eids);

  // Check all entries
  $entryIds = db\rows('id', 'tx_entries');
  $total = 0;
  foreach ($entryIds as $entryIdO) {
    $entryId = $entryIdO->id;
    $entry = r\TxEntry::read($entryId);
    foreignKeyCheck('TxEntry', $entryId, 'xid', 'tx_hdrs', 'xid', $entry);
    foreignKeyCheck('TxEntry', $entryId, 'uid', 'users', 'uid', $entry);
    foreignKeyCheck('TxEntry', $entryId, 'agentUid', 'users', 'uid', $entry);
    if ($entry->entryType > count(ray(E_TYPES))) sanityErr("TxEntry $entryId entryType is invalid: $entry->entryType");

    if (!empty($entry->rule)) {
      foreignKeyCheck('TxEntry', $entryId, 'rule', 'tx_rules', 'id', $entry);
    }
    
    $total += $entry->amount;
  }
// odd: this fails but the individual check above succeeds!  if ($total != 0) { sanityErr("Total of all tx_entries is not 0"); }

  if ($id = db\get('id', 'tx_entries e JOIN tx_hdrs h USING (xid)', 'uid IN (:uids) AND created>:dt1 AND (cat IS NULL OR cat=0)', ray('uids dt1', r\catUids(), strtotime('7/1/2021')))) sanityErr("Transaction entry #$id has no accounting category (cat).");
}

function txDisputes() {
  $ids = db\rows('id', 'tx_disputes');
  foreach ($ids as $idO) {
    $id = $idO->id;
    $info = db\get('*', 'tx_disputes', ['id'=>$id]);
    foreignKeyCheck('tx_disputes', $id, 'xid', 'tx_hdrs', 'xid', $info);
    foreignKeyCheck('tx_disputes', $id, 'uid', 'users', 'uid', $info);
    if (!in_array($info['status'], [DS_OPEN, DS_ACCEPTED, DS_DENIED])) {
      $status = $info['status'];
      sanityErr("tx_disputes $id status field is invalid: $status");
    }
  }

  // Check tx_requests */
  $nvids = db\rows('nvid', 'tx_requests');
  foreach ($nvids as $nvidO) {
    $nvid = $nvidO->nvid;
    $info = db\get('*', 'tx_requests', ['nvid'=>$nvid]);
    if ($info['status'] > 0) {
      foreignKeyCheck('tx_requests', $nvid, 'status', 'tx_hdrs', 'xid', $info);
      foreignKeyCheck('tx_requests', $nvid, 'payer', 'txs', 'uid1', $info);
      foreignKeyCheck('tx_requests', $nvid, 'payee', 'txs', 'uid2', $info);
    }
    foreignKeyCheck('tx_requests', $nvid, 'payer', 'users', 'uid', $info);
    foreignKeyCheck('tx_requests', $nvid, 'payee', 'users', 'uid', $info);
  }
}

function txs2() {
  $txids = db\rows('txid', 'txs2');
  foreach ($txids as $txidO) {
    $txid = $txidO->txid;
    $info = db\get('*', 'txs2', compact('txid'));
    if ($info['xid']) {
      $amount = $info['completed'] ? $info['amount'] : 0;
      if ((!db\exists('tx_entries', ['xid'=>$info['xid'], 'uid'=>$info['payee'], 'amount'=>$amount]))
        or (!db\exists('tx_hdrs', ['xid'=>$info['xid']]))) { // tx date need not match usd dates
        sanityErr("txs2 $txid has an xid, but has no corresponding transaction or the transaction doesn't match");
      }
      if (!$info['amount']) sanityErr("txs2 $txid has an associated transaction, but a zero amount");
    } elseif ($info['amount'] + 0) sanityErr("txs2 $txid has an amount but has no associated tx_hdr");
  }
}

function r_usd2() {
  $ids = db\rows('id', 'r_usd2');
  foreach ($ids as $idO) {
    $id = $idO->id;
    $info = db\get('*', 'r_usd2', ['id'=>$id]);
    if (!empty($info['xid'])) {
      if (!empty($info['completed'])) {
        if ((!db\exists('tx_entries', ['xid'=>$info['xid'], 'amount'=>$info['amount']]))
            or (!db\exists('tx_hdrs', ['xid'=>$info['xid'], 'created'=>$info['completed']]))) {
          sanityErr("r_usd2 $id is completed, but the xid doesn't match a tx_entry");
        }
      } else {
        sanityErr("r_usd2 $id is not marked completed, but has an associated tx_hdr");
      }
    } else {
      if (!empty($info['completed'])) {
        sanityErr("r_usd2 $id is marked completed, but has no associated tx_hdr");
      }
    }
  }
}

function accounts() {

}

function relations() {
  $jPerm = r\perm(B_JOINT);
  
  if ($res = db\get('reid, main, other', 'u_relations r LEFT JOIN users u1 ON u1.uid=r.main LEFT JOIN users u2 ON u2.uid=r.other', 'u1.uid IS NULL OR u2.uid IS NULL')) {
    extract($res);
    db\del('u_relations', 'reid', $reid);
    sanityErr("Fixed (deleted) relation record #$reid for non-existent account (main=$main other=$other)");
  }
    
  $q = db\q('SELECT reid, main, other FROM u_relations WHERE permission=:jPerm', compact('jPerm'));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if (!in($jid = db\get('jid', 'users', 'uid=:main', compact('main')), [0, $other])) sanityErr("Mismatch of main's jid with other in relation record #$reid (jid=$jid, main=$main, other=$other)");
  }
  
  $q = db\q('SELECT uid, jid, fullName FROM users WHERE jid<>0');
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $jid2 = db\get('jid', 'users', 'uid=:jid', compact('jid'));
    if (nn($jid2) != $uid) { // other account is not joined back
      db\del('u_relations', 'permission=:jPerm AND :uid IN (main, other)', compact(ray('jPerm uid')));
      db\update('users', ray('jid uid', 0, $uid), 'uid');
      sanityErr("Fixed (unjoined) orphaned joint account: $fullName ($uid)");
    } elseif (r\relation('permission', $uid, $jid) != $jPerm
           or r\relation('permission', $jid, $uid) != $jPerm) {
      sanityErr("Bad joint account relations records for uids: $uid ($fullName) and $jid");
    }

    $a = r\acct($uid);
    if ($a->masterA->hasBank xor $a->masterA->risk('hasBank')) {
      $a->setRisk('hasBank', $a->masterA->hasBank);
      sanityErr("Fixed wrong setting for hasBank in account: $fullName ($uid)");
    }
  }
}

/**
 * Make sure the given field in the given table is what's expected.
 * @param string $prefix: table being checked
 * @param string $ref: key of record being checked
 * @param string $field: name of field being checked
 * @param string $table: table that should have a record with that field's value as its key
 * @param string $keyFld: that table's key field name
 * @param assoc $info: object or array with the desired value in its member indexed by $field
 */
function foreignKeyCheck($prefix, $ref, $field, $table, $keyFld, $info) {
  if (empty($info)) return sanityErr("$prefix $ref record is empty!");
  $keyVal = ((array) $info)[$field];
  if (!empty($keyVal) and !db\exists($table, [$keyFld=>$keyVal])) {
    sanityErr("$prefix $ref $field refers to non-existent $table record $keyVal");
  }
}

function sanityErr($s) {
  global $channel;
  
  if (in($channel, BACK_CHANNELS)) {
    $s = "Sanity error: " . $s;
    flog($s);
    r\tellAdmin($s);
  } else w\say($s, 'err');
}
