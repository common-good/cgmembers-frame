<?php
namespace CG\Cron;
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Risk as k;
use CG\Web as w;
use CG\Testing as t;
use CG\Cron as cr;

/**
 * @file
 * Functions to run periodically.
 *
 * To run a SINGLE cron function "zot" from the PHP window, use: f('cr.zot', arg1,...);
 * To run all the daily functions starting with a particular function "zot", use:
 * cr\zot(arg1,...); // to run ALL daily functions, zot=everyDay
 *
 * Note: Timed cron is disabled on the staging site. Here's what the cron job would be:
  <star>/5 * * * * curl https://new2.CommonGood.earth/cron.php?cron_key=w8x3lfzNQK1bxMz6RMTuF54mMInAs4b8SiVJO9zvWYg
  where <star> means * (not the same key as for the production site) 
 
 * @todo:
- identify pending joint accounts and alert both parties (with link to complete or cancel) select reid, u1.fullName, u2.fullName FROM u_relations r LEFT JOIN users u1 ON r.main=u1.uid LEFT JOIN users u2 ON u2.uid=r.other WHERE (SELECT reid FROM u_relations WHERE main=r.other AND other=r.main) IS NULL AND r.permission=7 
DAILY (every 5 mins for 5 mins, starting at 2am)
- checkAccts() check accounts (and their activity) more thoroughly for consistency, security, scams
- notify and maybe fine late borrowers (accounts with a negative balance)
WEEKLY
- tell members what they did that was dumb
MONTHLY
- archive the 13th previous month (13 is a parameter -- transactions are accessible for ever, but slower)
YEARLY
 */

// Calculate risks twice, to take into account connections with scoundrels
const DAILY_OPS = 'START eachAcct setNear cttyStats trust acctRisk acctRiskFinish txRisk biggies tickle annualGift renewBacking lessOften recurs adjustFS nudges payInvoices getFunds completeUsdTxs notices END'; // getFunds, completeUsdTxs, and notices should probably always be the last 3 things
const LOGLESS_OPS = 'acctHourly oneAcct whatsUp tickleStep activeWeekly activeMonthly recur'; // don't log these calls
const POST_DETAIL_MAXLEN = 256; // maximum number of characters to show in post detail
include_once R_ROOT . '/rcron/rcron-subs.inc';

/**
 * Execute a cron run when called.
 */
function run() {
  r\launchWebsocket();
  global $channel; $channel = TX_CRON; // set this even if called from PHP window by admin

  ignore_user_abort(TRUE); // Allow execution to continue even if the request gets canceled.

  $original_session_saving = \drupal_save_session(); // Prevent session information from being saved while cron is running.
  \drupal_save_session(FALSE);

  $original_user = $GLOBALS['user']; // Force the current user to anonymous to ensure consistent permissions on cron runs.
  $GLOBALS['user'] = \drupal_anonymous_user();
  \drupal_set_time_limit(4 * 60); // time out a minute before the next cron starts
  if (!\lock_acquire('cron', 4 * 60)) return u\log(t('Attempted to run cron while it was already running.'));
  
  db\del('sessions', 'timestamp < :NOW-:SESSION_LIFE'); // remove stagnant sessions
  cr\periodic();
    
  $end = now() + R_CRON_PERIOD - R_MAX_TASK_TIME;
  $sql = 'SELECT id,item FROM queue ORDER BY id';
  $q = db\q($sql);
  
  while (now() < $end and $row = $q->fetchAssoc()) {
    extract($row);
    $item = unserialize($item);
    try {
      $DBTX = \db_transaction(); // this is crucial! otherwise we may partially do something several times (like add a bank transfer request to the queue)
      cr\doTask($item);
      db\del('queue', compact('id'));
      unset($DBTX);
    } catch (\Exception $e) {
      if (isset($DBTX)) $DBTX->rollback(); // if(isset) required (dunno why)
      u\log('cron task failed'); // In case of exception log it and leave the item in the queue to be processed again later.
      r\tellAdmin('cron task failed id=' . $id . pr($e), $item);
    }
  }
  // Restore the user.
  $GLOBALS['user'] = $original_user;
  \drupal_save_session($original_session_saving);
  \lock_release('cron');
}

/**
 * Do a queued task
 * @param assoc $item: task arguments, including 'op' -- the function to handle the task
 */
function doTask($item) {
  global $channel; $channel = TX_CRON;
  global $mya;

  $op = array_shift($item);
  $func = 'CG\\Cron\\' . $op;

  extract(just('uid', $item, NULL));
  if ($uid) $mya = r\acct($uid); // fudge current account
  if (!in($op, LOGLESS_OPS) and substr($op, -1, 1) != '1') u\log($op, 'cr', $item);

  $func($item);
}

/**
 *  Do this stuff every time cron is called (normally every 5 minutes)
 *  Check to see if it's time for a less-often periodic activity (and if so do it).
 */
function periodic() {
  if ($row = db\q('SELECT * FROM queue WHERE created<:NOW-:DAY_SECS/2 LIMIT 1')->fetchAssoc()) {
    r\tellAdmin('task timed out', $row); // shouldn't happen. Drupal's queue processing hangs until we delete.
    db\q('DELETE FROM queue WHERE id=:id', just('id', $row));
  }
    
  often('hour', '00');
  often('day', R_DAILY_HOUR); // longer periods get queued in lessOften(), so notices are last
  
  $sql = <<< X
    SELECT DISTINCT * FROM (
      SELECT 'users' AS tnm, uid AS id, '' AS address, city, state, zip, latitude, longitude
        FROM users WHERE risks&(1<<:K_ADDROFF)=0 AND uid NOT :CANONIC
      UNION SELECT 'people' AS tnm, pid AS id, address, city, state, zip, latitude, longitude
        FROM people
    ) p
    WHERE latitude=0 AND longitude=0 AND city IS NOT NULL AND state<>0
X;
  queueEach('geocode', $sql);  
}

function everyHour() { // stuff to do every hour
//  queueEach('acctHourly', "SELECT uid FROM users WHERE uid<>0");
  $hourNum = fmtDt(now(), 'H') + 0;
  if ($hourNum == (R_DAILY_HOUR + 10) % 24) { // if not successfully completed after 10 hours, report to admin
    $lastCron = getV('last_cron_end', []);
    if (now() - nni($lastCron, 'end', 0) > DAY_SECS) r\tellAdmin('daily cron FAILED');
  }

  if ($hourNum % 6 == 0 and now() > nni(getv('qboRefresh') ?: [], 'lastRefresh', 0) + 5 * HOUR_SECS) { // refresh the QBO Refresh Token every 6 hours (at least)
    require_once R_ROOT . '/classes/qb.class';
    new \QB(QBO_CREDS);
  }

  db\q('DELETE FROM cache_form WHERE created<:REQUEST_TIME-:HOUR_SECS'); // clean up form cache (is this needed?)
  
  // discard expired encrypted files
  if (is_dir(MSG_DIR)) foreach (scandir(MSG_DIR) as $flnm) { // gets created when first encrypted file is sent
    if (!is_numeric($flnm)) continue;
    if ($flnm + 0 < now()) u\deleteFile(MSG_DIR . "/$flnm"); else break;
  }
  
  clickup();
}

/**
 * Bring our local clickup data up to date.
 */
function clickup() {
  if (test()) return; // no tests for this yet
  require_once R_ROOT . '/classes/clickup.class';
  $cu = new \Clickup();
  $start = strtotime('-14 days'); // nobody should be adding or editing any times before that
  $end = strtotime('-2 hours'); // give people time to edit or delete mistakes
  $cu->updateMost();
  $cu->updateTimes($start, $end);
  $cu->updateTasks($start, NOW); // no harm in adding all possible tasks
  $cu->updateTasks('SELECT DISTINCT parent AS task FROM cu_tasks WHERE parent IS NOT NULL'); // add parent tasks
}

function geocode($item) {
  extract(just('tnm id address city state zip', $item));

  if ($tnm == 'users') {
    $a = r\acct($id);
    $address = $a->address;
  }
  
  list ($lat, $lon, $zot) = u\getLocus($address, $city, $state, $zip);
  if ($lat or $lon) {
    if ($tnm == 'people') {
      return db\update('people', ray('pid latitude longitude', $id, $lat, $lon), 'pid');
    } else return $a->setLocus($lat, $lon); // users
  }
  
  if ($tnm == 'users') $a->setRisk('addrOff');
  return r\tellAdmin(t('setLocus returned no match in %tnm record %id', 'tnm id', $tnm, $id));
}

/**
 * Account checks to do every hour.
 */
function acctHourly($item) {
  return; // nothing to do, currently
  extract(just('uid', $item));
  $a = r\acct($uid);
//  $a->shortfall(FOR_GOODS, PHP_INT_MAX); // notice recent deposits, if any
}

/**
 * Do miscellaneous stuff that needs doing every day
 */
function everyDay() {
  if (db\exists('queue')) { // daily tasks already in progress
    if (date('j') + 0 < 5) r\tellAdmin('Daily cron already in process!'); // not unexpected throughout the hour
    return;
  }
  
  queueNext('START'); // eachA seems to interfere with this and processing the queue is vital, so launch it first
  db\q('DELETE FROM r_do WHERE completed<0 AND :NOW>-completed'); // completed<0 is -expiry time

  $cttys = db\q('SELECT uid AS ctty FROM users WHERE uid<0')->fetchCol();
  foreach ($cttys as $ctty) $companies[$ctty] = r\companies($ctty, FALSE);  
  \setV('companies', nn($companies) ?: []);
  
  $sql = <<< X
  SELECT DISTINCT u.uid, c.uid AS ctty, u.fullName AS person, cu.fullName AS cttyName 
  FROM users u LEFT JOIN u_company c ON u.zip RLIKE c.zips LEFT JOIN users cu ON cu.uid=c.uid
  WHERE u.community=:seedpack AND c.uid<1 AND c.uid<>:seedpack AND !u.activated
X;
  $q = db\q($sql, ['seedpack' => r\serverUid()]);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    db\update('users', ray('uid community', $uid, $ctty), 'uid');
    r\tellAdmin(t('Not-yet-member %person was in Seedpack; should be (now is) in %cttyName', $row));
  }
  
  if ($xid = db\get('xid', '(SELECT xid,SUM(amount) AS amt FROM tx_entries GROUP BY xid) t', 'amt<>0')) r\tellAdmin(t('Not all transactions have entries totaling zero (eg xid=%xid).', compact('xid')));

  // Notice which bank accounts have been verified
  $where = 'deposit>UNIX_TIMESTAMP(DATE_SUB(NOW(), INTERVAL 7 DAY)) AND deposit<UNIX_TIMESTAMP(DATE_SUB(NOW(), INTERVAL :VERIFICATION_DELAY DAY))  AND (SELECT 1 FROM txs2_bank d2 WHERE d2.txid=-d.txid) IS NULL';

  if ($uids = db\col('payee', 'txs2_bank d', $where)) {
    $DBTX = \db_transaction();
    foreach ($uids as $uid) if ($a = r\acct($uid) and $a->hasBank) $a->setBit(B_BANKOK);
    db\del('txs2_bank', 'amount=0 AND payee IN (:uids)', compact('uids'));
    unset($DBTX);
  }
  db\q('DELETE FROM x_txs2 WHERE amount=0'); // no need to track these
}

/**
 * Delete accounts that aren't real or are abandoned.
 */
function eachAcct() {
  $testOFAC = now() - getv('ofac_updated') < DAY_SECS * 1.5 ?: 0;
  queueEach('oneAcct', "SELECT uid, $testOFAC AS testOFAC, (BINARY RIGHT(fullName, 8)=' PARTNER' AND pass IS NULL AND NOT :IS_CO AND activated=0) AS badPartner FROM users WHERE uid > 0 AND uid NOT :CANONIC");
  queueNext();
}

function oneAcct($item) {
  extract(just('uid testOFAC badPartner', $item));
  $a = r\acct($uid);
  if ($badPartner) { // delete stale unused partner accounts
    if ($jA = $a->jA and $jA->jid == $a->id) $jA->update('jid', 0);
    db\del('users', 'uid', $a->id);
  } elseif (!$a->member and !$a->ok and !$a->iclubq and in($a->task, 'co person') and now() - $a->created > 7*DAY_SECS and in($a->steps+0, [1, S_ONESTEP])) { // delete robots
    $s = strtr($a->source ?: '', ['self() '=>'', 'knows: '=>'']); // can't use nn() here
    
    $onestepAge = now() - $a->created;
    if ($onestepAge > WEEK_SECS) { // warn about slow joiners for a week, then delete them in 6 months
      if ($onestepAge < 2 * WEEK_SECS or now() < strtotime('10/1/2024')) {
        $msg = tr('stale single-step', ray('qid nm created', $a->mainQid, $a->fullName, fmtDt($a->created)));
        r\tellAdmin($msg, ray('email phone', $a->email, u\fmtPhone($a->phone)));
      } elseif ($onestepAge > YEAR_SECS / 2) f('a.deleteUid', $a->id);
    }
  }
 
  if ($testOFAC and r\isCriminal($a->fullName, $a->co)) r\tellAdmin(t('Name flagged as criminal by latest OFAC data'), ray('nm qid', $a->fullName, $a->mainQid));

  //look at agreement date and first transaction date and last transaction date. queue appropriate funcs
}

/**
 * Generate rules, transactions, and invoices from tx_timed.
 * Instantiate surtx rules first so they apply to subsequent transactions.
 * Action options are: 
 *   surtx: step ups, discounts, or subsidies that recur (just for food fund subsidies so far)
 *   pay or charge: timed or recurring payments and invoices (if pay amount is NULL, sweep all but the account's target balance)
 * OBSCURE NOTE: if you want to count a transaction as the latest recurrence, give it the right rule and set its date/time AFTER the expected recur date/time
 */
function recurs() {
  $sql = <<< EOF
    SELECT tm.*, MAX(
      IF(tm.action=:ACT_SURTX, ru.start, 
      IF(tm.action=:ACT_PAY, GREATEST(IFNULL(x.created,0), IFNULL(i.created,0)),
      i.created))
    ) AS lastRecur
    FROM tx_timed tm
    LEFT JOIN tx_rules ru ON ru.template=tm.id
    LEFT JOIN txs x ON x.recursId=tm.id
    LEFT JOIN tx_requests i ON i.recursId=tm.id
    WHERE tm.action IN (:ACT_SURTX, :ACT_PAY, :ACT_CHARGE)
    AND tm.start <= :NOW
    AND IFNULL(tm.end, :NOW+1) > :NOW
    GROUP BY tm.id
    ORDER BY (tm.action=:ACT_SURTX) DESC, tm.id
EOF;

  queueEach('recur', $sql);
  queueNext();
}

/**
 * Handle a delayed or recurring event.
 * Note: for non-member donations through Stripe, this function provides a useful return, since it may be called from stripe.inc
 * @return: [ok, msg] (for Stripe only)
 */
function recur($template) {
  global $testCCFail;
    
  extract(just('id lastRecur period periods start end duration durations action from to purpose flags amount payerType payer stripeId', $template));
  $recursId = $id;

  $lastRecurWanted = u\lastRecur($start, $periods, $period);
  
  $shouldStart = $lastRecur < $lastRecurWanted ? $lastRecurWanted : u\dateIncr($lastRecurWanted, $periods, $period);
  
  if ($end and $end < $shouldStart) return; // don't create anything after template expires
  $erSubs = compact(ray('template shouldStart'));

  if ($period and $period != PER_1) {
    $when = r\recurDesc($period, $periods);
    $erSubs += compact(ray('when purpose'));
  } else { $when = t('one-time'); }

  if ($action == 'surtx') { // create rollover surtx (UNUSED as of ~2020)
    if ($shouldStart > now() + WEEK_SECS) return; // rules can be created ahead, but don't do it more than a week in advance
    $rule = justNot(ray('id period periods duration durations start end lastRecur flags'), $template);
    $rule['template'] = $recursId;
    $rule['start'] = $shouldStart;
    $rule['end'] = u\dateIncr($shouldStart, $durations, $duration);
    unset($rule['stripeId']);
    $ok = db\insert('tx_rules', $rule, 'id');
    if (!$ok) r\tellAdmin(t('Insert rule failed in recur'), $erSubs);
    return;
  }

  if (!in($action, 'pay charge')) return FAIL(t('bad action in template: ') . pr($erSubs));

// keep transactions synced with template even if cron fails for a few days
  $created = now();
  if ($shouldStart > now()) return; // don't pay or charge until the right date and time

  if ($action == 'pay' and $payerType == REF_PERSON and $pid = $payer) { // recurring (or timed) payment from a non-member
    if (!$emailAndName = db\get('email, fullName', 'people', compact('pid'))) return r\tellAdmin(t('missing people record for tx_timed'), $template);
    extract($emailAndName);
    $flags |= u\bit(B_SELF);
    $info = compact(ray('pid amount purpose created flags')) + ray('completed recursId', now(), $recursId);
    $coA = r\acct($to); $coId = $coA->id;
    
    if ($stripeId) { // recurring (or timed) credit card payment
      require_once R_ROOT . '/stripe.inc';
      $info['deposit'] = now();
      $paymentIntent = stripe('charge', $erReport = compact(ray('stripeId shouldStart to amount')) + $info);
      if (!is_object($erMsg = $paymentIntent) or $erMsg = nn($testCCFail)) {
        include_once R_ROOT . '/forms/code.inc';
        r\tellAdmin(t('Stripe charge failed'), $erReport + compact('erMsg'));

        $recurs = ($period != PER_1);
        $howOften = $recurs ? r\recurDesc($period, $periods) : '';
        $recurCanceled = $recurs ? t('All future recurrences are canceled.') : '';
        $giftLink = BASE_URL . '/pay/code=' . w\code(ray('item account secret', $purpose, $coA->mainQid, $coA->emailCode));

        $subs = ray('noFrame date amount', TRUE, fmtDt(now()), u\fmtAmt($amount));
        $subs += ray('coName coPostalAddr coPhone erMsg', $coA->bestName, $coA->postalAddr, u\fmtPhone($coA->phone), lcfirst($erMsg));
        $subs += compact(ray('fullName howOften recurCanceled giftLink'));
        r\rMail('cc-payment-failed', [$email => $fullName], $subs);
        if ($recurs) db\update('tx_timed', ray('id end', $id, now()), 'id'); // end the recurring
        return [FALSE, $erMsg]; // probably a canceled or expired card
      }

      $DBTX = \db_transaction();
      $deposit = $completed = now();
      $info = compact(ray('purpose pid completed deposit recursId flags'));
      $tx = be\createUsdTransfer($coA, $amount, $info);

      if (db\count('tx_hdrs', compact('recursId')) == 1 // first recurrence: add note from donation form
        and $note = db\get('notes', 'people', compact('pid'))) { // if any 
          $info['purpose'] = $purpose .= " (\"$note\")";
      }
  
      if ($coId != CGID) { // no credit card fees for CG, for payments to CG
        be\txEntryPair($txes, $tx, round($amount * CC_PCT / 100 + CC_PLUS / 100, 2), [$coId, CGID], tr('cc fee'), E_XFEE);
        be\insertTxes($txes, $tx->xid);
      }
      unset($DBTX);
      
    } else { // recurring non-member ACH (legacy)
      if (!$res = db\get('bankAccount,isSavings,payee AS txs2Payee', 'txs2 x2 JOIN txs_noreverse t USING(xid)', 'pid=:pid ORDER BY x2.created DESC', compact('pid'))) return r\tellAdmin(t('missing previous non-member payment'), $template);
      extract($res);

      u\EXPECT($txs2Payee == $to, 'previous recurring non-member payment: payee mismatch');
      $info += compact(ray('bankAccount isSavings'));
      $tx = be\createUsdTransfer($coA, $amount, $info);
    }

    $adminMsg = $stripeId ? t('CC payment') : t('Non-member ACH');
    $adminMsg .= tr(' (to %whom)', 'whom', $coA->bestName);
    r\tellAdmin($adminMsg, compact(ray('amount period')) + $info + $emailAndName);
    $msg = be\thanks($tx->xid, $purpose);

  } else { // normal (member) pay or charge
    list ($fromA, $toA) = [r\acct($from), r\acct($to)];
    if (!$toA->ok) {
      db\q('UPDATE tx_timed SET `end`=:NOW WHERE `to`=:to', compact('to'));
      return r\tellAdmin(t('Terminating all recurring transactions for :fullName because account is closed', ray('fullName', $toA->fullName)), $erSubs);
    }
    
    if (!$fromA->ok) {
      if ($action == 'pay' or $fromA->closed) return; // keep recurring donations around in case member re-opens account (but until then don't do it)
    }

    $data = compact('created');
    if ($flags) foreach (ray(TX_FLAGS) as $k) if (!strhas($k, 'u') and u\getBit($flags, $k)) $data[$k] = TRUE;

    $DBTX = \db_transaction();
    if ($action == 'pay') {
      $data['self'] = TRUE;
      if ($to == CGID and $period != PER_1) $data['gift'] = TRUE; // assume any recurring payment to CG is a gift
      if (is_null($amount)) { // sweep (to bank or to another account)
        if (!$amount = max(0, $fromA->balance - $fromA->minimum) or $from == UID_BANK) return;
      }
      
      if (in(UID_BANK, [$from, $to])) { // bank transfer (NULL amount means sweep to bank)
        $info = compact('recursId');
        $why = tr('auto transfer', compact('when'));
        if ($to == UID_BANK) { // transfer from CG account to bank
          $avail = $fromA->avail(FOR_USD);
          if ($avail < $amount) return $fromA->tell('auto bankout nsf', ray('when avail amount', $when, u\fmtAmt($avail), u\fmtAmt($amount)));
          $fromA->bank(-$amount, $why, $info);
        } else $toA->bank($amount, $why, $info); // transfer to CG account from bank
      } else { // payment/transfer to another account
        $z = $fromA->payApproved($to, $amount, $purpose, $action == 'pay' ? FOR_GOODS : FOR_USD, $data, $recursId);
        if (!$z->ok) return r\tellAdmin(t('Pay failed in recur'), $erSubs);
        $fromA->tellOut('recur pay', ray('amount when purpose payee', u\fmtAmt($amount), $when, $purpose, $toA->fullName));
      }
    } else { // charge
      $z = be\invoice($toA, $fromA, $amount, $purpose, FOR_GOODS, $data, $recursId);
      if (!$z->ok) return r\tellAdmin(t('Invoice failed in recur'), $erSubs + just('msg', $z));
    }

    unset($DBTX);
    $msg = '';
  }
  
  if ($period == PER_1) r\endRule('tm', $recursId);
  if (db\count('tx_hdrs', 'recursId=:recursId AND created >= :NOW-:HOUR_SECS', $subs = compact('recursId')) != 1) r\tellAdmin(t('Double-dipped recurrence'), $subs);
  return [TRUE, $msg]; // this return is used only when called from stripe.inc
}

/**
 * Adjust fiscal sponsorship fees
 */
function adjustFS() {
  $uids = db\col('uid', 'u_company', '(coFlags&(1<<:CO_SPONSORED)<>0)');

  foreach ($uids as $uid) {
    $a = r\acct($uid); if (!$a->ok) continue;
    if (in($qid = $a->mainQid, 'NEWAZV NEWBTY NEWCBR NYAAUN')) continue; // internal and historic funds
    $nm = "$a->fullName ($qid)";
    $nmSub = compact('nm');
    $donsToDate = db\sum('amt', 'txs_noreverse', ':IS_GIFT AND uid2=:uid', compact('uid'));
    if ($donsToDate <= 0) continue;
    if (!$ruleRow = db\row('tx_rules', "action=:ACT_SURTX AND payee=:uid AND purpose LIKE 'fiscal sponsorship fee%' AND `end` IS NULL", compact('uid'))) {$msgs[] = t('missing FS rule for ') . $nm; continue;}
    extract(just('id portion', $ruleRow)); $rule = $id;
    $feesCrit = compact(ray('uid rule'));
    if (!$feesToDate = db\sum('amt', 'txs_noreverse', 'rule=:rule AND uid1=:uid', $feesCrit)) {$msgs[] = t('no FS fees to date for ') . $nm; continue;}

    $paidCG = db\sum('amt', 'txs_noreverse', 'uid2=:CGID AND uid1=:uid', compact('uid'));
    if (!$fee1desc = db\get('description', 'tx_entries', 'rule=:rule AND uid=:uid ORDER BY ABS(id)', compact('rule', 'uid'))) {$msgs[] = t('no first fee description for ') . $nm; continue;}

    if (!preg_match('/.*([\.0-9]+)%/', $fee1desc, $m)) {$msgs[] = t('missing percentage in first FS fee for ') . $nm; continue;}
    [$rightFee, $rightPct] = cr\fsFee($donsToDate, $fee1pct = $m[1]);
    $offBy = round($rightFee - $feesToDate, 2);

    if (abs($offBy) < 5) continue; // ignore if off by less than this (because of rounding -- we give partial cents back when rate changes)
    
    $feeXids = db\col('xid', 'tx_entries e', $feesCrit);
    $giftXids = db\col('xid', 'txs', ':IS_GIFT AND uid2=:uid', compact('uid'));
    if ($nFeeXids = array_diff($giftXids, $feeXids) and count($nFeeXids) > 0) $msgs[] = $nm . t(' gift xids with no FS fee: ') . pr($nFeeXids);
    if ($nGiftXids = array_diff($feeXids, $giftXids)and count($nGiftXids) > 0) $msgs[] = $nm . t(' FS fee xids with no gift: ') . pr($nGiftXids);
        
    if ($offBy > 0) {$msgs[] = t('Charged FS fees too low by $%offBy for %nm (charged $%feesToDate, should be $%rightFee)', compact(ray('nm offBy feesToDate paidCG rightFee'))); continue;}

    if ($rightPct == $fee1pct) continue; // not time to adjust yet

    // time to adjust the fee
    $purpose = t('fiscal sponsorship fee (%rightPct%)', compact('rightPct'));

    $allIncome = db\sum('amt', 'txs_noreverse', 'uid2=:uid AND (type IN (:E_PRIME, :E_OUTER) OR (type=:E_BANK AND amt>0 AND :IS_GIFT))', compact('uid'));
    $info = compact(ray('uid rule offBy portion rightFee rightPct donsToDate allIncome fee1pct fee1desc feesToDate purpose'));
    $msgs[] = t('adjusting FS fee for ') . $nm . ' ' . pr($info);
    db\update('tx_rules', ray('id portion purpose', $rule, round($rightPct/100, 4), $purpose), 'id');
    $cat = [NULL, r\nick2cat('FS-FEE')];
    r\acct($uid)->payApproved(CGID, $offBy, t('fiscal sponsorship fee adjustment'), FOR_GOODS, compact(ray('rule cat')));
  }

  if ($msg = join("\n", nn($msgs, []))) {
    if (test()) t\output($msg, 'normal'); else
    r\tellAdmin(t('FS rate adjustments'), compact('msg'));
  }
  queueNext();
}

/**
 * Calculate the appropriate fiscal sponsorship fee, given donation volume to date
 * @param numeric $dons: donations to date
 * @param numeric $fee1pct: original fee percentage
 * @return [$fee, $pct]: the expected fee total and correct percentage going forward
 */
function fsFee($dons, $fee1pct) {
  $CK = 100000;
  $pct = $fee1pct;
  $fee = 0;
  
  while ($dons > $CK) {
    $fee += $CK * $pct / 100;
    $dons -= $CK;
    $pct -= 0.5;
  }
  $fee += $dons * $pct / 100;
  $fee = round($fee, 2);

  return [$fee, $pct];
}

/**
 * Do things a certain number of days after the created, signed, invite, firstTx, last login, or last access date.
 */
function tickle() {
//  $delays = '1,2,3,5,8,13,21,34,55,89,144'; // Fibonacci progression of delays
  $timeSince = "(ROUND((:NOW-@DATE)/:DAY_SECS, 0))";

  // Remind invitee to accept
  $delays = '3,8,21';
  $elapsed = str_replace('@DATE', 'invited', $timeSince);
  $sql = <<< X
    SELECT DISTINCT i.email, i.inviter, i.code, $elapsed AS elapsed FROM r_invites i
    LEFT JOIN users u ON u.uid=i.inviter
    WHERE $elapsed IN ($delays) AND i.invitee=0 AND u.:IS_OK AND i.nonudge IS NULL AND i.email<>''
X;

  queueEach('tickleSignup', $sql);

  $delays = '1,4,9,22'; // delays proposed by staff (ultimately every week)
  $elapsed = str_replace('@DATE', 'access', $timeSince);
  $sql = <<< X
    SELECT DISTINCT uid, $elapsed AS elapsed FROM users 
    WHERE ($elapsed IN ($delays) OR ($elapsed>8 AND $elapsed%7=0))
    AND uid>:UID_CANON9 AND NOT activated AND NOT :IS_NONUDGE
X;
  queueEach('tickleStep', $sql);

  $sinceActivated = 'DATEDIFF(CURDATE(), FROM_UNIXTIME(activated))';
  // Do every 30 days after activation date
  $start = strtotime(-FLOOR_DAYS . ' days', now());
  $sql = <<< X
    SELECT DISTINCT uid, GREATEST(inVol, outVol) AS monthly, $sinceActivated/30 AS months
    FROM (
      SELECT uid, u.flags, activated,
      SUM(IF(amount > 0, amount, 0))/6 AS inVol,
      SUM(IF(amount < 0, -amount, 0))/6 AS outVol
      FROM users u LEFT JOIN (
        SELECT e.amount,e.uid FROM tx_entries e JOIN txs_noreverse tnr USING(xid) WHERE tnr.created>=$start
      ) t USING(uid)
      WHERE u.:IS_OK AND u.uid>:UID_CANON9
      AND $sinceActivated>0 AND $sinceActivated % 30 = 0 
      GROUP BY uid
    ) x 
X;

  queueEach('activeMonthly', $sql); // no need for jid here (floors get summed)
  foreach (db\col('uid', 'users', ':IS_OK AND uid<=:UID_CANON9') as $uid) r\acct($uid)->redoBal();
  queueEach('activeWeekly', "SELECT DISTINCT uid FROM users WHERE :IS_OK AND uid>:UID_CANON9 AND $sinceActivated>0 AND $sinceActivated % 7 = 0");
  queueNext();
}  

/**
 * Remind invitee to accept invitation.
 */
function tickleSignup($item) {
  extract(just('email inviter code elapsed', $item));
  if (r\acct($email = u\decry('P', $email))) return; // invitee already began to open an account
  $inviterA = r\acct($inviter);
  $inviterName = $inviterA->fullName;
  $nudge = t($elapsed < 10 ? 'reminder coming' : 'reminder last');
  r\rMail('nonmember', $email, ray('fullName nudge code noFrame', $inviterName, $nudge, $code, TRUE));
  $inviterA->tellMisc('invite languishing', compact('email', 'elapsed'));
}

/**
 * Remind newb to take another step.
 */
function tickleStep($item) {
  extract(just('uid elapsed', $item));
  $a = r\acct($uid);

  if (!$a->member and $step = $a->nextStep()) {
    $msg = ($a->stepCount() >= count(ray(S_PERSON)) - 2) ? 'do step one' : 'take another step';
    $a->tellMisc("$msg|sign in|daily messages");
    if ($elapsed >= 9 and $elapsed < 30) {
      $sql = 'SELECT DISTINCT inviter FROM r_invites i LEFT JOIN users u ON u.uid=i.inviter WHERE invitee=:uid AND u.:IS_OK';
      $q = db\q($sql, compact('uid'));
      while ($row = $q->fetchAssoc()) {
        extract($row);
        r\acct($inviter)->tellMisc('invitee slow', ray('fullName elapsed step', $a->fullName, $elapsed, $step));
      }
    }
  } elseif ($a->member and !$a->ok) {
    return r\tellAdmin('activate account', compact('uid'));
  }
}

/**
 * Do stuff monthly (every 30 days) since member has been active:
 * . Calculate credit line (for overdrafts etc.): give the member (cg)credit up to their average monthly throughput over the past 6 months
 *   (which comes to 1/6 their monthly throughput if they've been in for a month, 2/6 for 2 months, etc, up to 100% at 6 months)
 *   A community's credit limit is the amount in its Dollar Pool.
 * . Cashout automatically if account chose that monthly option.
 */
function activeMonthly($item) {
  extract(just('uid monthly', $item));
  $a = r\acct($uid);

  // adjust credit line
  $floorX = $a->o_floor; // current floor (of just this account, not summed with joint)
  
  if (!test()) // don't redo balance in tests because we fake balances
  $a->redoBal(); // balance gets off occasionally
  if ($a->balance < 0 and !$a->wentNeg) $a->update('wentNeg', now());
  if ($a->balance >= 0 and $a->wentNeg) $a->update('wentNeg', '');
  
  if ($a->balance < 0 and now() - $a->wentNeg > R_DEBT_DAYS * DAY_SECS) { // negative for too long
    $m = u\monthDif(now(), $a->wentNeg); // how many months ago the account went negative (rounded)
    $m -= $a->wentNeg > strtotime("-$m months", now()) ? 2 : 1; // don't count the first month or a partial subsequent month
    $floor = round($floorX * (1 - (1 / max(FLOOR_DECAY_MOS - $m, 1))), 2); // reduce floor progressively over time (while balance is negative)
    $floor = min(0, max($floor, $floorX + 1)); // reduce credit line by at least a dollar but not beyond zero
    r\tellAdmin(t('Account negative 30+ days'), ray('who balance wentNegative oldCreditLimit newCreditLimit', "$a->fullName ($a->mainQid)", $a->balance, fmtDt($a->wentNeg), -$floorX, -$floor));
  } else { // not overly negative, so calculate credit line (-floor) baseded on activity
    $floor = -$monthly;
    if ($a->isCtty) $floor = -db\sum('amount', 'txs2_bank t JOIN users u ON u.uid=t.payee', 'u.community=:ctty', ray('ctty', $a->id));
  }
  
  if ($floorX > 0) $floor = $floorX; // change a reserved amount only explicitly
  $a->update('floor', $floor);

  $ratio = $floor ? $floorX / $floor : 1;
  if ($ratio > 1.2 or $ratio < .8) { // big change (or any change in sign)
    $limit = u\fmtAmt(-$a->floor); // (report total to master of joint accounts)
    $msg = 'new floor' . ($a->can(B_DEBT) ? '' : '|no floor effect');
//    r\tellAdmin('new floor', compact(ray('limit monthly floor floorX ratio msg')));
  // DISABLED FOR now() if (!$a->slave) $a->tellMisc($msg, compact('limit'));
  }
  
  if ($a->cashoutM) $a->cashout(t('monthly'));
}

function activeWeekly($item) {
  extract(just('uid', $item));
  $a = r\acct($uid);
  if ($a->cashoutW) $a->cashout(t('weekly'));
}

/**
 * Warn members about upcoming annual donation (or lack thereof) -- a week before it's due.
 */
function annualGift() {
  $join = "LEFT JOIN tx_timed tm ON tm.action=:ACT_PAY AND tm.from=u.uid AND tm.to=:CGID AND tm.period='year' AND tm.end IS NULL";
  queueEach('warnAnnual1', annualSql('Donate', 'tm.amount', 'IFNULL(tm.start, u.activated)', $join));
  queueNext();
}

/**  
 * Warn members about upcoming backing renewal date (a week ahead).
 */
function renewBacking() {
  $sql = annualSql('Backing', 'backing AS amount', 'backingDate', '', 'backingNext IS NULL');
  queueEach('warnAnnual1', $sql);
  db\q('UPDATE users SET backing=IFNULL(backingNext, backing), backingDate=:today, backingNext=NULL WHERE backingDate+:YEAR_SECS<:NOW AND :IS_OK AND backing>0', ray('today', today())); // automatically renew (member decreased it or didn't update it in time)
  queueNext();
}
 
/**
 * Give the member a chance to renew their annual donation or backing.
 */
function warnAnnual1($item) {
  extract(just('func uid amount base target', $item));
  $tpls = ray('Donate:annual-gift, Backing:renew-backing');

  $a = r\acct($uid);
  $atag = w\atag("/do/do$func~" . $a->doCode());
  $track = u\cryRay(ray('qid type', $a->mainQid, strtolower($func)));
  $subs = ray('amount when atag track', u\fmtAmt($amount), fmtDt(strtotime($target)), $atag, $track);
  $a->tellMisc($tpls[$func], $subs);
}

function annualSql($func, $amtFld, $dtFld, $join = '', $where = '1') {
  return <<< X
    SELECT DISTINCT '$func' AS func, u.uid, $amtFld, FROM_UNIXTIME($dtFld) AS base,
      ADDDATE(CURDATE(), INTERVAL :RENEW_ADVANCE_DAYS DAY) AS target,
      ADDDATE(CURDATE(), INTERVAL (:RENEW_ADVANCE_DAYS-1) DAY) AS beforeTarget
    FROM users u $join
    WHERE u.:IS_OK AND u.uid>0 AND u.uid NOT :CANONIC AND $where
    HAVING TIMESTAMPDIFF(YEAR, base, target) > TIMESTAMPDIFF(YEAR, base, beforeTarget)
X;
}

/**
 * Do periodic activities where the period is longer than a day.
 */
function lessOften() {
  often('week', R_WEEKLY_DAY);
  often('month', R_MONTHLY_DAY);
  often('year', '01');  
  queueNext();
}

/**
 * Do something periodically if it's time and hasn't been done yet.
 * That is, if the chosen point in the period has been reached AND
 *   (it has been at least a whole period since the last run OR
 *   (it has been at least half a period and we're at no later a point than last time)
 * @param string $period: the period (hour, day, week, month, year)
 * @param string $chosen: the chosen value to match or exceed (if omitted, assume it's the current period)
 */
function often($period, $chosen = '') {
  if (!timeFor(now(), $period, $chosen)) return;

  $lastCron = getV('last_cron_end');
  $lastCron[$period] = now();
  setV('last_cron_end', $lastCron); // remember that this periodic activity has been done
  
  $function = 'CG\\Cron\\every' . ucwords($period);
  $topic = t('cron period', compact('period'));
  $time = fmtTime();
  $msg = t('Running cron for the %period, at %time', compact('period', 'time'));
  if ($period != 'hour') r\tellAdmin($msg, compact('topic'));

  $lastStart = getV('last_cron_start');
  $lastStart[$period] = now();
  setV('last_cron_start', $lastStart); // remember that this periodic activity has been done

  $function(); // do it
}

/**
 * Do weekly tasks.
 */
function everyWeek() {
  // help members increase their overall savings (or crawl out of primary account debt)
  $refillable = ':IS_OK AND :HAS_BANK AND :IS_REFILL';
  $newMin = "(minimum+GREATEST(saveWeekly, :MIN_WEEKLY_BUMP))"; // crawl out of debt by at least $1 a week
  db\q("UPDATE users SET minimum=IF(ABS($newMin)<.01, .01, $newMin) WHERE $refillable AND minimum<GREATEST(0, floor)"); // crawl out of debt
  db\q("UPDATE users SET minimum=minimum+saveWeekly WHERE $refillable AND saveWeekly>0 AND minimum>=GREATEST(0, floor)"); // build up savings
//  db\q('UPDATE users SET savingsAdd=GREATEST(0, savingsAdd+saveWeekly) WHERE :IS_OK AND :HAS_BANK AND saveWeekly<>0'); // save or unsave
}

/**
 * Do monthly tasks.
 */
function everyMonth() {
  $start = u\monthDay1($end = u\monthDay1() - 1); // mark start and end of preceding month
  if (test()) $start = strtotime('-30 days', $end = today() - 1) + 1; // test: the past 30 days

  $omitUids = r\qo('NEWARJ')->id // people who don't want to see the "whats up" message
  . ',' . r\qo('NEWBWN')->id;
  
  queueEach('whatsup', "SELECT DISTINCT uid, u.:IS_CO as co, MAX(t.created) AS lastTx FROM users u LEFT JOIN tx_entries e USING(uid) JOIN tx_hdrs t USING(xid) WHERE uid>:UID_CANON9 AND uid NOT IN ($omitUids) AND u.:IS_OK GROUP BY uid HAVING lastTx IS NULL OR lastTx < $start");
  
  $sql = <<< X
    SELECT DISTINCT u.uid, ROUND(SUM(t.amt)*u.crumbs, 2) AS donate, $end AS date
    FROM users u LEFT JOIN txs_noreverse t ON t.uid2=u.uid
    WHERE t.goods=:FOR_GOODS AND t.created BETWEEN $start AND $end AND u.:IS_OK
    AND (SELECT 1 FROM txs_noreverse WHERE :IS_CRUMBS AND created>=$end AND uid1=u.uid LIMIT 1) IS NULL
    AND (SELECT 1 FROM tx_requests WHERE :IS_CRUMBS AND created>=$end AND payer=u.uid LIMIT 1) IS NULL
    GROUP BY uid HAVING donate>0
X;
  queueEach('crumbs', $sql);
  queueEach('sweep', "SELECT $start AS start,$end AS end"); // must come after all crumbs
  
  $ffid = db\get('uid', 'users', "name='foodfund'");
  $amt = db\sum('amt', 'txs', 'uid2=:ffid', compact('ffid')) ?: 0;
  $amt = round($amt * FUND_TITHE - db\sum('amt', 'txs', 'uid1=:ffid AND uid2=:CGID', compact('ffid')), 2);
  $cats = [r\nick2cat('D-FBO'), r\nick2cat('FS-FEE')];
  if ($amt > 0) be\transfer('payment', r\acct($ffid), r\acct(CGID), $amt, t('reserved for administrative costs'), FOR_GOODS, ray('created cat', $end, $cats));

  // figure amount to share with each official Common Good Community
// Not until cgc means signed the CG/CGC agreement (and then not retroactive) queueEach('cgc', $cttyGiftsSql($end));
  
  $list = db\lst("CONCAT(fullName,' (',city,')')", 'users', ':IS_OK AND :IS_PAPER');
  r\tellAdmin(t('Send paper statements'), compact('list'), 0); // 0 = don't state current account
  
  if ($col = db\col('uid', 'users', 'NOT :IS_OK AND balance<>0')) r\tellAdmin(t('Closed account(s) with non-zero balance'), ray('uids', join(',', $col)));
}

/**
 * Ask the member what's up, if they haven't done anything for a month or more.
 */
function whatsup($item) {
  extract(just('uid lastTx co', $item));
  $msg = 'whats up' . ($co ? '|co promo' : '');
  r\acct($uid)->tellMisc($msg, ray('lastTx', fmtDt($lastTx)));
}

/**
 * Yearly
 */
function everyYear() {
  $topic = t('Annual task list');
  r\tellAdmin(t('Generate 1099-K forms for IRS and states'), compact('topic'));
  
  $year = date('Y') - 1;
  $start = strtotime("$year-01-01"); // previous year
  $end = u\plusMonths(12, $start) - 1;
  queueEach('taxInfo', "SELECT DISTINCT uid, $year AS year FROM tx_entries JOIN tx_hdrs USING(xid) WHERE created between $start AND $end AND uid NOT :CANONIC AND id>0 ORDER BY uid");
}

/**
 * Notify each user about their 1099-K form.
 */
function taxInfo($item) {
  extract(just('uid year', $item));
  r\acct($uid)->tellMisc('see tax info', compact('year'));
}

/**
 * Calculate and credit inflation adjustment to each member account.
 */
function inflationAdjust($item) {
  return; // UNUSED (but hey, we might do this someday)
  extract(just('uid start end', $item));
  $a = r\acct($uid);
  if ($a->slave) return; // slave in joint account gets no inflation (master gets inflation for both)
  if (db\exists('r_txs', 'payee=:uid AND type=:TX_INFLATION AND created>=:end', compact('uid', 'end'))) return;
  list ($avg, $min) = r\averageBalance($uid, $start, $end);
//  if ($adjustment > 0)  // don't adjust debts
//  foreach ([t('average balance') => $avg, t('credit reserve') => $ravg] as $k => $v) {
  foreach ([t('average balance') => $avg] as $k => $v) {
    $adjustment = round($v * R_INFLATION_RATE / 12, 2);
//    if ($adjustment) be\fund($uid, TX_INFLATION, $adjustment, INFLATION_DESC . t(' on your ') . $k);
  }
}

/**
 * Donate percentage of funds received to the community.
 * Allow this even if account is inactive (donation has already been made, from member's point of view).
 */
function crumbs($item) {
  extract(just('uid donate date', $item));
  $a = r\acct($uid);
  if ($donate > 0 and $a->ok) {
    $z = $a->payApproved(UID_CRUMBS, $donate, t('crumbs desc', 'pct', u\fmtAmt($a->crumbs, '%')), FOR_NONGOODS, ray('created crumbs gift', $date, TRUE, TRUE));
    if (!$z->ok) r\tellAdmin($z->msg, $z->subs);
  }
}

/**
 * Sweep roundups, crumbs, stepups, and regular contributions into Common Good's account.
 */
function sweep($item = ['end' => NOW]) {
  extract(just('end', $item ?: [], NOW)); // default is for testing
  
  $cgA = r\acct(CGID);
  foreach (ray(t('regular roundup crumb step-up'), UID_REGULARS, UID_ROUNDUPS, UID_CRUMBS, UID_STEPUPS) as $type => $uid) {
    $a = r\acct($uid);
    $amt = $a->balance - db\sum('amt', 'txs', 'uid2=:uid AND created>:end', compact(ray('uid end'))); // all but what's too new
    if ($amt > 0) {
      $z = be\transfer('payment', $a, $cgA, $amt, $type . t(' donations: ') . fmtDt($end, 'MMM YYYY'), FOR_GOODS, ray('created gift', $end, TRUE));
      if (!$z->ok) r\tellAdmin($type . t(' sweep failed: ') . $z->msg);
    }
  }
}  

/**
 * Kick half the donations originating in an organized CGC back to that community.
 */
function cgc($item) {
  extract(just('ctty in out end bounces', $item));
  $kickBack = round($in / 2 - $out - R_DEPOSIT_RETURN_FEE * $bounces, 2);
  if ($kickBack > 0) {
    list ($a, $cgA) = [r\acct($ctty), r\cgA()];
    $desc = t('50% share of local donations through %dt', 'dt', fmtDt($end));
    if ($a->ok) {
      $z = be\transfer('payment', $cgA, $a, $kickBack, $desc, FOR_NONGOODS, ray('created gift', u\monthDay1() - 1, TRUE));
      if (!$z->ok) r\tellAdmin(t('Failed sharing 50% with community ') . "$a->fullName: $z->msg");
    }
  }
}  

/**
 * Analyze anew how each member relates to other members.
 */
function setNear() {
  db\q('TRUNCATE r_near'); // start from scratch
  $queries = join(' UNION ', [
    'SELECT DISTINCT inviter AS id1, invitee AS id2, 1 AS w FROM r_invites WHERE invitee',
    'SELECT DISTINCT person AS id1, proxy AS id2, 1 AS w FROM r_proxies',
    'SELECT DISTINCT uid1 AS id1, uid2 AS id2, 1 AS w FROM txs WHERE uid1>:UID_CANON9 AND uid2>:UID_CANON9 AND uid1<uid2',
    'SELECT DISTINCT main AS id1, other AS id2, 1 AS w FROM u_relations',
    'SELECT DISTINCT u1.uid AS id1, u2.uid AS id2, 0.5 AS w FROM users u1 INNER JOIN users u2 ON (u2.city=u1.city OR u2.zip=u1.zip) WHERE u1.uid<>u2.uid',
  ]); // each type of relationship counts as one point
  $sql = <<< X
    INSERT INTO r_near
    SELECT DISTINCT LEAST(q.id1, q.id2) AS uid1, GREATEST(q.id1, q.id2) AS uid2, SUM(w) AS weight 
    FROM ($queries) q 
    LEFT JOIN users u ON u.uid=q.id1
    LEFT JOIN users v ON v.uid=q.id2
    WHERE NOT u.:IS_CO AND NOT v.:IS_CO
    GROUP BY LEAST(q.id1, q.id2), GREATEST(q.id1, q.id2)
X;
  db\q($sql);
  queueNext();
}

/**
 * Update individual account stats
 * DISABLED
 */
function acctStats() {
  /*
  db\q('UPDATE users SET picture=0'); // clear temp field
  queueEach('acctStats1', 'SELECT uid FROM users WHERE uid>:UID_CANON9 AND :IS_OK');
  */
  queueNext();
}

/**
 * Update stats for one account (joint accounts are aggregated in r\Acct::j_stats(), not here)
 * stats will be an assoc: [avgBalPastMo, avgBalEver, minPastMo, etc.]
 * picture (an otherwise unused field) will be minPastMo in cents (ready for r\stats to SUM)
 */
function acctStats1($item, $asof = NULL) {
  extract(just('uid', $item));
  u\setDft($asof, now());
  $a = r\acct($uid);
  $asof = strtotime('today', $asof);
  $monthAgo = u\plusMonths(-1, $asof);
  list ($avgBalPastMo, $minPastMo) = r\averageBalance($uid, $monthAgo, $asof, TRUE);
  list ($avgBalEver) = r\averageBalance($uid, strtotime('today', $a->created), $asof, TRUE);
///  debug(compact('avgBalEver','uid','asof') + ['created' => $a->created]);
  $subs = compact('uid', 'monthAgo', 'asof');
  
  $sql = <<< X
    SELECT SUM(amt) AS giftsEver, SUM(IF(created<:monthAgo, 0, amt)) AS giftsPastMo
    FROM txs WHERE uid1=:uid AND uid2=:CGID AND :IS_GIFT AND created<:asof
X;
  $row1 = db\q($sql, $subs)->fetchAssoc() ?: []; // giftsEver, giftsPastMo
  $row2 = [];  

  $sql = <<< X
    SELECT SUM(-amount) AS bankedEver, SUM(IF(completed<:monthAgo, 0, -amount)) AS bankedPastMo
    FROM txs2_bank WHERE payee=:uid AND completed AND completed<:asof AND amount<>0
X;
  $row3 = db\q($sql, $subs)->fetchAssoc() ?: []; // get bankedEver, bankedPastMo

  $newData = $row1 + $row2 + $row3 + compact(ray('avgBalPastMo avgBalEver minPastMo'));

  foreach ($newData as $k => $v) $newData[$k] = '' . round($v, 2); // avoid long decimals
  $a->update('stats picture', serialize($newData), $minPastMo * 100);
}

/**
 * Update system-wide and community-wide stats
 * @param int $dt1: date to start (-1 means pick up where we left off)
 * @param string $redo: space-delimited list of statistics to recalculate (ALL for all, '' for none) -- used by admin only
 */
function cttyStats($dt1 = 0, $redo = '') {
  if (!$dt1) { // normally this is called without a date
    cttyStats1(['ctty' => 0]); // for all our communities total
    cttyStats1(['ctty' => 1]); // for all communities in the server's region
    queueEach('cttyStats1', 'SELECT DISTINCT uid AS ctty FROM users WHERE uid<0'); // and one at a time
    queueNext();
  } else { // recreate stats data from the beginning of time (called only manually)
/*      SELECT community AS ctty, MIN(t.created) AS start
      FROM users u LEFT JOIN r_txs t ON t.payee=u.uid 
      WHERE uid>:UID_CANON9 AND (t.created>=:R_LAUNCH OR t.created IS NULL) GROUP BY community ORDER BY -community
      */
    $sql = 'SELECT uid AS ctty, created FROM users WHERE uid<0 ORDER BY -community';
    $cttys = db\q($sql)->fetchAllKeyed();
    u\preray([0 => min($cttys), 1 => min($cttys)], $cttys); // show all dates overall

    if ($dt1 < 0) $dt1 = db\max('created', 'r_stats'); // repeat last date to catch missing cttys
    if (!$dt1) $dt1 = nni($cttys, 0, 0) ?: now();
    $dt = strtotime('today', $dt1); // use first moment of the day

    while ($dt <= now()) {
      foreach ($cttys as $ctty => $start) if ($start <= $dt) {
        $item = compact(ray('ctty dt redo'));
        if (isPRODUCTION) cr\queue('cttyStats1', $item); else cttyStats1($item);
      }
      $dt = strtotime('+1 days', $dt); // every day, not just GRAIN
    }
  }
}

define('BAN_MONTHS', 3); // number of months in a row to have + or - demand, to set credit ban on/off

function cttyStats1($item) {
  extract(just('ctty dt redo', $item));
  u\setDft($dt, $today = today());

  $stats = r\stats($ctty, $dt, nn($redo)); // calculate stats for one community on the regional server, for the given date
  
  // see if we need to set or turn off a "credit ban"
  if ($dt == $today and $ctty) {
    $allPos = $allNeg = TRUE;
    for ($i = 0; $i <= BAN_MONTHS; $i++) {
      $usdX = nn($usd, 0); // following month's Dollar Pool total
      $ago = -30 * $i;
      $stats = r\stats($ctty, strtotime($ago . ' days', $dt));
      $usd = $stats['usdIn'] + $stats['usdOut']; // this month's Dollar Pool total++
      if ($i > 0) {if ($usdX - $usd < 0) $allPos = FALSE; else $allNeg = FALSE;}
    }
    $cttyA = r\acct($ctty);
    $subs = ray('months', BAN_MONTHS);
    if ($allNeg and !$cttyA->noneg) r\tellCAdmin('credit ban on', $subs, $ctty);
    if ($allPos and $cttyA->noneg) r\tellCAdmin('credit ban off', $subs, $ctty);
  }
}

/**
 * Calculate how much each member is trusted by the others.
 */
function trust() {
  db\q('UPDATE users SET trust=IF(:IS_CO,0,1)');
  $sql = <<< X
    UPDATE users u 
    INNER JOIN
      (SELECT DISTINCT proxy AS uid,SUM(straw) AS bale
       FROM 
         (SELECT proxy,up.trust/p.priority AS straw
          FROM r_proxies p 
          LEFT JOIN users up ON up.uid=p.person) s
       GROUP BY proxy) b
    ON b.uid=u.uid
    SET u.trust=u.trust+:weight*b.bale WHERE NOT :IS_CO
X;
  for ($i = 0, $weight = 1; $i < 32; $i++, $weight /= 2) db\q($sql, compact('weight'));
  if (test()) db\q('UPDATE users SET trust=ROUND(trust, 2)'); // make the results easily testable
  queueNext();
}

/**
 * Report on individual members or member businesses that have large activity over the past day (definitely > $3k)
 */
function biggies() {
  $admin = t('Consider whether any of these high totals reflect money laundering.'); // appears as "admin: ..."
  $start = strtotime('yesterday');
  $end = today() - 1;
  $sql = <<<X
    SELECT fullName, SUM(ABS(amount)) AS total 
    FROM tx_entries JOIN users USING (uid) JOIN tx_hdrs h USING (xid)
    WHERE h.created BETWEEN $start AND $end AND uid NOT :CANONIC
    GROUP BY uid
    HAVING total>1000
    ORDER BY total DESC
X;

  if ($list = db\q($sql)->fetchAllKeyed()) r\tellAdmin(t('Big Transfers on ') . fmtDt(), compact(ray('admin list')));
  queueNext();
}

/**
 * Calculate risks of financial misbehavior for each account.
 * Do companies first, so their risk can be used to help assess individual risk.
 * Do the whole thing twice, to take into account relations with suspicious individuals.
 * @todo: use a temp table instead of newThresholds array OR just a complex SQL
 */
function acctRisk() {
  setv('newThresholds', ray('bigDay bigWeek big7Week bigYear', [], [], [], []));

  db\q('UPDATE users SET risk=NULL'); // avoid unnecessary badConx. don't touch risks though!
  foreach ([1, 2] as $zot) { // do this twice, to take into account connections with scoundrels
    queueEach('acctRisk1', 'SELECT DISTINCT uid FROM users WHERE uid>:UID_CANON9 and :IS_OK ORDER BY :IS_CO DESC');
  }
  queueNext();
}

function acctRisk1($item) {
  include_once R_ROOT . '/cg-risk.inc';
  global $newThresholds;
  
  $newThresholds = getv('newThresholds');
  k\acctRisk1($item['uid'], getv('riskThresholds', ray('bigDay:0,bigWeek:0,big7Week:0,bigYear:0')));
  setv('newThresholds', $newThresholds);
}

/**
 * Finish aggregating threshold data collected from within acctRisk1.
 */
function acctRiskFinish() {
  global $testThresholdPct;
  $newThresholds = getv('newThresholds');
  $pct = K_THRESHOLD_PCT;
  if (test()) $pct = $testThresholdPct;
  $index = ceil(count($newThresholds[key($newThresholds)]) * $pct / 100); // pointer to threshold value
  foreach ($newThresholds as $k => $ray) {
    rsort($ray);
    $riskThresholds[$k] = $ray[$index];
  }
  setv('riskThresholds', $riskThresholds);
  queueNext();
}

/**
 * Calculate risks of financial misbehavior for each recent transaction, including bank transfers.
 * Include payments from the community.
 */
function txRisk() {
  $sql = <<<EOF
    SELECT DISTINCT xid FROM txs
    WHERE risk IS NULL AND uid2 NOT :CANONIC AND amt<>0
    ORDER BY created,xid
EOF;
  queueEach('txRisk1', $sql);
  queueNext();
}

function txRisk1($xid) {
  include_once R_ROOT . '/cg-risk.inc';
  k\txRisk1(is_array($xid) ? $xid['xid'] : $xid);
}

/**
 * Get funds from a connected bank account or drawable CG account, as needed, 
 */
function getFunds() {
  $sql = 'SELECT u.uid FROM users u WHERE (jid=0 OR u.uid<jid) AND u.uid>:UID_CANON9 AND u.:IS_OK AND uid NOT IN (:ours)';
  queueEach('getFunds1', $sql, ray('ours', r\cgsOwnUids())); // CG accounts are managed explicitly
  queueNext();
}

/**
 * Mark deposited transfers complete USDTX_DAYS business days after deposit (if not already marked complete).
 */
function completeUsdTxs() {
  $sql = 'SELECT DISTINCT xid,channel FROM txs2_bank WHERE amount>0 AND NOT completed AND :NOW-created>=:HOUR_SECS AND (amount<=:USDTX_FAST OR :NOW-created>=:USDTX_DAYS*:DAY_SECS)'; // this allows us to wait arbitrarily long to deposit chks or do ACHs
  queueEach('completeUsdTx', $sql);
  queueNext();
}

function completeUsdTx($item) {
  extract(just('xid channel', $item));
  be\completeUsdTx($xid);
  be\usdTxFollowup($xid);
}

/**
 * Get additional funds from bank account as needed.
 * Strategy:
 * (1) Make up for any shortfall of pending transfers (completed=0) compared to pending requests (status=approved, ignoring B_FUNDING bit).
 * (2) Plus bring balance up to target, if autofill (considering #1 only if shortfall is negative).
 * (3) If balance has been negative for too long and not autofill (or target is negative), bring it up to zero.
 */
function getFunds1($item) {
  extract(just('uid', $item, NULL));
  $a = r\acct($uid);
  $uids = $a->uids;

  $requests = db\sum('amount', 'tx_requests', 'payer IN (:uids) AND status=:TX_APPROVED', compact('uids'));
  $pendingIn = db\sum('amount', 'txs2', 'payee IN (:uids) AND completed=0', compact('uids')); // uids (not uid) in case joint account formed recently
  $need = $requests - $pendingIn;

  if ($need > 0) {
    $a->getFunds($need, tr('to pending')); // this should never happen after 9/1/2024 because getFunds happens when the request is approved
    if (now() > strtotime('9/1/2024')) r\tellAdmin('unexpected "to pending"', compact(ray('uid need')));
    $need = 0; // positive need no longer relevant
  }
  
  if ($a->balance < 0 and !$a->wentNeg) {
    r\tellAdmin('missing wentNeg', ray('qid', $a->mainQid)); // shouldn't happen, but always fix it if it does
    $a->update('wentNeg', now());
  }

  if ($a->refill) { // auto refills
    $need = $a->minimum - $a->balance + min(0, $need);
    if ($need > 0) $a->getFunds($need, tr('to target'));
  } elseif ($a->balance < 0 and $need >= 0) { // negative balance (not already pending from bank)
    if (!$a->debt) { // member chose never to go negative
      $a->getFunds(-$a->balance, tr('to zero'));
    } elseif ($a->wentNeg and now() - $a->wentNeg > R_DEBT_DAYS * DAY_SECS and !in($uid, r\cgsOwnUids())) { // debt timeout
      $a->getFunds(-$a->balance, tr('debt timeout', 'signed', fmtDt($a->signed)));
    }
  }
}

/**
 * Try to pay any approved transaction requests.
 */
function payInvoices() {
  queueEach('payInvoice', 'SELECT u.uid, i.nvid FROM users u JOIN tx_requests i ON i.payer IN (u.uid, u.jid) AND i.status=:TX_APPROVED WHERE (jid=0 OR u.uid<jid) AND u.uid NOT :CANONIC AND u.:IS_OK');
  queueNext();
}

function payInvoice($item) {
  extract(just('uid nvid', $item, NULL));
  $a = r\acct($uid);
  $remains = be\invRemains($nvid);
  $z = be\payInvoice($nvid, $a->balFirst, $remains);
}

/**
 * Handle pending requests (invoices and unfunded payments).
 */
function nudges() {
  $sql = <<< X
    SELECT DISTINCT i.*
    FROM tx_requests i JOIN users u1 ON u1.uid=i.payer JOIN users u2 ON u2.uid=i.payee
    WHERE i.status=:TX_PENDING AND u2.:IS_OK
    ORDER BY i.nvid
X;
  queueEach('nudge', $sql);
  queueNext();
}

function nudge($item) {
  extract(just('nvid payer payee amount purpose status created', $item));
  $a1 = r\acct($payer);
  $a2 = r\acct($payee);
  
  $daysAgo = max(1, round((now() - $created) / DAY_SECS));
  $nudge = $a2->staleNudge;
  if ($nudge !== 0 and $daysAgo % ($nudge ?: STALE_NUDGE) == 0) {
    $subs = ray('daysAgo created amount purpose nvid _a1 payerName payeeName email phone', $daysAgo, fmtDt($created), u\fmtAmt($amount), $purpose, $nvid, $a1->doLink('inv', $nvid), $a1->fullName, $a2->fullName, $a2->email, $a2->phone);
    $a1->tellMisc('stale invoice', $subs + ['reply' => $payee]); // tell other party by email about this invoice
    $a2->tellMisc('stale invoice report', $subs); // tell the payee we're following up on it
  }
}

/**
 * Send daily, weekly, or monthly message digests. (Newbies are always daily)
 */
function notices() {
  global $timeFor; // for testing
  
  $lastStart = getV('last_cron_start', []); // base timeForW and timeForM on cron start day, not current day (?!)
  $timeForW = (int) (fmtDt(nni($lastStart, 'day', now()), 'e') == R_WEEKLY_DAY);
  $timeForM = (int) (fmtDt(nni($lastStart, 'day', now()), 'd') == R_MONTHLY_DAY);
  if (test()) list ($timeForW, $timeForM) = [(int) nni($timeFor, 'weekly'), (int) nni($timeFor, 'monthly')];

  $sql = "SELECT DISTINCT u.uid, notices, $timeForW AS timeForW, $timeForM AS timeForM FROM r_notices n JOIN users u USING (uid) WHERE NOT sent AND NOT (:CLOSED)";
  queueEach('notice1', $sql);
  
  $fields = 'pid,uid,fullName,email,notices,latitude,longitude,zip'; // comma not space
  $sql = <<< X
    SELECT $timeForW AS timeForW, $timeForM AS timeForM, pid, $fields, '$fields' as fields FROM (
      SELECT $fields FROM people WHERE uid IS NULL
      UNION
      SELECT $fields
      FROM users u LEFT JOIN (SELECT pid, uid FROM people) p USING (uid)
      WHERE uid>:UID_CANON9 AND NOT u.:IS_CO AND IF(activated, :IS_OK, NOT :IS_NONUDGE)
    ) x
X;
  queueEach('postNotices1', $sql);
  
  $sql = <<< X
    SELECT p.postid, p.item, pp.fullName, pp.email, p.created + 365*:DAY_SECS AS dt, 
    FLOOR((p.created + 365*:DAY_SECS - :NOW) / :DAY_SECS) AS grace
    FROM posts p JOIN people pp USING(pid)
    WHERE p.end IS NULL HAVING grace IN (8, 5)
X;

  queueEach('postExpires1', $sql);
  queueNext();
}
  
function notice1($item) {
  extract(just('uid notices timeForW timeForM', $item));

  $having = "NOT sent AND IF(nType, 0, IF(wType, $timeForW, IF(mType, $timeForM, 1)))";
  if (!$args = r\noticeArgs($uid, $having, $timeForW, $timeForM)) return;
  $a = r\acct($uid);
  $track = u\cryRay(ray('qid type', $a->mainQid, 'notice'));
  $code = w\getEmailCode(compact('uid'));

  if ($a->ok) {
    $link = BASE_URL . '/do/doInvite~' . $a->doCode();
    list ($question, $btnText) 
    = ($a->co or $a->shouter) ? ['', t('Invite Someone!')] : [t('If you support %PROJECT, please say so! '), t('Endorse')];
    $style = TEXT_BUTTON_STYLE . ' padding:3px 7px; margin-left:4px;'; // smaller for emails
    $shout = '<p>' . $question . w\lnk($link, $btnText, ray('stule', $style)) . '</p>';
  } else { $shout = ''; }
  
  $args += compact(ray('shout track code'));
  if (!test()) foreach (ray('messages shout') as $k) $args[$k] = str_replace('stule=', 'style=', $args[$k]); // revert in emails (CSP forbids inline styles online)
  if (!$a->tell('notices', $args)) return; // don't mark as sent if it isn't
  db\q('UPDATE r_notices SET sent=:NOW WHERE uid=:uid AND NOT sent', compact('uid'));
}


/**
 * Send a notice about posts that are about to expire.
 */
function postExpires1($item) {
  extract(just('postid,fullName,email,dt,grace,item', $item, NULL));

  $to = [$email => $fullName];
  $dt = fmtDt($dt);
  $code = w\getEmailCode(compact('postid')); // code for recipient to cancel post
  $noFrame = TRUE;
  r\rMail('post-expires', $to, compact(ray('fullName item dt code grace noFrame')));
}

/**
 * Send a notice about recent posts to the given person.
 * The person's notices field is a string of comma-delimited lines, each one in the form:
 * $type:$freq, where
 *   $type: what type of notice (offer, need, tip)
 *   $freq: how often to notify (t=Text now/i=Email now/d=Daily/w=Weekly/m=Monthly)
 *   $radius: notify if within this mile radius (NYI)
 *   $words: notify if the post includes these words (NYI)
 */
function postNotices1($item) {
  global $base_url;
  require_once R_ROOT . '/forms/posts.inc'; // for getPosts()
  extract(just('fields timeForW timeForM', $item, NULL));
  extract(just($fields, $item, NULL));
  
  db\q('UPDATE posts SET end=:NOW WHERE end IS NULL AND created<:NOW-:YEAR_SECS');
  
  $posts = '';
  $radius = NEIGHBOR_MAX; // report everything within this many miles
  $logo = tr("<image src=\"$base_url/rcredits/images/%PROJECT_LOGO\" style=\"width:30px; margin-right:10px;\" />");
  $postLink = w\lnk('/community/posts/op=post', t('Post one!'));

  foreach (ray($notices) as $type => $freq) {
//    list ($type, $freq, $radius, $words) = u\padRay(explode('`', $one), 4);
    if (!in($type, 'offer need tip')) continue;
    if ($freq != 'd' and !($freq == 'w' and $timeForW) and !($freq == 'm' and $timeForM)) continue;

    $unit = ray('d:day,w:week,m:month')[$freq];
    $since = strtotime("-1 $unit", now());

    $q = w\getPosts($type, $latitude, $longitude, $zip, $radius, $pid ?: 0, 0, 0, $since, FALSE);
    if (!$q->rowCount()) continue;
    $rows = fmtPostRows($q);
    $Type = w\typeName($type, 2);
    $posts .= <<< X
<div style="display:table-cell; vertical-align:middle;">$logo</div>
<div style="display:table-cell; vertical-align:middle;"><b style="font-size:32px; color:navy;">$Type</b></div>
<div style="display:table-cell; vertical-align:middle; text-align:right; width:100%;">$postLink</div>
<br>
$rows
X;
  }
  if (!$posts) return;

  $posts = "<div style=\"font-family:Helvetica; max-width:512px; margin:0 auto;\">$posts</div>";
  if (test()) $posts = strtr($posts, ['src='=>'surc=', 'style='=>'stule']); // avoid CSP errors

  $noFrame = TRUE;  
  $code = w\getEmailCode(compact($uid ? 'uid' : 'pid'));
  r\rMail('post-notice', [u\decry('P', $email) => $fullName], compact(ray('fullName posts radius code noFrame')));
}

function fmtPostRows($q) {
  require_once R_ROOT . '/forms/posts.inc'; // for getPosts()

  while ($row = $q->fetchAssoc()) {
    extract(just('postid dist zip emergency item details city displayName', $row));

    if ($emergency) $item = '<b style="color:red;">!! </b> ' . $item;
    $btn = w\lnk('/community/posts/op=show&postid=' . $postid, t('View or Reply'), ray('style', TEXT_BUTTON_STYLE));
    if ($dist = w\fmtDist($dist, $zip)) $city .= " - $dist mi";
    if (strlen($details) > POST_DETAIL_MAXLEN) $details = substr($details, 0, POST_DETAIL_MAXLEN - 5) . '...';
    $rows[] = <<< X
<span style="font-size:22px">$item</span><br>
<b style="color:#595959; font-size:18px;">$displayName</b><span style="color:#999999; font-size:16px;">, $city</span><br>
<div style="margin-top:15px; color:#595959;">$details</div><br>
<center>$btn</center><br>
<hr style="color:silver;" />
X;
  }
  return join('', nn($rows, []));
}  

/**
 * Last task queued once the everyDay() function has gotten things started.
 */
function END() {
  global $mya; $mya = NULL; // don't tell Admin the last current member

  if ($cnt = db\count('queue') - 1 and $cnt > 0) r\tellAdmin(t('queue has %cnt dingleberries', compact('cnt'))); else db\q('TRUNCATE queue'); // reset autoincr
  $lastCron = getV('last_cron_end', []);
  $lastCron['end'] = now();
  setV('last_cron_end', $lastCron);
  $elapsed = now() - nni($lastCron, 'day', 0);
  $time = fmtTime();
  $msg = t('At %time, after %elapsed secs, daily cron ended successfully.', compact(ray('time elapsed')));
  $topic = t('cron end');
  r\tellAdmin($msg, compact('topic'));
}
