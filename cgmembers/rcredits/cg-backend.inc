<?php
/**
 * @file
 * Top level application interfaces (SMS, web, smartphone, etc.) to rCredits.
 * These high-level business functions are called by more than one interface.
 */

namespace CG\Backend; // typically abbreviated as "be"
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Web as w;
require_once __DIR__ . '/tx.class';
require_once __DIR__ . '/txentry.class';

define('REAL', nni($_SERVER, 'SystemRoot') != 'C:\\Windows'); // used?
// require_once __DIR__ . '/cg-util.inc';

/**
 * Generates an array of transactions, with added data.
 * @param mixed $uids: the uid or uids whose transactions we're going to process
 *        if $uids is an array it is the set of uids;
 *        if $uids is an integer it is the only uid
 * @param integer $starting: the starting unix time
 * @param integer $ending: the ending unix time
 * @return an array of transactions each element of which is a single entry associated with the transaction.  Note that tx_hdrs data is in each of the entries. If there's a failure return [].
 */
function getTxs($uids, $starting=NULL, $ending=NULL) {
  if (is_integer($uids)) { $uids = [ $uids ]; }

  $where = '';
  if (!is_null($starting)) { $where .= " AND t.created>=$starting"; }
  if (!is_null($ending)) { $where .= " AND t.created<=$ending"; }
  $fromMe = 't.uid1 IN (:uids)';
  $toMe = 't.uid2 IN (:uids)';

  // tReversed and eReversed refer to the transaction that this transaction reverses.
  // tReversedBy and eReversedBy refer to the transaction that reverses this transaction.
  // !(t.amt=0 AND t.type=:E_BANK) hides bank account verifications
  $sql = <<< EOF
    SELECT t.*, u.uid, u.fullName AS name, tp.period, tp.periods, ru.portion, i.nvid, $toMe AS toMe,
      IF($toMe, amt, -amt) AS amount, IF($toMe, for1, for2) AS description, IF($toMe, agt1, agt2) AS agt,
      (ru.action=:ACT_SURTX AND IF($toMe, ru.to=t.uid2, ru.from IN (:MATCH_PAYER, t.uid1))) AS stepup,
      (SELECT xid FROM tx_hdrs tRev WHERE tRev.reversesXid=t.xid) AS reversedBy,
      (EXISTS (SELECT id FROM tx_disputes d WHERE d.xid=t.xid AND d.status=:DS_OPEN)) AS isDisputed
    FROM txs t LEFT JOIN users u ON u.uid=IF($toMe, t.uid1, t.uid2)
    LEFT JOIN r_usd ON (r_usd.xid=t.xid)
    LEFT JOIN tx_templates tp ON tp.id=t.recursId
    LEFT JOIN tx_rules ru ON ru.id=t.rule
    LEFT JOIN r_invoices i ON i.status=t.xid
    WHERE (!(t.amt=0 AND t.type=:E_BANK) AND ($fromMe XOR $toMe)) $where
    ORDER BY t.created, t.xid, t.eid, name
EOF;
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
  foreach ($txs as $i => $entries) {
    $byMe = in_array($entries[0]['actorId'], $uids);
    $banking = ($entries[0]['type'] == E_BANK);
    $newEntries = [];
    $myAmount = 0; foreach ($entries as $entry) $myAmount += $entry['amount'];
    $balance = round($balance + $myAmount, 2);
    
    foreach ($entries as $entry) {
      extract(just('type reversesXid reversedBy description period periods stepup portion nvid', $entry, NULL));
      if (in($type, [E_PRIME, E_BANK])) {
        if ($period) $description .= t(' (%xly)', 'xly', r\recurDesc($period, $periods));
        if ($nvid) $description .= t(' (CG inv#%nvid)', compact('nvid'));
        if ($reversesXid) $description .= t(' (reverses #%reversesXid)', compact('reversesXid'));
        if ($reversedBy) $description .= t(' (reversed by #%reversedBy)', compact('reversedBy'));
      } else {
        if ($stepup) $description .= $portion > 0 ? t(' (%x step-up)', 'x', u\fmtAmt($portion, '%s')) : t(' (step-up)');
      }
      $newEntries[] = compact(ray('myAmount balance byMe banking description')) + $entry;
    }
    $txs[$i] = $newEntries;
  }
  return $txs;
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
 * @param integer $date: the unix time to consider
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
 * Return an array of member names (called with ajax, so current user is passed as arguments).
 * Restricted to within NEIGHBOR_MAX miles of the member
 * @param int $myid: account record ID
 * @param int $aid: agent account record ID
 */
function memberRay($myid, $aid, $coOnly = FALSE) {
  if (!$a = r\acct($myid, $aid)) return [];
//  $where = $coOnly ? ':IS_CO' : ($a->admin ? '1' : "state=$a->state OR community=$a->community OR :IS_CO");
  $where = $coOnly ? ':IS_CO' : ($a->admin ? '1' : tr("(%DISTANCE_BETWEEN<%NEIGHBOR_MAX OR community=$a->community OR :IS_CO)"));
  if (isDEV) $where = str_replace(DISTANCE_BETWEEN, '0', $where); // no lat/lon on dev machines yet
  $subs = compact('myid');
  if (!$coOnly and !$a->admin and !isDEV) $subs += ray('lat lon', $a->latitude, $a->longitude);
///   flog(db\col('fullName', 'users', ":IS_OK AND uid<>:myid AND $where ORDER BY fullName", $subs));
  return db\col('fullName', 'users', ":IS_OK AND uid<>:myid AND $where ORDER BY fullName", $subs);
}

/**
 * Return the next relevant transaction for form 1099b, for an account.
 * @param string $type: TEN99B or TEN99K
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
    $where = "uid2=:payee AND t.created BETWEEN $start AND $end AND type<>:E_BANK";

    if ($type == TEN99B) $sql = <<< X
      SELECT created, amt AS amount, uid1 AS payer, for2 AS `for`
      FROM txs_noreverse t WHERE $where ORDER BY t.created
X;

    if ($type == TEN99K ) {
      for ($m = 1; $m <= 12; $m++) {
        $dt1 = $m == 1 ? $start : ($dt2 + 1);
        $dt2 = u\plusMonths(1, $dt1) - 1;
        if (test()) $dt2 = $dt1 + 30 * DAY_SECS - 1;
        $moFlds[] = "SUM(IF(t.created BETWEEN $dt1 AND $dt2, amt, 0)) AS amt$m";
      }
      $moFlds = join(', ', $moFlds);
      $sql = <<< X
        SELECT uid2, SUM(amt) AS gross,
               SUM(IF(channel=:TX_POS, 0, amt)) AS noCard,
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
 * Update relations
 * @param array $updates: list of records to update, indexed by reid
 *   each element is an associative array of new values, keyed by field name
 *   if a record is to be updated to all zeros, it is simply deleted.
 * @return string: a report of what got updated
 */
function updateRelations($updates) {
///     debug("updateRelations(" . print_r($updates, true) . ")");
  $myid = r\acct()->id;
  $report = '';
  foreach ($updates as $reid => $data) {
    extract(just('main other fullName permission flags', $data['original'], NULL));
    $wasEmpty = (!$permission and !$flags);
    unset($data['original']);
    extract(just('permission flags', $data));
//    foreach (ray('employee owner customer draws') as $k) u\setBit($flags, $k, @$data[$k]);
    foreach (ray('employee owner customer draw') as $k) u\setBit($flags, $k, nni($data, $k)); // drawS?
    if (!u\getBit($flags, 'customer')) u\setbit($flags, 'autopay', FALSE);

    if (!$permission and !$flags and !$wasEmpty) { // empty now and wasn't before
      db\del('r_relations', compact('reid'));
      $msg = 'deleted relation';
    } else {
      $a = r\acct($main, $other);
      if ($data) $a->setRel($data); // db\update('r_relations', $data + $reidSub, 'reid');
      if (nni($data, 'draw')) r\acct($other)->setBit(B_DRAWS);
      $msg = 'updated relation';
    }
    $otherName = $fullName;
    $report .= tr($msg, compact('otherName')) . '<br>';
  }
  return $report;
}

/**
 * Undo the given transaction.
 * Situations:
 *     I paid someone (byMe):            REVERSE (invoice)
 *     I charged someone (toMe/byMe):    REVERSE (pay back)
 *     Someone paid me (toMe):           REVERSE (pay back)
 *     Someone invoiced me and I approved it: DISPUTED
 *     Someone charged me unilaterally: DISPUTED
 * @param int $xid: the transaction number
 * @param string $where: additional (security check) criteria, if any
 * @param assoc $subs: additional substitutions for $where
 * @param bool $force: <reverse despite any shortfall or lack of permission>
 * @return array [success?, message, subs] where subs is an assoc of [
 */
function undoTx($xid, $where = '1', $subs = [], $force = FALSE) {
  global $mya;

  u\EXPECT('int string assoc bool', $xid, $where, $subs, $force);

  $tx = r\Tx::lastTx([$mya->id], "xid=:xid AND $where", compact('xid') + $subs);
  if (empty($tx)) return [FALSE, 'undo no match', $subs];
  if (!$force and $err = $tx->noUndo($subs)) return [FALSE, $err, $subs];

  $byMe = $mya->isMe($tx->actorId);
  $toMe = $tx->getAmount($mya) > 0;
  if (!$byMe and !$toMe) { // someone charged me (either unilaterally or I approved it)
    r\disputeTx($xid); // mark it disputed
    $solution = t('marked "disputed"');
    r\notify($tx->otherId, 'charge disputed', $tx->reportArgs($mya)); //, $tx->getMyAmt(), $payer, $payee, $payerPurpose, $payeePurpose));
    $amount = u\fmtAmt($tx->otherEntry->amount);
    $tofrom = !$tx->toMe ? t('to') : t('from');
    $otherUid = $tx->otherUid;
    $other = $tx->otherA->fullName;
    return [TRUE, 'report undo', compact(ray('amount other tofrom solution otherUid'))];
  } else {
    list ($ok, $message, $info) = reverse($xid); // create an offsetting transaction
    sendNotifications('undo', $info['newTx'], 0);
    unset($info['newTx']);
    return [$ok, $message, $info];
  }
}

/**
 * Create an offsetting transaction (nearly the same as the original but with the entries negated).
 * @param int $xid: record ID of the transaction to reverse
 * @return [ok?, $msg, $subs]
 */
function reverse($xid) {
  global $mya, $channel;
  $subs = compact('xid');

  $DBTX = \db_transaction();
  if (db\exists('tx_hdrs', 'reversesXid=:xid', $subs)) return r\txErr(t('already reversed'), $subs);
  if (!$tx = r\Tx::read($xid)) return r\txErr(t('no transaction'), $subs);

  // It seems OK to reverse it
  $newTx = clone $tx;  // this is a shallow copy, entries points to the same entries
  $newTx->xid = NULL;
  $newTx->reversesXid = $xid;
  $newTx->entries = [];
  $newTx->created = now();
  foreach ($tx->entries as $oldEntry) {
    $newEntry = clone $oldEntry;
    $newEntry->id = NULL;
    $newEntry->xid = NULL;  // new entry belongs to reversing transaction
    $newEntry->amount = -$oldEntry->amount;
    $newTx->entries[] = $newEntry;
  }
  $xid = $newTx->insert();
  if (!$xid) {
    $DBTX->rollback();
    return r\txErr(t('tx save err'), $newTx);
  }

  unset($DBTX);

  $message = 'report undo' . ($tx->otherA->isCanonic ? '' : '|report tx');
  $did = ($tx->actorEntry->amount > 0) ? ($tx->goods==FOR_GOODS ? 'refunded' : 'credited') : 're-charged';
  $info = ['ok' => 1, 'txid' => $xid, 'created' => $newTx->created, // txid is xid in transaction reports
    'balance' => $mya->can(B_SECRET) ? '*0' : $mya->balance,
    'xid' => $xid, 'created0' => $newTx->created, 'newTx' => $newTx,
    'solution' => 'reversed', 'did' => $did, 'otherName' => $tx->otherA->fullName,
    'amount' => u\fmtAmt(abs($newTx->otherEntry->amount)), 'why'  =>  $tx->otherEntry->description];
  
  return [TRUE, $message, $info];
}

/**
 * Pay the given invoice, possibly without being signed in.
 * @param int $nvid: record ID of invoice to pay
 * @return [TRUE, explanation] if the payment was successful
 *         [FALSE, explanation] if the payment failed
 */
function payInvoice($nvid) {
  u\EXPECT('int', $nvid);
  $msg = r\badUnpaidInv($nvid, $inv);
  if ($msg) return [FALSE, $msg];
  extract(just('payer payee amount goods purpose data flags recursId', $inv, [NULL, NULL, 0.0, FOR_GOODS, NULL, '', 0, 0]));
  $payerA = r\acct($payer);
  $payeeA = r\acct($payee);
  $data = unserialize($data);
  extract(just('api request shid for', $data, NULL));
  
  $DBTX = \db_transaction();
	$extra = ray('inv', $nvid) + just('api for', $data, NULL);
	if ($flags) foreach (ray('gift crumbs recurs investment') as $k) if (u\getBit($flags, $k)) $extra[$k] = 1;

  list ($ok, $index, $subs) = be\transfer('invoice payment', $payerA, $payeeA, $amount, $purpose, $goods, $extra, $recursId);
  if ($ok) {
    db\update('r_invoices', ray('status nvid', $subs['xid'], $nvid), 'nvid');
    if (!empty($api)) u\post($api, ray('request ok msg', $request, (int) $ok, tr($index, $subs)));

    if (u\getBit($flags, B_INVESTMENT)) { // club reclaiming an investment
      if ($shid and $res = db\get('vestid,price', 'r_shares JOIN r_investments USING(vestid)', compact('shid'))) { // record sale
        extract($res);
        db\update('r_shares', ray('shid shares pending when', $shid, -round($amount / $price), 0, now()), 'shid');
        if (db\sum('shares', 'r_shares', compact('vestid')) <= 0) db\q('UPDATE r_shares SET sold=:NOW WHERE vestid=:vestid', compact('vestid'));
      } else r\tellCo(t('Invoice payment marked as "for investment (repayment)" has no corresponding SELL request in shares table.'), $inv, $payeeA);
    }
  }
  unset($DBTX);
  
  return [$ok, t($index, $subs)];
}

/**
 * Create a transaction
 * call by: list ($ok, $msg, $args) = be\transfer(...);
 * @param string $type: payment, charge, invoice payment, draw
 * @param acct $actorA: acct object for the initiating party (usually the current user)
 * @param acct $otherA: acct object for the other party
 *   OR 'ALL' (paying employees) -- currently UNUSED
 * @param number $amount: the transaction amount
 * @param string|array $purpose: description of the transaction's purpose (user input) (or [actorFor, otherFor])
 * @param int $goods: FOR_GOODS, FOR_USD, others?
 * @param array $data: associative array of extra info about the transaction (if any), indexed by any of:
 *   'created': time of transaction (defaults to current time)
 *   'force': <do the transaction despite any shortfall (1 for original tx, -1 for forced reversal)>
 *   'boxId': the id of the device on which the transaction was initiated
 *   'offline'
 *   'loan'
 *   'investment'
 *   'stake'
 *   'fine'
 *   'noask'
 *   'funding'
 *   'crumbs', 'recurs', 'gift': gift info
 *   'ruleId': the id of the rule associated with this transaction
 * @param int $recursId: record ID of related record in tx_templates (if any)
 * @return simple array: 
 *    ok: success?
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message
 *
 * The following used to be possible keys for $data, but not any more:
 *   'inv': record number of the invoice for which this is payment
 *   'roundups'
 *   'roundups': the payment is change being donated to the community
 *   'rebate': the rebate amount for this request (should be negative the original bonus amount)
 *   'bonus': the bonus amount (should be negative the original rebate amount)
 *   'undoes': transaction id of the related transaction (transaction being undone)
 *
 *   'short': ?
 *   'funding': ?
 *
 * Do something different, based on these boolean values: 
 *   $taking (charge vs. payment), 
 *   $oked UNUSED (user confirmed the transaction request),
 *   $shortfall (how inadequate the payer's balance is)
 */
function transfer($type, $actorA, $otherA, $amount, $purpose, $goods, $data = [], $recursId = 0) {
  u\EXPECT('string acct acct number string|array int assoc', $type, $actorA, $otherA, $amount, $purpose, $goods, $data);
  u\EXPECT($actorA and $otherA, 'missing a transfer account');
  global $scanned, $channel;
  
  if (is_null($goods)) $goods = FOR_GOODS;
  $taking = ($type == 'charge' or $type == 'draw' or ($type == 'transfer' and $amount < 0));
  
  extract(just('created force boxId ruleId', $data, [now(), 0, NULL, NULL]));
  extract(just('offline loan investment stake fine noask funding crumbs recurs gift api for', $data, FALSE));

  $short = FALSE;
  
  u\setDft($purpose, ray(R_WHYS)[$goods]);

  if ($force != 0) $offline = TRUE; // temporary until app handles this right

  $erInfo = compact(ray('goods amount purpose force')) + ray('actor other op', $actorA->fullName, $otherA->fullName, $taking ? t('charged') : t('paid'));
  $amount = round($amount, 2); // ignore fractions of cents
  if ($amount == 0) {
    r\tellAdmin('zero transaction', $erInfo + ['stack' => trace()]);
    return r\txErr(t('zero transaction'), $erInfo);
  }
  $reallyTaking = ($taking xor $amount < 0); // not taking if charging a negative amount

  list ($loserA, $gainerA) = u\order($reallyTaking, $otherA, $actorA);
  list ($e1A, $e2A) = u\order($taking, $otherA, $actorA);
  
  $ptrs = ray('actor other loser gainer e1 e2');
  foreach ($ptrs as $k) {
    ${$k . 'Id'} = ${$k . 'A'}->id;
    ${$k . 'Name'} = ${$k . 'A'}->fullName;
    ${$k . 'For'} = is_array($purpose) ? $purpose[${$k . 'Id'} == $actorId ? 0 : 1] : $purpose;
  }
    
  $erInfo += compact('loserId');

  $lastTx = $actorA->lastTx();
  if (!empty($lastTx) and !$force and $channel != TX_WEB and $lastTx->actorId == $actorId
      and $lastTx->otherId == $otherId and $lastTx->goods == $goods and $lastTx->getAmount($actorA) == $amount
      and ((now() - $lastTx->created) < R_DUPTX_MINS * MIN_SECS)) {
    return r\txErr(t('duplicate transaction'), $erInfo);
  }

  if ($err = r\txPermErr($actorA, $otherA, $taking, $amount < 0)) { // Check permissions
    list ($ok, $erMsg, $erSubs) = $err;
    if ($force) {
      $msg = 'forced without perm';
      $erInfo += ['date' => u\fmtDate($created)];
      foreach (['agent1'=>$actorA, 'agent2'=>$otherA] as $k => $a) $erInfo += [$k => $a->proSe ? t('self') : $a->fullName];
      foreach ([$actorA, $otherA] as $a) if (!$a->proSe) r\message($a->id, $msg, $erInfo);
      r\tellAdmin($msg . ' ' . t($erMsg, $erSubs), $erInfo + $erSubs);
    } else return r\txErr($erMsg, $erInfo + $erSubs);
  }

  foreach ([$actorA, $otherA] as $a) if (!$a->co and !($channel == TX_POS ? $a->agentA->ided : $a->ided)) {
    if ($force) {
      r\tellAdmin('forced without photoId', $erInfo); 
    } elseif ($gainerId != CGID) { // and !$gainer->coCan(CO_FAST)
//      r\tellAdmin('tx without photoId', $erInfo); 
//      return r\txErr($a == $actorA ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $otherA->fullName]);
    }
  }

  if ($error = u\badAmount($amount)) return r\txErr($error, $erInfo);
  $flagNames = 'offline short crumbs recurs gift loan investment stake fine noask funding';
  $flags = 0;
  foreach (ray($flagNames) as $k) if ($$k) u\setBit($flags, $k);

  txEntryPair($entries, $amount, [$e1A, $e2A], [$e1For, $e2For], E_PRIME, $ruleId);
  
  $DBTX = \db_transaction();

  // Create the transaction
  $info = ray('actorId actorAgentId',$actorA->id, $actorA->agentId) + compact(ray('flags channel boxId goods created recursId'));
  $tx = new r\Tx($info);
  if (!$tx) {
    $DBTX->rollback();
    return r\txErr(t('tx create err'), $erInfo);
  }

  $needed = abs($amount);

  // Apply rules
  if ($amount > 0 and ($goods != FOR_USD)) {
    $needed += handleSurTx($entries, $loserA, $gainerA, $amount);
  }

/*  // Apply food fund (or other special) donation
  } */
  // Apply any roundup
  if ($e1A->roundup and $needed > 0 and !($e2Id == CGID and $recurs and $needed < 1)) { // payer roundups, not actor or loser (don't round up reversals, cashouts, roundups, or recurring gifts under $1)
    if ($cents = fmod($needed, 1.0)) {
      txEntryPair($entries, 1 - $cents, [$loserId, CG_ROUNDUPS_UID], t('roundup donation'), E_DONATION);  
      $needed = ceil($needed); // == 1-$cents
    }
  }

  // Calculate any balance shortfall
  if ($shortfall = ($loserA->id < 0 ? 0 : $loserA->shortfall($goods, $needed, TRUE, $actorA))) {
    if ($force) {
      $short = TRUE; // flag overdraft
    } else {
      $DBTX->rollback();
      $short = u\fmtAmt($shortfall);
      $avail = u\fmtAmt($needed - $shortfall);
      $msg = $reallyTaking ? ($otherA->can(B_SECRET) ? 'short from vague' : 'short from') : 'short to';
      $loserA->suggestAuto();
      $details = t('|%loser was short by %short trying to pay %gainer %amount (available balance is %avail)', 'loser short gainer amount avail', $loserName, $short, $gainerName, u\fmtAmt($needed), $avail);
      return r\txErr($msg, compact(ray('otherName short details avail')) + ['success'=>false]);
    }
  }
  
  if (!$force) {
    if ($gainerA->co and $gainerA->depends and db\sum('amt', 'txs_noreverse', 'uid2=:gainerId', compact('gainerId')) + $needed > TRIALCO_AMT_LIMIT) {
      $DBTX->rollback();
      return r\txErr(t('trial co over'), $erInfo);
    }
    if ($gainerA->needSsnFor($needed)) {
      $DBTX->rollback();
      r\tellAdmin(t('SSN needed'), $erInfo);
      $gainerA->setTask('sell');
      $stepsLeft = $gainerA->stepsLeft();
      $stepsLeft[] = 'ssn';
      $gainerA->update('stepsLeft', array_values($stepsLeft));
      return r\txErr(t('ssn needed', $erInfo));
    }
  }
    
  $tx->addEntries($entries);
  if (array_key_exists('xid', $data)) {
    $tx->xid = $data['xid'];
    $xid = $tx->insert(TRUE);
  } else { $xid = $tx->insert(); }
  
  if (!$xid) {
    $DBTX->rollback();
    return r\txErr(t('tx save error'), $erInfo);
  }
  
  if ($goods != FOR_USD and $pot = $loserA->giftPot) $loserA->update('giftPot', max(0, $pot - $needed));
  if (nn($for)) advanceCredit($for, $loserA, $gainerA, $amount); // purchase of store credit
  
  unset($DBTX);
  
  // Report results
  // notify other party by email about any completed transaction
  foreach ([$actorA, $otherA] as $a) if (!$a->confirmed) $a->setBit(B_CONFIRMED);

  return sendNotifications($type, $tx, $shortfall);
}

/**
 * Add a pair of related entries to the transaction: one losing, one gaining.
 * Any ...0 parameter but $amount0 can be a 2-element array (one element for each party)
 * @param $txes array: flat array of entry objects to which to add 2 entries
 * @param numeric $amount0: the amount paid/received (amount to add to the first party's balance)
 * @param mixed $a0: account object(s) and/or uid(s)
 * @param string $description0: transaction purpose(s)
 * @param int $entryType0: entry type(s)
 * @param int $rule: relevant record ID in tx_rules table
 */
function txEntryPair(&$txes, $amount0, $a0, $description0, $entryType0, $rule = NULL) {
  foreach ([1, 0] as $i) { // payee first, to agree with tests and so entry id is postive for e2, negative for e1
    foreach (ray('a description entryType') as $k) $$k = is_array(${$k . 0}) ? ${$k . 0}[$i] : ${$k . 0};
    $amount = $i == 0 ? -$amount0 : $amount0;
    list ($uid, $agentUid) = is_object($a) ? [$a->id, $a->agentId] : [$a, $a];
    $txes[] = new r\TxEntry(compact(ray('entryType amount uid agentUid description rule')));
  }
}

/**
 * Create a personal credit for buyer or someone else, at the seller.
 */
function advanceCredit($for, $buyer, $seller, $amount) {
  if ($neg = ($amount < 0)) $amount = -$amount;
  
  if ($for !== SELF_CREDIT) { // gift!
    $for = r\acct($for);
    r\message($for->id, t('%name %action of %amount credit at %co. To redeem store credit you are given, simply pay with your %PROJECT card. Your account is credited automatically.'), ray('name action amount co', $buyer->fullName, $neg ? t('canceled your gift') : t('sent you a gift'), u\fmtAmt($amount), $seller->fullName), $neg ? t('Your gift is canceled') : t('You have received a gift through Common Good!'));
    $buyer = $for;
  }
  
  $portion = $for == SELF_CREDIT ? .5 : 1;
  $info = ray('action portion amtMax payerType payer payeeType payee from to', ACT_SURTX, $portion, $amount, REF_ACCOUNT, $buyer->id, REF_ACCOUNT, $seller->id, MATCH_PAYEE, MATCH_PAYER);

  if ($neg) { // undoing, so end the rule
    if ($id = db\get('id', 'tx_rules r', ['end' => NULL] + $info)) db\update('tx_rules', ray('id end', $id, now()) + $info, 'id');
  } else {
    db\insert('tx_rules', $info);
  }
}
  

/**
 * Send appropriate notifications and return a success message array
 * @return [TRUE, msg, subs]
 */
function sendNotifications($type, $tx, $shortfall) {
  u\EXPECT(compact(ray('type tx shortfall')), 'string tx int|float');

  $otherA = $tx->otherA;
  $toMe = $tx->actorAmount > 0;

  switch ($type) {
  case 'charge':
  case 'draw':
    if (!$toMe) {   // I made a payment to other
      if ($tx->goods == FOR_GOODS) {
        $notice = 'new refund';
        $did = 'refunded';
      } else {  // I made a payment to other
        $notice = 'new payment';
        $did = 'credited';
      }
    } else {
      $notice = 'new charge';  // I charged other
      $did = 'charged';
    }
    break;
  case 'payment':
    if (!$toMe) {
      $notice = 'new payment';  // I made a payment to other
      $did = 'paid';
    } else {
      $notice = 'new charge';  // I charged other
      $did = 'charged';
    }
    break;
  case 'invoice payment':
    if (!$toMe) {
      $notice = 'new payment';  // I made a payment to other
      $did = 'paid';
    } else {
      $notice = 'new charge';  // I charged other -- I don't think this should happen.  JVER
      $did = 'charged';
    }
    break;
  case 'undo':
    $mya = r\acct();
    $byMe = $mya->isMe($tx->actorId);
    if ($byMe and !$toMe) {
      $notice = ($tx->goods == FOR_GOODS) ? 'new refund' : 'new payment';
      $did = ($tx->goods == FOR_GOODS) ? 'refunded' : 'credited';
    } elseif ($byMe and $toMe) {
      $notice = 'new charge';  // I charged other, reversing is refunding
      $did = 'charged';
    } elseif (!$byMe and $toMe) {
      $notice = 'payment';  // Other made payment to me, reverse is refunding
      $did = 'paid';
    } else { /*!byMe and !$toMe*/
      $notice = 'dispute';  // Other charged me, reversing requires dispute
      $did = 'disputed';
    }
    break;
  default:
    u\EXPECT(false, "type must not be '$type'.");
  }
  if ($notice == 'charge' and $tx->goods == FOR_USD) $notice = 'cash out';

  $args = $tx->reportArgs($tx->actorA, TRUE, $shortfall);
  if ($notice == 'new payment' and $otherA->co) { // not "new charge" (the most common); payment at payer's initiative
    $notice = 'new payment linked';
    $args['_aPayLink'] = $otherA->makeDo('addr', $tx->actorId, 0); // link to address, so merchant can ship or nonprofit can thank
  }

  if ($otherA->id < 0 or $otherA->id > CANONIC_ACCTS) r\notify($tx->otherId, $notice, $args);  // no notifications to canonic accounts
  $msg = 'report tx';
//  return r\txErr($msg, $tx->reportArgs($tx->actorA, TRUE, $shortfall) + compact(ray('msg did')));
  return [TRUE, $msg, $args + compact(ray('msg did'))];
}

/**
 * Generate tentative transaction entries for a coupon
 * @param assoc $entries: (MODIFIED) an array of entries to add to, if appropriate
 * @param acct $buyer:
 * @param acct $seller:
 * @param number $price: pre-discount, pre-stepup amount
 * @param unixtime $when: when was the transaction (defaults to now)
 * @return the net increase in overall transaction amount
 */

function handleSurTx(&$entries, $buyer, $seller, $price) {
  u\EXPECT($price > 0, 'non-positive price handling coupon');
  
  $payerCo = $buyer->co ? 1 : 0;
  $payeeCo = $seller->co ? 1 : 0;

  // Get a list of applicable rules
  $payerCheck = u\genRuleSQL('payer', ':buyer');
  $payeeCheck = u\genRuleSQL('payee', ':seller');
  
  $sql = <<<EOF
    SELECT ru.id AS ruleId, ru.amount, ru.portion, ru.from, ru.to, ru.purpose, ru.minimum, ru.useMax, 
      ru.amtMax, ru.payerType,
    (SELECT IFNULL(SUM(e.amount), 0) FROM tx_entries e WHERE e.rule IS NOT NULL AND e.rule=ruleId AND e.uid=:buyer) AS `amtUsed`,
    (SELECT IFNULL(SUM(SIGN(e.amount)), 0) FROM tx_entries e WHERE e.rule IS NOT NULL AND e.rule=ruleId AND e.uid=:buyer) AS `timesUsed`
    FROM tx_rules ru
    WHERE $payerCheck
    AND $payeeCheck
    AND (ru.action = :ACT_SURTX)
    AND (ru.start <= :now AND IFNULL(:now < ru.end, TRUE))
    AND (:price >= minimum)
    GROUP BY ruleId
    HAVING (:price >= minimum) AND timesUsed < IFNULL(useMax, timesUsed+1) AND amtUsed < IFNULL(amtMax, amtUsed+1)
    ORDER BY ru.start
EOF;

  $subs = ray('buyer seller now price', $buyer->id, $seller->id, now(), $price);
  if (!$q = db\q($sql, $subs)) return 0;
///  showq($sql, $subs);
  if (!$res = $q->fetchAllAssoc('ruleId', \PDO::FETCH_ASSOC)) return 0;

  $rebate = 0;
  foreach ($res as $ruleId => $rule) {
    extract($rule);

    $value = $amount + $portion * $price; // potential rebate/stepup amount
    if ($amtMax) $value = min($value, $amtMax - $amtUsed); // limited by amtUsed

    $from = ($from == MATCH_PAYER) ? $buyer->id : (($from == MATCH_PAYEE) ? $seller->id : $from);
    $to = ($to == MATCH_PAYER) ? $buyer->id : (($to == MATCH_PAYEE) ? $seller->id : $to);
    list ($type, $desc) = $to == $buyer->id ? [E_REBATE, REBATE_DESC] : [E_DONATION, $purpose]; // any surtx not coming back to buyer is a donation of some sort
    if ($to == $buyer->id) $value = min($price, $value); // don't refund more than is being spent (food fund especially)
    $rebate += $value;
    
    txEntryPair($entries, $value, [$from, $to], $desc, $type, $ruleId);
    if (($amtMax and $amtUsed + $value >= $amtMax) or ($useMax and $value > 1 and $timesUsed + 1 >= $useMax)) {
      if ($payerType == REF_ACCOUNT) db\update('tx_rules', ray('id end', $ruleId, now()), 'id'); // rule is just for payer
    }
  }

  return -$rebate;
}

/**
 * Check whether the payer has a shortfall in paying for this transaction.
 * @param int $goods: the type of goods (treat USD transactions differently
 * @param acct $payer: account of the payer (we're checking this account for shortfall)
 * @param acct $payee: account of the payee
 * @param acct $actorA: account that initiated transaction, probably the same as $payer or $payee
 * @param acct $otherA: the other account, probably the same as $payee or $payer
 * @param acct $needed: amount needed to pay, should be positive; (e.g., price less rebates)
 * @return FALSE if there is no shortfall, otherwise a message (created by txErr)
 */
function checkForShortfall($goods, $payer, $payee, $actorA, $otherA, $needed, $force = FALSE) {
///    debug("checkForShortfall($goods, $payer->id, $payee->id, $actorA->id, $otherA->id, $needed, $force)");
  if ($payer->uid < 0) return FALSE;  // it's a community; they don't have shortfalls?
  
  $shortfall = $payer->shortfall($goods, $needed, TRUE, $payee);
  if ($shortfall == 0) return FALSE;
  
  $msg = ($actorA == $payer) ? ($payee->can(B_SECRET) ? 'short from vague' : 'short from') : 'short to';
  /*       if ($channel == TX_WEB) { */
  /* // too complex        if ($msg == 'short to' and $channel == TX_WEB and r\signedIn()) $msg .= '|increase min'; */
  /* /\*        if ($goods == FOR_GOODS) { */
  /*           if (!$payerA->can(B_DEBT) and $payerA->floor < 0 and $channel == TX_WEB) $msg .= '|try debt'; */
  /*           $_aDebt = w\atag('/settings/preferences'); */
  /*         } //elseif ($actorA->cttyRewardy) $msg .= '|short cash help|maybe not cash'; */
  /* 				*\/ */
  /*       } */
  if (!$force) $payer->suggestAuto();
  $details = t('|%payer was short by %short trying to pay %payee %amount (available balance is %avail)',
               'payer short payee amount avail',
               $payer->fullName, u\fmtAmt($shortfall), $payee->fullName, u\fmtAmt($needed),
               u\fmtAmt($needed - $shortfall));
  return r\txErr($msg, ray('otherName short details avail',
                           $otherA->fullName, u\fmtAmt($shortfall), $details, u\fmtAmt($needed-$shortfall)));
}
  
/**
 * Create an invoice
 * call by: list ($ok, $msg, $args) = be\invoice(...);
 * @param acct $payeeA: acct object for payee (usually current user)
 * @param acct $payerA: acct object for payer
 * @param numeric $amount: the invoiced amount
 * @param string $purpose: description of the transaction's purpose (user input)
 * @param assoc $extra:
 *    shid: record id of related record in r_shares
 *    status: used by acct->payApproved to mark transaction approved
 *    created: UNIXtime
 * @param int $recursId: record ID of related record in tx_templates (if any)
 * @return simple array: 
 *    ok: success?
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message
 *
 * NOTE: goods is no longer allowed in $extra, it's its own parameter
 */
function invoice($payeeA, $payerA, $amount, $purpose, $goods = FOR_GOODS, $extra = [], $recursId = 0) {
  global $channel;
  
  u\EXPECT(compact(ray('payeeA payerA amount purpose')), 'acct acct float string');
  u\EXPECT(! array_key_exists('goods', $extra), 'invoice() no longer accepts goods in extra');
  extract(just('created status', $extra, [now(), null]));
  $flags = 0;
  foreach (justNOT("status shid coupon coupid force inv api request created for", $extra) as $k => $v) {
    u\setBit($flags, $k, $v);
    unset($extra[$k]); // don't leave bits in extras (data field)
  }
  list ($payee, $payer) = array($payeeA->id, $payerA->id);
  u\setDft($purpose, $why = ray(R_WHYS)[$goods]);
  $subs = ray('myName otherName otherEmail otherPhone amount why purpose did op', $payeeA->fullName, $payerA->fullName, $payerA->email, u\fmtPhone($payerA->phone), u\fmtAmt($amount), $why, $purpose, t('charged'), t('charged'));
  if ($err = u\badAmount($amount, '>0')) return r\txErr($err, $subs);
  if ($err = r\txPermErr($payeeA, $payerA, TRUE)) return r\txErr($err, $subs);
  if (r\dupTx($payee, $payer, -$amount)) return r\txErr(t('duplicate transaction'), $subs);
  
  if (! $status) { $status = ($channel != TX_FOREIGN and r\relation(':IS_AUTOPAY', $payee, $payer)) ? TX_APPROVED : TX_PENDING; }
  $data = serialize($extra);
  $info = compact(ray('amount payer payee status purpose created flags data recursId'));
  $nvid = db\insert('r_invoices', $info, 'nvid');
  /* $payeeA->update('lastTx', [$nvid, $payer - $payee, round(-$amount, 2), $created, 'inv']); */

  if ($status == TX_PENDING) {
    $link = $payeeA->makeDo('inv', $nvid); // link to pay/deny
    //$payerA->update(ray('lastTx', array($nvid, $payee - $payer, round($amount, 2), $created, 'inv')));
    $subs += ray('_a1 reply email phone', $link, $payee, $payeeA->email, u\fmtPhone($payeeA->phone));
    r\message($payer, 'new invoice', $subs); // tell other party by email about this invoice
  } else { // pre-approved!
    if (!$payerA->shortfall(TX_TRANSFER, $amount)) { // don't try to pay yet if short
      list($ok, $msg) = be\payInvoice($nvid);
      if (!$ok) { r\tellAdmin(t('invoice payment failed'), compact('msg') + $subs + $info);
      } else { $status = TX_PAID; }
    }
  }

  $msg = 'report tx';
//  if ($payeeA->cttyRewardy) $msg .= '|for why';
  if ($amount > 0) $msg .= '|balance unchanged';
  return [TRUE, $msg, $subs + ray('success nvid status', TRUE, $nvid, $status)];
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

  $numberInternal = u\fmtPhone($number, '+n');
  $numberPretty = u\fmtPhone($number);
  $subs = compact('number'); // original format

  if ($validate) {
    if (!$numberPretty) return array('bad phone', $subs);
    if ($result = db\q('SELECT * FROM r_boxes WHERE code=:numberInternal', compact('numberInternal'))->fetchAssoc()) {
      extract($result, EXTR_PREFIX_ALL, 'old');
      if ($old_uid == $myid) return array('already cell', $subs);
      $accountName = "$mya->fullName ($mya->mainQid)";
    }
    return NULL;
  }
  
  //$status = SMS_PRIMARY;
  r\makeDevice($myid, $numberInternal, TX_SMS);
  if (!$mya->phone) $mya->update('phone', $numberInternal); // use it as contact phone, if none yet
  return array('report new cell', ray('number', $numberPretty));
}

function deleteCell($number) {
  $number = u\fmtPhone($number, '+n'); // internal format
  db\del('r_boxes', 'code=:number LIMIT 1', compact('number'));
  $number = u\fmtPhone($number); // pretty format
  return array('report delete cell', compact('number'));
}

/**
 * Figure out who the user meant. Only OK people in the user's community are eligible (except admin sees all).
 *
 * @param string $who: how the user referred to the trader (name, phone, email, uid, or temporary code)
 * @param int $mya: current user's account
 * @param bool $paying: <we're preparing to pay this person> (in which case the person has to have an active account)
 * @param string $self_message: index to error message for identifying oneself
 *   If true, the other trader will be created only if specified by email or phone
 * @return one of the following:
 *   the other trader's acct
 *   array(error index, array of substitutions, array of possible fullNames (0 or more) indexed by uid)
 */
function identify($who, $mya, $paying, $self_message = 'no self-trading') {
  if (!$mya) return ['search forbidden', [], []];
  $myid = $mya->id;
  $who = trim($who);
  $subs = compact('who');
  if ($phone = u\fmtPhone($who, '+n')) {
    $who = $phone; $cphone = u\cry('P', $phone);
  } else $cphone = '-';
  
  $short = u\shortName(" $who ", '%') ?: '-'; // abbreviations must not be null or zeroes
  $cmail = mb_strpos($who, '@') ? u\cry('P', mb_strtolower($who)) : '-';
  $subs = compact(ray('who short cphone cmail'));
  $anyAdmin = ($mya->cAdmin2 or $mya->isAdmin2);

  $searchable = $anyAdmin ? '1' : '!:IS_NOSEARCH';
  $where = "(name LIKE :short OR :who=fullName OR ($searchable AND (:cphone=phone OR :cmail=email)))";
  if ($anyAdmin) {
    $qid = mb_strtoupper(mb_strlen($who) == 3 ? R_SERVER_ID . $who : $who);
    if (u\isQid($qid) and $a = r\acct($qid)) $where .= " OR uid=$a->id";
    $where = "($where OR uid=:who)";
  }
  if ($paying) $where .= ' AND :IS_OK';
  if (!$mya->admin and !$mya->isAdmin2) {
    $where .= tr(' AND (:IS_CO OR %DISTANCE_BETWEEN<%NEIGHBOR_MAX)'); // restrict to nearby
    list ($lat, $lon) = $mya ? [$mya->latitude ?: 0, $mya->longitude ?: 0] : [0, 0]; // avoid NULL (PHP bug?)
    $subs += compact(ray('lat lon'));
  }
  
  $sql = <<<EOF
    SELECT DISTINCT uid,fullName
    FROM users 
    WHERE $where AND (uid<0 OR uid>:CANONIC_ACCTS)
    ORDER BY fullName
EOF;

  $result = db\q($sql, $subs)->fetchAllKeyed(0, 1);
  
  // QID? if not, maybe first word is a QID if this is from a list showing QID and fullName
  $qid = trim(($i = mb_strpos($who, ':')) ? mb_substr($who, 0, $i) : mb_strtoupper($who));
  if ($a = r\acct($qid)) $result[$a->id] = $a->fullName; // explicit QID is a result, regardless of community
  if (count($result) == 1 and nni($result, $myid)) return [$self_message, [], []];
  unset($result[$myid]); // self is one of many results, so ignore it
  if (!$result) return [($mya->admin ? 'unknown member' : 'unknown ctty member') . ("$cphone$cmail" == '--' ? '' : '|no search'), $subs, []];
  if (count($result) == 1) return r\acct(key($zot = $result)); // get the one entry (zot resets the pointer)
  return ['ambiguous other', $subs, $result];
}

function exception_error_handler($severity, $message, $file, $line) {
  if ($severity == E_NOTICE)
    throw new ErrorException($message, 0, $severity, $file, $line);
  return;
}
  

/**
 * Return financial information about the given account on this server
 * @param array uids: array of uids for the account to return information for
 * @param integer asof: one second past last time to include (defaults to 24 hours from now, meaning unrestricted)
 * @return an assoc of results (see $resultKeys, below)
 */
function creditInfo($uids, $asof = null) {
///     debug("creditInfo(" . print_r($uids, true) . ", $asof)");
  u\EXPECT(is_array($uids), 'bad call to creditInfo');
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

  $sql = "SELECT SUM(amount) AS pendingBank FROM r_usd t WHERE (payee IN (:uids)) AND created<:asof AND completed <= 0";
  $sums += db\q($sql, $subs)->fetchAssoc();

  foreach ($sums as $k => $v) if (is_null($v)) $sums[$k] = 0; // no nulls
  extract($sums);
  
  $balance = ($in - $out);
  $in -= ($fromBank - $toBank);
  
  $resultKeys = 'in out balance fromBank toBank pendingBank uids';
  $result = compact(ray($resultKeys));
///    $asof = u\fmtDate($asof); debug(compact(ray('asof sums in out fromBank toBank balance result')));
  foreach ($result as $k => $v) $result[$k] = $v = round($v, 2);

  return $result;
}

/**
 * Returns the roundup donations made by the specified users in the specified period.
 * @param array $uids -- the uids of the users, generally the main user or the main and joint users
 * @param unixtime $start -- the start of the period
 * @param unixtime $end -- the end of the period
 * @return the total roundup donations
 */
function roundupDonations($uids, $start, $end) {
  $where = 'uid1 IN (:uids) AND uid2=:CGID AND created BETWEEN :start AND :end';
  return db\sum('amt', 'txs_donation', $where, compact(ray('uids start end')));
}

/**
 * Return a list of account choices for the current user (agent of the current account)
 * @return an assoc of names keyed by account record IDs (FALSE if none)
 */
function accountChoices() {
  if (!$mya = r\acct()) return FALSE;
  $result[$aid = $mya->agentId] = $mya->agentA->fullName;
  $nameOrder = "IF(MID(fullName, 2, 2)='. ', MID(fullName, 4), IF(MID(fullName, 2, 1)=' ', MID(fullName, 3), IF(MID(fullName, 1, 4)='the ', MID(fullName, 5), fullName)))"; // ignore "the ", "a ", and initialled first name (eg "X." or "X")
  $sqlNormal = 'SELECT r.main AS id, u.fullname FROM r_relations r LEFT JOIN users u ON u.uid=r.main WHERE r.other=:aid AND r.permission>0'; // normal selection SQL

  if ($mya->admin or $mya->isAdmin2) {
    $where = $mya->superAdmin ? '1' : 'uid NOT IN (0,1,2,3)';
    $q = db\q("SELECT uid AS id, fullName FROM users WHERE $where ORDER BY (uid<0) DESC, $nameOrder");
  } elseif ($mya->cAdmin2) {
    $sql = <<<EOF
SELECT DISTINCT id, fullName FROM
  (SELECT uid AS id, fullName FROM users WHERE community=:ctty UNION $sqlNormal) w
ORDER BY (id<0) DESC, $nameOrder
EOF;
    $q = db\q($sql, ray('ctty aid', $mya->community, $aid));
  } else $q = db\q("$sqlNormal ORDER BY $nameOrder", compact('aid')); // the usual case

  return $result + $q->fetchAllKeyed();
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
 * Create a new bank transfer, including records in r_usd, txs_hdrs, and txs_entries (2).
 * @param acct $a: the account to or from which money is to be moved
 * @param numeric $amount: the amount to transfer
 * @param int $created: the UNIX time at which the transfer is deemed to be created.
 * @param string $txid: the internal transaction id for the transfer; normally NULL (assign a new id) but
 *                      can be specified (used during testing)
 * @param int $xid (RETURNED): the transaction ID associated with the transfer request
 * @return string $txid: the actual txid or FALSE on failure
 */
function createUsdTransfer($a, $amount, $created=NULL, $txid=NULL, &$xid = NULL) {
  u\EXPECT(compact(ray('a amount created txid')), 'acct int|float int|empty string|int|empty');
  global $channel;
  $automatic = ($channel == TX_CRON);
  $payee = $a->id;
  u\setDft($created, now());
  $bankAccount = nni($a->vsecure, 'bankAccount', NULL); // was   isDEV ? 'xxxxy' :
  $bankId = $amount >= 0 ? BANK_IN_ID : BANK_OUT_ID;

  $tx = new r\Tx(ray('goods flags actorId actorAgentId channel created', FOR_USD, 0, $payee, $payee, $channel, NULL));
  txEntryPair($txes, 0, [$bankId, $payee], $amount > 0 ? t('from bank') : t('to bank'), E_BANK); // tentative amount is 0
  $tx->addEntries($txes);
  
  $DBTX = \db_transaction();
  $xid = $tx->insert();
    
  $info = compact(ray('amount payee created bankAccount channel xid'));
  if (test() and nn($txid)) $info += compact('txid');
  if ($xid) $txid = db\insert('r_usd', $info, 'txid'); else $txid = FALSE;
  
  if (!$xid or !$txid) {
    $DBTX->rollback();
    r\tellAdmin(t('bank transfer insert failed'), compact($amount, $completed));
    return FALSE;
  }
  unset($DBTX);
  
  if ($amount < 0) { // transfer TO the bank (always completed immediately)
    if (!completeUsdTx($txid, $automatic, $created)) return FALSE;
  }
  return $txid;
}

/**
 * Return information about a same-direction request to combine with.
 * @param acct $a: the account
 * @param numeric $amount: amount requested
 * @param int $completed: desired completion date (NULL if none)
 * @return [$txid, $amount, $completed] where
 *    $txid is the transaction ID of transaction to modify (FALSE if none)
 *    $amount is the modified (i.e., new amount) -- unchanged if $txid is FALSE
 *    $completed is the completion date of the new or modified request (0 if not yet) -- unchanged if $txid is FALSE
 *//*
function combineUsdTx($a, $amount, $completed) {
  return [FALSE, $amount, $completed]; // disable this for now (needs rethinking)
  
  u\EXPECT([$a, $amount, $completed], 'acct int|float int');
///     debug("combineUsdTx($a->id, $amount, $completed)");

  if (!$a->bankOk) return [FALSE, $amount, $completed];

  $DBTX = \db_transaction();
  if ($amount > 0 and $amount + $a->advances() - $a->balance > -$a->floor) $completed = 0; // only 0 if bringing in beyond credit line
  $where = 'payee=:id AND amount>0 AND NOT deposit AND NOT completed';
  if ($res = db\get('txid, amount AS oldAmount', 'r_usd', $where, ray('id', $a->id))) {
    extract($res);
    $amount += $oldAmount;
  } else {
    $txid = FALSE;
  }
  $result = db\update('r_usd', compact(ray('txid amount completed')), 'txid');
  if (!$result) $txid = FALSE;
  unset($DBTX);
  
  return [$txid, $amount, $completed];
}*/


/**
 * Mark a USD transfer complete, update the amounts in the corresponding transaction entries, and report.
 * @param int $txid: the transfer record ID
 * @param bool $automatic: <transfer was created automatically>
 * @param datetime $completed: when the transaction was completed, defaults to the current time
 * @param datetime $deposit: when the request was sent to the bank (for tests only)
 * @return bool <success>
 */
function completeUsdTx($txid, $automatic, $completed = NULL, $deposit = 0) {
  u\EXPECT([$txid, $automatic], 'int|string|empty bool');
  u\setDft($completed, now());
  
  $info = db\get('d.xid,d.payee,d.amount,eid', 'r_usd d JOIN txs t USING(xid)', compact('txid'));
  u\EXPECT((bool) $info, 'bad txid in completeUsdTx');
  extract($info);

  $DBTX = \db_transaction();
  $res = db\update('r_usd', compact(ray('txid completed')), 'txid');
  $res0 = db\update('tx_hdrs', ray('xid created', $xid, $completed), 'xid');
  $res1 = db\update('tx_entries', ray('id amount', -$eid, -$amount), 'id');
  $res2 = db\update('tx_entries', ray('id amount', $eid, $amount), 'id');
  if (!($res and $res0 and $res1 and $res2)) {
    r\tellAdmin(t('usd update failed'), compact($txid, $amount, $automatic));
    $DBTX->rollback();
    return FALSE;
  }
  unset($DBTX);
  
  if ($amount > 0) {  
    $transfer = $automatic ? t('automatic transfer') : t('transfer');
    $amount = u\fmtAmt($amount);
    r\notify($payee, 'transfer-in complete', compact('transfer', 'amount'));
  }
  return TRUE;
}

/**
 * Check the sanity of the database.
 * This currently only checks the tables added or modified in the transaction reformatting.
 */
function checkSanity() {
  // Check transactions and their associated entries
  $xids = db\rows('xid', 'tx_hdrs');
  foreach ($xids as $xidO) {
    $xid = $xidO->xid;
    $tx = r\Tx::read($xid);
    foreignKeyCheck('Tx', $xid, 'actorId', 'users', 'uid', $tx->actorId);
    foreignKeyCheck('Tx', $xid, 'actorAgentId', 'users', 'uid', $tx->actorAgentId);
    foreignKeyCheck('Tx', $xid, 'box', 'r_boxes', 'id', $tx->boxId);
    foreignKeyCheck('Tx', $xid, 'reversesXid', 'tx_hdrs', 'xid', $tx->reversesXid);
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

  // Check all entries
  $entryIds = db\rows('id', 'tx_entries');
  $total = 0;
  foreach ($entryIds as $entryIdO) {
    $entryId = $entryIdO->id;
    $entry = r\TxEntry::read($entryId);
    foreignKeyCheck('TxEntry', $entryId, 'xid', 'tx_hdrs', 'xid', $entry->xid);
    foreignKeyCheck('TxEntry', $entryId, 'uid', 'users', 'uid', $entry->uid);
    foreignKeyCheck('TxEntry', $entryId, 'agentUid', 'users', 'uid', $entry->agentUid);
    if ($entry->entryType > count(ray(E_TYPES))) sanityErr("TxEntry $entryId entryType is invalid: $entry->entryType");

    if (!empty($entry->rule)) {
      foreignKeyCheck('TxEntry', $entryId, 'rule', 'tx_rules', 'id', $entry->rule);
    }
    
    $total += $entry->amount;
  }
// odd: this fails but the individual check above succeeds!  if ($total != 0) { sanityErr("Total of all tx_entries is not 0"); }

  // Check r_usd
  $txids = db\rows('txid', 'r_usd');
  foreach ($txids as $txidO) {
    $txid = $txidO->txid;
    $info = db\get('*', 'r_usd', compact('txid'));
    if ($info['xid']) {
      $amount = $info['completed'] ? $info['amount'] : 0;
      if ((!db\exists('tx_entries', ['xid'=>$info['xid'], 'uid'=>$info['payee'], 'amount'=>$amount]))
        or (!db\exists('tx_hdrs', ['xid'=>$info['xid']]))) { // tx date need not match usd dates
        sanityErr("r_usd $txid has an xid, but has no corresponding transaction or the transaction doesn't match");
      }
      if (!$info['amount']) sanityErr("r_usd $txid has an associated transaction, but a zero amount");
    } elseif ($info['amount'] + 0) sanityErr("r_usd $txid has an amount but has no associated tx_hdr");
  }

  // Check r_usd2
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

  // Check tx_disputes
  $ids = db\rows('id', 'tx_disputes');
  foreach ($ids as $idO) {
    $id = $idO->id;
    $info = db\get('*', 'tx_disputes', ['id'=>$id]);
    foreignKeyCheck('tx_disputes', $id, 'xid', 'tx_hdrs', 'xid', $info['xid']);
    foreignKeyCheck('tx_disputes', $id, 'uid', 'users', 'uid', $info['uid']);
    if (!in_array($info['status'], [DS_OPEN, DS_ACCEPTED, DS_DENIED])) {
      $status = $info['status'];
      sanityErr("tx_disputes $id status field is invalid: $status");
    }
  }

/*   // Check r_invoices */
/*   $nvids = db\rows('nvid', 'r_invoices'); */
/*   foreach ($nvids as $nvidO) { */
/*     $nvid = $nvidO->nvid; */
/*     $info = db\get('*', 'r_invoices', ['nvid'=>$nvid]); */
/*     if ($info['status'] > 0) { */
/*       foreignKeyCheck('r_invoices', $nvid, 'status', 'tx_hdrs', 'xid', $info['status']); */
/*       $ */

}

function foreignKeyCheck($prefix, $ref, $field, $table, $keyFld, $keyVal) {
  if (!empty($keyVal) and !db\exists($table, [$keyFld=>$keyVal])) {
    sanityErr("$prefix $ref $field refers to non-existent $table $keyVal");
  }
}

function sanityErr($s) {w\say("Sanity error: " . $s, 'err');}

