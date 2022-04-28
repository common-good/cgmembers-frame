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
 *   string return: u\pry'd url to return to (or empty)
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
    $uid = r\isCGs($uid1) ? $uid2
         : (r\isCGs($uid2) ? $uid1
         : ($uid1 == UID_OUTER ? $uid1 
         : ($uid1 <= UID_CANON9 ? $uid2 : $uid1) ));
  }
  $a = r\acct($uid); // not "in" either account, so choose me and you however is easy
  $toMe = ($uid == $uid1);
  list ($me, $you) = u\order($toMe, 2, 1);

  $sql = <<< X
    SELECT t.created, t.amt, t2.pid, t.type, t.for$me AS forMe, t.for$you AS forYou, t.cat$me AS cat, c.category,
      uAgt.fullName AS agt, uAgt.uid AS agtUid, uMe.uid AS meId, eid, recursId, reversesXid
    FROM txs t LEFT JOIN txs2 t2 USING(xid)
    LEFT JOIN users uMe ON uMe.uid=t.uid$me
    LEFT JOIN users uYou ON uYou.uid=t.uid$you
    LEFT JOIN people p ON p.pid=t2.pid
    LEFT JOIN users uAgt ON uAgt.uid=t.agt$me
    LEFT JOIN tx_cats c ON c.id=t.cat$me
    WHERE $where
    ORDER BY t.type LIMIT 1
X;

  if (!$tx = db\q($sql, $subs)->fetchAssoc()) return softErr('no such tx'); // for now just show main transaction
  extract($tx);
  $meA = r\acct($meId);
  
  list ($fullName, $addr, $phone, $email)
    = ($uid1 == UID_OUTER and $toMe) ? r\pContactInfo($pid) : [$a->fullName, $a->postalAddr, $a->phone, $a->email];

  list ($wayYou, $wayMe) = u\order($toMe, t('From:'), t('To:'));
///    debug(compact(ray('uids uid1 uid2 uid toMe amt wayYou wayMe youId fullName')));
  $title = t('Transaction #%xid Detail', compact('xid'));
  $created = editIf($mya->admEditTx, 'datetime', t('Date/Time:'), $created, fmtDt($created) . ' ' . fmtTime($created));
  $nmYou = item($fullName, $wayYou);
  
  if (!$toMe) $amt = -$amt; // get amount to me (negate if from me)
  $amount = editIf($mya->admEditTx, 'num', t('Amount:'), $amt, u\fmtAmt($amt), vmin(-999999));
  $addr = ($toMe or $mya->admin) ? item($addr, t('Postal Addr:')) : NULL; // no addr for people we pay
  $email = item($email, t('Email:'));
  $cat = ($mya->admEditTx and r\isCGs(${"uid$me"})) ? catFld($cat, '') : ($category ? item($category, t('Category:')) : NULL);
  $description = editIf($mya->admEditTx, '', t('For:'), $forMe);
  $forSame = hidFld($forMe == $forYou);
  $agt = item($agtUid == $meId ? t('self') : $agt, t('Our Agent:'));
  $chan = item(ray(TX_CHANNELS)[$channel], t('Channel:'));
  $nmMe = item($meA->fullName, $wayMe);

  if ($mya->admEditTx) {
    $phone = item($phone ? u\fmtPhone($phone) : '', t('Phone:'));
    $entryType = selectFld(REQ . t('Type:'), '', dft($type), ray(E_TYPES));
    $recursId = intFld(t('Recurs Id:'), '', dft($recursId));
    if ($reversesXid) jsx('txdetail', ray('xid url', $xid, $return ? u\dePry($return) : ''));
    $suffix = $reversesXid ? suffix(buttino('', t('Delete this reversed transaction pair'))) : [];
    $reversesXid = intFld(t('Reverses Xid:'), '', dft($reversesXid) + $suffix);

    $ids = db\keyed('DISTINCT ABS(id), entryType', 'tx_entries', compact('xid'));
    foreach ($ids as $id => $etyp) $eids[] = lnk("/history/transaction/xid=$xid&eid=$id&return=$return", ray(E_TYPES)[$etyp]);
    $submit = submit(t('Update'), '', '', suffix(join(' ', $eids)));
    
    foreach (ray('toMe xid eid') as $k) $$k = hidFld($$k);
  } else $phone = $entryType = $submit = $toMe = $xid = $eid = $recursId = $reversesXid = NULL;

  $return = hidFld($return);
  $back = item(w\backButton());

  return cgform(compact(ray('title created amount nmYou nmMe addr email phone description forSame cat agt chan recursId reversesXid entryType submit back toMe xid eid return')));
}

function formTxDetail_validate($form, &$sta) {
  extract(just($fields = 'created amount recursId reversesXid', $sta['input'])); // no default!! (see next line)
  foreach (ray('recursId reversesXid') as $k) if (isset($$k) and !$$k) $$k = NULL;

  if ($created and $err = u\badDate($created, today() + 2 * DAY_SECS - 48 * MONTH_SECS, tomorrow())) return say($err, 'created');
  if ($amount and $err = u\badAmount($amount, '!=0')) return say($err, 'amount');
  tweakSta($sta, compact(ray($fields)));
}

function formTxDetail_submit($form, &$sta) {
  $input = $sta['input'];
  extract(just('amount toMe forSame eid return', $input));
  list ($eidMe, $eidYou) = u\order($toMe, $eid, -$eid);
  
  $DBTX = \db_transaction();
  if ($hInfo = just('xid created recursId reversesXid', $input)) db\update('tx_hdrs_all', $hInfo, 'xid');

  $info = just('entryType amount description cat', $input, NULL);
  if (empty($input['cat'])) $input['cat'] = NULL; // '' (no value chosen) crashes otherwise
  $info['id'] = $eidMe;
  db\update('tx_entries_all', $info, 'id');

  $info['id'] = -$eidMe;
  $info['amount'] = -$amount;
  unset($info['cat']);
  if (!$forSame) unset($info['description']); // keep descriptions the same if they were the same before
  db\update('tx_entries_all', $info, 'id');

  unset($DBTX);
  
  if ($rows = svar('catsList')) { // called from formSetCats
    unset($rows[$eidMe]);
    svar('catsList', $rows);
  }

  return w\go($return ? u\dePry($return) : 'history/transactions');
}
