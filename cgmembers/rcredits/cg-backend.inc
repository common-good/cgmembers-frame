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
 * @param integer $starting: the starting unix time
 * @param integer $ending: the ending unix time (exclusive)
 * @param mixed $uids: the uid or uids whose transactions we're going to process
 *        if $uids is an array it is the set of uids;
 *        if $uids is an integer it is the only uid
 * @param integer $jid: the joint account holder (if any)
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
  $myBalance = balanceAsOf($uids[0], $starting);
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
 * @param unixtime $ending: the ending date/time for the period (exclusive), e.g., $starting plus one day
 */
function getCustTxs($aUids, $bUids, $starting, $ending) {
  u\EXPECT('array array int int', $aUids, $bUids, $starting, $ending);
  $sql = <<< X
    SELECT CONCAT('tx #', aEntry.acctTid) AS tid, -aEntry.amount AS paid, '' AS invoiced, created,
           :IS_DISPUTED AS disputed, aEntry.description AS purpose 
    FROM r_users ua JOIN r_entries aEntry USING(uid) JOIN r_tx_hdrs t USING(xid) JOIN r_entries bEntry USING(xid)
    WHERE aEntry.uid IN (:aUids) AND bEntry.uid IN (:bUids)
    UNION ALL
    SELECT CONCAT('inv #', nvid) AS tid, '' AS paid, amount AS invoiced,
           created, :IS_DISPUTED AS disputed, aEntry.description AS purpose
    FROM r_invoices WHERE (payer IN (:aUids) AND payee IN (:bUids)) OR (payee IN (:aUids) AND payer IN (:bUids))
    ORDER BY created
X;

  $results = db\q($sql, ['aUids' => $aUids, 'bUids' => $bUids]);
  u\EXPECT($results, "Failure reading transactions between users " . print_r($aUids, true) . " and " . print_r($bUids, true));

  $lastTid = null;  // no tid should ever be null
  $txs = [];  // an array of arrays of entries
  $entries = [];
  foreach ($results as $result) {
    $entry = (array)$result;
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
  $myBalance = balanceAsOf($uids[0], $starting);
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
    ($a->jid != 0) ? $uids = [$uid, $a->jid] : [$uid];
  }
  return db\sum('amount', 'r_entries JOIN r_tx_hdrs USING(xid)', 'uid IN (:uids) AND created<:date', ['uids' => $uids, 'date' => $date]);
}
  
/**
 * Return an array of member names (called with ajax, so current user is passed as arguments).
 * POLICY QUESTION: Restrict to member's community? (yes for now)
 * @param int $myid: account record ID
 * @param int $aid: agent account record ID
 */
function memberRay($myid, $aid, $coOnly = FALSE) {
  if (!$a = r\acct($myid, $aid)) return [];
  $where = $coOnly ? ':IS_CO' : ($a->admin ? '1' : "community=$a->community OR :IS_CO");
  return db\q("SELECT fullName FROM users WHERE :IS_OK AND $where ORDER BY fullName")->fetchCol();
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
      WHERE payeeE.uid=:payee AND NOT :HAS_REVERSER AND reverses IS NULL AND created BETWEEN $start AND $end
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
///  debug($updates);
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
  debug("undoTx($xid, $where, " . print_r($subs, true) . ", $force)");
  global $mya;
  if (!$tx = r\Tx::lastTx($mya, "xid=:xid AND $where", compact('xid') + $subs)) {
    return ['undo no match', $subs];
  }
  if (!$force and $error = $tx->nonoUndo($subs)) return array($error, $subs); // check nono before confirm AND here

  $byMe = $mya->isMe($tx->actor);
  $toMe = $tx->getAmount($mya);
  if (!$byMe and !$toMe) { // someone charged me (either unilaterally or I approved it)
    r\disputeTx($xid); // mark it disputed
    $solution = t('marked "disputed"');
    r\notify($tx->them, 'charge disputed', $tx->reportArgs($mya, $tx->getMyAmt(), $payer, $payee, $payerPurpose, $payeePurpose));
  } else {
//    $amounts = just('amount payerReward payeeReward', $tx->ray);
    /* $what = [$tx->payerFor, $tx->payeeFor]; */
    $res = reverse($xid/*, $tx->amount, $what, $tx->a2, $data, $tx->goods, $tx->tid, $tx->toMe, $tx->byMe, $tx->tid2, $force*/); // create an offsetting transaction
    debug($res);

    list($message, $info) = $res;

    /*if ($byMe and !$toMe and $wasInvoice) $original = 'invoice-payment';
      else*/
    if ($byMe and !$toMe) $original = 'payment';
    elseif ($byMe and $toMe) $original = 'charge';
    elseif (!$byMe and $toMe) $original = 'payment';
    else /*!byMe and !$toMe*/ $original = 'charge';

    /*if ($byMe and !$toMe and $wasInvoice) $original = 'invoice-payment';
      else*/
    if ($byMe and !$toMe) $revType = 'reverse-invoice';
    elseif ($byMe and $toMe) $revType = 'payment';
    elseif (!$byMe and $toMe) $revType = 'payment';
    else /*!byMe and !$toMe*/ $revType = 'dispute';
    
    sendNotifications(r\acct($tx->actor), r\acct($tx->otherEntry->uid), $info['newTx'], $original, 0);
    return $res;
  }

  $amount = u\fmtAmt($tx->amount);
  $tofrom = !$tx->toMe ? t('to') : t('from');
  $otherUid = $tx->otherUid;
  $other = r\acct($otherUid)->fullName;
  $success = TRUE;
  return ['report undo', compact(ray('success amount other tofrom solution otherUid'))];
}

/* /\** */
/*  * Undo the transaction containing the given entry.  The entry should not be one that refers to */
/*  * the current account. */
/*  * Situations: */
/*  *     I paid someone (byMe):            REVERSE (invoice) */
/*  *     I charged someone (toMe/byMe):    REVERSE (pay back) */
/*  *     Someone paid me (toMe):           REVERSE (pay back) */
/*  *     Someone invoiced me and I approved it: DISPUTED */
/*  *     Someone charged me unilaterally: DISPUTED */
/*  * @param int $eid: the id of the entry record */
/*  * @param string $where: additional (security check) criteria, if any */
/*  * @param assoc $subs: additional substitutions for $where */
/*  * @param bool $force: <reverse despite any shortfall or lack of permission> */
/*  * @return array [message, subs] where subs is an assoc of [ */
/*  *\/ */
/* function undoTxGivenEntry($eid, $force = FALSE) { */
/*   global $mya; */

/*   $entry = r\TxEntry::read($eid); */
/*   u\EXPECT($entry, "trouble reading entry $eid"); */
/*   if ($entry->uid == $mya->id) {  // this is an entry that refers to us, which doesn't give much information */
/*     return ['undo self entry', []]; */
/*   } */
/*   $tx = Tx::read($entry->xid); */
/*   if (!$tx) return ['undo no match', []]; */

/*   if (!$force) { */
/*     $error = $tx->nonoUndo($subs); */
/*     if ($error) return array($error, $subs); // check nono before confirm AND here */
/*   } */

/*   $otherA = r\acct($entry->uid); */
  
/*   $toMe = $entry->amount < 0; // because the entry is for another account */
/*   if ($tx->actor == $mya->id) { // I initiated the transaction */
/*     if ($toMe) { // I charged someone -- reverse the transaction */
/*       return reverse($xid); */
/*     } else { // I paid someone -- invoice them for reversal */
/*       $purpose = $entry->description . t(' (reverses #%tid)', 'tid', $entry->tid); */
/*       return invoice($otherA, $mya, $x->amount, $purpose); */
/*     } */
/*   } else { // the other party initiated the transaction */
/*     if ($toMe) { // Someone else paid me */
/*       return reverse($xid); */
/*     } else { // Someone charged me (either unilaterally or I approved it) */
/*       r\disputeTx($xid); */
/*       $solution = t('marked "disputed"'); */
/*       r\notify($other, 'charge disputed', $tx->reportArgs($myA)); */
/*     } */
/*   } */

/*   $amount = u\fmtAmt($tx->amount); */
/*   $tofrom = $toMe ? t('to') : t('from'); */
/*   $otherUid = $tx->otherUid; */
/*   $other = $otherA->fullName; */
/*   $success = TRUE; */
/*   return ['report undo', compact(ray('success amount other tofrom solution otherUid'))]; */
/* } */

/**
 * Create an offsetting transaction (nearly the same as the original but with the entries negated).
 */
function reverse($xid) {
  debug("reverse($xid)");
  global $mya, $channel;

  $DBTX = \db_transaction();
  if (db\exists('r_tx_hdrs', 'reverses=:xid', ['xid' => $xid])) {
    return r\txRet('already reversed', compact('xid'));
  }
  $tx = r\Tx::read($xid);
  if (is_null($tx)) {
    return r\txRet('no transaction', compact('xid'));
  }

  // It seems OK to reverse it
  $newTx = clone $tx;  // this is a shallow copy, entries points to the same entries
  $newTx->xid = null;
  $newTx->reverses = $xid;
  $newTx->entries = [];
  foreach ($tx->entries as $oldEntry) {
    $newEntry = clone $oldEntry;
    if ($newEntry->relType == 'D' and $newEntry->amount > 0) { // related to a coupated record
      if (!db\q('UPDATE r_coupated SET uses=uses-1 WHERE id=:coupatedId',
                ['coupatedId' => $newEntry->related])) {
        $DBTX->rollback();
        return r\txRet('tx create err', $entry);
      }
    }
    $newEntry->id = null;
    $newEntry->xid = null;  // new entry belongs to reversing transaction
    $newEntry->amount = 0-$oldEntry->amount;
    $newEntry->description .= ' (' . t('reverses #') . $oldEntry->acctTid . ')';
    $newEntry->acctTid = r\acct($newEntry->uid)->nextTid();
    debug("newEntry->acctTid=$newEntry->acctTid");
    $newTx->entries[] = $newEntry;
  }
  $xid = $newTx->insert();
  if (!$xid) {
    $DBTX->rollback();
    return r\txRet('tx save err', $newTx);
  }
  
  $DBTX = null;
  
  return r\txRet('report undo!', ['ok'=>1, 'txid'=>$xid, 'created'=>$newTx->created, 'balance'=>$mya->balance, 'solution' => 'reversed', 'success'=>true, 'xid'=>$xid, 'created0'=>$newTx->created, 'newTx'=>$newTx]);
}

/**
 * Pay the given invoice, possibly without being signed in.
 * @param int $nvid: record ID of invoice to pay
 * @param string $msg: success or error message
 * @return TRUE if the payment was successful
 */
function payInvoice($nvid, &$msg = '') {
  if ($msg = r\badUnpaidInv($nvid, $inv)) return FALSE;
  extract(just('payer payee amount goods purpose data flags', $inv));
  $payerA = r\acct($payer);
  $payeeA = r\acct($payee);
  
  $DBTX = \db_transaction();
  $purpose .= ' (' . PROJECT . " inv#$nvid)";
	$extra = ray('inv goods', $nvid, $goods);
	if ($flags) foreach (ray('gift recurs investment') as $k) if (u\getBit($flags, $k)) $extra[$k] = 1;
  $payerA->pay($payeeA, $amount, $purpose, $extra);
  list ($index, $subs) = be\transfer('payment', $this, $payeeA, $amount, $purpose, $extra);

  $ok = arrayGet($subs, 'success', FALSE);
  if ($ok) //db\q('UPDATE r_invoices SET status=:xid WHERE nvid=:nvid', ray('xid nvid', @$subs['xid'], $nvid));
  db\update('r_invoices', ray('status nvid', $subs['xid'], $nvid), 'nvid');

	if (u\getBit(@$flags, B_INVESTMENT)) { // club reclaiming an investment
/*
    $res = db\get('shid,vestid', 'r_shares s LEFT JOIN r_investments i ON i.vestid=s.vestid', 'coid=:coid AND clubid=:clubid AND pending<0', ray('coid clubid', $payer, $payee));
    if ($res) {
      extract($res); // shid vestid
      */
    if ($data = unserialize($data) and $shid = $data('shid')) { // record the sale
      db\update('r_shares', ray('shid shares pending when', $shid, -$amount, 0, NOW), 'shid');
    } else r\tellCo(t('Invoice payment marked as "for investment (repayment)" has no corresponding SELL request in shares table.'), $inv, $payee);
  }
  unset($DBTX);
  
  $msg = t($index, $subs);
  return $ok;
}

/**
 * Create a transaction
 * call by: list ($msg, $args) = be\transfer(...);
 * @param string $ttype: payment, charge, or invoice payment
 * @param acct $actor: acct object for the initiating party (usually the current user)
 * @param acct $other: acct object for the other party
 *   OR 'ALL' (paying employees) -- currently UNUSED
 * @param number $amount: the transaction amount
 * @param string|array $for: description of the transaction's purpose (user input) (or [actorFor, otherFor])
 * @param array $data: associative array of info about the transaction this request is reversing (if any), 
 *   indexed by any of:
 *   'goods': 0=for USD, 1=for real goods and services, 2=other
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
 *   $goods (the transaction is an exchange for real stuff),
 *   $shortfall (how inadequate the payer's balance is)
 */
function transfer($ttype, $actor, $other, $amount, $for, $data = []) {
  global $scanned, $channel;

  u\EXPECT($ttype == 'charge' or $ttype = 'payment', "ttype must be 'charge' or 'payment', but is '$ttype'.");
  u\EXPECT(compact(ray('actor other data')), 'acct acct assoc');
  extract(u\exactly('goods created box', $data, [FOR_GOODS, r\rTime(), null]));
  extract(u\exactly('force offline loan investment stake fine noask funding crumbs recurs gift', $data, FALSE));
  $short = FALSE;
  
  if (is_null($for)) $for = ray(R_WHYS)[$goods];
  list($actorFor, $otherFor) = (is_array($for)) ? $for : [$for, $for];

  $type = TX_TRANSFER;
  if (is_array($amount)) extract(just('amount', $amount));
  list ($actorId, $otherId) = array($actor->id, $other->id);

  if ($force) $offline = TRUE; // temporary until app handles this right
  
  $taking = ($ttype == 'charge');
  $erInfo = compact(ray('ttype amount goods for force')) + ray('actor other op', $actor->fullName, $other->fullName, $taking ? t('charged') : t('paid'));
  $amount = round($amount, 2); // ignore fractions of cents
  if ($amount == 0) {
/**/  r\tellAdmin('zero transaction', $erInfo + ['stack' => trace()]);
    return r\txRet('zero transaction', $erInfo);
  }
  $reallyTaking = ($taking xor $amount < 0); // not taking if charging a negative amount

  list ($loser, $gainer) = u\order($reallyTaking, $other, $actor);
  list ($loserFor, $gainerFor) = u\order($reallyTaking, $otherFor, $actorFor);
  $loserTid = $loser->nextTid();
  $gainerTid = $gainer->nextTid();
  
  $erInfo += ['loserId' => $loser->id];
  list ($payer, $payee) = u\order($taking, $otherId, $actorId); // Analyze who's buying or selling
  list ($payerAgent, $payeeAgent) = u\order($taking, $other->agentId, $actor->agentId);

  if (!$force /*and !@$data['inv']*/ and r\dupTx($other, $actor, $taking ? $amount : -$amount)) return r\txRet('duplicate transaction', $erInfo);
  if ($err = r\txPermErr($actor, $other, $taking, $amount < 0)) { // Check permissions
    list ($erMsg, $erSubs) = $err;
    if ($force) {
      $msg = 'forced without perm';
      $erInfo += ['date' => u\fmtDate($created)];
      foreach (['agent1'=>$actor, 'agent2'=>$other] as $k => $a) $erInfo += [$k => $a->proSe ? t('self') : $a->fullName];
      foreach ([$actor, $other] as $a) if (!$a->proSe) r\message($a->id, $msg, $erInfo);
      r\tellAdmin($msg . ' ' . t($erMsg, $erSubs), $erInfo + $erSubs);
    } else return r\txRet($erMsg, $erInfo + $erSubs);
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
  if ($error = u\badAmount($amount)) return r\txRet($error, $erInfo);
//  $need = $amount - @$payerReward; // leave room for reversing rewards (minus a negative)

  $gainerType = ($gainer->isMe($payer)) ? ENTRY_PAYER : ENTRY_PAYEE;
  $loserType = ($gainer->isMe($payer)) ? ENTRY_PAYEE : ENTRY_PAYER;
  $DBTX = db_transaction();
  // Create the transaction
  $flags = ($offline ? 1 << B_OFFLINE : 0) | ($short ? 1 << B_SHORT : 0) | ($crumbs ? 1 << B_CRUMBS : 0)
    | ($recurs ? 1 << B_RECURS : 0) | ($gift ? 1 << B_GIFT : 0) | ($loan ? 1 << B_LOAN : 0)
    | ($investment ? 1 << B_INVESTMENT : 0) | ($stake ? 1 << B_STAKE : 0) | ($fine ? 1 << B_FINE : 0)
    | ($noask ? 1 << B_NOASK : 0) | ($funding ? 1 << B_FUNDING : 0);
  $info = ray('type goods actor actorAgent flags channel box created',
              $type, $goods, $actor->id, $actor->agentId, $flags, $channel, $box, $created); //amount created payer payee payerAgent payeeAgent for data taking roundup goods force offline investment short'));
  print_r($info);
  $tx = new r\Tx($info);
  if (!$tx) {
    $DBTX->rollback();
    return r\txRet('tx create err', $erInfo);
  }
  $entries = [new r\TxEntry(['entryType' => $gainerType, 'amount' => abs($amount), 'uid' => $gainer->id, 'agentUid' => $gainer->agentId, 'description' => $gainerFor, 'acctTid' => $gainerTid]),
              new r\TxEntry(['entryType' => $loserType, 'amount' => 0-abs($amount), 'uid' => $loser->id, 'agentUid' => $loser->agentId, 'description' => $loserFor, 'acctTid' => $loserTid])];
  
  // Apply coupon
  if ($amount > 0 AND $goods == FOR_GOODS) {
    list($rebate, $coupatedId) = $loser->applyCoupon($gainer, $amount);
    if ($rebate > 0) {
      $entries[] = new r\TxEntry(['amount' => 0-$rebate, 'uid' => $gainer->id, 'agentUid' => $gainer->agentId, 'description' => t('discount rebate (on #%tid)', ['tid' => $gainerTid]), 'acctTid' => $gainerTid, 'relType' => 'D', 'related' => $coupatedId]);
      $entries[] = new r\TxEntry(['amount' => $rebate, 'uid' => $loser->id, 'agentUid' => $loser->agentId, 'description' => t('discount rebate (on #%tid)', ['tid' => $loserTid]), 'acctTid' => $loserTid, 'relType' => 'D', 'related' => $coupatedId]);
    }
    /* if ($pot = $loser->giftPot) $loser->update('giftPot', max(0, $pot - $price)); // use up gift coupons, if any (if tx gets reversed, gift recipient can cash it out, but that's no biggie) */
  } else { $rebate = 0; }

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
      return r\txRet($msg, compact(ray('otherName short details avail')));
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

  $tx->addEntries($entries);
  $xid = $tx->insert();
  if (!$xid) {
    $DBTX->rollback();
    return r\txRet('tx save error');
  }

  unset($DBTX);
  
  // Report results


  // possible notices are:
  //   new charge, new refund, new payment, new payment linked,
  // possible dids are:
  //   refunded, credited, charged,
  // or, for reversing transactions:
  //   re-charged, charged, credited, paid
  
  // notify other party by email about any completed transaction
  foreach ([$actor, $other] as $a) if (!$a->confirmed) $a->setBit(B_CONFIRMED);

  sendNotifications($actor, $other, $tx, $ttype, $shortfall);
/*   $notice = $reallyTaking ? 'new charge' */
/*     : (($amount < 0 and $goods == FOR_GOODS) ? 'new refund' : 'new payment'); */
/*   $args = $tx->reportArgs(r\acct(), TRUE, $shortfall); */
/*   if ($notice == 'new payment' and $other->co) { // not "new charge" (the most common); payment at payer's initiative */
/*     $notice = 'new payment linked'; */
/*     $args['_aPayLink'] = $other->makeDo('addr', $actorId, 0); // link to address, so merchant can ship or nonprofit can thank */
/* //      "<$atag>" . check_plain($args['myName']) . '</a>'; */
/*   } */
/* //  if (@$args['otherRewardAmount'] and $actor->cttyRewardy) $notice .= '|reward other'; */
/*   r\notify($otherId, $notice, $args); */
/*   $msg = 'report tx'; */
/* /\*  if ($actor->cttyRewardy) { */
/*     $msg .= '|for why'; */
/*     if ($goods == FOR_GOODS) $msg .=  ($channel == TX_POS and $actor->co) ? '|reward customer' : '|the reward'; */
/*   } *\/ */
/* //  } else $msg = $ttype; */

/* //  if (isPRODUCTION) r\tellAdmin('tx', compact('msg') + $args); // a backup record of all txs (until the backup server works) */
  $msg = 'report tx';
  $args = $tx->reportArgs($actor, TRUE, $shortfall);
  return r\txRet($msg, $args + compact('msg'));
}

function sendNotifications($actor, $other, $tx, $ttype, $shortfall) {
  $actorAmt = $tx->actorEntry->amount;
  $taking = ($ttype == 'charge');
  $reallyTaking = ($taking xor $actorAmt < 0); // not taking if charging a negative amount

  $notice = $reallyTaking ? 'new charge'
    : (($actorAmt < 0 and $tx->goods == FOR_GOODS) ? 'new refund' : 'new payment');
  $args = $tx->reportArgs($actor, TRUE, $shortfall);
  if ($notice == 'new payment' and $other->co) { // not "new charge" (the most common); payment at payer's initiative
    $notice = 'new payment linked';
    $args['_aPayLink'] = $other->makeDo('addr', $actorId, 0); // link to address, so merchant can ship or nonprofit can thank
  }
  r\notify($other->uid, $notice, $args);
  $msg = 'report tx';
}

/* function transfer($ttype, $actor, $other, $amount, $for, $data = []) { */
/*   u\EXPECT(compact(ray('ttype actor other for data')), 'string acct acct string|array assoc'); */

/*   if (is_null($for)) { $for = ray(R_WHYS)[$goods]; } */
/*   list($sellerFor, $buyerFor) = (is_array($for)) ? $for : [$for, $for]; */

/*   $erInfo = compact(ray('ttype amount goods for force')); */
/*   $error = u\badAmount($amount); */
/*   if ($error) return r\txRet($error, $erInfo); */
/*   $amount = round($amount, 2); // ignore fractions of cents */
/*   if ($amount == 0) { */
/*     r\tellAdmin('zero transaction', $erInfo + ['stack' => trace()]); */
/*     return r\txRet('zero transaction', $erInfo); */
/*   } */

/*   if (!array_key_exists('goods', $data)) $data['goods'] = FOR_GOODS; */
/*   if (!array_key_exists('created', $data)) $data['created'] = r\rTime(); */
/*   if (!array_key_exists('force', $data)) $data['force'] = FALSE; */
/*   if (!array_key_exists('box', $data)) $data['box'] = null; */

/*   if ($ttype == 'charge') { */
/*     if ($amount >= 0) { */
/*       return charge($actor, $other, $amount, $for, $data); */
/*     } else { */
/*       return refund($actor, $other, 0-$amount, $for, $data); */
/*     } */
/*   } elseif ($ttype == 'payment') { */
/*     if ($amount >= 0) { */
/*       return purchase($actor, $other, $amount, $for, $data); */
/*     } else { */
/*       u\EXPECT(false, 'clawback should not happen'); */
/*       /\* return clawback($actor, $other, 0-$amount, $for, $data); *\/ */
/*     } */
/*   } */
/* } */

/* /\** */
/*  * Create a transaction, the seller is charging the buyer an amount for certain goods, services, or cash */
/*  * call by: list ($msg, $args) = be\transfer(...); */
/*  * @param acct $seller: acct object for the seller (also the initiator) */
/*  * @param acct $other: acct object for the buyer */
/*  *   OR 'ALL' (paying employees) -- currently UNUSED */
/*  * @param number $amount: the transaction amount -- must be non-negative */
/*  * @param string|array $for: description of the transaction's purpose (user input) (or [actorFor, otherFor]) */
/*  * @param array $data: associative array of info about the transaction this request is reversing (if any),  */
/*  *   indexed by any of: */
/*  *   'goods': 0=for USD, 1=for real goods and services, 2=other */
/*  *   'created': time of transaction (defaults to current time) */
/*  *   'force': make the charge despite any shortfall */
/*  *   'box': the device on which the transaction was initiated */
/*  *   'offline' */
/*  *   'investment' */
/*  *   'crumbs', 'recurs', 'gift': gift info */
/*  * @return simple array:  */
/*  *    index: index to result message string (if success, the index begins with "report ") */
/*  *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE) */
/*  * */
/*  * The following used to be possible keys for $data, but not any more: */
/*  *   'inv': record number of the invoice for which this is payment */
/*  *   'isGift': UNUSED <the payment is a gift to a community or nonprofit> */
/*  *   'roundups' */
/*  *   'roundups': the payment is change being donated to the community */
/*  *   'rebate': the rebate amount for this request (should be negative the original bonus amount) */
/*  *   'bonus': the bonus amount (should be negative the original rebate amount) */
/*  *   'undoes': transaction id of the related transaction (transaction being undone) */
/*  * */
/*  *   'short': ? */
/*  *   'funding': ? */
/*  * */
/*  * Do something different, based on these boolean values:  */
/*  *   $taking (charge vs. payment),  */
/*  *   $oked UNUSED (user confirmed the transaction request), */
/*  *   $goods (the transaction is an exchange for real stuff), */
/*  *   $shortfall (how inadequate the payer's balance is) */
/*  *\/ */
/* function charge($seller, $buyer, $amount, $for, $data = []) { */
/*   global $scanned, $channel; */

/*   /\* u\EXPECT(compact(ray('seller buyer amount for data')), 'acct acct number array array'); *\/ */
/*   u\EXPECT($amount > 0, 'amount charged must be positive'); */
/*   extract(u\exactly('goods created force box', $data, [FOR_GOODS, r\rTime(), FALSE, null])); */
/*   extract(u\exactly('offline investment crumbs recurs gift', $data, FALSE)); */

/*   if ($force == 1) $offline = TRUE; // temporary until app handles this right */

/*   $erInfo = compact(ray('amount goods for force')) + ray('ttype a1 other op', 'charge', $seller->fullName, $buyer->fullName, t('charged')); */

/*   if (!$force and r\dupTx($buyer, $seller, $amount)) { */
/*     return r\txRet('duplicate transaction', $erInfo); */
/*   } */

/*   // Check permissions */
/*   $err = r\txPermErr($seller, $buyer, TRUE, FALSE); */
/*   if ($err) { */
/*     list ($erMsg, $erSubs) = $err; */
/*     if ($force) { */
/*       $msg = 'forced without perm'; */
/*       $erInfo += ['date' => u\fmtDate($created)]; */
/*       $erInfo += ['agent1' => $seller->proSe ? t('self') : $seller->fullName,  */
/*                   'agent2' => $buyer->proSe ? t('self') : $buyer->fullName]; */
/*       if (!$seller->proSe) r\message($seller->id, $msg, $erInfo); */
/*       if (!$buyer->proSe) r\message($buyer->id, $msg, $erInfo); */
/*       r\tellAdmin($msg . ' ' . t($erMsg, $erSubs), $erInfo + $erSubs); */
/*     } else { */
/*       return r\txRet($erMsg, $erInfo + $erSubs); */
/*     } */
/*   } */
  
/*   foreach ([$seller, $buyer] as $a) { */
/*     if (!$a->co and !($channel == TX_POS ? $a->agentA->ided : $a->ided)) { */
/*       if ($force) { */
/*         r\tellAdmin('forced without photoId', $erInfo);  */
/*       } /\* elseif ($payeeA->id != CGID) { // and !$payeeA->coCan(CO_FAST) *\/ */
/*       /\* //      r\tellAdmin('tx without photoId', $erInfo);  *\/ */
/*       /\* //      return r\txRet($a == $sellerA ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $buyerA->fullName]); *\/ */
/*       /\*     } *\/ */
/*     } */
/*   } */

/*   $buyerTid = 1 + maxTid([$buyer->id, $buyer->jid]); */
/*   $sellerTid = 1 + maxTid([$seller->id, $seller->jid]); */
/*   $basicEntries = [ new r\TxEntry(ray('amount uid agentUid description acctTid', */
/*                                       $amount, $seller->id, $seller->agentId, $for[0], $sellerTid)), */
/*                     new r\TxEntry(ray('amount uid agentUid description acctTid', */
/*                                       0-$amount, $buyer->id, $buyer->agentId, $for[1], $buyerTid)) ]; */
  
/*   // Deal with coupons */
/*   if ($goods == FOR_GOODS) { */
/*     list($couponEntries, $rebate) = handleCoupon($buyer, $seller, $amount, $buyerTid, $sellerTid); */
/*   } else { */
/*     $couponEntries = []; */
/*     $rebate = 0; */
/*   } */

/*   // Check for balance shortfall */
/*   $needed = $amount - $rebate; */
/*   if ($buyer->uid > 0) {  // if it's a community, don't check; they don't have shortfalls? */
/*     $shortfall = $buyer->shortfall($goods, $needed, TRUE, $seller); */
/*     if ($shortfall != 0) { */
/*       $msg = $seller->can(B_SECRET) ? 'short from vague' : 'short from'; */
/*       $buyer->suggestAuto(); */
/*       $details = t('|%payer was short by %short trying to pay %payee %amount (available balance is %avail)', */
/*                    'payer short payee amount avail', */
/*                    $buyer->fullName, u\fmtAmt($shortfall), $seller->fullName, u\fmtAmt($needed), */
/*                    u\fmtAmt($needed - $shortfall)); */
/*       return r\txRet($msg, ray('otherName short details avail', */
/*                                $buyer->fullName, u\fmtAmt($shortfall), $details, u\fmtAmt($needed-$shortfall))); */
/*     } */
/*   } */

/*   // Roundup donations?  For buyer only, and don't round up recurring donations to CGID less than $1. */
/*   $roundup = (r\acct($buyer)->roundup and !($seller == CGID and $recurs and $amount < 1)); */
/*   if ($roundup) { */
/*     $cents = round(fmod($needed, 1), 2); */
/*     if ($cents > 0) { */
/*       $donation = 1 - $cents; */
/*       $roundupEntries = [new r\TxEntry(ray('amount uid agentUid description', */
/*                                          $donation, CG_ROUNDUPS_UID, CG_ROUNDUPS_UID, t('roundup donation'))), */
/*                          new r\TxEntry(ray('amount uid agentUid description', */
/*                                          0-$donation, $payer->id, $payer->agentId, t('roundup donation')))]; */
/*     } */
/*   } else { */
/*     $roundupEntries = []; */
/*     $donation = 0; */
/*   } */
  
/*   $flags = ($offline ? (1 << B_OFFLINE) : 0) | ($short ? (1 << B_SHORT) : 0) */
/*     | ($recurs ? (1 << B_RECURS) : 0) | ($gift ? (1 << B_GIFT) : 0) */
/*     | ($funding ? (1 << B_FUNDING) : 0) | ($crumbs ? (1 << B_CRUMBS) : 0); */


/*   $tx = new r\Tx(ray('type goods actor actorAgent flags channel box created', */
/*                    TX_TRANSFER, $goods, $seller->id, $seller->agentId, $flags, $channel, $box, $created)); */
/*   $tx->addEntries($basicEntries, $couponEntries, $roundupEntries); */
/*   if (!$tx->insert()) { */
/*     return txRet('failure saving tx', $hdrInfo); */
/*   } */

/*   foreach ([$seller, $buyer] as $a) if (!$a->confirmed) $a->setBit(B_CONFIRMED); */

/*   r\notifyEntries($buyer, 'new charge', $tx->entries); */

/*   return r\txRet('report tx', []); */
/* } */

/* /\** */
/*  * Create a refund transaction; no coupons, no roundups. */
/*  * call by: list ($msg, $args) = be\refund(...); */
/*  * @param acct $payer: acct object for the refunder (usually the current user) */
/*  * @param acct $payee: acct object for the party getting the refund */
/*  * @param number $amount: the transaction amount -- must be positive */
/*  * @param array $for: description of the transaction's purpose */
/*  * @param array $data: associative array of info about the transaction */
/*  *   indexed by any of: */
/*  *   'goods': 0=for real goods and services, 1=for USD, 2=other */
/*  *   'created': time of transaction (defaults to current time) */
/*  *   'force': <do the transaction despite any shortfall (1 for original tx, -1 for forced reversal)> */
/*  *   'box': the device on which the transaction was initiated */
/*  *   'offline' */
/*  *   'investment' */
/*  *   'crumbs', 'recurs', 'gift': gift info */
/*  * @return simple array:  */
/*  *    index: index to result message string (if success, the index begins with "report ") */
/*  *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE) */
/*  * */
/*  * The following used to be possible keys for $data, but not any more: */
/*  *   'inv': record number of the invoice for which this is payment */
/*  *   'isGift': UNUSED <the payment is a gift to a community or nonprofit> */
/*  *   'roundups' */
/*  *   'roundups': the payment is change being donated to the community */
/*  *   'rebate': the rebate amount for this request (should be negative the original bonus amount) */
/*  *   'bonus': the bonus amount (should be negative the original rebate amount) */
/*  *   'undoes': transaction id of the related transaction (transaction being undone) */
/*  * */
/*  *   'short': ? */
/*  *   'funding': ? */
/*  * */
/*  * Do something different, based on these boolean values:  */
/*  *   $taking (charge vs. payment),  */
/*  *   $oked UNUSED (user confirmed the transaction request), */
/*  *   $goods (the transaction is an exchange for real stuff), */
/*  *   $shortfall (how inadequate the payer's balance is) */
/*  *\/ */
/* function refund($payer, $payee, $amount, $for, $data = []) { */
/*   global $scanned, $channel; */

/*   u\EXPECT(compact(ray('seller buyer amount for data')), 'acct acct number array array'); */
/*   u\EXPECT($amount > 0, 'amount refunded must be positive'); */
/*   extract(u\exactly('goods created force box', $data, [FOR_GOODS, r\rTime(), FALSE, null])); */
/*   extract(u\exactly('offline investment crumbs recurs gift', $data, FALSE)); */

/*   if ($force == 1) $offline = TRUE; // temporary until app handles this right */

/*   $erInfo = compact(ray('amount goods for force')) + ray('ttype a1 a2 op', 'charge', $payer->fullName, $payee->fullName, t('charged')); */

/*   if (!$force and r\dupTx($payee, $payer, $amount)) { */
/*     return r\txRet('duplicate transaction', $erInfo); */
/*   } */

/*   // Check permissions */
/*   $err = r\txPermErr($payer, $payee, TRUE, FALSE); */
/*   if ($err) { */
/*     list ($erMsg, $erSubs) = $err; */
/*     if ($force) { */
/*       $msg = 'forced without perm'; */
/*       $erInfo += ['date' => u\fmtDate($created)]; */
/*       $erInfo += ['agent1' => $payer->proSe ? t('self') : $payer->fullName,  */
/*                   'agent2' => $payee->proSe ? t('self') : $payee->fullName]; */
/*       if (!$payer->proSe) r\message($payer->id, $msg, $erInfo); */
/*       if (!$payee->proSe) r\message($payee->id, $msg, $erInfo); */
/*       r\tellAdmin($msg . ' ' . t($erMsg, $erSubs), $erInfo + $erSubs); */
/*     } else { */
/*       return r\txRet($erMsg, $erInfo + $erSubs); */
/*     } */
/*   } */
  
/*   foreach ([$payer, $payee] as $a) { */
/*     if (!$a->co and !($channel == TX_POS ? $a->agentA->ided : $a->ided)) { */
/*       if ($force) { */
/*         r\tellAdmin('forced without photoId', $erInfo);  */
/*       } /\* elseif ($payeeA->id != CGID) { // and !$payeeA->coCan(CO_FAST) *\/ */
/*       /\* //      r\tellAdmin('tx without photoId', $erInfo);  *\/ */
/*       /\* //      return r\txRet($a == $payerA ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $buyerA->fullName]); *\/ */
/*       /\*     } *\/ */
/*     } */
/*   } */

/*   $basicEntries = [ new r\TxEntry(ray('amount uid agentUid description', */
/*                                     0-$amount, $payer->id, $payer->agentId, $for[0])), */
/*                     new r\TxEntry(ray('amount uid agentUid description', */
/*                                     $amount, $payee->id, $payee->agentId, $for[1])) ]; */
  
/*   $flags = ($offline ? (1 << B_OFFLINE) : 0) | ($short ? (1 << B_SHORT) : 0) */
/*     | ($recurs ? (1 << B_RECURS) : 0) | ($gift ? (1 << B_GIFT) : 0) */
/*     | ($funding ? (1 << B_FUNDING) : 0) | ($crumbs ? (1 << B_CRUMBS) : 0); */


/*   $tx = new r\Tx(ray('type goods actor actorAgent flags channel box created', */
/*                    TX_TRANSFER, $goods, $seller->id, $seller->agentId, $flags, $channel, $box, $created)); */
/*   $tx->addEntries($basicEntries); */
/*   if (!$tx->insert()) { */
/*     return txRet('failure saving tx', $hdrInfo); */
/*   } */

/*   foreach ([$payer, $payee] as $a) if (!$a->confirmed) $a->setBit(B_CONFIRMED); */

/*   r\notifyEntries($payee, 'new refund', $tx->entries); */

/*   return r\txRet('report tx', $args); */
/* } */

/* /\** */
/*  * Create a purchase transaction (initiated by purchaser) */
/*  * call by: list ($msg, $args) = be\payment(...); */
/*  * @param acct $purchaser: acct object for the initiating party (usually the current user) */
/*  * @param acct $seller: acct object for the other party */
/*  * @param number $amount: the transaction amount -- must be positive */
/*  * @param array $for: description of the transaction's purpose [purchaserFor, sellerFor]) */
/*  * @param array $data: associative array of info about the transaction this request is reversing (if any),  */
/*  *   indexed by any of: */
/*  *   'goods': 0=for USD, 1=for real goods and services, 2=other */
/*  *   'created': time of transaction (defaults to current time) */
/*  *   'force': <do the transaction despite any shortfall (1 for original tx, -1 for forced reversal)> */
/*  *   'box': the device on which the transaction was initiated */
/*  *   'offline' */
/*  *   'investment' */
/*  *   'crumbs', 'recurs', 'gift': gift info */
/*  * @return simple array:  */
/*  *    index: index to result message string (if success, the index begins with "report ") */
/*  *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE) */
/*  * */
/*  * The following used to be possible keys for $data, but not any more: */
/*  *   'inv': record number of the invoice for which this is payment */
/*  *   'isGift': UNUSED <the payment is a gift to a community or nonprofit> */
/*  *   'roundups' */
/*  *   'roundups': the payment is change being donated to the community */
/*  *   'rebate': the rebate amount for this request (should be negative the original bonus amount) */
/*  *   'bonus': the bonus amount (should be negative the original rebate amount) */
/*  *   'undoes': transaction id of the related transaction (transaction being undone) */
/*  * */
/*  *   'short': ? */
/*  *   'funding': ? */
/*  * */
/*  * Do something different, based on these boolean values:  */
/*  *   $taking (charge vs. payment),  */
/*  *   $oked UNUSED (user confirmed the transaction request), */
/*  *   $goods (the transaction is an exchange for real stuff), */
/*  *   $shortfall (how inadequate the payer's balance is) */
/*  *\/ */
/* function payment($purchaser, $seller, $amount, $for, $data = []) { */
/*   global $scanned, $channel; */

/*   u\EXPECT(compact(ray('purchaser seller amount for data')), 'acct acct number array array'); */
/*   u\EXPECT($amount > 0, 'amount to pay must be positive'); */
/*   extract(u\exactly('goods created force box', $data, [FOR_GOODS, r\rTime(), FALSE, null])); */
/*   extract(u\exactly('offline investment crumbs recurs gift', $data, FALSE)); */

/*   if ($force == 1) $offline = TRUE; // temporary until app handles this right */

/*   $erInfo = compact(ray('amount goods for force')) + ray('ttype a1 a2 op', 'charge', $seller->fullName, $buyer->fullName, t('charged')); */

/*   if (!$force and r\dupTx($seller, $purchaser, $amount)) { */
/*     return r\txRet('duplicate transaction', $erInfo); */
/*   } */

/*   // Check permissions */
/*   $err = r\txPermErr($purchaser, $seller, TRUE, FALSE); */
/*   if ($err) { */
/*     list ($erMsg, $erSubs) = $err; */
/*     if ($force) { */
/*       $msg = 'forced without perm'; */
/*       $erInfo += ['date' => u\fmtDate($created)]; */
/*       $erInfo += ['agent1' => $purchaser->proSe ? t('self') : $purchaser->fullName,  */
/*                   'agent2' => $seller->proSe ? t('self') : $seller->fullName]; */
/*       if (!$purchaser->proSe) r\message($purchaser->id, $msg, $erInfo); */
/*       if (!$seller->proSe) r\message($seller->id, $msg, $erInfo); */
/*       r\tellAdmin($msg . ' ' . t($erMsg, $erSubs), $erInfo + $erSubs); */
/*     } else { */
/*       return r\txRet($erMsg, $erInfo + $erSubs); */
/*     } */
/*   } */
  
/*   foreach ([$purchaser, $seller] as $a) { */
/*     if (!$a->co and !($channel == TX_POS ? $a->agentA->ided : $a->ided)) { */
/*       if ($force) { */
/*         r\tellAdmin('forced without photoId', $erInfo);  */
/*       } /\* elseif ($payeeA->id != CGID) { // and !$payeeA->coCan(CO_FAST) *\/ */
/*       /\* //      r\tellAdmin('tx without photoId', $erInfo);  *\/ */
/*       /\* //      return r\txRet($a == $purchaserA ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $sellerA->fullName]); *\/ */
/*       /\*     } *\/ */
/*     } */
/*   } */

/*   $basicEntries = [ new r\TxEntry(ray('amount uid agentUid description', */
/*                                     $amount, $purchaser->id, $purchaser->agentId, $for[0])), */
/*                     new r\TxEntry(ray('amount uid agentUid description', */
/*                                     0-$amount, $seller->id, $seller->agentId, $for[1])) ]; */
  
/*   // Deal with coupons */
/*   if ($goods == FOR_GOODS) { */
/*     list($couponEntries, $rebate) = handleCoupon($seller, $purchaser, $amount, $sellerTid, $purchaserTid); */
/*   } else { */
/*     $couponEntries = []; */
/*     $rebate = 0; */
/*   } */

/*   // Check for balance shortfall */
/*   $needed = $amount - $rebate; */
/*   if ($seller->uid > 0) {  // if it's a community, don't check; they don't have shortfalls? */
/*     $shortfall = $seller->shortfall($goods, $needed, TRUE, $purchaser); */
/*     if ($shortfall != 0) { */
/*       $msg = $purchaser->can(B_SECRET) ? 'short from vague' : 'short from'; */
/*       $seller->suggestAuto(); */
/*       $details = t('|%payer was short by %short trying to pay %payee %amount (available balance is %avail)', */
/*                    'payer short payee amount avail', */
/*                    $seller->fullName, u\fmtAmt($shortfall), $purchaser->fullName, u\fmtAmt($needed), */
/*                    u\fmtAmt($needed - $shortfall)); */
/*       return r\txRet($msg, ray('otherName short details avail', */
/*                                $seller->fullName, u\fmtAmt($shortfall), $details, u\fmtAmt($needed-$shortfall))); */
/*     } */
/*   } */

/*   // Roundup donations?  For buyer only, and don't round up recurring donations to CGID less than $1. */
/*   $roundup = (r\acct($seller)->roundup and !($purchaser == CGID and $recurs and $amount < 1)); */
/*   if ($roundup) { */
/*     $cents = round(fmod($needed, 1), 2); */
/*     if ($cents > 0) { */
/*       $donation = 1 - $cents; */
/*       $roundupEntries = [new r\TxEntry(ray('amount uid agentUid description', */
/*                                          $donation, CG_ROUNDUPS_UID, CG_ROUNDUPS_UID, t('roundup donation'))), */
/*                          new r\TxEntry(ray('amount uid agentUid description', */
/*                                          0-$donation, $payer->id, $payer->agentId, t('roundup donation')))]; */
/*     } */
/*   } else { */
/*     $roundupEntries = []; */
/*     $donation = 0; */
/*   } */
  
/*   $flags = ($offline ? (1 << B_OFFLINE) : 0) | ($short ? (1 << B_SHORT) : 0) */
/*     | ($recurs ? (1 << B_RECURS) : 0) | ($gift ? (1 << B_GIFT) : 0) */
/*     | ($funding ? (1 << B_FUNDING) : 0) | ($crumbs ? (1 << B_CRUMBS) : 0); */


/*   $tx = new r\Tx(ray('type goods actor actorAgent flags channel box created', */
/*                    TX_TRANSFER, $goods, $purchaser->id, $purchaser->agentId, $flags, $channel, $box, $created)); */
/*   $tx->addEntries($basicEntries, $couponEntries, $roundupEntries); */
/*   if (!$tx->insert()) { */
/*     return txRet('failure saving tx', $hdrInfo); */
/*   } */

/*   foreach ([$purchaserA, $sellerA] as $a) if (!$a->confirmed) $a->setBit(B_CONFIRMED); */

/*   r\notifyEntries($seller, 'new charge', $tx->entries); */

/*   return r\txRet('report tx', $args); */
/* } */


/* /\** */
/*  * Create a transaction */
/*  * call by: list ($msg, $args) = be\transfer(...); */
/*  * @param string $ttype: payment, charge, or invoice payment */
/*  * @param acct $actor: acct object for the initiating party (usually the current user) */
/*  * @param acct $other: acct object for the other party */
/*  *   OR 'ALL' (paying employees) -- currently UNUSED */
/*  * @param number $amount: the transaction amount */
/*  * @param string|array $for: description of the transaction's purpose (user input) (or [actorFor, otherFor]) */
/*  * @param array $data: associative array of info about the transaction this request is reversing (if any),  */
/*  *   indexed by any of: */
/*  *   'goods': 0=for USD, 1=for real goods and services, 2=other */
/*  *   'created': time of transaction (defaults to current time) */
/*  *   'force': <do the transaction despite any shortfall (1 for original tx, -1 for forced reversal)> */
/*  *   'box': the device on which the transaction was initiated */
/*  *   'offline' */
/*  *   'investment' */
/*  *   'crumbs', 'recurs', 'gift': gift info */
/*  * @return simple array:  */
/*  *    index: index to result message string (if success, the index begins with "report ") */
/*  *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE) */
/*  * */
/*  * The following used to be possible keys for $data, but not any more: */
/*  *   'inv': record number of the invoice for which this is payment */
/*  *   'isGift': UNUSED <the payment is a gift to a community or nonprofit> */
/*  *   'roundups' */
/*  *   'roundups': the payment is change being donated to the community */
/*  *   'rebate': the rebate amount for this request (should be negative the original bonus amount) */
/*  *   'bonus': the bonus amount (should be negative the original rebate amount) */
/*  *   'undoes': transaction id of the related transaction (transaction being undone) */
/*  * */
/*  *   'short': ? */
/*  *   'funding': ? */
/*  * */
/*  * Do something different, based on these boolean values:  */
/*  *   $taking (charge vs. payment),  */
/*  *   $oked UNUSED (user confirmed the transaction request), */
/*  *   $goods (the transaction is an exchange for real stuff), */
/*  *   $shortfall (how inadequate the payer's balance is) */
/*  *\/ */
/* function transfer($ttype, $actor, $other, $amount, $for, $data = []) { */
/*   global $scanned, $channel; */

/*   u\EXPECT($ttype == 'charge' or $ttype == 'payment', "ttype must be 'charge' or 'payment'"); */
/*   u\EXPECT(compact(ray('actor other amount for data')), 'acct acct number string|array assoc'); */
/*   extract(u\exactly('goods created force box', $data, [null, r\rTime(), FOR_GOODS, null])); */
/*   extract(u\exactly('offline investment crumbs recurs gift', $data, FALSE)); */

/*   if (is_null($for)) { $for = ray(R_WHYS)[$goods]; } */
/*   if (is_array($for)) { */
/*     $actorFor = $for[0]; */
/*     $otherFor = $for[1]; */
/*   } else { */
/*     $actorFor = $for; */
/*     $otherFor = $for; */
/*   } */
/*   if ($force == 1) $offline = TRUE; // temporary until app handles this right */
  
/*   $erInfo = compact(ray('ttype amount goods for force')); */
/*   $error = u\badAmount($amount); */
/*   if ($error) return r\txRet($error, $erInfo); */
/*   $amount = round($amount, 2); // ignore fractions of cents */
/*   if ($amount == 0) { */
/*     r\tellAdmin('zero transaction', $erInfo + ['stack' => trace()]); */
/*     return r\txRet('zero transaction', $erInfo); */
/*   } */

/*   $DBTX = db_transaction(); */

/*   if (!$force and r\dupTx($otherId, $actorId, $taking ? $amount : -$amount)) { */
/*     $DBTX->rollback(); */
/*     return r\txRet('duplicate transaction', $erInfo); */
/*   } */

/*   // Check permissions */
/*   $err = r\txPermErr($actor, $other, $ttype == 'charge', $amount < 0); */
/*   if ($err) { */
/*     list ($erMsg, $erSubs) = $err; */
/*     if ($force) { */
/*       $msg = 'forced without perm'; */
/*       $erInfo += ['date' => u\fmtDate($created)]; */
/*       $erInfo += ['agent1' => $actor->proSe ? t('self') : $actor->fullName,  */
/*                   'agent2' => $other->proSe ? t('self') : $other->fullName]; */
/*       if (!$actor->proSe) r\message($actor->id, $msg, $erInfo); */
/*       if (!$other->proSe) r\message($other->id, $msg, $erInfo); */
/*       r\tellAdmin($msg . ' ' . t($erMsg, $erSubs), $erInfo + $erSubs); */
/*     } else { */
/*       $DBTX->rollback(); */
/*       return r\txRet($erMsg, $erInfo + $erSubs); */
/*     } */
/*   } */
  
/*   foreach ([$actor, $other] as $a) { */
/*     if (!$a->co and !($channel == TX_POS ? $a->agentA->ided : $a->ided)) { */
/*       if ($force) { */
/*         r\tellAdmin('forced without photoId', $erInfo);  */
/*       } /\* elseif ($payeeA->id != CGID) { // and !$payeeA->coCan(CO_FAST) *\/ */
/*       /\* //      r\tellAdmin('tx without photoId', $erInfo);  *\/ */
/*       /\* //      return r\txRet($a == $actorA ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $otherA->fullName]); *\/ */
/*       /\*     } *\/ */
/*     } */
/*   } */

/*   list($purchaser, $buyer) = ($ttype == 'charge') ? [$actor, $other] : [$other, $actor]; */
/*   list($loser, $gainer) = ($amount > 0) ? [$buyer, $seller] : [$seller, $buyer]; */

/*   /\* /\\* $erInfo = compact(ray('ttype amount goods for force')) *\\/ *\/ */
/*   /\* /\\*   + ray('actorA otherA op', $actorA->fullName, $otherA->fullName, $taking ? t('charged') : t('paid')); *\\/ *\/ */

/*   /\* list ($actorId, $otherId) = array($actorA->id, $otherA->id); *\/ */
  
/*   /\* $taking = ($ttype == 'charge'); *\/ */
/*   /\* $reallyTaking = ($taking xor $amount < 0); // not taking if charging a negative amount *\/ */

/*   /\* $actorEntry = new r\TxEntry(ray('amount uid agentUid description acctTid', *\/ */
/*   /\*                                  $actorAmt, $actorA->id, $actor->agentId, $actorFor, $actorTid)); *\/ */
/*   /\* $otherEntry = new r\TxEntry(ray('amount uid agentUid description acctTid', *\/ */
/*   /\*                               $otherAmt, $otherA->id, $otherA->agentId, $otherFor, $otherTid)); *\/ */

/*   /\* if ($actorAmt > 0) { *\/ */
/*   /\*   $payerInfo = $otherInfo; *\/ */
/*   /\*   $payeeInfo = $actorInfo; *\/ */
/*   /\*   $payerA = $otherA; *\/ */
/*   /\*   $payeeA = $actorA; *\/ */
/*   /\* } else { *\/ */
/*   /\*   $payerInfo = $actorInfo; *\/ */
/*   /\*   $payeeInfo = $otherInfo; *\/ */
/*   /\*   $payerA = $actorA; *\/ */
/*   /\*   $payeeA = $otherA; *\/ */
/*   /\* } *\/ */

/*   /\* list ($payerA, $payeeA) = u\order($reallyTaking, $otherA, $actorA); *\/ */
/*   /\* $erInfo += ['payerId' => $payerA->id]; *\/ */
/*   /\* list ($payer, $payee) = u\order($taking, $otherId, $actorId); // Analyze who's buying or selling *\/ */
/*   /\* list ($payerAgent, $payeeAgent) = u\order($taking, $otherA->agentId, $actorA->agentId); *\/ */

/*   /\* $payeeAmt = $amount; *\/ */
/*   /\* $payerAmt = $amount; *\/ */

/*   /\* $payerUids = $payerA->jid != 0 ? [$payerA->id, $payerA->jid] : [$payerA->id]; *\/ */
/*   /\* $payeeUids = $payeeA->jid != 0 ? [$payeeA->id, $payeeA->jid] : [$payeeA->id]; *\/ */
/*   /\* $payerTid = 1 + intval(maxTid($payerUids)); *\/ */
/*   /\* $payeeTid = 1 + intval(maxTid($payeeUids)); *\/ */

/*   /\* list ($sellerE, $buyerE) = ($actorAmt > 0) ? compact($actorEntry, $otherEntry) : compact($otherEntry, $actorEntry); *\/ */

/*   // Deal with coupons */
/*   if ($goods == FOR_GOODS and $amount > 0) { */
/*     list($couponEntries, $rebate) = handleCoupon($buyer, $seller, $amount, $buyerTid, $sellerTid); */
/*   } else { */
/*     $couponEntries = []; */
/*     $rebate = 0; */
/*   } */

/*   // Check for balance shortfall */
/*   if ($loser->uid > 0) {  // if it's a community, don't check; they don't have shortfalls? */
/*     $shortfall = $loser->shortfall($goods, $amount-$rebate, TRUE, $gainer); */
/*     if ($shortfall != 0) { */
/*       $msg = ($actor == $gainer) ? ($actor->can(B_SECRET) ? 'short from vague' : 'short from') : 'short to'; */
/*       $loser->suggestAuto(); */
/*       $details = t('|%payer was short by %short trying to pay %payee %amount (available balance is %avail)', */
/*                    'payer short payee amount avail', */
/*                    $loser->fullName, u\fmtAmt($shortfall), $gainer->fullName, u\fmtAmt($needed), */
/*                    u\fmtAmt($needed - $shortfall)); */
/*       $DBTX->rollback(); */
/*       return r\txRet($msg, ray('otherName short details avail', */
/*                                $other->fullName, u\fmtAmt($shortfall), $details, u\fmtAmt($needed-$shortfall))); */
/*     } */
/*   } */

/*   // Roundup donations?  For buyer only, and don't round up recurring donations to CGID less than $1. */
/*   $roundup = (r\acct($payer)->roundup and $payerAmt < 0 and !($payee == CGID and $recurs and $payerAmt < 1)); */
/*   $roundupInfo = null; */
/*   if ($roundup) { */
/*     $cents = round(fmod($payerAmt, 1), 2); */
/*     if ($cents > 0) { */
/*       $donation = 1 - $cents; */
/*       $payerAmt += $donation; */
/*       $roundupEntries = [new r\TxEntry(ray('amount uid agentUid description', */
/*                                          $donation, CG_ROUNDUPS_UID, CG_ROUNDUPS_UID, t('roundup donation'))), */
/*                          new r\TxEntry(ray('amount uid agentUid description', */
/*                                          0-$donation, $payer->id, $payer->agentId, t('roundup donation')))]; */
/*     } */
/*   } else { */
/*     $roundupEntries = []; */
/*   } */
  
/*   $flags = ($offline ? (1 << B_OFFLINE) : 0) | ($short ? (1 << B_SHORT) : 0) */
/*     | ($recurs ? (1 << B_RECURS) : 0) | ($gift ? (1 << B_GIFT) : 0) */
/*     | ($funding ? (1 << B_FUNDING) : 0) | ($crumbs ? (1 << B_CRUMBS) : 0); */


/*     $tx = new r\Tx(ray('type goods actor actorAgent flags channel box created', */
/*                    TX_TRANSFER, $goods, $actor->id, $actor->agentId, $flags, $channel, $box, $created)); */

/*     $xid = db\insert('r_tx_hdrs', $hdrInfo, 'xid'); */
/*   if (!$xid) { return txRet('failure saving tx', $hdrInfo); } */
/*   $payerInfo['xid'] = $xid; */
/*   if (!db\insert('r_entries', $payerInfo, 'id')) { return txRet('failure saving tx', $payerInfo); } */
/*   $payeeInfo['xid'] = $xid; */
/*   if (!db\insert('r_entries', $payeeInfo, 'id')) { return txRet('failure saving tx', $payeeInfo); } */
/*   if (!is_null($couponInfo)) { */
/*     $couponInfo['xid'] = $xid; */
/*     if (!db\insert('r_entries', $couponInfo, 'id')) { return txRet('failure saving tx', $couponInfo); } */
/*   } */
/*   if (!is_null($roundupInfo)) { */
/*     $roundupInfo['xid'] = $xid; */
/*     if (!db\insert('r_entries', $roundupInfo, 'id')) { return txRet('failure saving tx', $roundupInfo); } */
/*   } */
/*   updateBalance($payerInfo); */
/*   updateBalance($payeeInfo); */
/*   updateBalance($couponInfo); */
/*   updateBalance($roundupInfo); */

/*   unset($DBTX); */
  
/*   foreach ([$actorA, $otherA] as $a) if (!$a->confirmed) $a->setBit(B_CONFIRMED); */

/*   // Report results */
/*  /\*   *   $taking (charge vs. payment),  *\/ */
/*  /\* *   $oked UNUSED (user confirmed the transaction request), *\/ */
/*  /\* *   $goods (the transaction is an exchange for real stuff), *\/ */
/*  /\* *   $shortfall (how inadequate the payer's balance is) *\/ */
  
/*   /\* Notification depends on a number of factors: */
/*    * ttype: can be 'charge' or 'payment' */
/*    * amount: can be positive or negative */
/*    * goods: can be FOR_GOODS, FOR_USD, FOR_NONGOODS, FOR_SHARE */
/*    * shortfall: if there is one */
/*    * */
/*    * ttype = charge (taking is true) */
/*    *   amount > 0 : this is the actor charging the other, (reallyTaking is true) (other charged) */
/*    *     other is the payer, actor is the payee */
/*    *   amount < 0 : (reallyTaking is false) */
/*    *       actor is the payer, other is the payee */
/*    *     goods == FOR_GOODS : this is the actor refunding the other (other given refund) */
/*    *     goods <> FOR_GOODS : this is the actor making a payment to the other (other credited) */
/*    * ttype = payment (taking is false) */
/*    *   amount > 0 : (reallyTaking is false) */
/*    *       actor is the payer, other is the payee */
/*    *     goods == FOR_GOODS : this is the actor buying goods (other being paid) */
/*    *     goods <> FOR_GOODS : this is the actor making a payment to the other (other receiving payment) */
/*    *       otherA is company : new payment linked */
/*    *       otherA not company : just a payment */
/*    *   amount < 0 : this is the actor charging the other (reallyTaking is true) -- should never happen? */
/*    *     other is the payer, actor is the payee */
/*    *\/ */

/*   r\notify($otherId, [new charge, new refund, new payment, new payment linked], $args); */
/*   index = new charge, subs = %otherName, %amount, %payerPurpose */
/*     index = new refund, subs = ditto */
/*     index = new payment, subs = %payeePurpose */
/*     index = new payment linked, subs = %aPayLink, %otherName, %amount, %payeePurpose */

    
/*   /\* $args = $tx->reportArgs($mya, TRUE, $shortfall); *\/ */
/*   $args = ray('created created0 xid role why payerPurpose payeePurpose toMe amount tofrom balance otherUid myName otherName short success', u\fmtDate($created), $created, $xid, '?', ray(R_WHYS)[$goods], $payerInfo['description'], $payeeInfo['description'], '?', $amount, '?', '?', $otherA->id, $actorA->fullName, $otherA->fullName, $short, '?'); */

/*   // notify other party by email about any completed transaction */
/*   $notice = $reallyTaking ? 'new charge' : (($amount < 0 and $goods == FOR_GOODS) ? 'new refund' : 'new payment'); */
/*   if ($notice == 'new payment' and $otherA->co) { // not "new charge" (the most common); payment at payer's initiative */
/*     $notice = 'new payment linked'; */
/*     $args['_aPayLink'] = $otherA->makeDo('addr', $actorId, 0); // link to address, so merchant can ship or nonprofit can thank */
/*   } */
/*   $args['amount'] = ($otherId == $payee) ? $payeeAmt : -$payerAmt; */
/*   r\notify($otherId, $notice, $args); */
/*   $msg = 'report tx'; */

/* //  if (isPRODUCTION) r\tellAdmin('tx', compact('msg') + $args); // a backup record of all txs (until the backup server works) */
/*   return r\txRet($msg, $args); */
/* } */

/* /\** */
/*  * Create a transaction */
/*  * call by: list ($msg, $args) = be\transfer(...); */
/*  * @param string $ttype: payment, charge, or invoice payment */
/*  * @param acct $actor: acct object for the initiating party (usually the current user) */
/*  * @param acct $other: acct object for the other party */
/*  *   OR 'ALL' (paying employees) -- currently UNUSED */
/*  * @param number $amount: the transaction amount */
/*  * @param string|array $for: description of the transaction's purpose (user input) (or [actorFor, otherFor]) */
/*  * @param array $data: associative array of info about the transaction this request is reversing (if any),  */
/*  *   indexed by any of: */
/*  *   'goods': 0=for USD, 1=for real goods and services, 2=other */
/*  *   'created': time of transaction (defaults to current time) */
/*  *   'force': <do the transaction despite any shortfall (1 for original tx, -1 for forced reversal)> */
/*  *   'box': the device on which the transaction was initiated */
/*  *   'offline' */
/*  *   'investment' */
/*  *   'crumbs', 'recurs', 'gift': gift info */
/*  * @return simple array:  */
/*  *    index: index to result message string (if success, the index begins with "report ") */
/*  *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE) */
/*  * */
/*  * The following used to be possible keys for $data, but not any more: */
/*  *   'inv': record number of the invoice for which this is payment */
/*  *   'isGift': UNUSED <the payment is a gift to a community or nonprofit> */
/*  *   'roundups' */
/*  *   'roundups': the payment is change being donated to the community */
/*  *   'rebate': the rebate amount for this request (should be negative the original bonus amount) */
/*  *   'bonus': the bonus amount (should be negative the original rebate amount) */
/*  *   'undoes': transaction id of the related transaction (transaction being undone) */
/*  * */
/*  *   'short': ? */
/*  *   'funding': ? */
/*  * */
/*  * Do something different, based on these boolean values:  */
/*  *   $taking (charge vs. payment),  */
/*  *   $oked UNUSED (user confirmed the transaction request), */
/*  *   $goods (the transaction is an exchange for real stuff), */
/*  *   $shortfall (how inadequate the payer's balance is) */
/*  *\/ */
/* function transfer($ttype, $actor, $other, $amount, $for, $data = []) { */
/*   global $scanned, $channel; */

/*   u\EXPECT($ttype == 'charge' or $ttype == 'payment', "ttype must be 'charge' or 'payment'"); */
/*   u\EXPECT(compact(ray('actor other amount for data')), 'acct acct number string|array assoc'); */
/*   extract(u\exactly('goods created force box', $data, [null, r\rTime(), FOR_GOODS, null])); */
/*   extract(u\exactly('offline investment crumbs recurs gift', $data, FALSE)); */

/*   if (is_null($for)) { $for = ray(R_WHYS)[$goods]; } */
/*   if (is_array($for)) { */
/*     $actorFor = $for[0]; */
/*     $otherFor = $for[1]; */
/*   } else { */
/*     $actorFor = $for; */
/*     $otherFor = $for; */
/*   } */
/*   if ($force == 1) $offline = TRUE; // temporary until app handles this right */
  
/*   $erInfo = compact(ray('ttype amount goods for force')); */
/*   $error = u\badAmount($amount); */
/*   if ($error) return r\txRet($error, $erInfo); */
/*   $amount = round($amount, 2); // ignore fractions of cents */
/*   if ($amount == 0) { */
/*     r\tellAdmin('zero transaction', $erInfo + ['stack' => trace()]); */
/*     return r\txRet('zero transaction', $erInfo); */
/*   } */

/*   $DBTX = db_transaction(); */

/*   if (!$force and r\dupTx($otherId, $actorId, $taking ? $amount : -$amount)) { */
/*     $DBTX->rollback(); */
/*     return r\txRet('duplicate transaction', $erInfo); */
/*   } */

/*   // Check permissions */
/*   $err = r\txPermErr($actor, $other, $ttype == 'charge', $amount < 0); */
/*   if ($err) { */
/*     list ($erMsg, $erSubs) = $err; */
/*     if ($force) { */
/*       $msg = 'forced without perm'; */
/*       $erInfo += ['date' => u\fmtDate($created)]; */
/*       $erInfo += ['agent1' => $actor->proSe ? t('self') : $actor->fullName,  */
/*                   'agent2' => $other->proSe ? t('self') : $other->fullName]; */
/*       if (!$actor->proSe) r\message($actor->id, $msg, $erInfo); */
/*       if (!$other->proSe) r\message($other->id, $msg, $erInfo); */
/*       r\tellAdmin($msg . ' ' . t($erMsg, $erSubs), $erInfo + $erSubs); */
/*     } else { */
/*       $DBTX->rollback(); */
/*       return r\txRet($erMsg, $erInfo + $erSubs); */
/*     } */
/*   } */
  
/*   foreach ([$actor, $other] as $a) { */
/*     if (!$a->co and !($channel == TX_POS ? $a->agentA->ided : $a->ided)) { */
/*       if ($force) { */
/*         r\tellAdmin('forced without photoId', $erInfo);  */
/*       } /\* elseif ($payeeA->id != CGID) { // and !$payeeA->coCan(CO_FAST) *\/ */
/*       /\* //      r\tellAdmin('tx without photoId', $erInfo);  *\/ */
/*       /\* //      return r\txRet($a == $actorA ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $otherA->fullName]); *\/ */
/*       /\*     } *\/ */
/*     } */
/*   } */

/*   list($seller, $buyer) = ($ttype == 'charge') ? [$actor, $other] : [$other, $actor]; */
/*   list($loser, $gainer) = ($amount > 0) ? [$buyer, $seller] : [$seller, $buyer]; */

/*   /\* /\\* $erInfo = compact(ray('ttype amount goods for force')) *\\/ *\/ */
/*   /\* /\\*   + ray('actorA otherA op', $actorA->fullName, $otherA->fullName, $taking ? t('charged') : t('paid')); *\\/ *\/ */

/*   /\* list ($actorId, $otherId) = array($actorA->id, $otherA->id); *\/ */
  
/*   /\* $taking = ($ttype == 'charge'); *\/ */
/*   /\* $reallyTaking = ($taking xor $amount < 0); // not taking if charging a negative amount *\/ */

/*   /\* $actorEntry = new r\TxEntry(ray('amount uid agentUid description acctTid', *\/ */
/*   /\*                                  $actorAmt, $actorA->id, $actor->agentId, $actorFor, $actorTid)); *\/ */
/*   /\* $otherEntry = new r\TxEntry(ray('amount uid agentUid description acctTid', *\/ */
/*   /\*                               $otherAmt, $otherA->id, $otherA->agentId, $otherFor, $otherTid)); *\/ */

/*   /\* if ($actorAmt > 0) { *\/ */
/*   /\*   $payerInfo = $otherInfo; *\/ */
/*   /\*   $payeeInfo = $actorInfo; *\/ */
/*   /\*   $payerA = $otherA; *\/ */
/*   /\*   $payeeA = $actorA; *\/ */
/*   /\* } else { *\/ */
/*   /\*   $payerInfo = $actorInfo; *\/ */
/*   /\*   $payeeInfo = $otherInfo; *\/ */
/*   /\*   $payerA = $actorA; *\/ */
/*   /\*   $payeeA = $otherA; *\/ */
/*   /\* } *\/ */

/*   /\* list ($payerA, $payeeA) = u\order($reallyTaking, $otherA, $actorA); *\/ */
/*   /\* $erInfo += ['payerId' => $payerA->id]; *\/ */
/*   /\* list ($payer, $payee) = u\order($taking, $otherId, $actorId); // Analyze who's buying or selling *\/ */
/*   /\* list ($payerAgent, $payeeAgent) = u\order($taking, $otherA->agentId, $actorA->agentId); *\/ */

/*   /\* $payeeAmt = $amount; *\/ */
/*   /\* $payerAmt = $amount; *\/ */

/*   /\* $payerUids = $payerA->jid != 0 ? [$payerA->id, $payerA->jid] : [$payerA->id]; *\/ */
/*   /\* $payeeUids = $payeeA->jid != 0 ? [$payeeA->id, $payeeA->jid] : [$payeeA->id]; *\/ */
/*   /\* $payerTid = 1 + intval(maxTid($payerUids)); *\/ */
/*   /\* $payeeTid = 1 + intval(maxTid($payeeUids)); *\/ */

/*   /\* list ($sellerE, $buyerE) = ($actorAmt > 0) ? compact($actorEntry, $otherEntry) : compact($otherEntry, $actorEntry); *\/ */

/*   // Deal with coupons */
/*   if ($goods == FOR_GOODS and $amount > 0) { */
/*     list($couponEntries, $rebate) = handleCoupon($buyer, $seller, $amount, $buyerTid, $sellerTid); */
/*   } else { */
/*     $couponEntries = []; */
/*     $rebate = 0; */
/*   } */

/*   // Check for balance shortfall */
/*   if ($loser->uid > 0) {  // if it's a community, don't check; they don't have shortfalls? */
/*     $shortfall = $loser->shortfall($goods, $amount-$rebate, TRUE, $gainer); */
/*     if ($shortfall != 0) { */
/*       $msg = ($actor == $gainer) ? ($actor->can(B_SECRET) ? 'short from vague' : 'short from') : 'short to'; */
/*       $loser->suggestAuto(); */
/*       $details = t('|%payer was short by %short trying to pay %payee %amount (available balance is %avail)', */
/*                    'payer short payee amount avail', */
/*                    $loser->fullName, u\fmtAmt($shortfall), $gainer->fullName, u\fmtAmt($needed), */
/*                    u\fmtAmt($needed - $shortfall)); */
/*       $DBTX->rollback(); */
/*       return r\txRet($msg, ray('otherName short details avail', */
/*                                $other->fullName, u\fmtAmt($shortfall), $details, u\fmtAmt($needed-$shortfall))); */
/*     } */
/*   } */

/*   // Roundup donations?  For buyer only, and don't round up recurring donations to CGID less than $1. */
/*   $roundup = (r\acct($payer)->roundup and $payerAmt < 0 and !($payee == CGID and $recurs and $payerAmt < 1)); */
/*   $roundupInfo = null; */
/*   if ($roundup) { */
/*     $cents = round(fmod($payerAmt, 1), 2); */
/*     if ($cents > 0) { */
/*       $donation = 1 - $cents; */
/*       $payerAmt += $donation; */
/*       $roundupEntries = [new r\TxEntry(ray('amount uid agentUid description', */
/*                                          $donation, CG_ROUNDUPS_UID, CG_ROUNDUPS_UID, t('roundup donation'))), */
/*                          new r\TxEntry(ray('amount uid agentUid description', */
/*                                          0-$donation, $payer->id, $payer->agentId, t('roundup donation')))]; */
/*     } */
/*   } else { */
/*     $roundupEntries = []; */
/*   } */
  
/*   $flags = ($offline ? (1 << B_OFFLINE) : 0) | ($short ? (1 << B_SHORT) : 0) */
/*     | ($recurs ? (1 << B_RECURS) : 0) | ($gift ? (1 << B_GIFT) : 0) */
/*     | ($funding ? (1 << B_FUNDING) : 0) | ($crumbs ? (1 << B_CRUMBS) : 0); */


/*     $tx = new r\Tx(ray('type goods actor actorAgent flags channel box created', */
/*                    TX_TRANSFER, $goods, $actor->id, $actor->agentId, $flags, $channel, $box, $created)); */

/*     $xid = db\insert('r_tx_hdrs', $hdrInfo, 'xid'); */
/*   if (!$xid) { return txRet('failure saving tx', $hdrInfo); } */
/*   $payerInfo['xid'] = $xid; */
/*   if (!db\insert('r_entries', $payerInfo, 'id')) { return txRet('failure saving tx', $payerInfo); } */
/*   $payeeInfo['xid'] = $xid; */
/*   if (!db\insert('r_entries', $payeeInfo, 'id')) { return txRet('failure saving tx', $payeeInfo); } */
/*   if (!is_null($couponInfo)) { */
/*     $couponInfo['xid'] = $xid; */
/*     if (!db\insert('r_entries', $couponInfo, 'id')) { return txRet('failure saving tx', $couponInfo); } */
/*   } */
/*   if (!is_null($roundupInfo)) { */
/*     $roundupInfo['xid'] = $xid; */
/*     if (!db\insert('r_entries', $roundupInfo, 'id')) { return txRet('failure saving tx', $roundupInfo); } */
/*   } */
/*   updateBalance($payerInfo); */
/*   updateBalance($payeeInfo); */
/*   updateBalance($couponInfo); */
/*   updateBalance($roundupInfo); */

/*   unset($DBTX); */
  
/*   foreach ([$actorA, $otherA] as $a) if (!$a->confirmed) $a->setBit(B_CONFIRMED); */

/*   // Report results */
/*  /\*   *   $taking (charge vs. payment),  *\/ */
/*  /\* *   $oked UNUSED (user confirmed the transaction request), *\/ */
/*  /\* *   $goods (the transaction is an exchange for real stuff), *\/ */
/*  /\* *   $shortfall (how inadequate the payer's balance is) *\/ */
  
/*   /\* Notification depends on a number of factors: */
/*    * ttype: can be 'charge' or 'payment' */
/*    * amount: can be positive or negative */
/*    * goods: can be FOR_GOODS, FOR_USD, FOR_NONGOODS, FOR_SHARE */
/*    * shortfall: if there is one */
/*    * */
/*    * ttype = charge (taking is true) */
/*    *   amount > 0 : this is the actor charging the other, (reallyTaking is true) (other charged) */
/*    *     other is the payer, actor is the payee */
/*    *   amount < 0 : (reallyTaking is false) */
/*    *       actor is the payer, other is the payee */
/*    *     goods == FOR_GOODS : this is the actor refunding the other (other given refund) */
/*    *     goods <> FOR_GOODS : this is the actor making a payment to the other (other credited) */
/*    * ttype = payment (taking is false) */
/*    *   amount > 0 : (reallyTaking is false) */
/*    *       actor is the payer, other is the payee */
/*    *     goods == FOR_GOODS : this is the actor buying goods (other being paid) */
/*    *     goods <> FOR_GOODS : this is the actor making a payment to the other (other receiving payment) */
/*    *       otherA is company : new payment linked */
/*    *       otherA not company : just a payment */
/*    *   amount < 0 : this is the actor charging the other (reallyTaking is true) -- should never happen? */
/*    *     other is the payer, actor is the payee */
/*    *\/ */

/*   r\notify($otherId, [new charge, new refund, new payment, new payment linked], $args); */
/*   index = new charge, subs = %otherName, %amount, %payerPurpose */
/*     index = new refund, subs = ditto */
/*     index = new payment, subs = %payeePurpose */
/*     index = new payment linked, subs = %aPayLink, %otherName, %amount, %payeePurpose */

    
/*   /\* $args = $tx->reportArgs(TRUE, $shortfall); *\/ */
/*   $args = ray('created created0 xid role why payerPurpose payeePurpose toMe amount tofrom balance otherUid myName otherName short success', u\fmtDate($created), $created, $xid, '?', ray(R_WHYS)[$goods], $payerInfo['description'], $payeeInfo['description'], '?', $amount, '?', '?', $otherA->id, $actorA->fullName, $otherA->fullName, $short, '?'); */

/*   // notify other party by email about any completed transaction */
/*   $notice = $reallyTaking ? 'new charge' : (($amount < 0 and $goods == FOR_GOODS) ? 'new refund' : 'new payment'); */
/*   if ($notice == 'new payment' and $otherA->co) { // not "new charge" (the most common); payment at payer's initiative */
/*     $notice = 'new payment linked'; */
/*     $args['_aPayLink'] = $otherA->makeDo('addr', $actorId, 0); // link to address, so merchant can ship or nonprofit can thank */
/*   } */
/*   $args['amount'] = ($otherId == $payee) ? $payeeAmt : -$payerAmt; */
/*   r\notify($otherId, $notice, $args); */
/*   $msg = 'report tx'; */

/* //  if (isPRODUCTION) r\tellAdmin('tx', compact('msg') + $args); // a backup record of all txs (until the backup server works) */
/*   return r\txRet($msg, $args); */
/* } */

/**
 * Generate transaction entries for a coupon, and update the coupated table as appropriate.
 * @param acct $buyer:
 * @param acct $seller:
 * @param number $price: pre-discount amount
 * @param string|number $buyerTid: buyer's transaction id
 * @param string|number $sellerTid: seller's transaction id
 * @param unixtime $when: when was the transaction (defaults to now)
 * @return an array of entries for the coupon -- may be empty if no coupons apply
 */
function handleCoupon($buyer, $seller, $price, $buyerTid, $sellerTid, $when=null) {
  if (is_null($when)) $when = r\rTime();
  $sql = <<<EOF
    SELECT `on`, coupid, amount, ulimit, id, uses
    FROM r_coupons c LEFT JOIN r_coupated d USING (coupid)
    WHERE d.uid=:buyerId AND fromid=:sellerId AND :now BETWEEN start AND end AND :price>=minimum AND NOT flags
          AND (ulimit=0 OR uses<ulimit)
    ORDER BY end ASC
EOF;
  $subs = ray('buyerId sellerId now price', $buyer->id, $seller->id, $when, $price);
  $res = db\q($sql, $subs);
  if (!$res) return null;
  debug($res);
  extract($res->fetchAssoc());
  $rebate = ($amount < 0) ? $price * (0-$amount) * .01 : $amount;
  if ($rebate <= 0) return [];
  if (empty($id)) {
    $id = db\insert('r_coupated', ['coupid' => $coupid, 'uid' => $buyer->id, 'uses' => 1, 'when' => time()], 'id');
  } else {
    db\update('r_coupated', ['id' => $id, 'coupid' => $coupid, 'uid' => $buyer->id, 'uses' => $uses + 1,
                             'when' => r\rTime()], 'id');
  }
  return [ [new r\TxEntry(ray('amount uid agentUid description acctTid relType related',
                            $rebate, $buyerId, $buyer->agentid,
                            t('discount rebate (on #%tid)', ['tid' => $buyerTid]),
                            $buyerTid, 'D', $coupatedId)),
            new r\TxEntry(ray('amount uid agentUid description acctTid relType related',
                            0-$rebate, $seller->id, $seller->agentid,
                            t('discount rebate (on #%tid)', ['tid' => $sellerTid]),
                            $sellerTid, 'D', $coupatedId))],
           $price - $rebate ];
}

/**
 * Check whether the payer has a shortfall in paying for this transaction.
 * @param acct $payer: account of the payer (we're checking this account for shortfall)
 * @param acct $payee: account of the payee
 * @param acct $actor: account that initiated transaction, probably the same as $payer or $payee
 * @param acct $other: the other account, probably the same as $payee or $payer
 * @param integer $goods: the type of goods being purchased (handle USD differently than real goods)
 * @param acct $needed: amount needed to pay, should be positive; (e.g., price less rebates)
 * @return FALSE if there is no shortfall, otherwise a message (created by txRet)
 */
function checkForShortfall($payer, $payee, $actor, $other, $goods, $needed, $force = FALSE) {
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
 * Update the account balance to reflect the transaction being passed to this function.
 * @param assoc $info: the data for the r_entries record
 */
function updateBalance($info) {
  return;  // not needed
  if (is_null($info)) return;
  $acct = r\acct($info['uid']);
  $amt = $info['amount'];
  $uid = $info['uid'];
  $oldbal = $acct->balance;
  $newbal = $oldbal + $amt;
  /* t\output("uid: $uid, oldbal: $oldbal, newbal: $newbal"); */
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
 * @param acct $a1: acct object for payee (usually current user)
 * @param acct $a2: acct object for payer
 * @param numeric $amount: the invoiced amount
 * @param string $purpose: description of the transaction's purpose (user input)
 * @param assoc $extra:
 *    goods: FOR_GOODS, etc.
 *    shid: record id of related record in r_shares
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE)
 */
function invoice($a1, $a2, $amount, $purpose = NULL, $extra = []) {
  global $channel;
  
  u\EXPECT(compact(ray('a1 a2 amount purpose')), 'acct acct float string');
  extract(just($ks = 'goods status', $extra));
	foreach (justNOT("goods status shid coupon coupid force inv", $extra) as $k => $v) {
    u\setBit($flags, $k, $v);
    unset($extra[$k]); // don't leave bits in extras (data field)
  }
  list ($payee, $payer) = array($a1->id, $a2->id);
  u\setDft($goods, FOR_GOODS);
  u\setDft($purpose, $why = ray(R_WHYS)[$goods]);
  $subs = ray('myName otherName otherEmail otherPhone amount why purpose did op', $a1->fullName, $a2->fullName, $a2->email, u\fmtPhone($a2->phone), u\fmtAmt($amount), $why, $purpose, t('charged'), t('charged'));
  if ($err = u\badAmount($amount, '>0')) return r\txRet($err, $subs);
  if ($err = r\txPermErr($a1, $a2, TRUE)) return r\txRet($err, $subs);
  if (r\dupTx($payee, $payer, -$amount)) return r\txRet('duplicate transaction', $subs);
  
  $created = r\rTime();
  u\setDft($status, ($channel != TX_FOREIGN and r\relation(':IS_AUTOPAY', $payee, $payer)) ? TX_APPROVED : TX_PENDING);
  $data = serialize($extra);
  $info = compact(ray('amount status payer payee goods purpose created flags data'));
  $nvid = db\insert('r_invoices', $info, 'nvid');
  $a1->update('lastTx', [$nvid, $payer - $payee, round(-$amount, 2), $created, 'inv']);

  if ($status == TX_PENDING) {
    $link = $a1->makeDo('inv', $nvid); // link to pay/deny
    //$a2->update(ray('lastTx', array($nvid, $payee - $payer, round($amount, 2), $created, 'inv')));
    $subs += ray('_a1 reply email phone', $link, $payee, $a1->email, u\fmtPhone($a1->phone));
    r\message($payer, 'new invoice', $subs); // tell other party by email about this invoice
  } else { // pre-approved!
    if (!$a2->shortfall(FOR_GOODS, $amount)) { // don't try to pay yet if short
      $ok = be\payInvoice($nvid, $msg);
      if (!$ok) r\tellAdmin(t('invoice payment failed'), compact('msg') + $subs + $info);
    }
  }

  $msg = 'report tx';
//  if ($a1->cttyRewardy) $msg .= '|for why';
  if ($amount > 0) $msg .= '|balance unchanged';
  return array($msg, $subs + ['success'=>TRUE]);
}

/**
 * Send an email invoice to a nonmember, inviting them to join.
 * @param string $name: name of person to invoice
 * @param assoc $info: assoc [email, amount, goods, purpose] -- these params may also be specified individually
 */
function invoiceNonmember($name, $info) {
  extract(just('email amount goods purpose', rayy(func_get_args(), 1)));
  
}
  
/**
 * Record a transfer (of cgCredits) to or from the community.
 * @param int $uid: uid of the account to credit or debit
 * @param int $type: what type of transfer
 * @param int $amount: how much to transfer
 * @param string $for: purpose of funding
 * @param bool $oked: TRUE actually do it? (otherwise just see if it can be done)
 * @return int: the incentive amount (FALSE on failure)
 *//*
function fund($uid, $type, $amount, $for) {
  $payeeAgent = $payee = $uid;
  global $mya; // (null, if just registering or running as cron)
  $isReward = r\isReward($type);
  u\EXPECT($isReward or @$mya->can(B_MANAGING_CTTY), 'non-automatic reward by non ctty agent');
  if ($type == TX_FINE) $amount = -$amount;
//  if (!$oked) return $amount;
  if ($isReward) list ($payerReward, $payeeReward, $amount) = [0, $amount, 0];
///    debug(compact(ray('uid type amount for isReward payeeReward')));
  
  $a = r\acct($payee);
  $payer = $a->community;
  $payerAgent = (@$mya and $mya->id < 0) ? $mya->agentId : $payer; // take note if funded by community admin
  
  if (!new r\X(compact(ray('type amount payer payee payerReward payeeReward payerAgent payeeAgent for taking')))) return FALSE;
  $types = ray(TX_TYPES);
  $thing = $isReward ? t('reward') : $types[$type];
  $subs = ray('amount purpose thing', u\fmtAmt(abs($amount)), $for, $thing);
  $index = $type == TX_FINE ? 'got fine' : 'got funding';
  if ($type == TX_INFLATION) $index .= '|inflation details';
  if ($a->cttyRewardy or !$isReward) r\notify($uid, $index, $subs); // don't report rewards unless rewarding
  return $amount;
}
*/

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
 * @param int $myid: current user's account record ID
 * @param bool $paying: <we're preparing to pay this person> (in which case the person has to have an active account)
 * @param string $self_message: index to error message for identifying oneself
 *   If true, the other trader will be created only if specified by email or phone
 * @return one of the following:
 *   the other trader's acct
 *   array(error index, array of substitutions, array of possible fullNames (0 or more) indexed by uid)
 */
function identify($who, $myid, $paying, $self_message = 'no self-trading') {
  $who = trim($who);
  $subs = compact('who');
  if (!$mya = r\acct($myid)) return ['search forbidden', [], []];
  if ($phone = u\fmtPhone($who, '+n')) {
    $who = $phone; $cphone = u\cry('P', $phone);
  } else $cphone = '-';
  
  if (!$short = u\shortName(" $who ", '%')) $short = '-'; // abbreviations must not be null or zeroes
  $cmail = strpos($who, '@') ? u\cry('P', strtolower($who)) : '-';

  $cttyCrit = $mya->admin 
  ? '1' 
  : (($paying ? ':IS_OK AND ' : '') . "(:IS_CO OR community=$mya->community)"); // restrict to own community and approved (as of 7/6/2017)
  
  $sql = <<<EOF
    SELECT DISTINCT uid,fullName
    FROM users 
    WHERE (name LIKE :short OR :who=fullName OR (!:IS_NOSEARCH AND (:cphone=phone OR :cmail=email)))
    AND $cttyCrit
    ORDER BY fullName
EOF;

// LEFT JOIN r_boxes s ON u.uid = s.uid ... OR :who=s.code (search for cell not allowed if it isn't the account's primary phone)

  $result = db\q($sql, compact(ray('who short cphone cmail')))->fetchAllKeyed(0, 1);
  
  // QID? if not, maybe first word is a QID if this is from a list showing QID and fullName
  $qid = trim(($i = strpos($who, ':')) ? substr($who, 0, $i) : strtoupper($who));
  if ($a = r\acct($qid)) $result[$a->id] = $a->fullName; // explicit QID is a result, regardless of community
  if (count($result) == 1 and @$result[$myid]) return [$self_message, [], []];
  unset($result[$myid]); // self is one of many results, so ignore it
  if (!$result) return [($mya->admin ? 'unknown member' : 'unknown ctty member') . ("$cphone$cmail" == '--' ? '' : '|no search'), $subs, []];
  if (count($result) == 1) return r\acct(key($zot = $result)); // get the one entry (zot resets the pointer)
  return ['ambiguous other', $subs, $result];
}


/**
 * Return financial information about the given account on this server
 * @param array uids: array of uids for the account to return information for
 * @param bool pending: show pending transactions (applicable only to bank transfers)
 * @param integer asof: last time to include (defaults to 24 hours from now, meaning unrestricted) this is exclusive
 * @return an assoc of results (see $resultKeys, below)
 */
function creditInfo($uids, $asof = null, $pending = FALSE) {
  /* t\output("uids is next:"); */
  /* t\output($uids); */
  u\EXPECT(is_array($uids), 'bad call to creditInfo');
  global $mya;
  
  $asof = is_null($asof) ? (r\rTime() + DAY_SECS) : $asof;

  /* $mday1 = u\monthDay1($asof); // start of relevant month, for partial (not-yet-given) roundups calculation */
  /* if ($asof == u\plusMonths(1, $mday1) - 1) $mday1 = $asof + 1; // for asof the end of month, there are no partRounds */
  $subs = compact(ray('uids asof'));
  /* $isPayer = 't.payer IN (:uid, :jid)'; */
  /* $isPayee = 't.payee IN (:uid, :jid)'; // account can be both payer and payee, if joint account */
  /* $signedAmt = "IF($isPayer, -amount, amount)"; */
//    SUM(IF(t.type IN (:TX_REWARDS), amount, 0)) AS rewards,
  
  $sql = <<<EOF
    SELECT 
    SUM(IF(amount > 0, amount, 0)) AS `in`,
    SUM(IF(amount < 0, -amount, 0)) AS `out`, 
    SUM(IF(t.goods=:FOR_SHARE, amount, 0)) AS share,
    0 AS xfees,
    0 AS xin
    FROM r_entries e LEFT JOIN r_tx_hdrs t USING (xid)
    WHERE (e.uid IN (:uids)) AND t.created<:asof
EOF;
  $fields = <<<EOF
    SUM(IF(amount > 0, amount, 0)) AS `in`,
    SUM(IF(amount < 0, -amount, 0)) AS `out`, 
    SUM(IF(t.goods=:FOR_SHARE, amount, 0)) AS share,
    0 AS xfees,
    0 AS xin
EOF;

  /* t\output($fields); */
  /* t\output($subs); */
  $sums = $pending ? ray('in out xfees xin', 0, 0, 0, 0) : db\q($sql, $subs)->fetchAssoc();
  /* $sums = $pending ? ray('in out xfees xin', 0, 0, 0, 0) : db\q($sql, $subs)->fetchAssoc(); */
  /* print_r($sums); */
  /* t\output("asof $asof, be\creditInfo sums: "); */
  /* t\output($sums); */
  if ($pending) {
    $where = '';
    $sql = "SELECT SUM(amount) AS bank FROM r_usd t WHERE (payee IN (:uids)) AND created<:asof AND completed <= 0";
    $sums += db\q($sql, $subs)->fetchAssoc();
  } else {
    $sums['bank'] = 0;
  }

  foreach ($sums as $k => $v) if (is_null($v)) $sums[$k] = 0; // no nulls
  extract($sums);

  $balance = ($in - $out) + $bank;
  $outProper = $out;
  $inProper = $in;
  
  $resultKeys = 'in out refunds fees xfees xin inProper outProper balance r bank';
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
  } elseif ($mya->cAdmin) {
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

function accountName($uid) {
  $acct = r\acct($uid);
  return $acct->fullName . ' ' . u\hug($acct->mainQid);
}
