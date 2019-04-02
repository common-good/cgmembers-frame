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

use CG\Testing as t;

define('REAL', @$_SERVER['SystemRoot'] != 'C:\\Windows'); // used?
// require_once __DIR__ . '/cg-util.inc';

/**
 * Generates an array of transactions, with added data.
 * @param mixed $uids: the uid or uids whose transactions we're going to process
 *        if $uids is an array it is the set of uids;
 *        if $uids is an integer it is the only uid
 * @param integer $starting: the starting unix time
 * @param integer $ending: the ending unix time
 * @return an array of transactions each element of which is a single entry associated with the transaction.  Note that tx_hdrs data is in each of the entries.  If there's a failure it will return FALSE.
 */
function getTxs($uids, $starting=null, $ending=null) {
  if (is_integer($uids)) { $uids = [ $uids ]; }

  $where = '';
  if (!is_null($starting)) { $where .= " AND (t.created >= '$starting')"; }
  if (!is_null($ending)) { $where .= " AND (t.created <= '$ending')"; }

  $sql = <<< EOF
    SELECT t.*, e.*, u.fullName as name,
           (SELECT MAX(acctTid) FROM r_entries re WHERE xid = t.xid AND uid in (:uids)) AS tid
    FROM r_entries e LEFT JOIN r_tx_hdrs t USING (xid) LEFT JOIN users u USING (uid)
    WHERE (xid IN (SELECT DISTINCT xid FROM r_entries WHERE (uid IN (:uids))))
          $where
    ORDER BY created, e.xid, (uid BETWEEN 1 AND 512) ASC, name
EOF;
  $results = db\q($sql, ['uids' => $uids]);
  u\EXPECT($results, "Failure reading transactions for user " . print_r($uids, true));

  $lastXid = 0;  // no xid should ever be 0
  $txs = [];  // an array of arrays of entries
  $entries = [];
  foreach ($results as $result) {
    $entry = (array)$result;
    if ($entry['xid'] != $lastXid) {  // start of new transaction
      if ($entries != []) {  // if there was a transaction already
        $txs[] = $entries;
        $entries = [];
      }
      $lastXid = $entry['xid'];
    }
    $entries[] = $entry;
  }
  if ($entries != []) {
    $txs[] = $entries;
  }
  // arrange for running balance to appear in, and be the same in, every entry for a transaction
  $myBalance = balanceAsOf($uids, $starting);
  foreach ($txs as $i => $entries) {
    foreach ($entries as $entry) {
      if (in_array($entry['uid'], $uids)) $myBalance += $entry['amount'];
    }
    foreach ($entries as $j => $entry) {
      $entry['runBalance'] = $myBalance;
      $entries[$j] = $entry;
    }
    $txs[$i] = $entries;
  }
  return $txs;
}

/**
 * Generates an array of transactions between two accounts, with added data, for the given period.
 * @param array $aUids: the uids for member a
 * @param array $bUids: the uids for member b
 * @param unixtime $starting: the starting date/time for the period
 * @param unixtime $ending: the ending date/time for the period, e.g., $starting plus one day
 */
function getCustTxs($aUids, $bUids, $starting, $ending) {
  debug("getCustTxs(" . print_r($aUids, true) . ", " . print_r($bUids, true) . ", $starting, $ending)");
                                                              
  u\EXPECT('array array int int', $aUids, $bUids, $starting, $ending);
  $sql = <<< X
    SELECT CONCAT('tx #', aEntry.acctTid) AS tid,
           -aEntry.amount AS paid,
           '' AS invoiced, t.created,
           (d.status=:DS_OPEN) AS disputed, aEntry.description AS purpose 
    FROM r_entries aEntry JOIN r_tx_hdrs t USING(xid) JOIN r_entries bEntry USING(xid)
         LEFT JOIN r_disputes d USING(xid)
    WHERE aEntry.uid IN (:aUids) AND bEntry.uid IN (:bUids)
    UNION ALL
    SELECT CONCAT('inv #', nvid) AS tid, '' AS paid, amount AS invoiced,
           inv.created AS created, :IS_DISPUTED AS disputed, inv.purpose AS purpose
    FROM r_invoices inv
    WHERE (payer IN (:aUids) AND payee IN (:bUids)) OR (payee IN (:aUids) AND payer IN (:bUids))
    ORDER BY created
X;

  $results = db\q($sql, ['aUids' => $aUids, 'bUids' => $bUids]);
  u\EXPECT($results, "Failure reading transactions between users " . print_r($aUids, true) . " and " . print_r($bUids, true));

  $lastTid = null;  // no tid should ever be null
  $txs = [];  // an array of arrays of entries
  $entries = [];
  foreach ($results as $result) {
    $entry = (array)$result;
    debug($entry);
    if ($entry['tid'] != $lastTid) {  // start of new transaction
      if ($entries != []) {  // if there was a transaction already
        $txs[] = $entries;
        $entries = [];
      }
      $lastTid = $entry['tid'];
    }
    $entries[] = $entry;
  }
  if ($entries != []) {
    $txs[] = $entries;
  }
  // arrange for running balance to appear in, and be the same in, every entry for a transaction
  $myBalance = balanceAsOf($aUids[0], $starting);
  foreach ($txs as $i => $entries) {
    foreach ($entries as $entry) {
      if (in_array($entry['uid'], $aUids)) $myBalance += $entry['amount'];
    }
    foreach ($entries as $j => $entry) {
      $entry['runBalance'] = $myBalance;
      $entries[$j] = $entry;
    }
    $txs[$i] = $entries;
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
function processTxs($processor, $uids, $starting=null, $ending=null, $descending=FALSE) {
  $txs = getTxs($uids, $starting, $ending);

  if ($descending) { $txs = array_reverse($txs); }
  
  // Now process the transactions
  $processor->hdr();
  foreach ($txs as $entries) {
    $processor->txnHdr($entries[0]);
    foreach ($entries as $entry) {
      $processor->processEntry($entry);
    }
    $processor->txnFtr($entries[0]);
  }
  $processor->ftr();
}

/**
 * Returns the balance in an account as of a specified date.
 * @param integer $uid: the user id of the account
 * @param integer $date: the unix time to consider
 */
function balanceAsOf($uids, $date=null) {
  if (is_null($date)) return 0;
  if (is_integer($uids)) {
    $a = r\acct($uids);
    ($a->jid != 0) ? $uids = [$uids, $a->jid] : [$uids];
  }
  return db\sum('amount', 'r_entries JOIN r_tx_hdrs USING(xid)', 'uid IN (:uids) AND created<:date', ['uids' => $uids, 'date' => $date]);
}
  
/**
 * Return an array of member names (called with ajax, so current user is passed as arguments).
 * Restricted to within NEIGHBOR_RADIUS miles of the member
 * @param int $myid: account record ID
 * @param int $aid: agent account record ID
 */
function memberRay($myid, $aid, $coOnly = FALSE) {
  if (!$a = r\acct($myid, $aid)) return [];
//  $where = $coOnly ? ':IS_CO' : ($a->admin ? '1' : "state=$a->state OR community=$a->community OR :IS_CO");
  $where = $coOnly ? ':IS_CO' : ($a->admin ? '1' : "(:NEIGHBOR_QUERY<:NEIGHBOR_RMAX OR community=$a->community OR :IS_CO)");
  $subs = compact('myid');
  if (!$coOnly and !$a->admin) $subs += ray('lat lon', $a->latitude, $a->longitude);
  return db\col('fullName', 'users', ":IS_OK AND uid<>:myid AND $where ORDER BY fullName", $subs);
}

/**
 * Return the next relevant transaction for form 1099b, for an account.
 * @param int $payee: the account record ID
 * @param int $year: year or end-of-year date
 * @return the next relevant row (NULL if no more)
 * Note that the IRS prohibits reporting non-positive transactions (but requires all the others). So we compromise by weeding out the transactions with "undoneBy" or "undoes" in the data field. But we return other negative transactions so we can warn the member to report the total negative amount on line 2, Form C.
 * Also note that this report is always for just one account (and for just one SSN or EIN) 
 *   even if the account is joint.
 */
function get1099b($payee, $year = NULL) {
  global $q1099, $ignore1099;
  
  if (is_null($q1099)) {
    u\setDft($year, strtotime('last day of December last year'));
    $end = strlen($year) < 6 ? strtotime('1/1/' . ($year + 1)) - 1 : strtotime('tomorrow', $year);
    $start = strtotime('1 year ago', $end) + 1;
    $sql = <<<EOF
      SELECT created, payeeE.amount AS amount, payerE.uid AS payer, CONCAT(payeeE.description, t.type) AS `for`
      FROM r_entries payerE JOIN r_tx_hdrs t USING(xid) JOIN r_entries payeeE USING(xid)
      WHERE payeeE.uid=:payee AND NOT :HAS_REVERSER AND reversesXid IS NULL AND created BETWEEN $start AND $end
             AND payeeE.amount > 0 AND payerE.amount < 0 AND t.type <> :TX_BANK
      ORDER BY created
EOF;
//      AND (goods=:FOR_GOODS OR type<>:TX_TRANSFER)
//      AND (goods OR (type NOT IN (:TX_TRANSFER, :TX_REFUND)))
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
  /* debug("updateRelations(" . print_r($updates, true) . ")"); */
  $myid = r\acct()->id;
  $report = '';
  foreach ($updates as $reid => $data) {
    extract(just('main other fullName permission flags', $data['original']));
    $wasEmpty = (!@$permission and !@$flags);
    unset($data['original']);
    extract(just('permission flags', $data));
//    foreach (ray('employee owner customer draws') as $k) u\setBit($flags, $k, @$data[$k]);
    foreach (ray('employee owner customer draw') as $k) u\setBit($flags, $k, @$data[$k]); // drawS?
    if (!u\getBit($flags, 'customer')) u\setbit($flags, 'autopay', FALSE);

    if (!@$permission and !@$flags and !$wasEmpty) { // empty now and wasn't before
      db\del('r_relations', 'reid', $reid);
      $msg = 'deleted relation';
    } else {
      $a = r\acct($main, $other);
      if (@$data) $a->setRel($data); // db\update('r_relations', $data + $reidSub, 'reid');
      if (@$data['draw']) r\acct($other)->setBit(B_DRAWS);
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
 *     I paid someone (byMe):            REVERSE (invoice)  original was payment
 *     I charged someone (toMe/byMe):    REVERSE (pay back) original was charge
 *     Someone paid me (toMe):           REVERSE (pay back) original was payment
 *     Someone invoiced me and I approved it: DISPUTED      original was invoice-payment
 *     Someone charged me unilaterally: DISPUTED            original was charge
 * @param int $xid: the transaction number
 * @param string $where: additional (security check) criteria, if any
 * @param assoc $subs: additional substitutions for $where
 * @param bool $force: <reverse despite any shortfall or lack of permission>
 * @return array [message, subs] where subs is an assoc of [
 */
function undoTx($xid, $where = '1', $subs = [], $force = FALSE) {
  global $mya;

  /* debug("undoTx($xid, $where, , $force)" . print_r($subs, true)); */
  
  u\EXPECT('int string assoc bool', $xid, $where, $subs, $force);

  $tx = r\Tx::lastTx([$mya->id], "xid=:xid AND $where", compact('xid') + $subs);
  if (empty($tx)) {
    return ['undo no match', $subs];
  }
  if (!$force and $error = $tx->nonoUndo($subs)) return array($error, $subs); // check nono before confirm AND here

  $byMe = $mya->isMe($tx->actorId);
  $toMe = $tx->getAmount($mya) > 0;
  if (!$byMe and !$toMe) { // someone charged me (either unilaterally or I approved it)
    r\disputeTx($xid); // mark it disputed
    $solution = t('marked "disputed"');
    r\notify($tx->them, 'charge disputed', $tx->reportArgs($mya)); //, $tx->getMyAmt(), $payer, $payee, $payerPurpose, $payeePurpose));
    $amount = u\fmtAmt($tx->amount);
    $tofrom = !$tx->toMe ? t('to') : t('from');
    $otherUid = $tx->otherUid;
    $other = r\acct($otherUid)->fullName;
    $success = TRUE;
    return ['report undo', compact(ray('success amount other tofrom solution otherUid'))];
  } else {
    $res = reverse($xid); // create an offsetting transaction
    list($message, $info) = $res;
    $newTx = $info['newTx'];
    unset($info['newTx']);
    
    $message = t($message, $info);
    
    sendNotifications(true, $newTx, 0);
    return [$message, $info];
  }
}

/**
 * Create an offsetting transaction (nearly the same as the original but with the entries negated).
 */
function reverse($xid) {
  global $mya, $channel;

  /* debug("reverse($xid)"); */

  $DBTX = \db_transaction();
  if (db\exists('r_tx_hdrs', 'reversesXid=:xid', ['xid' => $xid])) {
    return r\txRet('already reversed', compact('xid'));
  }
  $tx = r\Tx::read($xid);
  if (is_null($tx)) {
    return r\txRet('no transaction', compact('xid'));
  }

  // It seems OK to reverse it
  $newTx = clone $tx;  // this is a shallow copy, entries points to the same entries
  $newTx->xid = null;
  $newTx->reversesXid = $xid;
  $newTx->entries = [];
  $newTx->created = r\rTime();
  foreach ($tx->entries as $oldEntry) {
    $newEntry = clone $oldEntry;
    if ($newEntry->relType == 'D' and $newEntry->amount > 0) { // related to a coupated record
      if (!db\q('UPDATE r_coupated SET uses=uses-1 WHERE id=:coupatedId AND uid=:uid',
                ['coupatedId' => $newEntry->relatedId, 'uid' => $newEntry->uid])) {
        $DBTX->rollback();
        return r\txRet('tx create err', $entry);
      }
    }
    $newEntry->id = null;
    $newEntry->xid = null;  // new entry belongs to reversing transaction
    $newEntry->amount = 0-$oldEntry->amount;
    $newEntry->description .= ' (' . t('reverses #') . $oldEntry->acctTid . ')';
    $newEntry->acctTid = r\acct($newEntry->uid)->nextTid();
    $newTx->entries[] = $newEntry;
  }
  $xid = $newTx->insert();
  if (!$xid) {
    $DBTX->rollback();
    return r\txRet('tx save err', $newTx);
  }

  $DBTX = null;

  $message = 'report undo|report tx';
  $did = ($tx->actorEntry->amount > 0) ? ($tx->goods==FOR_GOODS ? 'refunded' : 'credited') : 're-charged';
  
  return r\txRet($message, ['ok'=>1, 'txid'=>$xid, 'created'=>$newTx->created,
                            'balance'=>$mya->can(B_SECRET) ? '*0' : $mya->balance,
                            'success'=>true, 'xid'=>$xid, 'created0'=>$newTx->created, 'newTx'=>$newTx,
                            'solution'=>'reversed', 'did'=>$did, 'otherName'=>$tx->other->fullName,
                            'amount'=>u\fmtAmt(abs($newTx->otherEntry->amount)), 'why' => $tx->otherEntry->description]);
}

/**
 * Pay the given invoice, possibly without being signed in.
 * @param int $nvid: record ID of invoice to pay
 * @return [TRUE, explanation] if the payment was successful
 *         [FALSE, explanation] if the payment failed
 */
function payInvoice($nvid) {
  /// debug("payInvoice($nvid)");
  
  u\EXPECT('int', $nvid);
  $msg = r\badUnpaidInv($nvid, $inv);
  if ($msg) return [FALSE, $msg];
  extract(u\exactly('payer payee amount goods purpose data flags', $inv, [null, null, 0.0, FOR_GOODS, null, '', 0]));
  $payerId = $payer;
  $payeeId = $payee;
  $payer = r\acct($payer);
  $payee = r\acct($payee);
  $data = unserialize($data);
  extract(u\exactly('api request shid', $data, null));
  
  $DBTX = \db_transaction();
  $purpose .= ' (' . PROJECT . " inv#$nvid)";
	$extra = ray('inv', $nvid);
	if ($flags) foreach (ray('gift recurs investment') as $k) if (u\getBit($flags, $k)) $extra[$k] = 1;

  list ($index, $subs) = be\transfer('invoice payment', $payer, $payee, $amount, $purpose, $goods, $extra);

  $ok = u\arrayGet($subs, 'success', FALSE);
  if ($ok) {
    db\update('r_invoices', ray('status nvid', $subs['xid'], $nvid), 'nvid');
    if (!empty($api)) u\post($api, ray('request ok msg', $request, (int) $ok, tr($index, $subs)));
  }

	if (u\getBit(@$flags, B_INVESTMENT)) { // club reclaiming an investment
/*
    $res = db\get('shid,vestid', 'r_shares s LEFT JOIN r_investments i ON i.vestid=s.vestid', 'coid=:coid AND clubid=:clubid AND pending<0', ray('coid clubid', $payerId, $payeeId));
    if ($res) {
      extract($res); // shid vestid
      */
    if ($shid) { // record the sale
      db\update('r_shares', ray('shid shares pending when', $shid, -$amount, 0, NOW), 'shid');
    } else r\tellCo(t('Invoice payment marked as "for investment (repayment)" has no corresponding SELL request in shares table.'), $inv, $payeeId);
  }
  unset($DBTX);
  
  $msg = t($index, $subs);
  return [$ok, $msg];
}

/**
 * Create a transaction
 * call by: list ($msg, $args) = be\transfer(...);
 * @param string $type: payment, charge, invoice payment, draw
 * @param acct $actor: acct object for the initiating party (usually the current user)
 * @param acct $other: acct object for the other party
 *   OR 'ALL' (paying employees) -- currently UNUSED
 * @param number $amount: the transaction amount
 * @param string|array $for: description of the transaction's purpose (user input) (or [actorFor, otherFor])
 * @param int $goods: FOR_GOODS, FOR_USD, others?
 * @param array $data: associative array of info about the transaction this request is reversing (if any), 
 *   indexed by any of:
 *   'created': time of transaction (defaults to current time)
 *   'force': <do the transaction despite any shortfall (1 for original tx, -1 for forced reversal)>
 *   'box': the device on which the transaction was initiated
 *   'offline'
 *   'loan'
 *   'investment'
 *   'stake'
 *   'fine'
 *   'noask'
 *   'funding'
 *   'crumbs', 'recurs', 'gift': gift info
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE)
 *
 * The following used to be possible keys for $data, but not any more:
 *   'inv': record number of the invoice for which this is payment
 *   'isGift': UNUSED <the payment is a gift to a community or nonprofit>
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
function transfer($type, $actor, $other, $amount, $for, $goods, $data = []) {
  global $scanned, $channel;
  debug("transfer($type, $actor->id, $other->id, $amount, " . print_r($for, true) . ", $goods, )" . print_r($data, true));
  
  u\EXPECT('string acct acct number string|array int assoc', $type, $actor, $other, $amount, $for, $goods, $data);
  if (is_null($goods)) $goods = FOR_GOODS;
  u\EXPECT(!is_null($goods), "bad goods");
  switch ($type) {
  case 'charge':
  case 'draw':
    $taking = true;
    break;
  case 'payment':
  case 'invoice payment':
    $taking = false;
    break;
  case 'transfer': // until we figure out what it should be
    $taking = $amount < 0;
    break;
  default:
    u\EXPECT(false, "type must not be '$type'.");
  }
  
  extract(u\exactly('created force box', $data, [r\rTime(), 0, null]));
  extract(u\exactly('offline loan investment stake fine noask funding crumbs recurs gift', $data, FALSE));

  $short = FALSE;
  
  if (is_null($for)) $for = ray(R_WHYS)[$goods];
  list($otherFor, $actorFor) = (is_array($for)) ? $for : [$for, $for];

  if (is_array($amount)) extract(just('amount', $amount));
  list ($actorId, $otherId) = array($actor->id, $other->id);

  if ($force != 0) $offline = TRUE; // temporary until app handles this right

  $erInfo = compact(ray('goods amount for force')) + ray('actor other op', $actor->fullName, $other->fullName, $taking ? t('charged') : t('paid'));
  $amount = round($amount, 2); // ignore fractions of cents
  if ($amount == 0) {
/**/  r\tellAdmin('zero transaction', $erInfo + ['stack' => trace()]);
    return r\txRet('zero transaction', $erInfo + ['success' => false]);
  }
  $reallyTaking = ($taking xor $amount < 0); // not taking if charging a negative amount

  list ($loser, $gainer) = u\order($reallyTaking, $other, $actor);
  list ($loserFor, $gainerFor) = u\order($reallyTaking, $otherFor, $actorFor);
  $loserTid = $loser->nextTid();
  $gainerTid = $gainer->nextTid();
  
  $erInfo += ['loserId' => $loser->id];
  list ($payer, $payee) = u\order($taking, $otherId, $actorId); // Analyze who's buying or selling
  list ($payerAgent, $payeeAgent) = u\order($taking, $other->agentId, $actor->agentId);

  $lastTx = $actor->lastTx();
  if (!empty($lastTx) and !$force and $channel != TX_WEB and $lastTx->actorId == $actorId
      and $lastTx->otherId == $otherId and $lastTx->goods == $goods and $lastTx->getAmount($actor) == $amount
      and ((r\rTime() - $lastTx->created) < R_DUPTX_MINS * MIN_SECS)) {
    return r\txRet('duplicate transaction', $erInfo + ['success' => false]);
  }

  if ($err = r\txPermErr($actor, $other, $taking, $amount < 0)) { // Check permissions
    list ($erMsg, $erSubs) = $err;
    if ($force) {
      $msg = 'forced without perm';
      $erInfo += ['date' => u\fmtDate($created)];
      foreach (['agent1'=>$actor, 'agent2'=>$other] as $k => $a) $erInfo += [$k => $a->proSe ? t('self') : $a->fullName];
      foreach ([$actor, $other] as $a) if (!$a->proSe) r\message($a->id, $msg, $erInfo);
      r\tellAdmin($msg . ' ' . t($erMsg, $erSubs), $erInfo + $erSubs);
    } else return r\txRet($erMsg, $erInfo + $erSubs + ['success'=>false]);
  }

  foreach ([$actor, $other] as $a) if (!$a->co and !($channel == TX_POS ? $a->agentA->ided : $a->ided)) {
    if ($force) {
      r\tellAdmin('forced without photoId', $erInfo); 
    } elseif ($gainer->id != CGID) { // and !$gainer->coCan(CO_FAST)
//      r\tellAdmin('tx without photoId', $erInfo); 
//      return r\txRet($a == $actor ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $other->fullName]);
    }
  }
  // Check the amount
  if ($error = u\badAmount($amount)) return r\txRet($error, $erInfo+['success'=>false]);
//  $need = $amount - @$payerReward; // leave room for reversing rewards (minus a negative)

  $gainerType = ($gainer->isMe($payer)) ? ENTRY_PAYER : ENTRY_PAYEE;
  $loserType = ($gainer->isMe($payer)) ? ENTRY_PAYEE : ENTRY_PAYER;
  $DBTX = db_transaction();
  // Create the transaction
  $flags = ($offline ? 1 << B_OFFLINE : 0) | ($short ? 1 << B_SHORT : 0) | ($crumbs ? 1 << B_CRUMBS : 0)
    | ($recurs ? 1 << B_RECURS : 0) | ($gift ? 1 << B_GIFT : 0) | ($loan ? 1 << B_LOAN : 0)
    | ($investment ? 1 << B_INVESTMENT : 0) | ($stake ? 1 << B_STAKE : 0) | ($fine ? 1 << B_FINE : 0)
    | ($noask ? 1 << B_NOASK : 0) | ($funding ? 1 << B_FUNDING : 0);
  $info = ray('actorId actorAgentId flags channel box goods created',
              $actor->id, $actor->agentId, $flags, $channel, $box, $goods, $created); //amount created payer payee payerAgent payeeAgent for data taking roundup goods force offline investment short'));
  /* print_r($info); */
  $tx = new r\Tx($info);
  if (!$tx) {
    $DBTX->rollback();
    return r\txRet('tx create err', $erInfo+['success'=>false]);
  }
  $entries = [new r\TxEntry(['entryType' => $gainerType, 'amount' => abs($amount), 'uid' => $gainer->id, 'agentUid' => $gainer->agentId, 'description' => $gainerFor, 'acctTid' => $gainerTid]),
              new r\TxEntry(['entryType' => $loserType, 'amount' => 0-abs($amount), 'uid' => $loser->id, 'agentUid' => $loser->agentId, 'description' => $loserFor, 'acctTid' => $loserTid])];

  // Apply coupon
  if ($amount > 0 AND ($goods != TX_USD)) {
    list($rebate, $couponEntries) = handleCoupon($loser, $gainer, $amount, $loserTid, $gainerTid);
    $entries = array_merge($entries, $couponEntries);
  } else {
    $rebate = 0;
  }
  
  // Calculate any balance shortfall
  $needed = abs($amount)-$rebate;
  if ($shortfall = $loser->id < 0 ? 0 : $loser->shortfall($goods, $needed, TRUE, $actor)) {
    if ($force) {
      $short = TRUE; // flag overdraft
    } else {
      $short = u\fmtAmt($shortfall);
      $avail = u\fmtAmt($needed - $shortfall);
      $otherName = $other->fullName;
      $msg = $reallyTaking ? ($other->can(B_SECRET) ? 'short from vague' : 'short from') : 'short to';
      if ($channel == TX_WEB) {
// too complex        if ($msg == 'short to' and $channel == TX_WEB and r\signedIn()) $msg .= '|increase min';
/*        if ($goods == FOR_GOODS) {
          if (!$loser->can(B_DEBT) and $loser->floor < 0 and $channel == TX_WEB) $msg .= '|try debt';
          $_aDebt = w\atag('/settings/preferences');
        } //elseif ($actor->cttyRewardy) $msg .= '|short cash help|maybe not cash';
				*/
      }
      $loser->suggestAuto();
      $details = t('|%loser was short by %short trying to pay %gainer %amount (available balance is %avail)', 'loser short gainer amount avail', $loser->fullName, $short, $gainer->fullName, u\fmtAmt($needed), $avail);
      $DBTX->rollback();
      return r\txRet($msg, compact(ray('otherName short details avail')) + ['success'=>false]);
    }
  }

  // Apply any roundup
  $roundup = (r\acct($payer)->roundup and $needed > 0 and !($payee == CGID and $recurs and $needed < 1)); // payer roundups, not actor or loser (don't round up reversals, cashouts, roundups, or recurring gifts under $1)
  if ($roundup) { // do some stuff
    $cents = fmod($needed, 1.0);
    if ($cents != 0) {
      $donation = 1 - $cents;
      $entries[] = new r\TxEntry(['amount' => $donation, 'uid' => CG_ROUNDUPS_UID, 'agentUid' => CG_ROUNDUPS_UID, 'description' => t('roundup donation'), 'entryType' => ENTRY_DONATION]);
      $entries[] = new r\TxEntry(['amount' => 0-$donation, 'uid' => $loser->id, 'agentUid' => $loser->agentId, 'description' => t('roundup donation (on %tid)', ['tid' => $loser->nextTid]), 'entryType' => ENTRY_DONATION]);
    }
  }
  $rbal = r\acct($payer)->balance;
  $ebal = r\acct($payee)->balance;

  $tx->addEntries($entries);
  if (array_key_exists('xid', $data)) {
    $tx->xid = $data['xid'];
    $xid = $tx->insert(TRUE);
  } else {
    $xid = $tx->insert();
  }
  if (!$xid) {
    $DBTX->rollback();
    return r\txRet('tx save error', ['success'=>false]);
  }
  unset($DBTX);
  
  // Report results
  // notify other party by email about any completed transaction
  foreach ([$actor, $other] as $a) if (!$a->confirmed) $a->setBit(B_CONFIRMED);

  sendNotifications(false, $tx, $shortfall);

/* //  if (isPRODUCTION) r\tellAdmin('tx', compact('msg') + $args); // a backup record of all txs (until the backup server works) */
  $msg = 'report tx';
  $args = $tx->reportArgs($actor, TRUE, $shortfall);
  $retVal = r\txRet($msg, $args + compact('msg'));
  return $retVal;
}

// $reversing=1, actor is 553, $tx = $80 from 553 to 551
// $byme = true; $tome = false
function sendNotifications($reversing, $tx, $shortfall) {

  ///  debug("sendNotifications($reversing, " . print_r($tx,true) . ", $shortfall");
  
  global $mya;
  $byMe = true; //$mya->isMe($tx->actorId);
  $toMe = $tx->actorAmount > 0;
  if ($reversing) {
    /*if ($byMe and !$toMe and $wasInvoice) $original = 'invoice-payment';
      else*/
    if ($byMe and !$toMe) {
      $notice = ($tx->goods == FOR_GOODS) ? 'new refund' : 'new payment';
    } // I made a payment, reversing requires invoice
    elseif ($byMe and $toMe) $notice = 'new charge';  // I charged other, reversing is refunding
    elseif (!$byMe and $toMe) $notice = 'payment';  // Other made payment to me, reverse is refunding
    else /*!byMe and !$toMe*/ $notice = 'dispute';  // Other charged me, reversing requires dispute
  } else {
    /*if ($byMe and !$toMe and $wasInvoice) $original = 'invoice-payment';
      else*/
    if ($byMe and !$toMe) {
      if ($tx->goods == FOR_GOODS) $notice = 'new refund';  // I made a payment to other
      else $notice = 'new payment';  // I made a payment to other
    }
    elseif ($byMe and $toMe) $notice = 'new charge';  // I charged other
    elseif (!$byMe and $toMe) $notice = 'new payment';  // Other made a payment to me
    else /*!byMe and !$toMe*/ $notice = 'new charge'; // Other charged me
  }
  if ($notice == 'charge' and $tx->goods == FOR_USD) $notice = 'cash out';

  $other = $tx->other;
  $args = $tx->reportArgs($tx->actor, TRUE, $shortfall);
  if ($notice == 'new payment' and $other->co) { // not "new charge" (the most common); payment at payer's initiative
    $notice = 'new payment linked';
    $args['_aPayLink'] = $other->makeDo('addr', $tx->actorId, 0); // link to address, so merchant can ship or nonprofit can thank
  }
  r\notify($tx->otherId, $notice, $args);
  $msg = 'report tx';
}

/**
 * Is the given account id a bank account?
 * @param int $uid;
 * @return boolean
 */
function isBankUid($uid) {
  return $uid == CG_INCOMING_BANK_UID or $uid == CG_OUTGOING_BANK_UID;
}
    

/**
 * Generate transaction entries for a coupon, and update the coupated table as appropriate.
 * @param acct $buyer:
 * @param acct $seller:
 * @param number $price: pre-discount amount
 * @param string|number $buyerTid: buyer's transaction id
 * @param string|number $sellerTid: seller's transaction id
 * @param unixtime $when: when was the transaction (defaults to now)
 * @return [ rebate, entries ] where entries is an array of entries
 */
function handleCoupon($buyer, $seller, $price, $buyerTid, $sellerTid, $when=null) {
  /// debug("handleCoupon($buyer->id, $seller->id, $price, $buyerTid, $sellerTid, $when)");
  if (is_null($when)) $when = r\rTime();
  $sql = <<<EOF
    SELECT `on`, c.coupid, amount, ulimit, id, uses
    FROM r_coupons c LEFT JOIN r_coupated d ON (c.coupid=d.coupid AND d.uid=:buyerId)
    WHERE fromId=:sellerId AND :now BETWEEN start AND end AND :price>=minimum AND NOT flags
          AND (ulimit=0 OR uses IS NULL OR uses<ulimit)
    ORDER BY end ASC
EOF;
  $subs = ray('buyerId sellerId now price', $buyer->id, $seller->id, $when, $price);
  $res = db\q($sql, $subs);
  if (empty($res)) return [0, []];
  $res = $res->fetchAssoc();
  if (empty($res)) return [0, []];
  extract($res);
  $rebate = ($amount < 0) ? $price * (0-$amount) * .01 : $amount;
  if ($rebate <= 0) return [0, []];
  if (empty($id)) {
    $id = db\insert('r_coupated', ['coupid' => $coupid, 'uid' => $buyer->id, 'uses' => 1, 'when' => time()], 'id');
  } else {
    db\update('r_coupated', ['id' => $id, 'coupid' => $coupid, 'uid' => $buyer->id, 'uses' => $uses + 1,
                             'when' => r\rTime()], 'id');
  }
  return [ $rebate,
           [ new r\TxEntry(ray('amount entryType uid agentUid description acctTid relType relatedId',
                               $rebate, ENTRY_OTHER, $buyer->id, $buyer->agentId,
                               t('discount rebate (on #%tid)', ['tid' => $buyerTid]),
                               $buyerTid, 'D', $id)),
             new r\TxEntry(ray('amount entryType uid agentUid description acctTid relType relatedId',
                               0-$rebate, ENTRY_OTHER, $seller->id, $seller->agentId,
                               t('discount rebate (on #%tid)', ['tid' => $sellerTid]),
                               $sellerTid, 'D', $id)) ] ];
}

/**
 * Check whether the payer has a shortfall in paying for this transaction.
 * @param int $goods: the type of goods (treat USD transactions differently
 * @param acct $payer: account of the payer (we're checking this account for shortfall)
 * @param acct $payee: account of the payee
 * @param acct $actor: account that initiated transaction, probably the same as $payer or $payee
 * @param acct $other: the other account, probably the same as $payee or $payer
 * @param acct $needed: amount needed to pay, should be positive; (e.g., price less rebates)
 * @return FALSE if there is no shortfall, otherwise a message (created by txRet)
 */
function checkForShortfall($goods, $payer, $payee, $actor, $other, $needed, $force = FALSE) {
  /// debug("checkForShortfall($goods, $payer->id, $payee->id, $actor->id, $other->id, $needed, $force)");
  if ($payer->uid < 0) return FALSE;  // it's a community; they don't have shortfalls?
  
  $shortfall = $payer->shortfall($goods, $needed, TRUE, $payee);
  if ($shortfall == 0) return FALSE;
  
  $msg = ($actor == $payer) ? ($payee->can(B_SECRET) ? 'short from vague' : 'short from') : 'short to';
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
  return r\txRet($msg, ray('otherName short details avail',
                           $other->fullName, u\fmtAmt($shortfall), $details, u\fmtAmt($needed-$shortfall)));
}
  
/**
 * Find the maximum tid used by any of the accounts in the argument.
 * @param array uids: the uids of the accounts to check.
 * @return the maximum tid used.
 */
function maxTid($uids) {
  if (!is_array($uids)) {
    $a = r\acct($uids);
    $uids = $a->jid != 0 ? [$uids, $a->jid] : [$uids];
  }
  u\EXPECT(is_array($uids), 'argument should be an array');
  return db\max('acctTid', 'r_entries', 'uid IN (:uids)', ['uids' => $uids]);
}

/**
 * Create an invoice
 * call by: list ($msg, $args) = be\invoice(...);
 * @param acct $actor: acct object for payee (usually current user)
 * @param acct $other: acct object for payer
 * @param numeric $amount: the invoiced amount
 * @param string $purpose: description of the transaction's purpose (user input)
 * @param assoc $extra:
 *    goods: what we're trading in, defaults to FOR_GOODS
 *    shid: record id of related record in r_shares
 *    status: ?
 *    created: UNIXtime
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE)
 */
function invoice($actor, $other, $amount, $purpose, $goods, $extra = []) {
  ///  debug("invoice($actor->id, $other->id, " . print_r($amount, true) . ", " . print_r($purpose, true) . ", " . print_r($goods, true) . ", " . print_r($extra, true) . ")");
  global $channel;
  
  u\EXPECT(compact(ray('actor other amount purpose')), 'acct acct float string');
  $ks = 'status';
  extract(just($ks, $extra));
  extract(u\exactly('goods created', $extra, [FOR_GOODS, r\rTime()]));
	foreach (justNOT("status shid coupon coupid force inv api request created", $extra) as $k => $v) {
    u\setBit($flags, $k, $v);
    unset($extra[$k]); // don't leave bits in extras (data field)
  }
  list ($payeeId, $payerId) = array($actor->id, $other->id);
  u\setDft($purpose, $why = ray(R_WHYS)[$goods]);
  $subs = ray('myName otherName otherEmail otherPhone amount why purpose did op', $actor->fullName, $other->fullName, $other->email, u\fmtPhone($other->phone), u\fmtAmt($amount), $why, $purpose, t('charged'), t('charged'));
  if ($err = u\badAmount($amount, '>0')) return r\txRet($err, $subs);
  if ($err = r\txPermErr($actor, $other, TRUE)) return r\txRet($err, $subs);
  if (r\dupTx($payeeId, $payerId, -$amount)) return r\txRet('duplicate transaction', $subs);
  
  u\setDft($status, ($channel != TX_FOREIGN and r\relation(':IS_AUTOPAY', $payeeId, $payerId)) ? TX_APPROVED : TX_PENDING);
  $data = serialize($extra);
  $info = compact(ray('amount status purpose created flags data')) + ['payer'=>$payerId, 'payee'=>$payeeId];
  $nvid = db\insert('r_invoices', $info, 'nvid');
  /* $actor->update('lastTx', [$nvid, $payerId - $payeeId, round(-$amount, 2), $created, 'inv']); */

  if ($status == TX_PENDING) {
    $link = $actor->makeDo('inv', $nvid); // link to pay/deny
    //$other->update(ray('lastTx', array($nvid, $payeeId - $payerId, round($amount, 2), $created, 'inv')));
    $subs += ray('_a1 reply email phone', $link, $payeeId, $actor->email, u\fmtPhone($actor->phone));
    r\message($payerId, 'new invoice', $subs); // tell other party by email about this invoice
  } else { // pre-approved!
    if (!$other->shortfall(TX_TRANSFER, $amount)) { // don't try to pay yet if short
      list($ok, $msg) = be\payInvoice($nvid);
      if (!$ok) r\tellAdmin(t('invoice payment failed'), compact('msg') + $subs + $info);
    }
  }

  $msg = 'report tx';
//  if ($actor->cttyRewardy) $msg .= '|for why';
  if ($amount > 0) $msg .= '|balance unchanged';
  return array($msg, $subs + ['success'=>TRUE]);
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
      $accountName = accountName($old_uid);
//      if (!isTempAccount($old_uid)) return array('cell taken', compact(ray('number accountName')));
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
  db\q('DELETE FROM r_boxes WHERE code=:number LIMIT 1', compact('number'));
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
  
  if (!$short = u\shortName(" $who ", '%')) $short = '-'; // abbreviations must not be null or zeroes
  $cmail = strpos($who, '@') ? u\cry('P', strtolower($who)) : '-';
  $subs = compact(ray('who short cphone cmail'));

  $where = '(name LIKE :short OR :who=fullName OR (!:IS_NOSEARCH AND (:cphone=phone OR :cmail=email)))';
  if ($paying) $where .= ' AND :IS_OK';
  if (!$mya->admin) {
    $where .= " AND (:IS_CO OR :NEIGHBOR_QUERY<:NEIGHBOR_RMAX)"; // restrict to nearby
    list ($lat, $lon) = $mya ? [$mya->latitude ?: 0, $mya->longitude ?: 0] : [0, 0]; // avoid NULL (PHP bug?)
    $subs += compact(ray('lat lon'));
  }
  
  $sql = <<<EOF
    SELECT DISTINCT uid,fullName
    FROM users 
    WHERE $where
    ORDER BY fullName
EOF;

  $result = db\q($sql, $subs)->fetchAllKeyed(0, 1);
  
  // QID? if not, maybe first word is a QID if this is from a list showing QID and fullName
  $qid = trim(($i = strpos($who, ':')) ? substr($who, 0, $i) : strtoupper($who));
  if ($a = r\acct($qid)) $result[$a->id] = $a->fullName; // explicit QID is a result, regardless of community
  if (count($result) == 1 and @$result[$myid]) return [$self_message, [], []];
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
 * @param integer asof: last time to include (defaults to 24 hours from now, meaning unrestricted) this is exclusive
 * @return an assoc of results (see $resultKeys, below)
 */
function creditInfo($uids, $asof = null) {
  ///  debug("creditInfo(" . print_r($uids, true) . ", $asof)");
  u\EXPECT(is_array($uids), 'bad call to creditInfo');
  global $mya;
  
  $asof = empty($asof) ? (r\rTime() + DAY_SECS) : $asof;

  /* $mday1 = u\monthDay1($asof); // start of relevant month, for partial (not-yet-given) roundups calculation */
  /* if ($asof == u\plusMonths(1, $mday1) - 1) $mday1 = $asof + 1; // for asof the end of month, there are no partRounds */
  $subs = compact(ray('uids asof'));
  $isPayer = '(t.payer IN (:uids) AND e.entryType=:ENTRY_PAYER)'; 
  $isPayee = '(t.payee IN (:uids) AND e.entryType=:ENTRY_PAYEE)'; // account can be both payer and payee, if joint account
  /* $signedAmt = "IF($isPayer, -amount, amount)"; */
//    SUM(IF(t.type IN (:TX_REWARDS), amount, 0)) AS rewards,
  
    /* select sum(amount) AS txAmount, sum(if(amount>0 and txid IS NOT NULL, amount, 0)) AS bankAmount */
    /* from r_tx_hdrs t join r_entries e using(xid) left outer join r_usd d using(xid) */
    /* where uid in (:uids) and t.created<:asof GROUP BY xid; */
  
  $sql = <<<EOF
    SELECT SUM(IF(txAmount>0, txAmount, 0)) AS `in`,
           SUM(IF(txAmount<0, 0-txAmount, 0)) AS `out`,
           SUM(IF(bankAmount>0, bankAmount, 0)) AS `fromBank`,
           SUM(IF(bankAmount<0, 0-bankAmount, 0)) AS `toBank`
    FROM ( SELECT xid, SUM(e.amount) AS txAmount,
                  SUM(IF(d.txid IS NOT NULL, e.amount, 0)) AS bankAmount
           FROM r_tx_hdrs t JOIN r_entries e USING(xid) LEFT OUTER JOIN r_usd d USING(xid)
           WHERE uid IN (:uids) and t.created<:asof
           GROUP BY t.xid) t
EOF;

  $sums = db\q($sql, $subs)->fetchAssoc();

  $sql = "SELECT SUM(amount) AS pendingBank FROM r_usd t WHERE (payee IN (:uids)) AND created<:asof AND completed <= 0";
  $sums += db\q($sql, $subs)->fetchAssoc();

  foreach ($sums as $k => $v) if (is_null($v)) $sums[$k] = 0; // no nulls
  extract($sums);
  
  $balance = ($in - $out);
  $in = $in - $fromBank;
  $outProper = $out;
  $inProper = $in;
  
  $resultKeys = 'in out refunds fees xfees xin inProper outProper balance fromBank toBank pendingBank';
  $result = compact(ray($resultKeys));
  
  foreach ($result as $k => $v) $result[$k] = $v = round($v, 2);

  return $result;
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
    $q = @db\q("SELECT uid AS id, fullName FROM users WHERE $where ORDER BY (uid<0) DESC, $nameOrder");
  } elseif ($mya->cAdmin2) {
    $sql = <<<EOF
SELECT DISTINCT id, fullName FROM
  (SELECT uid AS id, fullName FROM users WHERE community=:ctty UNION $sqlNormal) w
ORDER BY (id<0) DESC, $nameOrder
EOF;
    $q = @db\q($sql, ray('ctty aid', $mya->community, $aid));
  } else $q = @db\q("$sqlNormal ORDER BY $nameOrder", compact('aid')); // the usual case

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
 * Create a new bank transfer.
 * @param acct $payee: the account to or from which money is to be moved
 * @param numeric $amount: the amount to transfer
 * @param int $created: the UNIX time at which the transfer is deemed to be created.
 * @param string $txid: the internal transaction id for the transfer; normally null (assign a new id) but
 *                      can be specified (used during testing)
 * @return string $txid: the actual txid or FALSE on failure
 */
function createUsdTransfer($payee, $amount, $created=null, $txid=null) {
  u\EXPECT(compact(ray('payee amount created txid')), 'acct int|float int|empty string|int|empty');
  global $channel;
  ///  debug("createUsdTransfer($payee->id, $amount, $created, $txid)");
  
  $created = empty($created) ? r\rTime() : $created;
  if (empty($txid)) $txid = 1 + db\max('txid', 'r_usd');
  $bankAccount = u\arrayGet($payee->vsecure, 'bankAccount', null);
  
  $info = ray('amount payee created bankAccount channel',
              $amount, $payee->id, $created, $bankAccount, $channel);
  if (empty($txid)) {
    $txid = db\insert('r_usd', $info, 'txid');
    u\EXPECT(!empty($txid), 'failure inserting r_usd');
  } else {
    u\EXPECT(db\updateOrInsert('r_usd', ['txid'=>$txid] + $info, 'txid'), 'failure updating r_usd');
  }
  /* if ($amount < 0) { // transfer to the bank */
  /*   if (!completeUsdTx($payee, $txid, $amount, FALSE)) return FALSE; */
  /* } */
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
 */
function combineUsdTx($a, $amount, $completed) {
  u\EXPECT([$a, $amount, $completed], 'acct int|float int');
  ///  debug("combineUsdTx($a->id, $amount, $completed)");

  $DBTX = \db_transaction();
  if (!$a->bankOk) return [FALSE, $amount, $completed];
  if ($amount > 0 and $amount + $a->advances() - $a->balance > -$a->floor) $completed = 0; // only 0 if bringing in beyond credit line
  $where = 'payee=:id AND amount>0 AND NOT deposit AND NOT completed';
  if ($res = db\get('txid, amount AS oldAmount', 'r_usd', $where, ray('id', $a->id))) {
    extract($res);
    $amount += $oldAmount;
  } else {
    $txid = FALSE;
  }
  $result = db\update('r_usd', compact(ray('txid amount completed')), 'txid');
  if (!$result) {
    $txid = FALSE;
  }
  
  return [$txid, $amount, $completed];
}


/**
 * Mark an incoming USD transfer complete and report.
 * @param acct $a: the account
 * @param int $txid: the transfer record ID (if empty, no need to mark it complete)
 * @param numeric $amount: transfer amount from bank to CG (positive)
 * @param bool $automatic: <transfer was created automatically>
 * @param datetime $completed: when the transaction was completed, defaults to the current time
 * @return bool <success>
 */
function completeUsdTx($a, $txid, $amount, $automatic, $completed = null) {
  ///  debug("completeUsdTx($a->id, $txid, $amount, $automatic, $completed)");
  u\EXPECT([$a, $txid, $automatic], 'acct int|string|empty bool');
  if (empty($completed)) $completed = r\rTime();
  $err = u\badAmount($amount);
  if ($err) {
    t\output($err);
    return FALSE;
  }
  $DBTX = \db_transaction();

  if ($txid) { // unless transfer is already marked complete
    $values = db\get('amount as realAmt, payee AS payeeId, risk, risks, channel, xid', 'r_usd', 'txid=:txid', compact('txid'));
    extract(u\exactly('realAmt payeeId risk risks channel xid', array_filter($values), [0, 0, null, 0, TX_SYS, null]));
      
    if ($amount != $realAmt) {
      r\tellAdmin(t('bad completeUsdTx amt'), compact(ray('txid amount realAmt automatic')));
      return FALSE;  // ??
    }
    if (!empty($xid)) {
      return TRUE;  // already completed
    }
    $bankId =  ($amount > 0) ? CG_INCOMING_BANK_UID : CG_OUTGOING_BANK_UID;
    $tx = new r\Tx(['goods' => FOR_USD, 'actorId' => $payeeId, 'actorAgentId' => $payeeId, 'flags' => 0, 'channel' => $channel,
                    'risk' => $risk, 'risks' => $risks, 'created' => $completed]);
    $tx->addEntry(new r\TxEntry(['entryType' => ENTRY_PAYER, 'amount' => 0-$amount, 'uid' => $bankId, 'agentUid' => $bankId,
                                 'description' => ($amount > 0) ? t('from bank') : t('to bank')]));
    $tx->addEntry(new r\TxEntry(['entryType' => ENTRY_PAYEE, 'amount' => $amount, 'uid' => $payeeId, 'agentUid' => $payeeId,
                                 'description' => $amount > 0 ? t('from bank') : t('to bank')]));
    $xid = $tx->insert();
    if (!$xid) {
      $DBTX.rollback();
      r\tellAdmin(t('tx insert failed'), compact($txid, $amount, $automatic));
      return FALSE;
    }
    
    if (!db\update('r_usd', ['txid'=>$txid, 'xid'=>$xid, 'completed'=>r\rTime()], 'txid')) {
      r\tellAdmin(t('usd update failed'), compact($txid, $amount, $automatic));
      $DBTX.rollback();
      return FALSE;
    }
  }
    
  unset($DBTX);
    
  $transfer = $automatic ? t('automatic transfer') : t('transfer');
  $amount = u\fmtAmt($amount);
  r\notify($a->id, 'transfer complete', compact('transfer', 'amount'));
  return TRUE;
}
                           
function accountName($uid) {
  $acct = r\acct($uid);
  return $acct->fullName . ' ' . u\hug($acct->mainQid);
}
