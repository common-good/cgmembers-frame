<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Display details about one transaction.
 * @param string $args: URL parameter list:
 *   int xid: transaction ID
 *   int eid: entry pair ID (defaults to E_PRIME, E_BANK, or E_OUTER -- whichever exists)
 */
function formTxDetail($form, &$sta, $args = '') {
  global $mya; $myid = $mya->id;
  global $channel;
  
  extract(just('xid eid return', $args, NULL));

  list ($eidCrit, $eidV) = $eid ? ['ABS(t.eid)=:eidV', $eid] : ['t.type IN (:eidV)', E_PRIMES];
  $where = "xid=:xid AND $eidCrit";
  $subs = compact(ray('xid eidV'));

  if (!$uids = db\get('uid1,uid2', 'txs t', $where, $subs)) return softErr('no such tx'); // for now just show main transaction
  extract($uids);
  
  if (in_array($myid, $uids)) {
    $uid = $uid1 == $myid ? $uid2 : $uid1; // set $uid to other account
  } else {
    if (!$mya->admRegion) return softErr(t('That transaction is not one of yours.'));
    $uid = $uid1 == UID_OUTER ? $uid1 
    : (($uid1 <= UID_CANON9 or $uid1 == CGID or ($uid2 != CGID and r\isCGs($uid1))) ? $uid2 : $uid1);
  }
  $a = r\acct($uid); // not "in" either account, so choose me and you however is easy
  $toMe = $uid == $uid1 ? 1 : 0;
  $forFld = $toMe ? 'for2' : 'for1';

  $sql = <<< X
    SELECT t.created, t.amt, t2.pid, t.type, t.$forFld AS purpose, t.cat, c.category, uAgt.fullName AS agt, uAgt.uid AS agtUid, uMe.uid AS meId, eid, recursId, reversesXid
    FROM txs t LEFT JOIN txs2 t2 USING(xid)
    LEFT JOIN users uMe ON uMe.uid=IF($toMe, t.uid2, t.uid1)
    LEFT JOIN users uYou ON uYou.uid=IF($toMe, t.uid1, t.uid2)
    LEFT JOIN people p ON p.pid=t2.pid
    LEFT JOIN users uAgt ON uAgt.uid=IF($toMe, t.agt2, t.agt1)
    LEFT JOIN budget_cats c ON c.id=t.cat
    WHERE $where
    ORDER BY t.type LIMIT 1
X;

  if (!$tx = db\q($sql, $subs)->fetchAssoc()) return softErr('no such tx'); // for now just show main transaction
  extract($tx);
  $meA = r\acct($meId);
  
  list ($fullName, $addr, $phone, $email)
    = ($uid1 == UID_OUTER and $toMe) ? r\pContactInfo($pid) : [$a->fullName, $a->postalAddr, $a->phone, $a->email];

  list ($wayYou, $wayMe) = u\order($toMe xor $amt < 0, t('From:'), t('To:'));
///    debug(compact(ray('uids uid1 uid2 uid toMe amt wayYou wayMe youId fullName')));
  $title = t('Transaction #%xid Detail', compact('xid'));
  $created = editIf($mya->admEditTx, 'datetime', t('Date/Time:'), $created, fmtDt($created) . ' ' . fmtTime($created));
  $you = item($fullName, $wayYou);
  $me = item($meA->fullName, $wayMe);
  
  $amt = editIf($mya->admEditTx, 'num', t('Amount:'), $amt, u\fmtAmt($toMe ? $amt : -$amt), vmin(-999999));
  $addr = ($toMe or $mya->admin) ? item($addr, t('Postal Addr:')) : NULL; // no addr for people we pay
  $email = item($email, t('Email:'));
  $cat = $mya->admEditTx ? catFld($cat, '') : ($category ? item($category, t('Category:')) : NULL);
  $for = editIf($mya->admEditTx, '', t('For:'), $purpose);
  $agt = item($agtUid == $meId ? t('self') : $agt, t('Our Agent:'));
  $chan = item(ray(TX_CHANNELS)[$channel], t('Channel:'));

  if ($mya->admEditTx) {
    $phone = item($phone ? u\fmtPhone($phone) : '', t('Phone:'));
    $type = selectFld(REQ . t('Type:'), '', dft($type), ray(E_TYPES));
    $recursId = intFld(t('Recurs Id:'), '', dft($recursId));
    if ($reversesXid) jsx('txdetail', ray('xid url', $xid, u\dePry($return)));
    $suffix = $reversesXid ? suffix(buttino('', t('Delete this reversed transaction pair'))) : [];
    $reversesXid = intFld(t('Reverses Xid:'), '', dft($reversesXid) + $suffix);

    $ids = db\keyed('DISTINCT ABS(id), entryType', 'tx_entries', compact('xid'));
    foreach ($ids as $id => $etyp) $eids[] = lnk("/history/transaction/xid=$xid&eid=$id&return=$return", ray(E_TYPES)[$etyp]);
    $submit = submit(t('Update'), '', '', suffix(join(' ', $eids)));
    
    foreach (ray('toMe xid eid') as $k) $$k = hidFld($$k);
  } else $phone = $type = $submit = $toMe = $xid = $eid = $recursId = $reversesXid = NULL;

  $return = hidFld($return);
  $back = item(w\backButton());

  return cgform(compact(ray('title created amt you me addr email phone for cat agt chan recursId reversesXid type submit back toMe xid eid return')));
}

function formTxDetail_validate($form, &$sta) {
  extract(just($fields = 'created amt recursId reversesXid', $sta['input'])); // no default!! (see next line)
  foreach (ray('recursId reversesXid') as $k) if (isset($$k) and !$$k) $$k = NULL;

  if ($created and $err = u\badDate($created, today() + 2 * DAY_SECS - 12 * MONTH_SECS, tomorrow())) return say($err, 'created');
  if ($amt and $err = u\badAmount($amt, '!=0')) return say($err, 'amt');
  tweakSta($sta, compact(ray($fields)));
}

function formTxDetail_submit($form, &$sta) {
  $input = $sta['input'];
  extract(just('toMe for xid eid return', $input));
  $myEid = $toMe ? $eid : -$eid;
  
  $DBTX = \db_transaction();
  if ($info = just('xid created recursId reversesXid', $input)) db\update('tx_hdrs_all', $info, 'xid');

  if ($info = just('xid eid type amt cat', $input)) {
    if (!$info['cat']) $info['cat'] = NULL;
    db\q('UPDATE tx_entries_all SET entryType=:type, amount=IF(id<0, -:amt, :amt), cat=:cat WHERE xid=:xid AND ABS(id)=:eid', $info);
    if (db\exists('txs', 'for1=for2 AND xid=:xid', compact('xid'))) {
      db\update('tx_entries_all', ray('id description', -$myEid, $for), 'id'); // keep description the same if it is
    }
    db\update('tx_entries_all', ray('id description', $myEid, $for), 'id');
  }
  unset($DBTX);

  return w\go($return ? u\dePry($return) : 'history/transactions');
}
