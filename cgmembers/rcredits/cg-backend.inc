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
  $special = be\specialTxFlds();
  // tReversed and eReversed refer to the transaction that this transaction reverses.
  // tReversedBy and eReversedBy refer to the transaction that reverses this transaction.
  // !(t.amt=0 AND t.type=:E_BANK) hides bank account verifications
  $sql = <<< EOF
    SELECT t.*, t2.pid, u.uid, u.fullName AS name, tm.period, tm.periods, ru.portion, $toMe AS toMe,
      IF($toMe, amt, -amt) AS amount, IF($toMe, for1, for2) AS description, IF($toMe, agt1, agt2) AS agt, $special,
      (ru.action=:ACT_SURTX AND IF($toMe, ru.to=t.uid2, ru.from IN (:MATCH_PAYER, t.uid1))) AS stepup,
      (SELECT xid FROM tx_hdrs tRev WHERE tRev.reversesXid=t.xid) AS reversedBy,
      (EXISTS (SELECT id FROM tx_disputes d WHERE d.xid=t.xid AND d.status=:DS_OPEN)) AS isDisputed
    FROM txs t LEFT JOIN users u ON u.uid=IF($toMe, t.uid1, t.uid2)
    LEFT JOIN txs2 t2 ON t2.xid=t.xid
    LEFT JOIN tx_timed tm ON tm.id=t.recursId
    LEFT JOIN tx_rules ru ON ru.id=t.rule
    LEFT JOIN tx_requests i ON i.nvid=t.rel AND t.relType='I'
    WHERE (!(t.amt=0 AND t.type=:E_BANK) AND ($fromMe XOR $toMe)) $where
    ORDER BY t.created, t.xid, t.eid, name
EOF;
  /// showq($sql, ray('uids', $uids[0]));
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
    extract(just('actorId type xid pid', $entries[0]));
    $byMe = in_array($actorId, $uids);
    $banking = ($type == E_BANK);
    $newEntries = [];
    $myAmount = 0; foreach ($entries as $entry) $myAmount += $entry['amount'];
    $balance = round($balance + $myAmount, 2);
    if ($xid < 124873 and $banking and !isDEV) $xid = db\get('txid', 'txs2_bank', compact('xid')) ?: $xid; // old checks
    
    foreach ($entries as $j => $entry) {
      if ($j == 0) {
        if ($banking) $entry['name'] = '--';
        if ($pid) $entry['name'] = db\get('fullName', 'people', compact('pid')) . t(' (non-member)');
      }
      $description = txDesc($entry);
      $newEntries[] = compact(ray('myAmount balance byMe banking xid description')) + $entry;
    }
    $txs[$i] = $newEntries;
  }

  return $txs;
}

/**
 * Return SQL for nvid, part, and stepup in a tx entry.
 * Don't include invoice number for payApproved requests
 * Expects t=txs, i=tx_requests
 */
function specialTxFlds() {
  list ($finalTxt, $partTxt) = [t('final'), t('partial')];
  return <<< X
    IF(t.relType='I' AND NOT i.:IS_SELF, t.rel, NULL) AS nvid, 
    IF(t.:IS_PARTIAL, IF(t.xid=i.status, ' $finalTxt', ' $partTxt'), '') AS part
X;
}

/**
 * Return a (modified) description for the given transaction entry.
 */
function txDesc($entry) {
  extract(just('type reversesXid reversedBy description period periods stepup portion nvid part', $entry, NULL));

  if (in($type, [E_PRIME, E_BANK])) {
    if ($period) $description .= t(' (%xly)', 'xly', r\recurDesc($period, $periods));
    if ($nvid) $description .= t(' (CG inv#%nvid%part)', compact(ray('nvid part')));
    if ($reversesXid) $description = t('(reverses tx #%reversesXid)', compact('reversesXid'));
    if ($reversedBy) $description .= t(' (reversed by #%reversedBy)', compact('reversedBy'));
  } else {
    if ($stepup) $description .= $portion > 0 ? t(' (%x step-up)', 'x', u\fmtAmt($portion, '%s')) : t(' (step-up)');
  }
  return $description;
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
  $res = identify('', $a, '', $restrict);
  return is_array($res) ? $res : (is_object($res) ? [$res->id => $res->fullName] : []);
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
    $where = "uid2=:payee AND t.created BETWEEN $start AND $end AND type<>:E_BANK";

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
 * @return a txRet object
 */
function undoTx($xid, $where = '1', $subs = [], $force = FALSE) {
  global $mya, $channel;

  u\EXPECT('int string assoc bool', $xid, $where, $subs, $force);

  $xidSub = compact('xid');
  $tx = $mya->lastTx("xid=:xid AND $where", $xidSub + $subs);
  if (empty($tx)) return txRet(FALSE, 'undo no match', $subs);
  if (!$force and $zErr = $tx->noUndo($subs)) return $zErr;

  if (db\exists('tx_hdrs', 'reversesXid=:xid', $xidSub)) return txErr(t('already reversed', $xidSub), $subs);
  if (db\exists('tx_requests', 'reversesXid=:xid AND status NOT IN (:TX_DENIED)', $xidSub)) return txErr(t('already reverse requested', $xidSub), $subs);
  $otherE = $tx->otherEntry;
  $byMe = $mya->isMe($tx->actorId);
  $toMe = $tx->getAmount($mya) > 0;
  $posUndo = ($channel == TX_APP and $tx->created >= now() - REVERSE_WINDOW);

  if ($toMe or $posUndo) { // someone paid me (at my initiative or theirs)
    return reverse($xid); // create an offsetting transaction
  } elseif ($byMe) { // I paid someone (on my initiative)
///    debug("other=$tx->otherId actor=$tx->actorId amt=$otherE->amount posUndo=$posUndo toMe=$toMe");
    return invoice($tx->actorA, $tx->otherA, $otherE->amount, $otherE->description, $tx->goods, ray('reversesXid', $xid), $tx->recursId);
  } else { // !$toMe and !$byMe: someone charged me (either unilaterally or I approved it)
    if (db\exists('tx_disputes', compact('xid'))) return txErr(t('already reverse requested', $xidSub), $subs);
    $result = db\insert('tx_disputes', ray('xid reason status uid agentUid', $xid, nn($reason) ?: t('reason not specified'), DS_OPEN, $mya->uid, $mya->agentId), 'id');
  
    $solution = t('marked "disputed"');
    $tx->otherA->tellOut('charge disputed', $tx->reportArgs($mya)); //, $tx->getMyAmt(), $payer, $payee, $payerPurpose, $payeePurpose));
    $amount = u\fmtAmt($otherE->amount);
    $tofrom = !$tx->toMe ? t('to') : t('from');
    $otherUid = $tx->otherUid;
    $other = $tx->otherA->fullName;
    return txRet(TRUE, 'report undo', compact(ray('amount other tofrom solution otherUid')));
  }
}

/**
 * Create an offsetting transaction (nearly the same as the original but with the entries negated).
 * @param int $xid: record ID of the transaction to reverse
 * @returns object $txRet: result, message, and subs
 */
function reverse($xid) {
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
  foreach ($tx->entries as $oldE) {
    $newE = clone $oldE;
    $newE->id = NULL;
    $newE->xid = NULL;  // new entry belongs to reversing transaction
    $newE->amount = -$oldE->amount;
    $newTx->entries[] = $newE;
  }
  $xid = $newTx->insert();
  
  unset($DBTX);

  sendNotifications('undo', $newTx, 0);

  $message = 'report undo' . ($tx->otherA->isCanonic ? '' : '|report tx');

  $ok = TRUE;
  $did = ($e->amount < 0) ? ($tx->goods==FOR_GOODS ? 'refunded' : 'credited') : 're-charged';
  $txid = $xid; // txid is xid in transaction reports
  $created = $newTx->created;
  $balance = $tx->otherA->can(B_SECRET) ? '*0' : $tx->otherA->balance;
  $solution = 'reversed';
  $otherName = $tx->otherA->fullName;
  $amount = u\fmtAmt(abs($e->amount));
  $why = $e->description;
  $subs = compact(ray('ok did xid txid created balance solution otherName amount why'));
  
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
 * @return a txRet object
 */
function payInvoice($nvid, $payAmount = NULL) {
  u\EXPECT('int', $nvid);
  if ($err = r\badUnpaidInv($nvid, $inv)) return txRet(FALSE, $err);

  extract(just('payer payee purpose recursId', $inv, NULL));
  extract(just('amount goods data flags', $inv, [0.0, FOR_GOODS, '', 0]));

  u\setDft($payAmount, $amount);
  $payerA = r\acct($payer);
  $payeeA = r\acct($payee);
  $data = unserialize($data);
  extract(just('api request shid for', $data, NULL));

  $DBTX = \db_transaction();
  $extra = ray('inv', $nvid) + just('api for cat', $data, NULL);
  if ($payAmount < $amount) $extra['partial'] = 1;
  if ($flags) foreach (ray('gift crumbs recurs investment') as $k) if (u\getBit($flags, $k)) $extra[$k] = 1;

  $z = be\transfer('invoice payment', $payerA, $payeeA, $payAmount, $purpose, $goods, $extra, $recursId);
  if ($z->ok) {
    if ($payAmount == $amount or !invRemains($nvid)) { // == is for efficiency
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
    if ($shortfall = nni($z->subs, 'shortfall')) {
      $index = 'short invoice' . ($payerA->hasBank ? '|expect a transfer' : '|when funded|how to fund');
      $subs = ray('short payeeName nvid', u\fmtAmt($shortfall), $payeeA->fullName, $nvid);
      $payerA->tellOut($index, $subs);
      if ($payerA->hasBank) $payerA->getFunds($shortfall, t('for invoice', compact('nvid')));

      $z->index = $index; // replace short payment message with something more explicit about the invoice
      $z->msg = tr($index, $subs);
      u\preray($subs, $z->subs);
    }
    db\update('tx_requests', ray('nvid status', $nvid, TX_APPROVED), 'nvid'); // in case it wasn't marked approved already
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
 *   'investment'
 *   'stake'
 *   'fine'
 *   'noask'
 *   'funding'
 *   'button': paid from a CGPay button on a company website
 *   'crumbs', 'recurs', 'gift': gift info
 *   'ruleId': the id of the rule associated with this transaction
 *   'for': SELF_CREDIT, SELF_CREDIT50, gift, donate, other (or empty)
 *   'credit': amount of credit, if for=SELF_CREDIT or SELF_CREDIT50
 *   'cat': transaction category
 * @param int $recursId: record ID of related record in tx_timed (if any)
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
  $taking = ($type == 'charge' or $type == 'draw' or ($type == 'transfer' and $amount < 0));
  
  extract(just('created force boxId ruleId cat', $data, [now(), 0, NULL, NULL, NULL, NULL]));
  extract(just(TX_FLAGS . ' inv api for credit', $data, FALSE));
  
  u\setDft($purpose, ray(R_WHYS)[$goods]);

  if ($force != 0) $offline = TRUE; // temporary until app handles this right

  $erInfo = compact(ray('goods amount purpose force')) + ray('actor other op snap', $actorA->fullName, $otherA->fullName, $taking ? t('charged') : t('paid'), $actorA->snap or $otherA->snap);
  if ($amount <= 0) r\tellAdmin(t('non-positive amount in be\\transfer'), $erInfo); // should never happen


  foreach ([$actorA, $otherA] as $a) if ($a->snap) r\tellAdmin(t('SNAP member tx: %who', 'who', $a->fullName), $erInfo);

  $amount = round($amount, 2); // ignore fractions of cents
  if ($amount == 0) {
    r\tellAdmin('zero transaction', $erInfo + ['stack' => trace()]);
    return txErr(t('zero transaction'), $erInfo);
  }
  $reallyTaking = ($taking xor $amount < 0); // not taking if charging a negative amount

  list ($loserA, $gainerA) = u\order($reallyTaking, $otherA, $actorA);
  list ($e1A, $e2A) = u\order($taking, $otherA, $actorA);
  
  $ptrs = ray('actor other loser gainer e1 e2'); // set up sets of variable names
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
    return txErr(t('duplicate transaction'), $erInfo);
  }

  if ($zErr = r\txPermErr($actorA, $otherA, $taking, $amount < 0)) { // Check permissions
    if ($force) {
      $msg = 'forced without perm';
      $erInfo += ['date' => u\fmtDate($created)];
      foreach (['agent1'=>$actorA, 'agent2'=>$otherA] as $k => $a) $erInfo += [$k => $a->proSe ? t('self') : $a->fullName];
      foreach ([$actorA, $otherA] as $a) if (!$a->proSe) $a->tell($msg, $erInfo);
      r\tellAdmin($msg . ' ' . tr($zErr->msg, $zErr->subs), $erInfo + $zErr->subs);
    } else return txErr($zErr->msg, $erInfo + $zErr->subs);
  }

  foreach ([$actorA, $otherA] as $a) if (!$a->co and !($channel == TX_APP ? $a->agentA->ided : $a->ided)) {
    if ($force) {
      r\tellAdmin('forced without photoId', $erInfo); 
    } elseif ($gainerId != CGID) { // and !$gainer->coCan(CO_FAST)
//      r\tellAdmin('tx without photoId', $erInfo); 
//      return txErr($a == $actorA ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $otherA->fullName]);
    }
  }

  if ($error = u\badAmount($amount)) return txErr($error, $erInfo);
  $flags = 0; foreach (ray(TX_FLAGS) as $k) if ($$k) u\setBit($flags, $k);

  txEntryPair($entries, $amount, [$e1A, $e2A], [$e1For, $e2For], $cat, E_PRIME, $ruleId, $inv);
  
  $DBTX = \db_transaction();

  // Create the transaction
  $info = ray('actorId actorAgentId',$actorA->id, $actorA->agentId) + compact(ray('flags channel boxId goods created recursId'));
  $tx = new r\Tx($info);
  if (!$tx) {
    $DBTX->rollback();
    return txErr(t('tx create err'), $erInfo);
  }

  $needed = abs($amount);

  if ($goods != FOR_USD) {
    // Apply rules
    if ($amount > 0) {
      $needed += handleSurTx($entries, $loserA, $gainerA, $amount, nn($for) and $for != 'other');
    }

    // Apply any roundup
    if ($e1A->roundup and $needed > 0 and !($e2Id == CGID and $recurs and $needed < 1)) { // payer roundups, not actor or loser (don't round up reversals, cashouts, roundups, or recurring gifts under $1)
      if ($cents = fmod($needed, 1.0)) {
        txEntryPair($entries, 1 - $cents, [$loserId, UID_ROUNDUPS], t('roundup donation'), BCAT_GIFT, E_AUX);
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
      $msg = $reallyTaking ? ($otherA->can(B_SECRET) ? 'short from vague' : 'short from') : 'short to';
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
  
  if ($ruid = advanceCredit($for, $loserA, $gainerA, $credit ?: $amount)) { // purchase/gift of store credit
    u\EXPECT(!$ruleId, 'credit purchase already has a rule!');
    for ($i = 0; $i < 2; $i++) $entries[$i]->rule = $ruid; // point the main entry pair at the rule (in case of reversal)
  }
    
  $tx->addEntries($entries);
  if (array_key_exists('xid', $data)) {
    $tx->xid = $data['xid'];
    $xid = $tx->insert(TRUE);
  } else { $xid = $tx->insert(); }
  
  if (!$xid) {
    $DBTX->rollback();
    return txErr(t('tx save error'), $erInfo);
  }
  
  if ($goods != FOR_USD and $pot = $loserA->giftPot) $loserA->update('giftPot', max(0, $pot - $needed));
  
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
 * @param int $cat: budget category (record ID in budget_cats), if any
 * @param int $entryType0: entry type(s)
 * @param int $rule: relevant record ID in tx_rules table
 * @param int $inv: related invoice ID
 */
function txEntryPair(&$txes, $amount0, $a0, $description0, $cat, $entryType0, $rule = NULL, $inv = NULL) {
  foreach ([1, 0] as $i) { // payee first, to agree with tests and so entry id is postive for e2, negative for e1
    foreach (ray('a description entryType') as $k) $$k = is_array(${$k . 0}) ? ${$k . 0}[$i] : ${$k . 0};
    $amount = $i == 0 ? -$amount0 : $amount0;
    list ($uid, $agentUid) = is_object($a) ? [$a->id, $a->agentId] : [$a, $a];
    list ($relType, $relatedId) = $inv ? ['I', $inv] : [NULL, NULL];
    $txes[] = new r\TxEntry(compact(ray('entryType amount uid agentUid description cat rule relType relatedId')));
  }
}

/**
 * Insert entries for an existing transaction.
 * @param array of entry objects $txes: the entries
 * @param int $xid: the transaction header record ID
 */
function insertTxes($txes, $xid) {
  $DBTX = \db_transaction();
  foreach ($txes as $i => $entry) {
    $txes[$i]->id = $eid = $entry->insert($xid, ($i % 2) ? -$eid : 0); // eid of second entry in each pair is the inverse of its companion
    u\EXPECT($eid, "Failed to insert transaction entry");
  }
  unset($DBTX);
}

/**
 * Create a personal credit for buyer or someone else, at the seller.
 * @param string $for: SELF_CREDIT, SELF_CREDIT50, or qid of gift recipient
 * @param acct $buyer: payer acct
 * @param acct $seller: seller acct (where the recipient gets store credit)
 * @param float $credit: amount of credit the recipient gets
 * @return the new rule's record ID (FALSE if not a purchase of credit)
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
  $info = ray('action portion amtMax payerType payer payeeType payee from to purpose', ACT_SURTX, $portion, $credit, REF_ACCOUNT, $buyer->id, REF_ACCOUNT, $seller->id, MATCH_PAYEE, MATCH_PAYER, t('any purchase'));

  return db\insert('tx_rules', $info);
}

/**
 * Send appropriate notifications and return a success message array
 * @return a txRet object
 */
function sendNotifications($type, $tx, $shortfall) {
  u\EXPECT(compact(ray('type tx shortfall')), 'string tx int|float');

  $otherA = $tx->otherA;
  $toMe = $tx->actorAmount > 0;
  $goods = ($tx->goods == FOR_GOODS);

  switch ($type) {
  case 'charge':
  case 'draw':
    $notices = $goods ? ['new payment', 'new refund'] : 'new charge';
    $dids = $goods ? ['credited', 'refunded'] : 'charged';
    if (!$toMe) {   // I made a payment to other
      if ($goods) {
        $notice = 'new refund';
        $did = 'refunded';
      } else {  // I made a payment to other
        $notice = 'new payment';
        $did = 'credited';
      }
    } else { // I charged other
      $notice = 'new charge';
      $did = 'charged';
    }
    break;
  case 'payment':
    $notices = ['new charge', 'new payment'];
    $dids = ['charged', 'paid'];
    if (!$toMe) { // I made a payment to other
      $notice = 'new payment';
      $did = 'paid';
    } else {
      $notice = 'new charge';  // I charged other
      $did = 'charged';
    }
    break;
  case 'invoice payment':
    $notices = ['new charge', 'new payment'];
    $dids = ['charged', 'paid'];
    if (!$toMe) { // I made a payment to other
      $notice = 'new payment';
      $did = 'paid';
    } else { // I charged other -- I don't think this should happen.  JVER
      $notice = 'new charge';
      $did = 'charged';
    }
    break;
  case 'undo':
    $mya = r\acct();
    $byMe = $mya->isMe($tx->actorId);
    if ($tx->otherA->isBank()) {
      $notice = 'bank tx canceled';
      $did = 'reversed';
    } elseif ($byMe and !$toMe) {
      $notice = ($goods) ? 'new refund' : 'new payment';
      $did = ($goods) ? 'refunded' : 'credited';
    } elseif ($byMe and $toMe) {
      $notice = 'new charge';  // I charged other, reversing is refunding
      $did = 'charged';
    } elseif (!$byMe and $toMe) {
      $notice = 'payment';  // Other made payment to me, reverse is refunding
      $did = 'paid';
    } else { // !byMe and !$toMe
      $notice = 'dispute';  // Other charged me, reversing requires dispute
      $did = 'disputed';
    }
    break;
  default:
    u\EXPECT(false, "type must not be '$type'.");
  }
  if ($notice == 'charge' and $tx->goods == FOR_USD) $notice = 'cash out';
  list ($tell1, $tell2) = u\order($toMe, 'tellIn', 'tellOut');

  $args = $tx->reportArgs($tx->actorA, TRUE, $shortfall);
  if ($notice == 'new payment' and $otherA->co) { // not "new charge" (the most common); payment at payer's initiative
    $notice = 'new payment linked';
    $args['_aPayLink'] = $otherA->doLink('addr', $tx->actorId); // link to address, so merchant can ship or nonprofit can thank
  }

  if (!$otherA->isCanonic) $otherA->$tell2($notice, $args);  // no notifications to canonic accounts
  $msg = 'report tx';
  
  if ($tx->flags & u\bit(B_PARTIAL) and $nvid = $tx->entries[0]->relatedId) { // invoice payment or retraction
    if ($left = be\invRemains($nvid)) {
      $msg .= '|left on invoice';
      $args += ray('remaining', u\fmtAmt($left));
    }
  }
  return txRet(TRUE, $msg, $args + compact(ray('msg did')));
}

/**
 * Generate tentative transaction entries for a coupon
 * @param assoc $entries: (MODIFIED) an array of entries to add to, if appropriate
 * @param acct $buyer:
 * @param acct $seller:
 * @param number $price: pre-discount, pre-stepup amount
 * @param bool $forCredit: <buying store credit as a gift or for oneself>
 * @param unixtime $when: when was the transaction (defaults to now)
 * @return the net increase in overall transaction amount
 */

function handleSurTx(&$entries, $buyer, $seller, $price, $forCredit) {
  u\EXPECT($price > 0, 'non-positive price handling coupon');
  
  $payerCo = $buyer->co ? 1 : 0;
  $payeeCo = $seller->co ? 1 : 0;

  // Get a list of applicable rules
  $payerCheck = u\genRuleSQL('payer', ':buyer'); // rule applies to this buyer
  $payeeCheck = u\genRuleSQL('payee', ':seller'); // rule applies to this seller
  $forCreditCheck = $forCredit ? '`from`<>:MATCH_PAYEE' : '1'; // no buying credit with credit or discount
  
  $sql = <<<EOF
    SELECT ru.id AS ruleId, ru.amount, ru.portion, ru.from, ru.to, ru.purpose, ru.minimum, ru.useMax, 
      ru.amtMax, ru.payerType,
    (SELECT IFNULL(SUM(e.amount), 0) FROM tx_entries e WHERE e.rule IS NOT NULL AND e.rule=ruleId AND e.uid=:buyer AND e.entryType=:E_REBATE) AS `amtUsed`,
    (SELECT IFNULL(SUM(SIGN(e.amount)), 0) FROM tx_entries e WHERE e.rule IS NOT NULL AND e.rule=ruleId AND e.uid=:buyer AND e.entryType=:E_REBATE) AS `timesUsed`
    FROM tx_rules ru
    WHERE $payerCheck AND $payeeCheck
    AND $forCreditCheck
    AND (ru.action = :ACT_SURTX)
    AND (ru.start <= :now AND IFNULL(:now < ru.end, TRUE))
    AND (:price >= minimum)
    GROUP BY ruleId
    HAVING :price >= IFNULL(minimum, 0)
    AND timesUsed < IFNULL(useMax, timesUsed+1)
    AND amtUsed < IFNULL(amtMax, amtUsed+1)
    ORDER BY ru.start
EOF;

  $subs = ray('buyer seller now price', $buyer->id, $seller->id, now(), $price);
  /// showq($sql, $subs);
  $q = db\q($sql, $subs);

  $rebate = 0;
  while ($row = $q->fetchAssoc()) {
    extract($row);

    $value = round($amount + $portion * $price, 2); // potential rebate/stepup amount
    if ($amtMax) $value = min($value, $amtMax - $amtUsed); // limited by amtUsed

    $from = ($from == MATCH_PAYER) ? $buyer->id : (($from == MATCH_PAYEE) ? $seller->id : $from);
    $to = ($to == MATCH_PAYER) ? $buyer->id : (($to == MATCH_PAYEE) ? $seller->id : $to);
    list ($type, $desc) = $to == $buyer->id ? [E_REBATE, REBATE_DESC] : [E_AUX, $purpose]; // any surtx not coming back to buyer (from seller or sponsor) is a donation of some sort
    $cat = ($type == E_AUX and r\isCGs($to) and !r\isCGs($from)) ? BCAT_GIFT : NULL;
    if ($to == $buyer->id) $value = min($price - $rebate, $value); // don't refund more than is being spent (food fund especially)

    if ($value <= 0) continue;
    $rebate += $value;
    
    txEntryPair($entries, $value, [$from, $to], $desc, $cat, $type, $ruleId);
    if (($amtMax and $amtUsed + $value >= $amtMax) or ($useMax and $value > 1 and $timesUsed + 1 >= $useMax)) {
      if ($payerType == REF_ACCOUNT) db\update('tx_rules', ray('id end', $ruleId, now()), 'id'); // rule is just for payer
    }
  }

  return -$rebate;
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
  extract(just('created status reversesXid cat', $extra, [now(), NULL, NULL, NULL]));
  $flags = 0;

  foreach (justNOT("status shid coupon coupid force inv api request created for reversesXid cat", $extra) as $k => $v) {
    u\setBit($flags, $k, $v);
    unset($extra[$k]); // don't leave bits in extras (data field)
  }
  foreach (just('api request for', $extra) as $k => $v) if (empty($v)) unset($extra[$k]);
  list ($payee, $payer) = array($payeeA->id, $payerA->id);
  u\setDft($purpose, $why = ray(R_WHYS)[$goods]);
  $subs = ray('myName otherName otherEmail otherPhone amount why purpose did op created', $payeeA->fullName, $payerA->fullName, $payerA->email, u\fmtPhone($payerA->phone), u\fmtAmt($amount), $why, $purpose, t('charged'), t('charged'), $created);
  if ($err = u\badAmount($amount, '>0')) return txErr($err, $subs);
  if ($zErr = r\txPermErr($payeeA, $payerA, TRUE)) return $zErr;
  if (r\dupTx($payee, $payer, -$amount)) return txErr(t('duplicate transaction'), $subs);
  
  u\setDft($status, ($channel != TX_FOREIGN and r\relation(':IS_AUTOPAY', $payee, $payer)) ? TX_APPROVED : TX_PENDING);
  $data = serialize($extra);
  $info = compact(ray('amount payer payee status purpose created flags data recursId reversesXid cat'));
  $nvid = db\insert('tx_requests', $info, 'nvid');

  if ($status == TX_PENDING) {
    $link = $payerA->doLink('inv', $nvid); // link to pay/deny
    $subs += ray('_a1 reply email phone', $link, $payee, $payeeA->email, u\fmtPhone($payeeA->phone));
    $payerA->tell('new invoice', $subs); // tell other party by email about this invoice
  } else { // pre-approved!
    if (!$payerA->shortfall(TX_TRANSFER, $amount)) { // don't try to pay yet if short (for example, this prevents payApproved from duplicating messages)
      $z = be\payInvoice($nvid);
      if ($z->ok) $status = TX_PAID; else r\tellAdmin(t('invoice payment failed'), just('msg', $z) + $subs + $info);
    }
  }

  $msg = 'report tx';
//  if ($payeeA->cttyRewardy) $msg .= '|for why';
  if ($amount > 0) $msg .= '|balance unchanged';
  return txRet(TRUE, $msg, $subs + ray('success nvid status', TRUE, $nvid, $status));
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
 * Figure out who the user meant. Only OK people in the user's community are eligible (except admin sees all).
 *
 * @param string $who: how the user referred to the other account:
 *   name, phone, email, or account ID
 *   name@city, zip, or city, ST
 * @param int $mya: current user's account
 * @param string $selfErr: index to error message for identifying oneself
 * @param string $restrict: MySQL to restrict the selection (for example ':IS_CO' or ':IS_OK')
 * @return one of the following:
 *   the other trader's acct
 *   error message index
 *   array of possible fullNames with city and state, indexed by uid
 */
function identify($who, $mya, $selfErr, $restrict) {
  if (!$mya) return 'search forbidden';
  $myid = $mya->id;
  $who = $who0 = trim($who);
  $searchable = $mya->anyAdmin ? '1' : '(IF(activated, :IS_OK, :IS_MEMBER) AND !:IS_NOSEARCH)';

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
  } elseif ($mya->anyAdmin and is_numeric($who) and mb_strlen($who) > 10) {
    list ($where, $subs) = ['uid=:who', compact('who')];
    $err = tr('unfound', 'who where', t('anyone'), t('with that record ID'));
  } elseif ($phone = u\fmtPhone($who, '+n')) { // phone
    $cphone = u\cry('P', $phone);
    list ($where, $subs) = ['phone=:cphone', compact('cphone')];
    $err = tr('unfound', 'who where', t('anyone'), t('with that phone number'));
  } else { // name or qid or empty (empty when called from memberRay)
    list ($where, $subs) = nameCriteria($who);

    if (preg_match('/^([A-Z]+|[a-z]+)$/', $who)) { // all upper or all lower
      $qid = mb_strtoupper(($mya->anyAdmin and mb_strlen($who) == 3) ? R_SERVER_ID . $who : $who);
      if (u\isQid($qid) and $a = r\acct($qid)) $where = "($where OR uid=$a->id)";
    }
    $err = tr('unfound', 'who where', $who, t('(Try using their email or phone or, if they are not nearby, add "@" followed by their postal code or city (or city, state))'));
  }
      
  if ($restrict) $where .= ' AND ' . $restrict;
  if (!$mya->admin and !$mya->isAdmin2 and !$atPos and !nn($cphone)) { // look only locally unless company
    $where .= tr(' AND (:IS_CO OR community=:ctty OR %DISTANCE_BETWEEN<%NEIGHBOR_MAX)'); // restrict to nearby
    list ($lat, $lon) = [$mya->latitude ?: 0, $mya->longitude ?: 0]; // avoid NULL (PHP bug?)
    $subs += compact(ray('lat lon'));
  }
  
  $individual = t('individual');
  $company = t('company');
  $anon = "IF(:IS_CO, '$individual', '$company')";
  $uidCrit = 'uid NOT :CANONIC';
  if ($mya->admin) $uidCrit = "($uidCrit OR uid=1)"; // needed for helper when activating

  $sql = <<<EOF
    SELECT DISTINCT uid, 
      CONCAT(IF($searchable, fullName, $anon), ' @ ', IFNULL(city, ''), ', ', IFNULL(abbreviation, '')) AS nm
    FROM users u
    LEFT JOIN r_states s ON s.id=u.state
    WHERE $where AND $uidCrit
    ORDER BY community<>:ctty, fullName
EOF;

  $subs['ctty'] = test() ? $mya->community : ($mya->seedpacker ? -1 : $mya->community);
  $result = db\q($sql, $subs)->fetchAllKeyed(0, 1);
  
  if (count($result) == 1 and key($result) == $myid) return $selfErr;
  unset($result[$myid]); // if self is one of many results, ignore it
  if (!$result) return $err ?: tr('unknown member', compact('who'));
  
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

  $sql = "SELECT SUM(amount) AS pendingBank FROM txs2 t WHERE (payee IN (:uids)) AND created<:asof AND completed <= 0";
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
  $where = 'uid1 IN (:uids) AND uid2=:UID_ROUNDUPS AND created BETWEEN :start AND :end';
  return db\sum('amt', 'txs_aux', $where, compact(ray('uids start end')));
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
    $q = db\q("SELECT uid AS id, fullName FROM users WHERE $where UNION $sqlNormal ORDER BY (uid<0) DESC, $nameOrder");
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
 * Create a new bank transfer, including records in txs2, txs_hdrs, and txs_entries (2).
 * @param acct $a: the account to or from which money is to be moved
 * @param numeric $amount: the amount to transfer
 * @param assoc $options: any of these keyed values:
 *   bool $explicit: request was created manually, not automatically
 *   int $created: the UNIX time at which the transfer is deemed to be created (defaults to now)
 *   int $completed: mark the transaction complete on this date
 *   int $pid: record ID of associated people record (NULL if none)
 *   int $cat: record ID in budget_cats of transaction category
 *   string $bankAccount: the non-member bank account, if any (for ACH donations (or payments, NYI))
 *   string $purpose: transaction description
 *   int $deposit: date deposited
 * @return the transaction ID associated with the transfer request
 */
function createUsdTransfer($a, $amount, $options = []) {
  u\EXPECT(compact(ray('a amount options')), 'acct int|float assoc');
  global $channel; 
  
  extract(just('explicit created completed pid cat purpose deposit bankAccount', $options, NULL));

  u\setDft($explicit, $channel != TX_CRON);
  u\setDft($created, now());
  u\setDft($purpose, $amount > 0 ? t('from bank') : t('to bank'));
  $payee = $a->id;

  list ($eType, $deposit, $bankId, $bankAccount) = $pid
  ? [E_OUTER, $deposit ?: 0, UID_OUTER, $bankAccount] // transfer to or from a non-member (with no CG account)
  : [E_BANK, 0, $amount >= 0 ? UID_BANK_IN : UID_BANK_OUT, nni($a->vsecure, 'bankAccount', NULL)];
  u\EXPECT($bankAccount or $deposit or ($pid and $amount < 0), 'bad bank account in bank transfer request: ' . pr($options + compact('payee')));

  $tx = new r\Tx(ray('goods flags actorId actorAgentId channel created', FOR_USD, 0, $payee, $payee, $channel, NULL));
  txEntryPair($txes, 0, [$bankId, $payee], $purpose, $cat, $eType); // tentative amount is 0
  list ($buyer, $seller) = u\order($amount > 0, r\acct($bankId), $a);
  if ($amount) handleSurTx($txes, $buyer, $seller, abs($amount), FALSE);
  $tx->addEntries($txes);
  
  $DBTX = \db_transaction();
  $xid = $tx->insert();
    
  $info = compact(ray('amount payee created deposit bankAccount channel xid'));
  $info += just('pid txid', $options);
  $txid = db\insert('txs2', $info, 'txid');
    
  if ($completed and !completeUsdTx($info + ['eid' => $tx->entries[0]->id], $explicit, $completed)) return FALSE;
  
  unset($DBTX);
  
  return $xid;
}

/**
 * Mark a USD transfer complete, update the amounts in the corresponding transaction entries, and report.
 * @param mixed $xid: record ID of the transfer transaction or an assoc of the relevant information
 * @param bool $explicit: <transfer was not created automatically>
 * @param datetime $completed: when the transaction was completed, defaults to the current time
 * @return bool <success>
 */
function completeUsdTx($xid, $explicit, $completed = NULL) {
  u\EXPECT([$xid, $explicit, $completed], 'int|assoc bool int|empty');
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
  $res0 = db\update('tx_hdrs', ray('xid created', $xid, $completed), 'xid');
  $res1 = db\update('tx_entries', ray('id amount', -$eid, -$amount), 'id');
  $res2 = db\update('tx_entries', ray('id amount', $eid, $amount), 'id');
  if (!($res and $res0 and $res1 and $res2)) {
    r\tellAdmin(t('usd update failed'), $info);
    $DBTX->rollback();
    return FALSE;
  }
  unset($DBTX);
  
  if ($amount > 0 and !$pid) {  
    $transfer = $explicit ? t('transfer') : t('automatic transfer');
    $amount = u\fmtAmt($amount);
    r\acct($payee)->tellIn('transfer-in complete', compact('transfer', 'amount'));
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

  // Check txs2
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

/**
 * Handle balance changes for an account.
 * @param assoc $info: assoc containing the uid and how much it just changed
 */
function balChange($info) {
  extract(just('uid amount', $info));
  $a = r\acct($uid);
  if ($a->isCanonic) return;
  
  $bal = $a->balance; // balance after insert
  if ($bal - $amount >= 0 and !$a->wentNeg) { // !$a->wentNeg makes this self-correcting
    if ($bal < 0) $a->update('wentNeg', now());
  } elseif ($bal >= 0) $a->update('wentNeg', '');
  
//  if ($uid > 0) f('cr.queue', $amount > 0 ? 'payRequests' : 'getFunds1', compact('uid'), TRUE); // TRUE keeps this from looping in tests
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
  $keyVal = ((array) $info)[$field];
  if (!empty($keyVal) and !db\exists($table, [$keyFld=>$keyVal])) {
    sanityErr("$prefix $ref $field refers to non-existent $table record $keyVal");
  }
}

function sanityErr($s) {w\say("Sanity error: " . $s, 'err');}
function invPayments($nvid) {return db\sum('amt', 'txs', "relType='I' AND rel=:nvid", compact('nvid'));}
function invRemains($nvid) {return round(db\get('amount', 'tx_requests', compact('nvid')) - invPayments($nvid), 2);}
