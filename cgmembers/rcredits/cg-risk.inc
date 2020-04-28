<?php
namespace CG\Risk; // typically abbreviated as "k"
use CG as r;
use CG\DB as db;
use CG\Util as u;

/**
 * @file
 * Calculate risks of financial crime
 */

/*
 * Calculate a risk score for the given account.
 * @param int $uid: account record ID
 * @param assoc $thresholds: volume thresholds for each period length
 * @param numeric $risk: (RETURNED) the normalized risk score
 * @param numeric $calc: (RETURNED) the original calculated risk
 */ 
function acctRisk1($uid, $thresholds, &$risk = 0, &$calc = 0) {
  global $_risk; $_risk = '';
  
  $a = r\acct($uid);
  global $_qid; $_qid = $a->mainQid; // for debugging
///  if ($a->mainQid != 'NEW.ZZA') return; // debug(
  $idSub = ray('id', $a->id);
  
  setrisk('adminOk', $a); // administrator blessed this account
//  if ($a->community <> r\serverUid()) setRisk('geography'); // HIDTA (Washtenaw cty, MI)
  if (!$a->cttyA or $a->cttyA->risk('geography')) setRisk('geography'); // HIDTA (eg Washtenaw cty, MI) (!$a->cttyA is for tests)
  // criminal
  setRisk('cashCo', $a); // high-risk company type according to regs
  $tenure = $a->tenure / 12 + (now() - $a->created) / YEAR_SECS; // years of residence ($a->tenure is in months)
  setRisk('new', $tenure ? .5 / ($tenure * $tenure) : 99999); // half a year is the standard
  setRisk('moves', $a->moves); // moves over past decade
  setRisk('rents', $a);
  
  if (!$a->co) { // never trust a company
    $trusted = $a->trust - 1; // how well trusted (trust is always >=1)
    $inviteTrust = db\get('SUM(trust - 1)', 'users u INNER JOIN r_invites i ON i.inviter=u.uid', 'i.invitee=:id OR i.email=:email', ray('id email', $a->id, u\cry('P', $a->email))); // trustedness of accounts that invited this one
    $trusted += $inviteTrust; // invited by trusted people
    $relations = db\count('r_relations r LEFT JOIN users u ON u.uid=r.main', 'r.permission>0 AND NOT r.:IS_OWNER AND r.other=:id AND u.risk<:K_GREEN', $idSub);
    $trusted += $relations; // access to (trusted by) non-suspect businesses
    setRisk('trusted', $trusted);
  }
  
  $conxRisk = db\count('users u INNER JOIN r_invites i ON i.invitee=u.uid', "i.inviter=:id AND u.risk>=:K_RED", $idSub); // invited by this account
  $relationsRisk = db\count('r_relations r LEFT JOIN users u ON u.uid=r.main', 'r.permission>0 AND r.other=:id AND u.risk>=:K_RED', $idSub); // access to (trusted by) suspect businesses
  setRisk('badConx', $conxRisk + $relationsRisk); // invited suspicious accounts

// UNUSED  setRisk('socialConx', $a->mediaConx, 'ifany'); // 1st or 2nd-degree social media connections to administrator

//  if ($a->co and db\exists('users', 'uid<>:id AND address=:address AND zip=:zip', ray('id address zip', $a->id, $a->address, $a->zip))) setRisk('homeCo'); // company shares physical address
  if ($a->co and $a->dupAddr()) setRisk('homeCo'); // company shares physical address (encryption requires func)
  setRisk('shady', $a); // no substantial internet search results and/or no SSN history
//  if ($a->can(B_BANK)) setRisk('hasBank'); // has a connected bank account
  setRisk('hasBank', $a); // has a connected bank account
  if ($uid != CGID and db\get('SUM(amount*perYear(period, periods))', 'tx_templates t', 'action=:ACT_PAY AND t.from=:id AND t.to=:CGID AND t.end IS NULL', $idSub) + 0 < K_MISER_DONATION and !$a->roundup and $a->crumbs == 0) setRisk('miser'); // unusually small donation to CGF
  
//  setRisk('photoOff', $a); // photo mismatches ID or DOB
  setRisk('addrOff', $a); // addr mismatches ID
  if ($a->postalAddr) {
    if (mb_strpos($a->postalAddr, $a->address) === FALSE or mb_strpos($a->postalAddr, 'box') !== FALSE) setRisk('poBox'); // separate postal address
  }
  setRisk('ssnOff', $a); // weird, conflicting, or unknown SSN history
  setRisk('dobOff', $a); // birth date unverifiable in SSN check
    // activity
  setRisk('fishy', $a); // something else fishy
  setBigRisks($uid, $thresholds);

  extract(setRisk());
  $calc = $risk;
  $risk *= K_RED; // normalize
///  u\deb(compact('risk','risks'));
//  if ($a->mainQid == R_ADMIN_QID) $risk = 0; // otherwise sketchy invites skew all the calculations
  $a->update(compact('risk','risks'));
}

/*
 * Calculate a risk score for the given transaction.
 * @param xid $xid: the transaction identifier
 * @param numeric $risk: (RETURNED) the normalized risk score
 * @param numeric $calc: (RETURNED) the original calculated risk
 */ 
function txRisk1($xid, &$risk = 0, &$calc = 0) {
  global $_risk; $_risk = ''; // reset risk calculation
  global $_xid; $_xid = $xid; // for debugging

  $fields = 'xid, uid1, uid2, amt AS amount, risks, :IS_OFFLINE AS offline, goods, created, channel, reversesXid';

  $row = db\get($fields, 'txs', compact('xid'));
  extract($row);
  if ($risks & u\bit(K_ADMINOK)) setRisk('txAdminOk'); // administrator blessed this transaction, perpetuate it

  if ($reversesXid AND db\get('reversesXid', 'tx_hdrs', 'xid=:reversesXid', compact('reversesXid'))) setRisk('redo');

  if ($amount < 0) list ($amount, $uid1, $uid2) = [-$amount, $uid2, $uid1];
  $a1 = r\acct($uid1);
  $a2 = r\acct($uid2);

  if ($goods != FOR_GOODS) {
    if ($a1->co xor $a2->co) {
      setRisk($a1->co ? 'cashOut' : 'cashIn');
    } else setRisk('exchange');

    if ($a1->risk >= K_RED) setRisk('suspectOut');
  } else {
    if ($a1->co and !$a2->co and !r\isEmployee($uid2, $uid1)) setRisk('b2p'); // company payment to non-employee individual
  }

  $samePostal = ($a1->postalAddr == $a2->postalAddr);
  $withCo = ($a1->co or $a2->co); // at least one party is a company
  if (!$withCo and !$samePostal) setRisk('p2p'); // transaction between individuals at different postal addresses
  if ($withCo and (r\owning($uid1, $uid2) or $samePostal or r\isEmployee($uid1, $uid2))) setRisk('inhouse'); // transaction with owner or family member same address or payment by employee

  if ($amount > 0) { // never flag reversals
    $since = $created - WEEK7_SECS; // just recent activity by customers, in case many are new
    $count = db\count('txs', 'uid1=:uid1 AND uid2=:uid2 AND created BETWEEN :since AND :created', compact(ray('uid1 uid2 since created')));
    setBigToFrom('From', 'uid1', $uid1, $amount, $created, $count);
    setBigToFrom('To', 'uid2', $uid2, $amount, $created, $count);
  }
  setOriginRisk($uid1, $created, $amount);
    
  if ($channel == TX_WEB or $channel == TX_SMS) {
    setRisk('absent'); // transaction by web or SMS (no in person)
    if (!db\exists('r_invoices', 'status=:xid', compact('xid'))) setRisk('invoiceless'); // paid non-employee without an invoice
  }

  if ($a1->risk+0 >= K_RED) setRisk('fromSuspect', $a1->risk / K_RED); // payment from suspicious account
  if ($a2->risk+0 >= K_RED) setRisk('toSuspect', $a2->risk / K_RED); // payment to suspicious account

  if ($offline) { // offline
    setRisk('offline');
    if (!db\exists('txs', 'uid2=:uid2 and created<:created', compact(ray('uid2 created')))) setRisk('firstOffline'); // offline for first payment to this account
  }
  
  extract(setRisk());
  $calc = $risk;
  if ($risk > 0) $risk *= $amount; // (don't exaggerate negative risk)
  $risk *= K_RED / K_THRESHOLD; // normalize
  db\update('tx_hdrs', compact('xid', 'risk', 'risks'), 'xid');
}

/**
 * Update the risk variables.
 * call by:
 *   setrisk(bitName, multiplier); OR
 *   setrisk(bitName, $a);
 * @param string $bitName: what risk type -- allows lookup of risk weight and bit number. 
 * @param float $multiplier: (>0) what to multiply inverted weight by, before adding to risk score
 * @param acct $a: add the standard risk score for this risk type, if the bit is already set in this acct
 * @return assoc [risk, risks] if called with no params
 */
function setRisk($bitName = NULL, $multiplier = 1) {
  global $showRiskCalc;
  global $_risk, $_risks; // use _ to distinguish from extracted risk and risks fields (not needed, but clearer)
  global $_acctRisks, $_txRisks, $_acctRiskBits, $_txRiskBits;
  if (!$multiplier) $multiplier = 0;
  
  if (@$_risk === '') {
    $_risk = $_risks = 0;
    $_acctRisks = ray(K_ACCT_RISKS);
    $_txRisks = ray(K_TX_RISKS);
    $_acctRiskBits = array_flip(array_keys($_acctRisks));
    $_txRiskBits = array_flip(array_keys($_txRisks));
  }
  if (is_null($bitName)) return ray('risk risks', $_risk, $_risks); // must come after $_risks=0
  
  if (is_numeric($bit = @$_acctRiskBits[$bitName])) { // look up bit and weight in proper table (K_ACCT_RISKS or K_TX_RISKS)
    $weight = $_acctRisks[$bitName];
  } else list ($bit, $weight) = [$_txRiskBits[$bitName], $_txRisks[$bitName]];

  if (is_object($a = $multiplier)) $multiplier = $a->risk($bit) ? 1 : 0;
  $risk = $multiplier / $weight;
  $_risk += $risk;

  if (abs($risk) >= K_BIT_MIN) {
    u\setBit($_risks, $bit);
    $setBit = t('yes');
  } else $setBit = ''; // don't show bit if risk is reversed
 
  if (isset($showRiskCalc) and $risk) {
    $class = $weight < 0 ? 'goodRisk' : 'badRisk';
    $showRiskCalc .= <<< EOF
<tr>
  <td class="$class">$bitName</td>
  <td class="weight">[$weight]</td>
  <td class="multiplier">$multiplier</td>
  <td class="risk">$risk</td>
  <td class="set">$setBit</td>
</tr>
EOF;
  }
}

/**
 * Set one of the "big" risk bits and/or the "more" bits.
 * @param int $uid: the account record ID
 * @param int $ago: how many seconds ago to start the period to be analyzed
 * @param assoc $thresholds: max total dollar amounts for an account's activity before it is flagged (indexed by $bitName)
 * @param string $bitName: the bit to set
 */
function setBigRisk($uid, $ago, $thresholds, $bitName) {
  global $bigYearIn, $bigYearOut, $newThresholds;
  
  $threshold = $thresholds[$bitName];
  $now = now();
  $since = $now - $ago - 2 * HOUR_SECS; // an extra couple hours to handle daylight time changes and cron timing
  $mySql = <<<EOF
    SELECT COUNT(*) AS count0, 
      SUM(IF(:uid=uid2 XOR amt<0, ABS(amt), 0)) AS `in`, 
      SUM(IF(:uid=uid1 XOR amt<0, ABS(amt), 0)) AS `out`
    FROM txs 
    WHERE :uid IN (uid1, uid2) AND created BETWEEN :since AND :now
EOF;
  if (!$res = db\q($mySql, compact(ray('uid since now')))->fetchAssoc()) return;
  extract($res);
  $a = r\acct($uid);
  $max = max($in, $out) / max(1, $a->employees);
  if ($max > $threshold) setRisk($bitName);
  if (isset($newThresholds)) $newThresholds[$bitName][] = $max; // collect data for next time (see cr\acctRisk)
//  if (max($in, $out) > $threshold * max(1, $a->employees)) setRisk($bitName);
  if ($ago == YEAR_SECS) list ($bigYearIn, $bigYearOut) = [$in, $out];
  if ($ago == WEEK7_SECS) {
    $firstTxDate = db\get('MIN(created)', 'txs', ':uid IN (uid1, uid2)', compact('uid')) ?: $now;
    $yearLen = min(YEAR_SECS, $now - $firstTxDate);
    if ($yearLen) {
      if ($in > (1 + K_UNUSUAL_BUMP / 100) * $bigYearIn * $ago / $yearLen) setRisk('moreIn');
      if ($out > (1 + K_UNUSUAL_BUMP / 100) * $bigYearOut * $ago / $yearLen) setRisk('moreOut');
    }
  }
}

/**
 * Set the various "big" risk bits as appropriate.
 * @param int $uid: the account record ID
 */
function setBigRisks($uid, $thresholds) {
  setBigRisk($uid, YEAR_SECS, $thresholds, 'bigYear'); // must precede big7Week
  setBigRisk($uid, WEEK7_SECS, $thresholds, 'big7Week');
  setBigRisk($uid, WEEK_SECS, $thresholds, 'bigWeek');
  setBigRisk($uid, DAY_SECS, $thresholds, 'bigDay');
}

/**
 * Set risks according to an account's standard transaction size.
 * @param string $way: To or From (for risk names)
 * @param string $field: name of field to examine (uid1 or uid2)
 * @param string $uid: value of that field
 * @param numeric $amount: (>0) transaction amount
 * @param int $created: transaction creation date
 * @param int $count: number of transactions between payer and payee in the past 7 weeks
 */
function setBigToFrom($way, $field, $uid, $amount, $created, $count) {
  $since = $created - WEEK7_SECS; // just recent activity by customers, in case many are new
  $other = $field == 'uid1' ? 'uid2' : 'uid1';
  
  // look at all this account's recent txs with each other acct (avg0=how much on average, max0=biggest, count0=how many)
  $subtotals =<<<EOF
    SELECT AVG(ABS(amt)) AS avg0, MAX(ABS(amt)) AS max0, COUNT(*) AS count0 
    FROM txs
    WHERE IF(amt<0,$other,$field)=:uid AND created BETWEEN :since AND :created-1
    GROUP BY IF(amt<0,$field,$other)
EOF;

  // get lessCount=how many of those averages were less than this amount, maxAmt=biggest amount, 
  // fewerCount=how many of those others did less business with this account, othersCount=how many other accts
  $sql = <<< EOF
    SELECT SUM(IF(avg0<:amount, 1, 0)) AS lessCount, 
    MAX(max0) AS maxAmt, 
    SUM(IF(count0<:count, 1, 0)) AS fewerCount, COUNT(*) AS othersCount 
    FROM ($subtotals) s
EOF;

  if ($res = db\q($sql, compact(ray('amount count since created uid')))->fetchAssoc()) {
    extract($res); // average amt and number of purchases per other
    if ($maxAmt > 0) {
      if ($lessCount / $othersCount > .90) setRisk("big$way"); // unusually big for this payer/payee (top decile)
      $over = abs($amount) - $maxAmt;
      if ($over >= 0) setRisk("biggest$way", $over / $maxAmt); // biggest ever for this payer/payee
      if ($fewerCount / $othersCount > .90) setRisk("often$way"); // unusually frequent for this payer/payee
    }
  }
}

/**
 * Set the "origins" risk bit in the current transaction or outgoing ACH, 
 *   if the paying account's transactions were sketchy most of the time over the past year.
 * @param int $uid: the account record ID
 * @param int $created: transaction date
 * @param currency $amount: (UNUSED) transaction amount
 */
function setOriginRisk($uid, $created, $amount) {
  $since = $created - YEAR_SECS;
  $where = ':uid IN (uid1,uid2) AND risk IS NOT NULL AND created BETWEEN :since AND :created';
  $avgRisk = db\get('AVG(risk/ABS(amt))', 'txs', $where, compact(ray('uid since created')));
  if ($avgRisk > 0) setRisk('origins', $avgRisk);
}
