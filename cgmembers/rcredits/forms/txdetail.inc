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
  
  extract(just('xid eid return', $args, NULL));

  list ($eidCrit, $eidV) = $eid ? ['ABS(t.eid)=:eidV', $eid] : ['t.type IN (:eidV)', E_PRIMES];
  $where = "xid=:xid AND $eidCrit";
  $subs = compact(ray('xid eidV'));

  if (!$orig = db\get('*', 'txs t', $where, $subs)) return softErr('no such tx'); // for now just show main transaction
  extract(just('uid1,uid2', $orig));
  
  if ($myid == $uid1) {
    $uid = $uid2; // set $uid to other account
  } elseif ($myid == $uid2) {
    $uid = $uid1; // set $uid to other account
  } else { // not my transaction, so choose me and you however is easy
    if (!$mya->admRegion) return softErr(t('That transaction is not one of yours.'));
    $uid = r\hasCats($uid1) ? $uid2 // be sure to supply cat dropdown if appropriate
         : (r\hasCats($uid2) ? $uid1
         : ($uid1 == UID_OUTER ? $uid1 
         : ($uid1 <= UID_CANON9 ? $uid2 : $uid1) ));
  }
  $a = r\acct($uid);
  $toMe = ($uid == $uid1);
  list ($me, $you) = u\order($toMe, 2, 1);

  $sql = <<< X
    SELECT t.created, t.channel, t.boxId, t.amt, t2.pid, t.type, t.for$me AS forMe, t.for$you AS forYou, t.cat$me AS cat, c.category,
      uAgt.fullName AS agt, uAgt.uid AS agtUid, uMe.uid AS meId, eid, recursId, reversesXid, t.flags
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

  $cat = ($mya->admEditTx and r\hasCats(${"uid$me"}))
  ? (($mya->sponsored and !$mya->admV) ? fboCatFld($cat, $toMe ? 'IN' : 'OUT') : catFld($cat, '')) // admV lets you choose ANY category
  : ($category ? item(str_replace(FBO_OUT_HDR, '', $category), t('Category:')) : NULL);
  
  $description = editIf($mya->admEditTx, '', t('For:'), $forMe);
  $forSame = hidFld($forMe == $forYou);
  $agt = item($agtUid == $meId ? t('self') : $agt, t('Our Agent:'));
  
  $box = t(' -- <b>Box</b> #%boxId', compact('boxId'));
  $version = $channel == TX_APP ? t(' (v. %version)', 'version', db\get('version', 'r_boxes', ray('id', $boxId))) : '';
  $chan = item(ray(TX_CHANNELS)[$channel] . $box . $version, t('Channel:'));
  $flags = editIf($mya->admEditTx, '', t('Flags:'), $flags);
  $nmMe = item($meA->fullName, $wayMe);

  if ($mya->admEditTx) {
    $phone = item($phone ? u\fmtPhone($phone) : '', t('Phone:'));
    $entryType = selectFld(REQ . t('Type:'), '', dft($type), ray(E_TYPES));
    $recursId = intFld(t('Recurs Id:'), '', dft($recursId));
    $suffix = ($reversesXid and !in($uid2, [UID_OUTER, UID_BANK])) ? suffix(buttino('', t('Delete this reversed transaction pair'))) : [];
    $reversesXid = intFld(t('Reverses Xid:'), '', dft($reversesXid) + $suffix);

    $ids = db\keyed('DISTINCT ABS(id), entryType', 'tx_entries', compact('xid'));
    foreach ($ids as $id => $etyp) $eids[] = lnk("/history/transaction/xid=$xid&eid=$id&return=$return", ray(E_TYPES)[$etyp]);
    $suffix = join(' ', $eids);
    if ($ids[$eid] != E_PRIME) $suffix .=  ' ' . buttino('', t('Delete this auxiliary transaction'));
    $submit = submit(t('Update'), '', '', suffix($suffix));
    if ($reversesXid or count($ids) > 1) jsx('txdetail', ray('xid eid url', $xid, $eid, $return ? u\dePry($return) : ''));
    foreach (ray('toMe xid eid') as $k) $$k = hidFld($$k);
  } else $phone = $entryType = $submit = $toMe = $xid = $eid = $recursId = $reversesXid = NULL;

  $return = hidFld($return);
  $back = item(w\backButton());
  $orig = hidFld($orig);

  return cgform(compact(ray('title created amount nmYou nmMe addr email phone description forSame cat agt chan flags recursId reversesXid entryType submit back toMe xid eid return orig')));
}

function formTxDetail_validate($form, &$sta) {
  global $mya;
  extract(just($fields = 'created amount recursId reversesXid orig', $sta['input'])); // no default!! (see next line)
  foreach (ray('recursId reversesXid') as $k) if (isset($$k) and !$$k) $$k = NULL;
  
  if ($created and $err = u\badDate($created, today() + 2 * DAY_SECS - 48 * MONTH_SECS, tomorrow())) return say($err, 'created');
  if ($mya->admEditTx and min($created, nni($orig, 'created')) < getV(VAR_CLOSE_BOOKS)) return err(t('You cannot change this transaction without first changing the "close books" date.'));
  
  if ($amount and $err = u\badAmount($amount, '!=0')) return say($err, 'amount');
  tweakSta($sta, compact(ray($fields)));
}

function formTxDetail_submit($form, &$sta) {
  global $mya;
  $input = $sta['input'];
  extract(just('amount toMe forSame eid return cat orig created xid', $input, ''));
  list ($eidMe, $eidYou) = u\order($toMe, $eid, -$eid);
  
  $DBTX = \db_transaction();
  if ($hInfo = just('xid created recursId reversesXid flags', $input)) db\update('tx_hdrs_all', $hInfo, 'xid');
  if (now() - $created > 3 * MONTH_SECS) say(t('WARNING: This transaction is more than 3 months in the past.'));

  $info = just('entryType amount description cat', $input, NULL);
  if (!$cat) $input['cat'] = NULL; // '' (no value chosen) crashes otherwise
  $info['id'] = $eidMe;
  db\update('tx_entries_all', $info, 'id');
  if ($cat == CAT_CG2CG) db\update('tx_entries_all', ray('id cat', -$eidMe, CAT_CG2CG), 'id'); // cg2cg transfer means cats are both CG2CG
  
  extract(just('cat1 cat2 uid1 uid2 flags created', $orig, NULL), EXTR_PREFIX_ALL, 'o');
  if ($mya->admEditTx and u\getBit($o_flags, B_QBOK) 
    and ($cat != ($toMe ? $o_cat2 : $o_cat1) or $created != $o_created)
    and (r\hasCats($o_uid1) or r\hasCats($o_uid2))) { // cat changed, re-export to QBO
    require_once(R_ROOT . '/classes/qb.class');
    db\q('UPDATE tx_hdrs_all SET flags=flags & ~' . u\bit(B_QBOK), compact('xid')); // resend this xid to QBO (do this before delQbTx)
    $qb = new \QB(QBO_CREDS, FALSE);
    $qb->delQbTx("cg#$xid"); // delete this xid from QBO
    setV(VAR_SETCAT_DT0, min(today($created), getV(VAR_SETCAT_DT0, now()))); // tell setCats when to start
  }

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
