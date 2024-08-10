<?php
/**
 * @file
 * Top level application interfaces (SMS, web, mobile, etc.) to Commnon Good.
 * These high-level business functions are called by more than one interface.
 */

namespace CG\Backend; // typically abbreviated as "be"
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Web as w;
use CG\QR as qr;
require_once R_ROOT . '/classes/tx.class';
require_once R_ROOT . '/classes/txentry.class';

/**
 * Return an array of transactions, with added data.
 * @param mixed $uids: the uid or uids whose transactions we're going to process
 *        if $uids is an array it is the set of uids;
 *        if $uids is an integer it is the only uid
 * @param integer $starting: the starting unix time
 * @param integer $ending: the ending unix time
 * @param assoc $extra: extra parameters
 *   includeSelfTxs: <include transactions between the various $uids>
 *   catsOnly: <include only transactions that have cat1 or cat2 set (or that are a credit card payment) AND B_QBOK flag off> 
 *     NOTE: usually cats are set manually or by be\setCat(), but CC donations are handled differently and cat1 gets set in Qb::sendCCTx()
 * @return an array of transactions each element of which is a single entry associated with the transaction.  Note that tx_hdrs data is in each of the entries. If there's a failure return [].
 */
function getTxs($uids, $starting = NULL, $ending = NULL, $extra = []) {
  if (is_integer($uids)) $uids = [$uids];

  extract(just('includeSelfTxs catsOnly', $extra, FALSE));

  $fromMe = 'x.uid1 IN (:uids)';
  $toMe = 'x.uid2 IN (:uids)';
  $where = tr("($fromMe %or $toMe)", 'or', $includeSelfTxs ? 'OR' : 'XOR');
  $where .= ' AND !(x.amt=0 AND x.type=:E_BANK)'; // hides bank account verifications
  if (is_integer($starting)) $where .= " AND x.created>=$starting";
  if (is_integer($ending)) $where .= " AND x.created<=$ending";
  if ($catsOnly) {
    $where .= " AND (x.cat1 IS NOT NULL OR x.cat2 IS NOT NULL OR (x.:IS_CC AND x.uid1=:UID_OUTER)) AND NOT x.:IS_QBOK";
    $catUids = join(',', r\catUids());
    $toMe = "($toMe AND (NOT ($fromMe) OR x.uid2 IN ($catUids)))";
  }
  $special = be\specialTxFlds();
  $order = 'x.created, x.xid, x.eid, name';

  // tReversed and eReversed refer to the transaction that this transaction reverses.
  // tReversedBy and eReversedBy refer to the transaction that reverses this transaction. 

  $sql = <<< X
    SELECT x.*, t2.txid, (t2.bankAccount IS NULL) AS noBank, t2.pid, u.uid, u.fullName AS name, tm.period, tm.periods, ru.portion, $toMe AS toMe, 
      IF($toMe, amt, -amt) AS amount, IF($toMe, for2, for1) AS description, IF($toMe, agt2, agt1) AS agt, $special,
      (ru.action=:ACT_SURTX AND IF($toMe, ru.to=x.uid2, ru.from IN (:MATCH_PAYER, x.uid1))) AS stepup,
      (SELECT xid FROM tx_hdrs tRev WHERE tRev.reversesXid=x.xid) AS reversedBy,
      (EXISTS (SELECT id FROM tx_disputes d WHERE d.xid=x.xid AND d.status=:DS_OPEN)) AS isDisputed
    FROM txs x
    LEFT JOIN users u ON u.uid=IF($toMe, x.uid1, x.uid2)
    LEFT JOIN txs2 t2 ON t2.xid=x.xid
    LEFT JOIN tx_timed tm ON tm.id=x.recursId
    LEFT JOIN tx_rules ru ON ru.id=x.rule
    LEFT JOIN tx_requests i ON i.nvid=x.rel AND x.relType='I'
    WHERE $where ORDER BY $order
X;

///  showq($sql, ray('uids', join(',', $uids)));
  $q = db\q($sql, compact('uids'));
  u\EXPECT($q, "Failure reading transactions for user " . pr($uids));
  $lastXid = 0;  // no xid should ever be 0
  $txs = [];  // an array of arrays of entries
  $entries = [];

  while ($row = $q->fetchAssoc()) {
    $xid = $row['xid'];
    if ($xid != $lastXid) {  // start of new transaction
      if ($entries) $txs[] = $entries; // if there was a transaction already
      $entries = [];
      $lastXid = $xid;
    }
    $entries[] = $row;
  }
  if ($entries) $txs[] = $entries;
  
  // arrange for running balance to appear in, and be the same in, every entry for a transaction
  $balance = balanceAsOf($uids, $starting);
  $catUids = r\catUids();
  
  foreach ($txs as $i => $entries) {
    extract(just('actorId type xid pid uid name', $entries[0]));
    $byMe = in_array($actorId, $uids);
    $banking = ($type == E_BANK);
    $newEntries = [];
    $myAmount = 0; foreach ($entries as $entry) $myAmount += $entry['amount'];
    $balance = round($balance + $myAmount, 2);
    if ($xid < 124873 and $banking and !isDEV) $xid = db\get('txid', 'txs2_bank', compact('xid')) ?: $xid; // old checks
    
    foreach ($entries as $j => $entry) {
      if ($j == 0) { // use values extracted above to handle special names for first entry in a tx
        $entry['name'] = $banking ? '--' // bank transfer
        : (in($uid, BUCKET_UIDS) ? (in(CGID, $uids) ? w\lnk("/change-account/acct=$uid", t('various')) : PROJECT) // CG
        : (($type == E_OUTER and $pid) ? db\get('fullName', 'people', compact('pid')) . t(' (non-member)') // non-member
        : $name )); // normal
      }

      $description = txDesc($entry);
      $newEntries[] = compact(ray('myAmount balance byMe banking xid description')) + $entry;
    }
    $txs[$i] = $newEntries;
  }

  return $txs;
}

/**
 * Return an array of transactions and invoices between two accounts (the first of which has invoiced the second), for the given period.
 * @param array $aUids: the uids for member a (normally a business) -- NULL for all members
 * @param array $bUids: the uids for member b -- NULL for all members (used for Combo download)
 * @param unixtime $starting: the starting date/time for the period
 * @param unixtime $ending: the ending date/time for the period, e.g., $starting plus one day
 */
function getCustTxs($aUids, $bUids, $starting, $ending) {
  u\EXPECT('array array int int', $aUids, $bUids, $starting, $ending);
  // Note that we show the company's tx description, not the customer's  
  $reverseText = t('reverses tx #');
  $special = be\specialTxFlds();
  $markedPAID = 'status=:TX_PAID';
  [$fromA, $fromB] = ['uid1 IN (:aUids)', 'uid1 IN (:bUids)'];
  [$toA, $toB] = ['uid2 IN (:aUids)', 'uid2 IN (:bUids)'];
  foreach (ray('a b') as $k) {
    $K = strtoupper($k);
    if (!${$k . 'Uids'}) ${"from$K"} = ${"to$K"} = 'TRUE';
  }
  $amtInvoiced = "IF ($toA, amount, -amount)"; // applies only to invoices
  $where = "(($fromA AND $toB) OR ($fromB AND $toA)) AND x.created BETWEEN $starting AND $ending";
  
  $sql = <<< X
    SELECT DISTINCT CONCAT('tx #', xid) AS tid, '' AS reasonData,
      IF($fromA, -amt, amt) AS paid, $special, :E_PRIME AS type,
      u.uid, u.fullName AS name, i.status,
      '' AS invoiced, x.created, (d.status=:DS_OPEN) AS disputed,
      IF(x.reversesXid, CONCAT('($reverseText', x.reversesXid, ')'), IF($fromA, for1, for2)) AS description
    FROM txs_prime x LEFT JOIN tx_disputes d USING(xid)
    LEFT JOIN tx_requests i ON i.nvid=x.rel
    LEFT JOIN users u ON u.uid=IF($fromA, uid2, uid1)
    WHERE $where
    
    UNION ALL
    SELECT CONCAT('inv #', nvid) AS tid, IF($markedPAID, x.data, '') AS reasonData,
      IF($markedPAID, $amtInvoiced, '') AS paid,
      NULL AS nvid, NULL AS part, NULL AS txCanceled, 'inv' AS type,
      u.uid, u.fullName AS name, status,
      $amtInvoiced AS invoiced,
      x.created AS created, status=:TX_DENIED AS disputed, x.`purpose` AS description
    FROM (SELECT inv0.*, payer AS uid1, payee AS uid2 FROM tx_requests inv0) x
    LEFT JOIN users u ON u.uid=IF($fromA, uid2, uid1)
    WHERE $where AND NOT x.:IS_SELF
  
    ORDER BY created, tid, invoiced DESC, paid<>0
X;

///  showq($sql, ['aUids' => $aUids, 'bUids' => $bUids]);
  $q = db\q($sql, ['aUids' => $aUids, 'bUids' => $bUids]);
  u\EXPECT($q, "Failure reading transactions between users " . pr($aUids) . " and " . pr($bUids));

  $myBalance = balanceAsOf($aUids[0], $starting);
  $entries[] = ray('tid created uid name description balance invoiced paid', '', $starting, '', '', t('Opening balance'), $myBalance, '', '');
  foreach ($q as $row) {
    $row = (array) $row;
    extract(just('invoiced paid disputed reasonData status', $row));
//    unset($row['reasonData']);
    $myBalance -= $paid ?: 0;
    $row['balance'] = $myBalance;
    $disputed = $disputed ? t('(DISPUTED) ') : '';
    $row['status'] = !$invoiced ? '' : (is_numeric($status) ? be\statusDesc($status) : $status);
    if ($reasonData and $data = unserialize($reasonData)) {
      $reason = nni($data, 'reason') ?: nni($data, 'whyNot'); // whyNot is deprecated
      $reason = $status == TX_PAID ? t(' (marked PAID: %reason)', compact('reason'))
      : ($status == TX_CANCELED ? t(' (canceled: %reason)', compact('reason'))
      : " $reason");
    } else $reason = '';
    $row['description'] = $disputed . be\txDesc($row) . $reason;
    $entries[] = $row;
  }

  return $entries;
}

/**
 * Return SQL for nvid, part, and stepup in a tx entry.
 * Don't include invoice number for payApproved requests
 * Expects t=txs, i=tx_requests
 */
function specialTxFlds() {
  list ($finalTxt, $partTxt, $overTxt) = [t('final'), t('partial'), t('overpaid')];
  return <<< X
    IF(x.relType='I' AND NOT i.:IS_SELF, x.rel, NULL) AS nvid, 
    IF(x.xid=i.status, 
      CONCAT(IF(x.:IS_PARTIAL, ' $finalTxt', ''), IF(x.:IS_OVERPAID, ' - $overTxt', '')),
      IF(x.:IS_PARTIAL, ' $partTxt', '')) AS part,
    (x.relType='I' AND NOT i.:IS_SELF AND (i.status IS NULL OR i.status=:TX_CANCELED)) AS txCanceled
X;
}

/**
 * Return a (modified) description for the given transaction entry.
 */
function txDesc($entry) {
  extract(just('type reversesXid reversedBy description period periods stepup portion nvid part txCanceled', $entry, NULL));

  if (in($type, E_PRIMES)) {
    if ($reversesXid) {
      $description .= t(' (reverses tx #%reversesXid)', compact('reversesXid'));
    } else {
      if ($period) $description .= t(' (%xly)', 'xly', r\recurDesc($period, $periods));
      if ($txCanceled) $part .= t(' -- invoice canceled');
      if ($nvid) $description .= t(' (CG inv#%nvid%part)', compact(ray('nvid part')));
      if ($reversedBy) $description .= t(' (reversed by #%reversedBy)', compact('reversedBy'));
    }
  } else {
    if ($stepup) $description .= $portion > 0 ? t(' (%x step-up)', 'x', u\fmtAmt($portion, '%s')) : t(' (step-up)');
  }
  return $description;
}

/**
 * Return a description of the given payment request status.
 */
function statusDesc($status) {
  $statuses = ray(TX_STATUSES);
  if ($status >= TX_PAID) {
    return t('paid') . ($status > TX_PAID ? " (Tx#$status)" : ''); // xid=TX_PAID always has a reason unless the first tx was invoiced;
  } else return ucfirst($statuses[$status]);
}

/**
 * Processes a set of transactions, e.g., for generating a report.
 * @param TxProcessor $processor: an implementation of the TxProcessor interface to interact with
 * @param integer $starting: the starting unix time
 * @param integer $ending: the ending unix time
 * @param mixed $uids: the uid or uids whose transactions we're going to process
 *        if $uids is an array it is the set of uids;
 *        if $uids is an integer it is the only uid
 * @param integer $jid: the joint account holder (if any)
 * @return TRUE on success, FALSE otherwise.
 */
function processTxs($processor, $uids, $starting=NULL, $ending=NULL, $descending=FALSE) {
  if (strhas(get_class($processor), 'Combo')) {
// NO    $bUids = db\col('payer', 'tx_requests', 'payee IN (:uids)', compact('uids'));
    $txs = getCustTxs($uids, NULL, $starting, $ending);
    unset($txs[0]); // discard opening balance (not relevant for multiple customers)
    if ($descending) $txs = array_reverse($txs);
    $processor->hdr();
    foreach ($txs as $tx) $processor->processEntry($tx);
    $processor->ftr();
    return;
  }
  
  $txs = getTxs($uids, $starting, $ending);
  if ($descending) $txs = array_reverse($txs);
  
  // Now process the transactions
  $processor->hdr();
  $first = TRUE;
  
  foreach ($txs as $entries) {
    $processor->txnHdr($entries[0], $first ? 'none' : '');
    foreach ($entries as $j => $entry) {
      $processor->processEntry($entry, $j & 1);
    }
    $processor->txnFtr($entries[0]);
    $first = FALSE;
  }
  
  $processor->ftr();
}

/**
 * Returns the balance in an account as of a specified date.
 * @param integer $uid: the user id of the account
 * @param integer $date: the unix time to consider (NULL means at the beginning of time)
 */
function balanceAsOf($uids, $date=NULL) {
  if (is_null($date)) return 0;
  if (is_integer($uids)) {
    $a = r\acct($uids);
    ($a->jid != 0) ? $uids = [$uids, $a->jid] : [$uids];
  }
  return db\sum('amount', 'tx_entries JOIN tx_hdrs USING(xid)', 'uid IN (:uids) AND created<:date', compact(ray('uids date')));
}
  
/**
 * Return a list of members to choose from (called with ajax, so current user is passed as arguments).
 * Restricted to within NEIGHBOR_MAX miles of the member
 * Called only from 'suggestWho' in ajax.inc
 * @param int $myid: account record ID
 * @param int $aid: agent account record ID
 * @param string $restrict: MySQL to restrict the selection (for example ':IS_CO' or ':IS_OK')
 * @return array of choices: member fullName (city, ST), indexed by uid
 */
function memberRay($myid, $aid, $restrict = '') {
  if (!$a = r\acct($myid, $aid)) return []; // no choices if not signed in
  return identify('', $a, '', $restrict, TRUE);
}

/**
 * Return the next relevant transaction for form 1099-B or 1099-K, for an account.
 * @param string $type: B or K
 * @param int $payee: the account record ID
 * @param int $year: year or end-of-year date
 * @return the next relevant row (NULL if no more)
 * Note that the IRS prohibits reporting non-positive transactions (but requires all the others).
 * Also note that this report is always for just one account (and for just one SSN or EIN) 
 *   even if the account is joint.
 */
function get1099($type, $payee, $year = NULL) {
  global $q1099, $ignore1099;
  
  if (is_null($q1099)) {
    u\setDft($year, strtotime('last day of December last year'));
    $end = strlen($year) < 6 ? strtotime('1/1/' . ($year + 1)) - 1 : strtotime('tomorrow', $year);
    $start = strtotime('1 year ago', $end) + 1;
    if (test()) list ($start, $end) = [NOW - 12 * 30 * DAY_SECS, NOW - 1];
    $where = "uid2=:payee AND t.created BETWEEN $start AND $end AND type NOT IN (:E_BANK,:E_OUTER,:E_REBATE)";

    if ($type == 'B') $sql = <<< X
      SELECT created, amt AS amount, uid1 AS payer, for2 AS `for`
      FROM txs_noreverse t WHERE $where ORDER BY t.created
X;

    if ($type == 'K' ) {
      for ($m = 1; $m <= 12; $m++) {
        $dt1 = $m == 1 ? $start : ($dt2 + 1);
        $dt2 = u\plusMonths(1, $dt1) - 1;
        if (test()) $dt2 = $dt1 + 30 * DAY_SECS - 1;
        $moFlds[] = "SUM(IF(t.created BETWEEN $dt1 AND $dt2, amt, 0)) AS amt$m";
      }
      $moFlds = join(', ', $moFlds);
      $sql = <<< X
        SELECT uid2, SUM(amt) AS gross,
               SUM(IF(channel=:TX_APP, 0, amt)) AS noCard,
               SUM(1) AS txCount, $moFlds,
               s.abbreviation AS st
        FROM txs_noreverse t
        LEFT JOIN users u ON u.uid=t.uid2
        LEFT JOIN r_states s ON s.id=u.state AND s.country_id=u.country
        WHERE $where
X;
    }
    
    $q1099 = db\q($sql, compact('payee'));
  }
  return $q1099->fetchAssoc() ?: ($q1099 = NULL);
}

/**
 * Reverse a recent transaction at point of sale.
 * @param acct $mya: current account (possibly from scanner cookie, not signed in)
 * @param int $xid: record ID of transaction to reverse
 * @param int $force: reverse the transaction even if there are insufficient funds or lack of permission
 * @param string $where: additional criteria, to identify transaction to reverse
 * @return a txRet object, with ->subs including:
 *   xid: record ID of transaction that reverses the given tx OR
 *   nvid: record ID of invoice that reverses the given tx
 *   created: date of that reversing transaction or invoice
 */
function posUndo($mya, $xid, $force = FALSE, $where = 1) {
  if ($subs = db\get('xid,created', 'tx_hdrs', 'reversesXid=:xid', compact('xid'))) { // already reversed
    $msg = t('Transaction has been reversed.');
    return obj('ok index subs msg', TRUE, $msg, $subs, $msg);
  }
  
  $z = be\undoTx($mya, $xid, $where, [], $force);
  if ($z->ok) r\tellAdmin('reversing tx', ray('xid who created message subs', $xid, "$mya->fullName ($mya->mainQid)", fmtDt($z->subs['created']), $z->msg, $z->subs)); // if ($z->ok and nni($z->subs, 'xid'))
  return $z;
}

/**
 * Undo the given transaction.
 * Situations:
 *     I paid someone (byMe):            REVERSE (invoice)
 *     I charged someone (toMe/byMe):    REVERSE (pay back)
 *     Someone paid me (toMe):           REVERSE (pay back)
 *     Someone invoiced me and I approved it: DISPUTED
 *     Someone charged me unilaterally: DISPUTED
 * @param acct $mya: current account (possibly from scanner cookie, not signed in)
 * @param int $xid: the transaction number
 * @param string $where: additional (security check) criteria, if any
 * @param assoc $subs: additional substitutions for $where
 * @param bool $force: <reverse despite any shortfall or lack of permission>
 * @return a txRet object
 */
function undoTx($mya, $xid, $where = '1', $subs = [], $force = FALSE) {
  global $channel;

  u\EXPECT('int string assoc bool', $xid, $where, $subs, $force);

  $xidSub = compact('xid');
  $tx = $mya->lastTx("xid=:xid AND $where", $xidSub + $subs);
  if (empty($tx)) return txRet(FALSE, 'undo no match', $subs);
  if (!$force and $zErr = $tx->noUndo($mya, $subs)) return $zErr;

  if (db\exists('tx_hdrs', 'reversesXid=:xid', $xidSub)) return txErr(t('already reversed', $xidSub), $subs);
  if (db\exists('tx_requests', 'reversesXid=:xid AND status NOT IN (:TX_DENIED)', $xidSub)) return txErr(t('already reverse requested', $xidSub), $subs);
  $otherE = $tx->otherEntry;
  $byMe = $mya->isMe($tx->actorId);
  $toMe = $tx->getAmount($mya) > 0;
  $posUndo = ($channel == TX_APP and $tx->created >= now() - APP_REVERSE_WINDOW);
  $donUndo = ($tx->uid2 == UID_REGULARS 
    and ($tx->created >= now() - DON_REVERSE_WINDOW or ($mya0 = r\acct() and in($mya0->id, r\cgsOwnUids()) and $mya0->can(B_REFUND)))
  );

  if ($toMe or $posUndo or $donUndo) { // someone paid me (at my initiative or theirs) or just charged me at POS or I recently made a recurring donation
    return reverse($xid, $mya); // create an offsetting transaction
  } elseif ($byMe) { // I paid someone (on my initiative)
///    debug("other=$tx->otherId actor=$tx->actorId amt=$otherE->amount posUndo=$posUndo toMe=$toMe");
    return invoice($tx->actorA, $tx->otherA, $otherE->amount, $otherE->description, $tx->goods, ray('reversesXid', $xid), $tx->recursId);
  } else { // !$toMe and !$byMe: someone charged me (either unilaterally or I approved it)
    if (db\exists('tx_disputes', compact('xid'))) return txErr(t('already reverse requested', $xidSub), $subs);
    $result = db\insert('tx_disputes', ray('xid reason status uid agentUid', $xid, nn($reason) ?: t('reason not specified'), DS_OPEN, $mya->uid, $mya->agentId), 'id');
  
    $solution = t('marked "disputed"');
    $tx->otherA->tellOut('charge disputed', $subs = $tx->reportArgs($mya)); //, $tx->getMyAmt(), $payer, $payee, $payerPurpose, $payeePurpose));
    $amount = u\fmtAmt($otherE->amount);
    $tofrom = !$tx->toMe ? t('to') : t('from');
    $otherUid = $tx->otherId;
    $other = $tx->otherA->fullName;
    return txRet(TRUE, 'report undo', compact(ray('amount other tofrom solution otherUid')));
  }
}

/**
 * Create an offsetting transaction (nearly the same as the original but with the entries negated).
 * @param int $xid: record ID of the transaction to reverse
 * @param acct $mya: current account (not global, in case not signed in)
 * @returns object $txRet: result, message, and subs
 */
function reverse($xid, $mya) {
  global $channel;
  $subs = compact('xid');

  if (!$tx = r\Tx::read($xid)) return txErr(t('no transaction'), $subs);
  $e = $tx->otherEntry; // get payer half of prime entry pair  

  $DBTX = \db_transaction();

  if ($ruid = $e->rule and $cred = creditPeops($ruid)) { // returning a purchase of credit, so end the rule
    db\update('tx_rules', ray('id end', $ruid, now()), 'id');
    extract(just('buyerA coA', $cred));
    $amt = u\fmtAmt($e->amount);
    $buyerA->tellIn('your credit canceled', ray('amount co', $amt, $coA->fullName), t('Credit canceled'));
    $coA->tellOut('customer credit canceled', ray('customer amount', $buyerA->fullName, $amt), t('Credit canceled'));
  }   

  $newTx = clone $tx;  // this is a shallow copy, entries points to the same entries
  $newTx->xid = NULL;
  $newTx->reversesXid = $xid;
  $newTx->entries = [];
  $newTx->created = now();
  $newTx->actorId = $mya->id;
  $newTx->actorAgentId = $mya->agentId;
  $newTx->flags &= ~u\bit(B_QBOK); // reversing transaction has not been sent to QB yet
  
  foreach ($tx->entries as $oldE) {
    $newE = clone $oldE;
    $newE->id = NULL;
    $newE->xid = NULL;  // new entry belongs to reversing transaction
    $newE->amount = -$oldE->amount;
    $newTx->entries[] = $newE;
    if ($oldE->relType == 'I') db\update('tx_requests', ray('nvid status', $oldE->relatedId, TX_PENDING), 'nvid');
  }
  
  if (r\isBank($newTx->uid1)) foreach ([0, 1] as $i) $newTx->entries[$i]->description = t('bank transfer adjustment');

  $xid = $newTx->insert();
  
  unset($DBTX);

  $z = sendNotifications($mya, 'undo', $newTx, 0);

  $message = 'report undo' . ($tx->otherA->isCanonic ? '' : '|report tx');

  $ok = TRUE;
//  $txid = $xid; // txid is xid in transaction reports
//  $created = $newTx->created;
  if ($channel == TX_APP) $z->subs['balance'] = $tx->otherA->secret ? '*0' : $tx->otherA->balance; // report customer balance for POS transactions, not actor's
  $solution = 'reversed';
//  $otherName = $tx->otherA->fullName;
  $subs = compact(ray('ok solution')) + $z->subs;
  
  return txRet(TRUE, $message, $subs);
}

/**
 * Return details about a store credit rule.
 * @param int $id: rule id
 * @return assoc [buyerA, coA] (buyer account and store account) -- FALSE if $id does not point to a store credit rule
 */
function creditPeops($id) {
  $info = db\get('*', 'tx_rules', compact('id'));
  extract(just('action portion from to payerType payeeType payer payee', $info));
  if ($action == ACT_SURTX and $portion == 1 and $from == MATCH_PAYEE and $to == MATCH_PAYER and $payerType == REF_ACCOUNT and $payeeType == REF_ACCOUNT and $buyerA = r\acct($payer) and $coA = r\acct($payee)) {
    return compact(ray('buyerA coA'));
  } else return FALSE;
}

/**
 * Try to pay the given invoice, possibly without being signed in.
 * @param int $nvid: record ID of invoice to pay
 * @param numeric $payAmount: amount to pay (defaults to the invoice amount)
 * @param bool $auto: don't pull any more from bank than needed
 * @return a txRet object
 */
function payInvoice($nvid, $payAmount = NULL, $auto = TRUE) {
  u\EXPECT('int', $nvid);
  if ($err = r\badUnpaidInv($nvid, $inv)) return txRet(FALSE, $err);

  extract(just('payer payee purpose recursId reversesXid cat', $inv, NULL));
  extract(just('amount goods data flags', $inv, [0.0, FOR_GOODS, '', 0]));

  u\setDft($payAmount, $amount);
  $payerA = r\acct($payer);
  $payeeA = r\acct($payee);
  $data = unserialize(nn($data));
  extract(just('api request shid for', $data, NULL));

  $DBTX = \db_transaction();
  $extra = ray('inv', $nvid) + just('api for', $data, NULL);
  if ($cat) $extra += ray('cat', r\hasCats($payer) ? [$cat, NULL] : [NULL, $cat]);
  if ($reversesXid) $extra += compact('reversesXid');
  if ($payAmount < $amount) $extra['partial'] = 1;
  if ($payAmount > invRemains($nvid)) $extra['overpaid'] = 1;
  if ($flags) foreach (ray('gift crumbs investment') as $k) if (u\getBit($flags, $k)) $extra[$k] = 1;

  $z = be\transfer('invoice payment', $payerA, $payeeA, $payAmount, $purpose, $goods, $extra, $recursId);
  if ($z->ok) {
    if ($payAmount >= $amount or !invRemains($nvid)) { // == is for efficiency
      db\update('tx_requests', ray('nvid status', $nvid, $z->subs['xid']), 'nvid');
      if (!empty($api)) u\post($api, ray('request ok msg', $request, (int) $z->ok, $z->msg));
    }

    if (u\getBit($flags, B_INVESTMENT)) { // club reclaiming an investment
      if ($shid and $res = db\get('vestid,price', 'r_shares JOIN r_investments USING(vestid)', compact('shid'))) { // record sale
        extract($res);
        db\update('r_shares', ray('shid shares pending when', $shid, -round($payAmount / $price), 0, now()), 'shid');
        if (db\sum('shares', 'r_shares', compact('vestid')) <= 0) db\q('UPDATE r_shares SET sold=:NOW WHERE vestid=:vestid', compact('vestid'));
      } else r\tellCAdmin(t('Invoice payment marked as "for investment (repayment)" has no corresponding SELL request in shares table.'), $inv, $payeeA);
    }
  } else {
    if ($shortfall = ($auto ? nni($z->subs, 'shortfall') : $payAmount)) {
      $index = ($auto ? 'short invoice|' : '') . ($payerA->hasBank ? 'expect a transfer' : 'when funded|how to fund');
      $subs = ray('short payeeName nvid', $auto ? u\fmtAmt($shortfall) : NULL, $payeeA->fullName, $nvid);
      $payerA->tellOut($index, $subs);


      $z->index = $index; // replace short payment message with something more explicit about the invoice
      $z->msg = tr($index, $subs);
      if (!$auto) $z->ok = TRUE;
      u\preray($subs, $z->subs);
    }
    db\update('tx_requests', ray('nvid status', $nvid, TX_APPROVED), 'nvid'); // in case it wasn't marked approved already
    if ($payerA->hasBank) {
      $completed = $payerA->getFunds($fromBank, t('for invoice', compact('nvid')));
    }
  }

  unset($DBTX);
  
  return $z;
}

/**
 * Create a transaction
 * call by: $z = be\transfer(...);
 * @param string $type: payment, charge, invoice payment, draw
 * @param acct $actorA: acct object for the initiating party (usually the current user)
 * @param acct $otherA: acct object for the other party
 *   OR 'ALL' (paying employees) -- currently UNUSED
 * @param number $amount: the transaction amount (always positive -- reversals are only in reverse())
 * @param string|array $purpose: description of the transaction's purpose (user input) (or [actorFor, otherFor])
 * @param int $goods: FOR_GOODS, FOR_USD, others?
 * @param array $data: associative array of extra info about the transaction (if any), indexed by any of:
 *   'created': time of transaction (defaults to current time)
 *   'force': <do the transaction despite any shortfall (1 for original tx, -1 for forced reversal)>
 *   'boxId': the id of the device on which the transaction was initiated
 *   'offline'
 *   'loan'
 *   'investment': community investment (UNUSED?) or investment club investment
 *   'method': the TX_FLAGS index of the payment type, if non-standard (check, cc, directach, wire)
 *   'stake': member buying or selling stake in investment club
 *   'fine'
 *   'noask'
 *   'funding'
 *   'button': paid from a CGPay button on a company website
 *   'crumbs'
 *   'gift': payment is a gift
 *   'rule': the id of the rule associated with this transaction
 *   'for': SELF_CREDIT, SELF_CREDIT50, gift, donate, other (or empty)
 *   'credit': amount of credit, if for=SELF_CREDIT or SELF_CREDIT50
 *   'cat': transaction categories (record IDs in tx_cats) -- [payerCat, payeeCat] or NULL (payerCat and payeeCat can also be NULL)
 * @param int $recursId: record ID of related record in tx_timed (or NULL)
 * @return a txRet object
 *
 * Do something different, based on these boolean values: 
 *   $taking (charge vs. payment), 
 *   $shortfall (how inadequate the payer's balance is)
 */
function transfer($type, $actorA, $otherA, $amount, $purpose, $goods, $data = [], $recursId = NULL) {
  u\EXPECT('string acct acct number string|array int assoc', $type, $actorA, $otherA, $amount, $purpose, $goods, $data);
  u\EXPECT($actorA and $otherA, 'missing a transfer account');
  global $scanned, $channel;

  if (is_null($goods)) $goods = FOR_GOODS;
  $amount = $amount + 0; // convert from string (otherwise some functions fail in PHP 8.2)
  $taking = ($type == 'charge' or $type == 'draw' or ($type == 'transfer' and $amount < 0));
  
  extract(just('created force boxId rule cat reversesXid method', $data, [now(), 0, NULL, NULL, NULL, NULL, NULL]));
  extract(just(TX_FLAGS . ' inv api for credit', $data, FALSE));
  
  u\setDft($purpose, ray(R_WHYS)[$goods]);

  if ($force != 0) $offline = TRUE; // temporary until app handles this right

  $erInfo = compact(ray('goods amount purpose force')) + ray('actor other op snap', $actorA->fullName, $otherA->fullName, $taking ? t('charged') : t('paid'), $actorA->snap or $otherA->snap);
  if ($amount <= 0 and !test()) r\tellAdmin(t('non-positive amount in be\\transfer'), $erInfo); // should never happen except in tests

  $amount = round($amount, 2); // ignore fractions of cents
  if ($amount == 0) {
    r\tellAdmin('zero transaction', $erInfo + ['stack' => trace()]);
    return txErr(t('zero transaction'), $erInfo);
  }
  $reallyTaking = ($taking xor $amount < 0); // not taking if charging a negative amount

  list ($loserA, $gainerA) = u\order($reallyTaking, $otherA, $actorA);
  list ($e1A, $e2A) = u\order($taking, $otherA, $actorA);
  if ($e2A->id == CGID and $gift and $recursId) $e2A = r\acct(UID_REGULARS); // bring just the total of the month's regular donations into CG's account
  
  $ptrs = ray('actor other loser gainer e1 e2'); // set up sets of variable names
  foreach ($ptrs as $k) {
    ${$k . 'Id'} = ${$k . 'A'}->id;
    ${$k . 'Name'} = ${$k . 'A'}->fullName;
    ${$k . 'For'} = is_array($purpose) ? $purpose[${$k . 'Id'} == $actorId ? 0 : 1] : $purpose;
  }
    
  $erInfo += compact('loserId');

  if (r\dupTx($actorA, $otherA, $taking ? $amount : -$amount, $force)) return txErr(t('duplicate transaction'), $erInfo);

  if ($zErr = r\txPermErr($actorA, $otherA, $taking, $amount < 0)) { // Check permissions
    if ($force) {
      $msg = 'forced without perm';
      $erInfo += ['date' => fmtDt($created)];
      foreach (['agent1'=>$actorA, 'agent2'=>$otherA] as $k => $a) $erInfo += [$k => $a->proSe ? t('self') : $a->fullName];
      foreach ([$actorA, $otherA] as $a) if (!$a->proSe) $a->tell($msg, $erInfo);
      r\tellAdmin($msg . ' ' . tr($zErr->msg, $zErr->subs), $erInfo + $zErr->subs);
    } else return txErr($zErr->msg, $erInfo + $zErr->subs);
  }

  if ($error = u\badAmount($amount)) return txErr($error, $erInfo);
  $flags = $method ? u\bit($method) : 0;
  foreach (ray(TX_FLAGS) as $k) if ($$k) u\setBit($flags, $k);

  $DBTX = \db_transaction();

  // Create the transaction
  $tx0 = ray('actorId actorAgentId',$actorA->id, $actorA->agentId) + compact(ray('flags channel boxId goods created recursId reversesXid'));
  
  if (!$tx = new r\Tx($tx0)) {
    $DBTX->rollback();
    return txErr(t('tx create err'), $erInfo);
  }

  txEntryPair($entries, $tx, $amount, [$e1A, $e2A], [$e1For, $e2For], E_PRIME, compact(ray('rule inv cat')));
  $needed = abs($amount);

  if ($goods != FOR_USD) {
    // Apply rules
    if ($amount > 0) {
      $needed += handleSurTx($entries, $tx, $loserA, $gainerA, $amount, nn($for) and $for != 'other');
    }

    // Apply any roundup
    if ($e1A->roundup and $needed > 0 and !($e2Id == CGID and $recursId and $needed < 1)) { // payer roundups, not actor or loser (don't round up reversals, cashouts, roundups, or recurring gifts under $1)
      if ($cents = fmod($needed, 1.0)) {
        txEntryPair($entries, $tx, 1 - $cents, [$loserId, UID_ROUNDUPS], t('roundup donation'), E_AUX);
        $needed = ceil($needed); // == 1-$cents
      }
    }
  }

  // Calculate any balance shortfall
  if ($shortfall = ($loserA->id < 0 ? 0 : $loserA->shortfall($goods, $needed, TRUE, $actorA))) {
    if ($force) {
      $short = TRUE; // flag overdraft
    } else {
      $DBTX->rollback();
      $short = u\fmtAmt($shortfall);
      if ($loserA->refill) $shortfall += $loserA->avail($goods); // don't leave refillable accounts using credit line in acct::getFunds()
      $avail = u\fmtAmt($needed - $shortfall);
      $msg = $reallyTaking ? ($otherA->secret ? 'short from vague' : 'short from') : 'short to';
      $loserA->suggestAuto();
      $details = t('|%loser was short by %short trying to pay %gainer %amount (available balance is %avail)', 'loser short gainer amount avail', $loserName, $short, $gainerName, u\fmtAmt($needed), $avail);
      return txErr($msg, compact(ray('otherName short shortfall details avail')) + ['success'=>false]);
    }
  }
  
  if (!$force) {
    if ($gainerA->co and $gainerA->depends and db\sum('amt', 'txs_noreverse', 'uid2=:gainerId', compact('gainerId')) + $needed > TRIALCO_AMT_LIMIT) {
      $DBTX->rollback();
      return txErr(t('trial co over'), $erInfo);
    }
    if ($gainerA->needSsnFor($needed)) {
      $DBTX->rollback();
      r\tellAdmin(t('SSN needed'), $erInfo);
      $gainerA->setTask('sell');
      $stepsLeft = $gainerA->stepsLeft();
      $stepsLeft[] = 'ssn';
      $gainerA->update('stepsLeft', array_values($stepsLeft));
      return txErr(t('ssn needed', $erInfo));
    }
  }
  
  if ($res = advanceCredit($for, $loserA, $gainerA, $credit ?: $amount)) { // purchase/gift of store credit
    u\EXPECT(!$rule, 'credit purchase already has a rule!');
    extract($res); // crid and ruid
    for ($i = 0; $i < 2; $i++) $entries[$i]->rule = $ruid; // point the main entry pair at the rule (in case of reversal)
  } else $crid = NULL;
    
  $tx->addEntries($entries);
  if (array_key_exists('xid', $data)) {
    $tx->xid = $data['xid'];
    $xid = $tx->insert(TRUE);
  } else $xid = $tx->insert();
  
  if (!$xid) {
    $DBTX->rollback();
    return txErr(t('tx save error'), $erInfo);
  }
  
  if ($crid) db\update('tx_credits', ray('id xid', $crid, $xid));
  if ($goods != FOR_USD and $pot = $loserA->giftPot) $loserA->update('giftPot', max(0, $pot - $needed));
  
  // Notify both parties by email about any completed transaction
  $res = sendNotifications($actorA, $type, $tx, $shortfall);
  if ($type == 'payment' and $gift) { // gift is a TX_FLAGS flag extracted from $data
    list ($period, $periods) = [PER_1, 0]; // purpose defaults to whatever is passed to the function
    if ($per = db\get('purpose, period, periods', 'tx_timed', 'id=:recursId', compact('recursId'))) extract($per);
    be\thanks(r\ccGiftType($gainerId), TRUE, $loserId, $purpose, $amount, $period, $periods, $gainerId, FALSE);
  }
  if ($stake and $gainerA->iclub and $stakeInfo = db\row('r_stakes', ray('uid clubid', $loserId, $gainerId))) { // stake here is a flag bit
    extract(just('stakeid stake request', $stakeInfo, 0)); // stake is now an amount
    $stake += $amount;
    $request = max(0, $request - $amount); // just invested more, so request should be less, but not negative
    db\update('r_stakes', compact(ray('stakeid stake request')), 'stakeid');
    $z->index .= '|investment increase';
    $z->msg = tr($z->index, $z->subs);
  }
  
  foreach ([$actorA, $otherA] as $a) {
    if (!$a->confirmed) $a->setBit(B_CONFIRMED); 
//    if ($a->snap) r\tellAdmin(t('SNAP member tx: %who', 'who', $a->fullName), $erInfo);
    if (!$a->co and !($channel == TX_APP ? $a->agentA->ided : $a->ided)) {
      if ($force) {
        r\tellAdmin('forced without photoId', $erInfo); 
      } elseif ($gainerId != CGID) { // and !$gainer->coCan(CO_FAST)
//      r\tellAdmin('tx without photoId', $erInfo); 
//      return txErr($a == $actorA ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $otherA->fullName]);
      }
    }
  }
  
  unset($DBTX);

  return $res;
}

/**
 * Add a pair of related entries to the transaction: one losing, one gaining.
 * Any ...0 parameter but $amount0 can be a 2-element array (one element for each party)
 * @param $txes array: flat array of entry objects to which to add 2 entries
 * @param Tx $tx: the transaction header data
 * @param numeric $amount0: the amount paid/received (amount to add to the first party's balance)
 * @param mixed $a0: account object(s) and/or uid(s)
 * @param string $description0: transaction purpose(s)
 * @param int $entryType0: entry type(s)
 * @param assoc $other: optional parameters
 * - array $cat: budget category for each entry in the pair (record ID in tx_cats), if any
 * - int $method: payment method (index into TX_FLAGS)
 * - int $rule: relevant record ID in tx_rules table
 * - int $inv: related invoice ID
 */
function txEntryPair(&$txes, $tx, $amount0, $a0, $description0, $entryType0, $other = []) {
  extract(just('cat rule inv', $other, NULL));
  $cat0 = $cat ?: [NULL, NULL];

  foreach ([1, 0] as $i) { // payee first, to agree with tests and so entry id is postive for e2, negative for e1
    foreach (ray('a description entryType cat') as $k) $$k = is_array(${$k . 0}) ? ${$k . 0}[$i] : ${$k . 0};
    $amount = $i == 0 ? -$amount0 : $amount0;
    list ($uid, $agentUid) = is_object($a) ? [$a->id, $a->agentId] : [$a, $a];
    list ($relType, $relatedId) = $inv ? ['I', $inv] : [NULL, NULL];
    $txes[] = new r\TxEntry(compact(ray('entryType amount uid agentUid description cat rule relType relatedId')));
  }
  $i9 = count($txes) - 1;
  foreach ([1, 0] as $i) if (!$cat0[$i]) be\setCat($txes, $tx, $i9 - $i); // -$i NOT (+ $i - 1) because entries are switched with cat2 first ([0])
}

/**
 * Set the transaction account category if the account is owned by Common Good.
 * @param ray(assoc) $txes: (MODIFIED) the transaction entries
 * @param Tx $tx: the transaction header data (only flags and recursId are used)
 * @param int $i: index into $txes of the entry to examine (expects the account to be owned by CG)
 * $i is assumed to be the payee, if even.
 */
function setCat(&$txes, $tx, $i) { 
  if (!r\hasCats($txes[$i]->uid)) return $txes[$i]->cat = NULL;

  extract(just('uid amount entryType description rule relType relatedId', $txes[$i]), EXTR_PREFIX_ALL, 'i');
  list ($for, $type) = [$i_description, $i_entryType]; // shorthand
  $iA = r\acct($i_uid);
  $j = be\i2j($i); // point to the other entry in the pair
  $j_uid = $txes[$j]->uid;
  $in = ($i < $j); $out = !$in;

  $gift = in($type, [E_PRIME, E_OUTER]) ? u\getBit($tx->flags, 'gift') : ($for == t('donation')); // "donation" includes stepups
  $inv = ($i_relType == 'I');
  $recurs = $tx->recursId;
  $refund = (($in xor $i_amount < 0) and !in($type, [E_BANK, E_OUTER]));
  
  $nick = NULL; // in the if/else chain below, either return with $txes[$i]->cat set OR try to set $nick

  if ($type == E_AUX and strhas($for, FS_NOTE)) {
    $nick = $in ? 'FS-FEE' : 'D-FBO';
  } elseif (r\hasCats($j_uid) and in($type, E_PRIMES)) { // transfer from one CG-owned CG account to another
    $nick = 'CG2CG';
  } elseif ($in and $type == E_XFEE) {
    $nick = 'TX-FEE-BACK';
  } elseif ($type == E_BANK and $iA->mainQid == 'NEWAAB') { // "to bank" is just a negative transfer IN
    $nick = $tx->created < strtotime('7/1/2021') ? 'OLD-BANK' : 'OPERATIONS';
  } elseif ($in and $iA->mainQid == 'NEWAIN') { // 457 retirement (!!!Must come before $iA->sponsored test)
    $nick = 'NEWAAA457'; // liability to William -- will require additional code and accounts for other 457 participants
  } elseif ($in and $iA->sponsored) {
      $nick = $type == E_AUX ? 'D-FBO-STEPUP' : 'D-FBO';
  } elseif ($in and $iA->ourOwn) {
      $nick = strhas($iA->fullName, t('Loan Fund')) ? $iA->mainQid : 'D-FBO';
  } elseif ($in and $gift) {
    if ($j_uid == UID_STEPUPS) {
      $nick = 'D-STEPUP';
    } elseif ($j_uid == UID_CRUMBS) {
      $nick = 'D-CRUMB';
    } elseif ($j_uid == UID_REGULARS) {
      $nick = 'D-REGULAR';
    } elseif ($j_uid == UID_ROUNDUPS) {
      $nick = 'D-ROUNDUP';
    } elseif ($type != E_OUTER and r\acct($j_uid)->co) {
      $nick = 'D-COMPANY';
    } else $nick = 'D-ONCE';

  } elseif ($out and $iA->sponsored and $type == E_XFEE) {
    $nick = 'FBO-TX-FEE'; // often balanced by TX-FEE-BACK
  } elseif ($out and $type == E_REBATE and $iA->mainQid == 'NEWAZV') {
    $nick = 'TO-PERSON';
  } elseif ($out and (strhas($for, t('sponsor')) or strhas($for, t('grant')) or $gift)) {
    $nick = 'TO-ORG';
  } elseif ($out and !$i_relType and u\starts($for, t('labor'))) {
    $nick = $iA->sponsored ? 'FBO-LABOR' : 'LABOR';
  } elseif ($out and $inv and $type != E_OUTER) { // pretty safe to assume a recent repeat invoice from the same account is for the same purpose as all others
    $cats = db\col('DISTINCT cat1', 'txs', "relType='I' AND type<>:E_OUTER AND uid2=:j_uid AND cat1 IS NOT NULL AND created>:NOW-2*:MONTH_SECS", compact('j_uid'));
    if (count($cats) == 1) return $txes[$i]->cat = $cats[0];
  }

  $txes[$i]->cat = $nick ? (r\nick2cat($nick) ?: NULL) : NULL; // inner ?: is for dev machine where sponsored account has no QBO account
}

function i2j($i) {return $i + (($i & 1) ? -1 : 1);} // pointer to paired entry index in an array of tx entries

/**
 * Insert entries for an existing transaction.
 * @param array of entry objects $txes: the entries
 * @param int $xid: the transaction header record ID
 */
function insertTxes(&$txes, $xid) {
  $DBTX = \db_transaction();
  foreach ($txes as $i => $entry) {
    $txes[$i]->id = $eid = $entry->insert($xid, ($i % 2) ? -$eid : 0); // eid of second entry in each pair is the inverse of its companion
    u\EXPECT($eid, "Failed to insert transaction entry");
  }
  unset($DBTX);
}

/**
 * Create a personal credit for buyer or someone else, at the seller. Add to any existing identical credit
 * @param string $for: SELF_CREDIT, SELF_CREDIT50, or qid of gift recipient
 * @param acct $buyer: payer acct
 * @param acct $seller: seller acct (where the recipient gets store credit)
 * @param float $credit: amount of credit the recipient gets
 * @return assoc of the credit record ID and the the rule record ID: [crid, ruid] (FALSE if not a purchase of credit)
 */
function advanceCredit($for, $buyer, $seller, $credit) {
  if (!nn($for) or $for == 'other' or !$credit) return FALSE; // not a purchase of credit
  u\EXPECT($credit >= 0, 'negative credit in transfer');
  
  if (!in($for, [SELF_CREDIT, SELF_CREDIT50])) { // gift!
    $for = r\acct($for);
    u\EXPECT($for, 'gift account does not exist');
    $for->tell('you got credit|to redeem', ray('name action credit co', $buyer->fullName, t('sent you a gift'), u\fmtAmt($credit), $seller->fullName), t('You have received a gift through Common Good!'));
    $buyer = $for;
  }
  
  $portion = $for == SELF_CREDIT50 ? .5 : 1;
  $info = ray('action portion payerType payer payeeType payee from to purpose', ACT_SURTX, $portion, REF_ACCOUNT, $buyer->id, REF_ACCOUNT, $seller->id, MATCH_PAYEE, MATCH_PAYER, t('any purchase'));
  $rule = db\get('id,end,amtMax', 'tx_rules', $info);

  $DBTX = db_transaction();
  $crid = r\credit($seller->id, $buyer->id, -$credit);
  $info['amtMax'] = $credit;

  if ($rule and !$rule['end']) {
    $info['id'] = $ruid = $rule['id'];
    $info['amtMax'] += $rule['amtMax'];
    db\update('tx_rules', $info);
  } else $ruid = db\insert('tx_rules', $info);
  unset($DBTX);
  
  return compact(ray('crid ruid'));
}

/**
 * Send appropriate notifications and return a success message array
 * @param bool $doTell: actually send notifications (otherwise just return the txRet object)
 * @return a txRet object
 */
function sendNotifications($mya, $type, $tx, $shortfall, $doTell = TRUE) {
  global $testLink, $channel;
  u\EXPECT(compact(ray('type tx shortfall')), 'string tx int|float');
  if ($tx->xid < 0) return txRet(TRUE, '', []); // no notices for setup transactions during testing

  $goods = ($tx->goods == FOR_GOODS);
  $positive = ($tx->amt2 > 0); // transaction amount is positive (normal)
  $toMe = ($mya and $mya->id == $tx->uid2);

  if ($type == 'charge') {
    $notices = $goods ? ($positive ? ['charged you', 'you charged'] : ['refunded you', 'you refunded'])
    : u\order($positive, 'you got cash', 'you gave cash');
    $did = $positive ? t('charged') : ($goods ? t('refunded') : t('credited'));
    $appMsg = $positive ? t('charged') : ($goods ? t('refunded') : t('credited'));
  } elseif ($type == 'draw') {
    $notices = ['drew from you', 'you drew'];
    $did = ''; // no real-time report from here
    $appMsg = $toMe ? '' : t('drew from');
  } elseif (in($type, 'payment, invoice payment')) {
    $notices = ['you paid', 'paid you'];
    $did = t('paid');
    $appMsg = 'paid';
  } elseif ($type == 'undo') {
    if ($tx->otherA->isBank()) {
      $notices = ['', 'bank tx canceled'];
      $did = t('reversed');
      $appMsg = '';
    } else {
      $notices = $positive ? ($tx->taking ? ['re-charged you', 'you re-charged'] : ['you refunded', 'refunded you']) : ['refunded you', 'you refunded'];
      $did = $positive ? ($tx->taking ? 're-charged' : 'refunded') : ($goods ? 'refunded' : 'credited'); // positive is double reverse
      $appMsg = $toMe ? ($goods ? t('refunded') : t('credited')) : t('re-charged');
    }
  } else u\FAIL("unknown type: '$type'.");

  if ($mya and in($appMsg, 'charged paid')) { // only for payments for now (and requests -- see be\invoice())
    $otherA = $toMe ? $tx->a1 : $tx->a2;
    u\tellApp($appMsg, $mya, $otherA, abs($tx->amt2), $tx->for2, $tx->created);
  }
  list ($tell1, $tell2) = u\order($positive, 'tellOut', 'tellIn');

  foreach ([1,2] as $i) { // notify both parties (from=1 then to=2)
    $notice = $notices[$i - 1];
    list ($a, $otherA, $inout) = [$tx->{"a$i"}, $tx->{'a' . (3 - $i)}, $i == 1 ? 'out' : 'in'];
    $args = $tx->reportArgs($a, TRUE, $shortfall);

    if ($notice == 'paid you' and $a->co) {
      $notice = 'paid you linked';
      $args['_aPayLink'] = $testLink = $a->doLink('addr', $otherA->id); // link to address, so merchant can ship or nonprofit can thank
    }
    if ($notice and !$a->isCanonic and $doTell) $a->tell($notice, $args, '', $inout);
    $isMe = (($mya ? $mya->id : $tx->actorId) == $a->uid);
    if ($isMe) $myArgs = $args; // don't use tx->toMe here, in case using eLinkAcct, not $mya
  }

  $msg = 'report tx';
  
  if ($tx->flags & (u\bit(B_PARTIAL) | u\bit(B_OVERPAID)) and $nvid = $tx->entries[0]->relatedId) { // invoice payment or retraction
    if ($left = be\invRemains($nvid)) {
      $msg .= '|left on invoice';
      $myArgs += ray('remaining', u\fmtAmt($left));
    }
  }
  return txRet(TRUE, $msg, $myArgs + compact(ray('msg did')));
}

/**
 * Generate tentative transaction entries for an auxiliary transaction (surcharge, coupon, etc.)
 * @param assoc $entries: (MODIFIED) an array of entries to add to, if appropriate
 * @param Tx $tx: the transaction header data
 * @param acct $buyer:
 * @param acct $seller:
 * @param number $price: pre-discount, pre-stepup amount (must be positive)
 * @param bool $forCredit: <buying store credit as a gift or for oneself>
 * @param bool $test: don't actually change anything? (default FALSE)
 * @return the net increase in overall transaction amount
 */

function handleSurTx(&$entries, $tx, $buyer, $seller, $price, $forCredit, $test = FALSE) {
  u\EXPECT($price > 0, 'non-positive price handling surTx');
  
  $payerCo = $buyer->co ? 1 : 0;
  $payeeCo = $seller->co ? 1 : 0;
  
  // Get a list of applicable rules
  $payerCheck = u\genRuleSQL('payer', ':buyer'); // rule applies to this buyer
  $payeeCheck = u\genRuleSQL('payee', ':seller'); // rule applies to this seller
  $forCreditCheck = $forCredit ? '`from`<>:MATCH_PAYEE' : '1'; // no buying credit with credit or discount
  
  $sql = <<<EOF
    SELECT ru.id AS rule, ru.amount, ru.portion, ru.from, ru.to, ru.purpose, ru.minimum, ru.useMax, 
      ru.amtMax, ru.payerType,
    (SELECT IFNULL(SUM(e.amount), 0) FROM tx_entries e WHERE e.rule IS NOT NULL AND e.rule=rule AND e.uid=:buyer AND e.entryType=:E_REBATE) AS `amtUsed`,
    (SELECT IFNULL(SUM(SIGN(e.amount)), 0) FROM tx_entries e WHERE e.rule IS NOT NULL AND e.rule=rule AND e.uid=:buyer AND e.entryType=:E_REBATE) AS `timesUsed`
    FROM tx_rules ru
    WHERE $payerCheck AND $payeeCheck
    AND $forCreditCheck
    AND (ru.action = :ACT_SURTX)
    AND (ru.start <= :now AND IFNULL(:now < ru.end, TRUE))
    AND (:price >= minimum)
    GROUP BY rule
    HAVING :price >= IFNULL(minimum, 0)
    AND timesUsed < IFNULL(useMax, timesUsed+1)
    AND amtUsed < IFNULL(amtMax, amtUsed+1)
    ORDER BY ru.start
EOF;

  $subs = ray('buyer seller now price', $buyer->id, $seller->id, now(), $price);
///  showq($sql, $subs); // debug()
  $q = db\q($sql, $subs);

  $stepups = $rebates = 0;
  while ($row = $q->fetchAssoc()) {
    extract($row);

    $absValue = round($amount + $portion * $price, 2); // potential rebate/stepup amount
    if ($amtMax) $absValue = round(min($absValue, $amtMax - $amtUsed), 2); // limited by amtUsed
    if (!$absValue) continue;

    $from = ($from == MATCH_PAYER) ? $buyer->id : (($from == MATCH_PAYEE) ? $seller->id : $from);
    $to = ($to == MATCH_PAYER) ? $buyer->id : (($to == MATCH_PAYEE) ? $seller->id : $to);

    if ($to == $buyer->id) { // rebate
      list ($type, $desc) = [E_REBATE, REBATE_DESC];
      $absValue = min($price - $rebates, $absValue); // don't refund more than is being spent (food fund especially)
      if ($absValue <= 0) continue;
      $rebates += $absValue;
    } else { // everything else is an auxiliary transaction
      list ($type, $desc) = [E_AUX, $purpose];
      if ($from == $buyer->id) $stepups += $absValue; // stepup -- anything else from the buyer is a donation of some sort
    }

    if ($test) continue;
    
    if ($to == CGID and $purpose == STEPUP_PURPOSE) $to = UID_STEPUPS; // bring just the total of the month's stepup donations into CG's account
    txEntryPair($entries, $tx, $absValue, [$from, $to], $desc, $type, compact('rule'));
    if (($amtMax and $amtUsed + $absValue >= $amtMax) or ($useMax and $absValue > 1 and $timesUsed + 1 >= $useMax)) {
      if ($payerType == REF_ACCOUNT) db\update('tx_rules', ray('id end', $rule, now()), 'id'); // rule is just for payer
    }
  }

  return round($stepups - $rebates, 2);
}

/**
 * Thank a donor to CG or a fiscally sponsored account and report to that organization.
 * @param string $type: gift (donation not to CG), cggift, fbo (donation to a sponsored organization), or purchase
 * @param bool $member: <payer is a member>
 * @param int $id: account record ID or people record ID for the payer
 * @param string $item: item description
 * @param numeric $amount: donation or payment amount
 * @param string $period: how often to recur
 * @param int $periods: how many periods between recurrences
 * @param int $coId: sponsored organization's account record ID
 * @param bool $redirect: <redirect to empty page when done>
 */
function thanks($type, $member, $id, $item, $amount, $period, $periods, $coId, $redirect) {
  $coA = r\acct($coId);
  list ($report, $thxMsg) = $type == 'purchase' ? ['purchase-report', 'purchase thanks'] : ['gift-report', 'gift thanks']; // just purchase or gift (type is purchase/gift/cggift)
  if ($coId == CGID) $thxMsg .= '|cggift thanks';

  if ($coA->co) { // no special emails for individual gifts
    if ($member) {
      $a = r\acct($id);
      foreach (ray('fullName email phone postalAddr notes') as $k) $subs0[$k] = $$k = $a->$k;
    } else { // non-member
      if (!$subs0 = db\get('fullName,email,phone,address,city,state,zip,notes', 'people', ray('pid', $id))) u\FAIL('missing pid in thanks');
      extract($subs0);
      $st = r\realState($state);
      $postalAddr = u\unite($address, "$city, $st $zip", ', ');
    }

//    $note = (!$member and u\starts($notes, fmtDt(now(), 'dmY'))) ? str_replace(': ', '', strstr(strstr($notes, ': ') . "\n", "\n", TRUE)) : '';
    
    $amtDpy = u\fmtAmt($amount) . ($period != PER_1 ? (' ' . r\recurDesc($period, $periods)) : '');
    $subs0 += ray('date item amount', fmtDt(now()), $item, $amtDpy);

    $subs = $subs0 + ray('fromName fromPhone fromAddress fromEmail', $fullName, u\fmtPhone($phone), $postalAddr, $email);
    unset($subs['fullName']); // salutation to co name, not payer name
    $coA->tellIn($report, $subs);

    $msg = ''; // NYI tr($coA->giftThanksMsg, $subs0); // thanks for donation to an organization not sponsored by CG
    $subs = $subs0 + ray('noFrame coName coPostalAddr coPhone msg', !$member, $coA->bestName, $coA->postalAddr, u\fmtPhone($coA->phone), $msg);
    if ($member) {
      $a->tellOut("$type-thanks-member", $subs);
    } else r\rMail("$type-thanks-nonmember", [$email => $fullName], $subs);
  }
  
  $msg = tr($thxMsg . ($member ? '' : '|check it out'), ray('coName', $coA->bestName));
  return $redirect ? w\sayEmpty($msg) : w\say($msg);
}
 
/**
 * Create an invoice
 * call by: $z = be\invoice(...);
 * @param acct $payeeA: acct object for payee (usually current user)
 * @param acct $payerA: acct object for payer
 * @param numeric $amount: the invoiced amount
 * @param string $purpose: description of the transaction's purpose (user input)
 * @param assoc $extra:
 *    shid: record id of related record in r_shares
 *    status: used by acct->payApproved to mark transaction approved
 *    created: UNIXtime
 *    request: unique request identifier for actor using API
 *    api: URL to which to report success or failure (for API requests)
 * @param int $recursId: record ID of related record in tx_timed (if any)
 * @return a txRet object
 * NOTE: goods is no longer allowed in $extra, it's its own parameter
 */
function invoice($payeeA, $payerA, $amount, $purpose, $goods = FOR_GOODS, $extra = [], $recursId = NULL) {
  global $channel;

  u\EXPECT(compact(ray('payeeA payerA amount purpose')), 'acct acct float string');
  u\EXPECT(! array_key_exists('goods', $extra), 'invoice() no longer accepts goods in extra');
  extract(just('created status reversesXid cat method', $extra, [now(), NULL, NULL, NULL, NULL]));
  if ($cat) $cat = $cat[0] ?: $cat[1]; // extract the only non-null entry in the pair, if any
  $flags = $method ? u\bit($method) : 0;
  foreach (ray('cat reversesXid method') as $k) unset($extra[$k]);

  foreach (justNOT("status shid coupon coupid force inv api request created for reversesXid cat", $extra) as $k => $v) {
    u\setBit($flags, $k, $v);
    unset($extra[$k]); // don't leave bits in extras (data field)
  }
  foreach (just('api request for', $extra) as $k => $v) if (empty($v)) unset($extra[$k]);
  list ($payee, $payer) = array($payeeA->id, $payerA->id);
  u\setDft($purpose, $why = ray(R_WHYS)[$goods]);
  $subs2 = ray('myName otherName otherEmail otherPhone amount why purpose did op created createdDpy', $payeeA->fullName, $payerA->fullName, $payerA->email, u\fmtPhone($payerA->phone), u\fmtAmt($amount), $why, $purpose, t('charged'), t('charged'), $created, fmtDt($created));
  $subs2 += r\balAndCred($payeeA);
  if ($err = u\badAmount($amount, '>0')) return txErr($err, $subs2);
  if ($zErr = r\txPermErr($payeeA, $payerA, TRUE)) return $zErr;
  if (r\dupTx($payeeA, $payerA, $amount)) return txErr(t('duplicate transaction'), $subs2);
  
  u\setDft($status, ($channel != TX_FOREIGN and r\relation(':IS_AUTOPAY', $payee, $payer)) ? TX_APPROVED : TX_PENDING);
  $data = serialize($extra);
  $info = compact(ray('amount payer payee status purpose created flags data recursId reversesXid cat'));
  $nvid = db\insert('tx_requests', $info, 'nvid');

  if ($status == TX_PENDING) {
    $payeeA->tell('you invoiced', $subs2);
    $link = $payerA->doLink('inv', $nvid); // link to pay/deny
    $subs1 = ray('myName otherName otherEmail otherPhone _a1 reply', 
      $payerA->fullName, $payeeA->fullName, $payeeA->email, u\fmtPhone($payeeA->phone), $link, $payee);
    $subs1 += r\balAndCred($payerA) + $subs2;
    $payerA->tell('invoiced you', $subs1); // tell other party by email about this invoice
    u\tellApp('request', $payeeA, $payerA, $amount, $purpose, $created, $nvid);
  } else { // pre-approved!
    if (!$payerA->shortfall(TX_TRANSFER, $amount)) { // don't try to pay yet if short (for example, this prevents payApproved from duplicating messages)
      $z = be\payInvoice($nvid);
      if ($z->ok) $status = TX_PAID; else r\tellAdmin(t('invoice payment failed'), just('msg', $z) + $z->subs + $info);
    } else { $payeeA->tell('you will be paid', $subs2); }
  }

  $msg = 'report tx';
//  if ($payeeA->cttyRewardy) $msg .= '|for why';
  if ($amount > 0) $msg .= '|balance unchanged';
  return txRet(TRUE, $msg, $subs2 + ray('success nvid status', TRUE, $nvid, $status));
}

/**
 * Package the return arguments and alert staff on certain errors.
 * @param mixed $message: the error message or an object of parameters
 * @param assoc $info: (optional) message substitutions and other info about transaction
 * @return [ok, msg, info]
 */
function txErr($message, $info = []) {
  if (nni($info, 'snap')) $info['topic'] = nni($info, 'topic') . t(' SNAP');
//  r\tellAdmin(t('failed transaction'), $info + compact('message')); // usually at night, for lack of bank account
  return is_object($message) ? $message : txRet(FALSE, $message, $info);
}

/*
 * Return an object of the given arguments, as the return from a transaction function:
 *    ok: success?
 *    index: message index or message
 *    msg: success or error message (returned with replacements already made)
 *    subs: replacement parameters (returned in case they are needed twice)
 */
function txRet($ok, $index, $subs = []) {
  $msg = tr($index, $subs);
  return (object) compact(ray('ok index msg subs'));
}

/* /\** */
/*  * Send an email invoice to a nonmember, inviting them to join. */
/*  * @param string $name: name of person to invoice */
/*  * @param assoc $info: assoc [email, amount, purpose] -- these params may also be specified individually */
/*  *\/ */
/* function invoiceNonmember($name, $info) { */
/*   extract(just('email amount goods purpose', rayy(func_get_args(), 1))); */
  
/* } */
  
/**
 * @return FALSE if success; else array($error_message, $subs)
 */
function addCell($number, $validate = FALSE) {
  global $mya;
  $myid = $mya->id;

  $phone = u\fmtPhone($number, '+n');
  $phoneDesc = u\fmtPhone($number);
  $subs = compact('number'); // original format

  if ($validate) {
    if (!$phoneDesc) return array('bad phone', $subs);
    if ($uid = db\get('uid', 'r_boxes', 'code=:phone', compact('phone'))) {
      if ($uid == $myid) return array('already cell', $subs);
      $accountName = "$mya->fullName ($mya->mainQid)";
    }
    return NULL;
  }
  
  //$status = SMS_PRIMARY;
  $mya->makeBox($phone, TX_SMS);
  if (!$mya->phone) $mya->update('phone', $phone); // use it as contact phone, if none yet
  return array('report new cell', ray('number', $numberDesc));
}

function deleteCell($number) {
  $number = u\fmtPhone($number, '+n'); // internal format
  db\del('r_boxes', 'code=:number LIMIT 1', compact('number'));
  $number = u\fmtPhone($number); // pretty format
  return array('report delete cell', compact('number'));
}

/**
 * Figure out who the user meant. Only OK people in the user's community, nearby, 
 *   or previous tx partners are eligible (except admin sees all).
 *
 * @param string $who: how the user referred to the other account:
 *   name, phone, email, or account ID
 *   name@city, zip, or city, ST
 * @param int $mya: current user's account
 * @param string $selfErr: index to error message for identifying oneself
 * @param string $restrict: MySQL to restrict the selection (for example ':IS_CO' or ':IS_OK')
 * @param bool $returnObj: <return an array of objects> (else return a keyed array of strings)
 * @return one of the following:
 *   the other trader's acct
 *   error message index
 *   array of possible fullNames with city and state, indexed by uid
 *     OR an array of hashes: {uid, nm}
 */
function identify($who, $mya, $selfErr, $restrict, $returnObj = FALSE) {
  if (!$mya) return 'search forbidden';
  $myid = $mya->id;
  $who = $who0 = trim($who);
  $searchable = $mya->admin ? '1' : '(IF(activated, :IS_OK, :IS_MEMBER) AND !:IS_NOSEARCH)';
  $a = NULL; // set this if a specific account ID is specified

  if ($atPos = mb_strpos($who, '@')) { // email or location (point to @)
    $at = trim(mb_substr($who, $atPos + 1)); // what's after the @
    if (strhas($at, '.') and !strhas($who, ' ')) { // email
      $cmail = u\cry('P', mb_strtolower($who));
      list ($where, $subs) = ['email=:cmail', compact('cmail')];
      $err = tr('unfound', 'who where', t('anyone'), t('with that email address'));
    } else {
      $who = trim(mb_substr($who, 0, $atPos));
      list ($where, $subs) = nameCriteria($who);

      if (preg_match('/[0-9]/', $at)) { // postal code (anything with a digit)
        if ($err = u\badZip($at)) return $err;
        $where .= ' AND zip LIKE :zip';
        $subs['zip'] = $at . '%';
        $err = tr('unfound', 'who where', $who, t('in that postal code'));
      } elseif (strhas($at, ',')) { // city, ST
        list ($city, $st) = explode(',', $at);
        $city = '%' . trim($city) . '%';
        if (!$state = r\stateNum(trim($st))) return 'bad state';
        $where .= ' AND city LIKE :city AND state=:state';
        $subs += compact(ray('city state'));
        $err = tr('unfound', 'who where', $who, t('in that city'));
      } else { // city
        $city = '%' . trim($at) . '%';
        $where .= ' AND city LIKE :city';
        $subs += compact('city');
        $err = tr('unfound', 'who where', $who, t('in that city'));
      }
    }
  } elseif (($mya->admin or test()) and is_numeric($who) and mb_strlen($who) > 10) { // test() needed because account ID fields (like customer) get converted to uids
    list ($where, $subs) = ['uid=:who', compact('who')];
    $err = tr('unfound', 'who where', t('anyone'), t('with that record ID'));
  } elseif ($phone = u\fmtPhone($who, '+n')) { // phone
    $cphone = u\cry('P', $phone);
    list ($where, $subs) = ['phone=:cphone', compact('cphone')];
    $err = tr('unfound', 'who where', t('anyone'), t('with that phone number'));
  } else { // name or qid or empty (empty when called from memberRay)
    list ($where, $subs) = nameCriteria($who);
    $err = tr('unfound', 'who where', $who, t('(Try using their email or phone or, if they are not nearby, add "@" followed by their postal code or city (or city, state))'));
    if (preg_match('/^([A-Z]+|[a-z]+|[A-Z][a-z]{5})$/', $who)) { // all upper or all lower or 6 letters with first capitalized (happens on Android mobile)
      $qid = mb_strtoupper(($mya->admin and mb_strlen($who) == 3) ? R_SERVER_ID . $who : $who);
      if (u\isQid($qid) and $a = r\acct($qid)) $where = "($where OR uid=$a->id)";
    }
    if (!nn($qid) and mb_strlen($who) > 3 and !strhas($who, ' ')) {
      include_once R_ROOT . '/cg-qr.inc';
      if ($qid = qr\qid($who, $cardCode) and $a = r\acct($qid)) {
        if ($cardCode and $cardCode != $a->cardCode()) {
          $agt = $a->proSe ? '' : t(' with agent ') . $a->agentId;
          $err = tr('unfound', 'who where', $who, t('(that is the wrong cardCode for account ID %qid)', compact(ray('qid agt'))));
        } else $where = "($where OR uid=$a->id)";
      }
    }
  }
      
  if ($restrict) $where .= ' AND ' . $restrict;
  if (!$mya->admRegion and !$a and !$atPos and !nn($cphone)) { // look only locally unless company
    $txPartner = "uid IN (SELECT DISTINCT IF(uid1=$myid, uid2, uid1) AS uid FROM txs WHERE $myid IN (uid1, uid2))";
    $where .= tr(' AND (:IS_CO OR community=:ctty OR %DISTANCE_BETWEEN<%NEIGHBOR_MAX OR %txPartner)', compact('txPartner')); // restrict to nearby
    list ($lat, $lon) = [$mya->latitude ?: 0, $mya->longitude ?: 0]; // avoid NULL (PHP bug?)
    $subs += compact(ray('lat lon'));
  }
  
  $individual = t('individual');
  $company = t('company');
  $anon = "IF(:IS_CO, '$individual', '$company')";
  $uidCrit = 'uid NOT :CANONIC';
  if ($mya->admActivate) $uidCrit = "($uidCrit OR uid=:UID_SUPER)"; // needed for helper when activating
  if ($returnObj) $uidCrit .= " AND uid NOT IN ($myid, :UID_SUPER)";

  $sql = <<<EOF
    SELECT DISTINCT uid, 
      CONCAT(IF($searchable, fullName, $anon), ' @ ', IFNULL(city, ''), ', ', IFNULL(abbreviation, '')) AS nm
    FROM users u
    LEFT JOIN r_states s ON s.id=u.state
    WHERE $where AND $uidCrit
    ORDER BY community<>:ctty, fullName
EOF;
  
  $subs['ctty'] = test() ? $mya->community : ($mya->seedpacker ? -1 : $mya->community);
  $q = db\q($sql, $subs);
  
  if ($returnObj) return $q->fetchAll();

  $result = $q->fetchAllKeyed(0, 1);
  
  if (count($result) == 1 and key($result) == $myid) return $selfErr;
  unset($result[$myid]); // if self is one of many results, ignore it
  if (!$result) return $err ?: tr('unknown member', compact('who'));
  
  if ($atPos and $uid = array_search($who0, $result)) $result = [$uid => $who0];
  if (count($result) == 1) return r\acct(key($zot = $result)); // return the one entry (zot resets the pointer)
  return $result; // array of choices
}

function nameCriteria($who) {
  $short = u\shortName(" $who ", '%') ?: '-'; // abbreviations must not be null or zeroes
  return ['(u.name LIKE :short OR fullName LIKE :short)', compact('short')];
}

function exception_error_handler($severity, $message, $file, $line) {
  if ($severity == E_NOTICE)
    throw new ErrorException($message, 0, $severity, $file, $line);
  return;
}
  

/**
 * Return financial information about the given account on this server
 * @param mixed uids: a uid or an array of uids for the account to return information for
 * @param integer asof: one second past last time to include (defaults to 24 hours from now, meaning unrestricted)
 * @return an assoc of results (see $resultKeys, below)
 */
function creditInfo($uids, $asof = null) {
///     debug("creditInfo(" . print_r($uids, true) . ", $asof)");
  if (!is_array($uids)) $uids = [$uids];
  global $mya;
  
  u\setDft($asof, now() + DAY_SECS);

  $subs = compact(ray('uids asof'));
  $isPayer = '(uid1 IN (:uids))'; 
  $isPayee = '(uid2 IN (:uids))'; // account can be both payer and payee, if joint account
  
  $sql = <<<EOF
    SELECT SUM(IF($isPayee, txAmt, 0)) AS `in`,
           SUM(IF($isPayer, txAmt, 0)) AS `out`,
           SUM(IF(bankAmt>0, bankAmt, 0)) AS `fromBank`,
           SUM(IF(bankAmt<0, -bankAmt, 0)) AS `toBank`
    FROM (
      SELECT uid1, uid2, amt AS txAmt, IF(type=:E_BANK, amt, 0) AS bankAmt
      FROM txs t
      WHERE ($isPayer XOR $isPayee) AND t.created<:asof
    ) t
EOF;

  $sums = db\q($sql, $subs)->fetchAssoc();
///   die(pr(showq($sql, ray('uids asof', join(',', $uids), $asof))));

  $sql = "SELECT SUM(amount) AS pendingBank FROM txs2 t WHERE (payee IN (:uids)) AND created<:asof AND completed <= 0";
  $sums += db\q($sql, $subs)->fetchAssoc();

  foreach ($sums as $k => $v) if (is_null($v)) $sums[$k] = 0; // no nulls
  extract($sums);
  
  $balance = ($in - $out);
  $in -= ($fromBank - $toBank);
  
  $resultKeys = 'in out balance fromBank toBank pendingBank uids';
  $result = compact(ray($resultKeys));
///    $asof = fmtDt($asof); debug(compact(ray('asof sums in out fromBank toBank balance result')));
  foreach ($result as $k => $v) if ($k != 'uids') $result[$k] = round($v, 2);

  return $result;
}

/**
 * Returns the roundup donations made by the specified users in the specified period.
 * @param array $uids -- the uids of the users, generally the main user or the main and joint users
 * @param unixtime $start -- the start of the period
 * @param unixtime $end -- past end of the period
 * @return the total roundup donations
 */
function roundupDonations($uids, $start, $end) {
  $where = 'uid1 IN (:uids) AND uid2=:UID_ROUNDUPS AND created BETWEEN :start AND :end-1';
  return db\sum('amt', 'txs_aux', $where, compact(ray('uids start end')));
}

/**
 * Return a list of account choices for the given account -- normally the current user (agent of the current account)
 * @param acct $a: the account for which to find related accounts
 * @param int $perm: $a's minimum permissions to include an account in the list
 * @param string $where: additional criteria
 * @param bool $noAdmin: if true, ignore admin permissions
 * @return an assoc of names keyed by account record IDs (FALSE if none)
 */
function accountChoices($a, $perm = B_READ, $where = 1, $noAdmin = FALSE) {
  if (!$a) return FALSE;

  $res[$aid = $a->agentId] = $a->agentA->bestName;
  $nameOrder = "IF(MID(fullName, 2, 2)='. ', MID(fullName, 4), IF(MID(fullName, 2, 1)=' ', MID(fullName, 3), IF(MID(fullName, 1, 4)='the ', MID(fullName, 5), fullName)))"; // ignore "the ", "a ", and initialled first name (eg "X." or "X")
  $sqlNormal = "SELECT r.main AS id, u.fullname FROM u_relations r LEFT JOIN users u ON u.uid=r.main WHERE $where AND r.other=:aid AND r.permission>=" . r\perm($perm); // normal selection SQL

  if ($a->admRegion and !$noAdmin) {
    $admWhere = $a->admSeeCanonic ? '1' : 'uid NOT IN (0,1,2,3)';
    $q = db\q("SELECT uid AS id, fullName FROM users WHERE $admWhere UNION $sqlNormal ORDER BY (id<0) DESC, $nameOrder", compact('aid'));
  } elseif ($a->admSeeAccts and !$noAdmin) {
    $sql = <<<EOF
SELECT DISTINCT id, fullName FROM
  (SELECT uid AS id, fullName FROM users WHERE community=:ctty UNION $sqlNormal) w
ORDER BY (id<0) DESC, $nameOrder
EOF;
    $q = db\q($sql, ray('ctty aid', $a->community, $aid));
  } else $q = db\q("$sqlNormal ORDER BY $nameOrder", compact('aid')); // the usual case

  $res += $q->fetchAllKeyed();
  if (sizeof($res) < 30) foreach ($res as $uid => $nm) $res[$uid] = r\acct($uid)->bestName;
  return $res;
}

/**
 * Remember what to do, once the user confirms.
 *
 * @parm string $number: the user's cell number (in standard format +1dddddddddd)
 * @parm string $todo: what to do once the user responds with the expected nonce (defaults to 'nothing')
 * @parm string $nonce: what to store as the nonce (defaults to 'whatever', meaning pick one at random)
 *
 * @return object(nonce, todo) if $todo is 'nothing', else the nonce
 */
function todo($number, $todo = 'nothing', $nonce = 'whatever') {
  if ($todo == 'nothing') {
    return (object) db\get('nonce,todo', 'r_boxes', 'code=:number', compact('number'));
  } else {
    $nonce = $nonce == 'whatever' ? u\nonce() : $nonce;
    \db_update('r_boxes')->fields(compact('nonce', 'todo'))->condition('code', $number)->execute();
    u\EXPECT(todo($number)->nonce == $nonce, 'setting nonce failed');
    return $nonce;
  }
}

/**
 * Create a new bank transfer, including records in txs2, txs_hdrs, and txs_entries (2).
 * @param acct $a: the account to or from which money is to be moved
 * @param numeric $amount: the amount to transfer
 * @param assoc $options: any of these keyed values:
 *   int $created: the UNIX time at which the transfer is deemed to be created (defaults to now)
 *   int $completed: mark the transaction complete on this date
 *   int $pid: record ID of associated people record (NULL if none)
 *   int $cat: record ID in tx_cats of transaction category (or NULL)
 *   string $bankAccount: the non-member bank account, if any (for ACH donations (or payments, NYI))
 *   string $purpose: transaction description
 *   int $deposit: date deposited
 *   int $recursId: related tx_timed record ID
 *   int $flags: transaction flags
 * @return the transaction object for the requested transfer
 */
function createUsdTransfer($a, $amount, $options = []) {
  u\EXPECT(compact(ray('a amount options')), 'acct int|float assoc');
  global $channel; 
  
  extract(just('created completed pid cat purpose deposit bankAccount recursId flags', $options, NULL));

  u\setDft($created, now());
  u\setDft($purpose, $amount > 0 ? t('from bank') : t('to bank'));
  $payee = $a->id;

  list ($eType, $deposit, $bankId, $bankAccount) = $pid
  ? [E_OUTER, $deposit ?: 0, UID_OUTER, $bankAccount] // transfer to or from a non-member (with no CG account)
  : [E_BANK, 0, $amount >= 0 ? UID_BANK : UID_BANK, nni($a->vsecure, 'bankAccount', NULL)];
  u\EXPECT($bankAccount or $deposit or ($pid and $amount < 0), 'bad bank account in bank transfer request: ' . pr($options + compact('payee')));

  $tx0 = ray('goods flags actorId actorAgentId channel created recursId', FOR_USD, $flags ?: 0, $payee, $payee, $channel, $created, $recursId);
  $tx = new r\Tx($tx0);
  $cat1 = u\getBit($flags, B_CC) ? r\nick2cat(r\acct(UID_BANK)->mainQid) : NULL; // set this here because bank and outer are not handled by form setCats and be\setCat()
  $cat2 = is_array($cat) ? $cat[1] : $cat;
  $extra = ($cat1 or $cat2) ? ray('cat', [$cat1, $cat2]) : [];
  if (isset($method)) $extra += compact('method');
  
  txEntryPair($txes, $tx, 0, [$bankId, $payee], $purpose, $eType, $extra); // tentative amount is 0
  list ($buyer, $seller) = u\order($amount > 0, r\acct($bankId), $a);
  if ($amount) handleSurTx($txes, $tx, $buyer, $seller, abs($amount), FALSE);
  $tx->addEntries($txes);
  
  $DBTX = \db_transaction();
  $xid = $tx->insert();
    
  $info = compact(ray('amount payee created deposit bankAccount channel xid'));
  $info += just('pid txid', $options);
  $txid = db\insert('txs2', $info, 'txid');

  if ($completed) completeUsdTx($info + ['eid' => $tx->entries[0]->id], $completed);
  
  unset($DBTX);
  
  return $tx;
}

/**
 * Mark a USD transfer complete, update the amounts in the corresponding transaction entries, and report.
 * @param mixed $xid: record ID of the transfer transaction or an assoc of the relevant information
 * @param datetime $completed: when the transaction was completed, defaults to the current time
 * @return bool <success>
 */
function completeUsdTx($xid, $completed = NULL) {
  u\EXPECT([$xid, $completed], 'int|assoc int|empty');
  u\setDft($completed, now());
  
  if (is_array($info = $xid)) {
    $info = just('payee amount eid pid xid', $info, NULL);
  } else {
    $info = db\get('d.payee,d.amount,eid,pid', 'txs2 d JOIN txs t USING(xid)', compact('xid'));
  }
  u\EXPECT((bool) $info, 'bad xid in completeUsdTx');
  extract($info);

  $DBTX = \db_transaction();
  $res = db\update('txs2', compact(ray('xid completed')), 'xid');
//  $res0 = db\update('tx_hdrs', ray('xid created', $xid, $completed), 'xid');
  $res1 = db\update('tx_entries', ray('id amount', -$eid, -$amount), 'id');
  $res2 = db\update('tx_entries', ray('id amount', $eid, $amount), 'id');
  if (!($res and $res1 and $res2)) {
    r\tellAdmin(t('usd update failed'), $info);
    $DBTX->rollback();
    return FALSE;
  }
  unset($DBTX);

  return TRUE;
}

/**
 * Report completion of a transfer from a member's bank account to their CG account.
 * @param int $xid: transaction record ID
 * @param bool $report: <report success rather than returning it>
 * @return FALSE if transfer is outgoing, non-member, or not complete
 *    otherwise if report is TRUE, report success and return TRUE, else return a success message
 */
function usdTxFollowup($xid, $report = TRUE) {
  $info = db\get('payee,amount,pid,completed,channel', 'txs2', compact('xid'));
  u\EXPECT((bool) $info, 'txs2 record missing: ' . $xid);
  extract($info);
  
  if ($completed and $amount > 0 and !$pid) {   
    $transfer = $channel == TX_CRON ? t('automatic transfer') : t('transfer');
    $amount = u\fmtAmt($amount);
    $msg = tr('transfer-in complete', compact('transfer', 'amount'));
    if ($report) {
      r\acct($payee)->tellIn($msg);
      return TRUE;
    } else return $msg;
  } else return FALSE;
}

/**
 * Handle balance changes for an account.
 * @param assoc $info: assoc containing the uid and how much it just changed
 */
function balChange($info) {
  extract(just('uid amount', $info));
  $a = r\acct($uid);
  u\EXPECT($a, 'invalid account in balChange');
  if ($a->isCanonic) return;
  
  $bal = $a->balance; // balance after insert
  if ($bal - $amount >= 0 and !$a->wentNeg) { // !$a->wentNeg makes this self-correcting
    if ($bal < 0) $a->update('wentNeg', now());
  } elseif ($bal >= 0) $a->update('wentNeg', '');
  
//  if ($uid > 0) f('cr.queue', $amount > 0 ? 'payRequests' : 'getFunds1', compact('uid'), TRUE); // TRUE keeps this from looping in tests
}

function invPayments($nvid) {return db\sum('amt', 'txs', "relType='I' AND rel=:nvid", compact('nvid'));}
function invRemains($nvid) {return round(db\get('amount', 'tx_requests', compact('nvid')) - invPayments($nvid), 2);}
