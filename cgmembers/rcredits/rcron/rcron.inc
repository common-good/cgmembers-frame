<?php
namespace CG\Cron;
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Risk as k;
use CG\Web as w;
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
- identify pending joint accounts and alert both parties (with link to complete or cancel) select reid, u1.fullName, u2.fullName FROM r_relations r LEFT JOIN users u1 ON r.main=u1.uid LEFT JOIN users u2 ON u2.uid=r.other WHERE (SELECT reid FROM r_relations WHERE main=r.other AND other=r.main) IS NULL AND r.permission=7 
DAILY (every 5 mins for 5 mins, starting at 2am)
- checkAccts() check accounts (and their activity) more thoroughly for consistency, security, scams
- notify and maybe fine late borrowers
- suggest another donation (after a year?) if first was small
WEEKLY
- tell members what they did that was dumb
MONTHLY
- distribute community dividends (based on growth of account's wider economic ctty)
- archive the 13th previous month (13 is a parameter -- transactions are accessible for ever, but slower)
YEARLY
 */

// Calculate risks twice, to take into account connections with scoundrels
const DAILY_OPS = 'START checkAccts setNear cttyStats trust acctRisk acctRiskFinish txRisk tickle annualGift renewBacking lessOften recurs getFunds invoices invest notices END';
const LOGLESS_OPS = 'acctHourly checkAcct whatsUp tickleStep activeWeekly activeMonthly recur'; // don't log these calls
include_once __DIR__ . '/rcron-subs.inc';

/**
 * Execute a cron run when called.
 */
function run() {
  global $channel; $channel = TX_CRON; // set this even if called from PHP window by admin
// NO (happens too often)  u\log('running cron');
  // Allow execution to continue even if the request gets canceled.
  if (between(now(), strtotime('2:40am'), strtotime('2:44am'))) return; // swiftMailer crashes during this period
  
  ignore_user_abort(TRUE);

  // Prevent session information from being saved while cron is running.
  $original_session_saving = \drupal_save_session();
  \drupal_save_session(FALSE);

  // Force the current user to anonymous to ensure consistent permissions on cron runs.
  $original_user = $GLOBALS['user'];
  $GLOBALS['user'] = \drupal_anonymous_user();

  \drupal_set_time_limit(4 * 60);

  if (\lock_acquire('cron', 240.0)) {
    cr\periodic();
    \lock_release('cron');
  } else u\log(t('Attempted to run cron while it was already running.'));

  $end = now() + R_CRON_PERIOD - R_MAX_TASK_TIME;
  $sql = 'SELECT id,item FROM queue ORDER BY id';
  $q = db\q($sql);
  
  while (now() < $end and $row = $q->fetchAssoc()) {
    extract($row);
    $item = unserialize($item);
    try {
      cr\doTask($item);
      db\q('DELETE FROM queue WHERE id=:id', compact('id'));
    } catch (\Exception $e) {
      u\log('cron task failed'); // In case of exception log it and leave the item in the queue to be processed again later.
      r\tellAdmin('cron task failed id=' . $id, $item);
    }
  }
  // Restore the user.
  $GLOBALS['user'] = $original_user;
  \drupal_save_session($original_session_saving);
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
    SELECT * FROM (
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
  if (strftime('%H') + 0 == (R_DAILY_HOUR + 6) % 24) {
    $lastCron = getV('last_cron_end', []);
    if (now() - nni($lastCron, 'end', 0) > DAY_SECS) r\tellAdmin('daily cron FAILED');
  }
  
  db\q('DELETE FROM cache_form WHERE created<:REQUEST_TIME-:HOUR_SECS'); // clean up form cache (is this needed?)

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
  SELECT DISTINCT u.uid, c.uid AS ctty, u.fullName AS person, c.fullName AS cttyName 
  FROM users u LEFT JOIN users c ON u.zip RLIKE c.zip
  WHERE u.community=:seedpack AND c.uid<1 AND c.uid<>:seedpack AND !u.activated
X;
  $q = db\q($sql, ['seedpack' => r\serverUid()]);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    db\update('users', ray('uid community', $uid, $ctty), 'uid');
    r\tellAdmin(t('Not-yet-member %person was in Seedpack; should be (now is) in %cttyName', $row));
  }
  
  if ($xid = db\get('xid', '(SELECT xid,SUM(amount) AS amt FROM tx_entries GROUP BY xid) t', 'amt<>0')) tellAdmin(t('Not all transactions have entries totaling zero (eg xid=%xid).', compact('xid')));

//  $where = "RIGHT(fullName, 8)=' PARTNER' COLLATE utf8_bin AND pass='' AND NOT :IS_CO AND activated=0";
  $where = "BINARY RIGHT(fullName, 8)=' PARTNER' AND pass IS NULL AND NOT :IS_CO AND activated=0";
  eachA(function ($a) { // acts weird; do this last
    if ($jA = $a->jA and $jA->jid == $a->id) $jA->update('jid', 0);
    f('a.deleteUid', $a->id);
  } , $where);
  if ($uid = db\get('u1.uid', 'users u1 JOIN users u2 ON u2.uid=u1.jid', 'u1.jid<>0 AND u2.jid<>u1.uid')) r\tellAdmin('Orphaned joint account: ' . r\acct($uid)->fullName, [], $uid);
//  db\del('users', $where); // abandoned new joint acct (must postcede setting jids)

  // Notice which bank accounts have been verified
	$where = 'deposit<>0 AND deposit<UNIX_TIMESTAMP()-:VERIFICATION_DELAY*:DAY_SECS AND (SELECT 1 FROM r_usd d2 WHERE d2.txid=-d.txid) IS NULL';
	$uids = db\col('payee', 'r_usd d', $where);
	$DBTX = \db_transaction();
	foreach ($uids as $uid) r\acct($uid)->setBit(B_BANKOK);
	if ($uids) db\del('r_usd', 'amount=0 AND payee IN (:uids)', compact('uids'));
	unset($DBTX);
	db\q('DELETE FROM x_usd WHERE amount=0'); // no need to track these
}

/**
 * Generate rules, transactions, and invoices from template.
 * Instantiate surtx rules first so they apply to subsequent transactions.
 */
function recurs() {
  $sql = <<< EOF
    SELECT tp.*, MAX(
      IF(tp.action=:ACT_SURTX, ru.start, 
      IF(tp.action=:ACT_PAY, GREATEST(IFNULL(x.created,0), IFNULL(i.created,0)),
      i.created))
    ) AS maxStart
    FROM tx_templates tp
    LEFT JOIN tx_rules ru ON ru.template=tp.id
    LEFT JOIN txs x ON x.recursId=tp.id
    LEFT JOIN r_invoices i ON i.recursId=tp.id
    WHERE tp.action IN (:ACT_SURTX, :ACT_PAY, :ACT_CHARGE)
    AND tp.start <= :NOW
    AND IFNULL(tp.end, :NOW+1) > :NOW
    GROUP BY tp.id
    ORDER BY (tp.action=:ACT_SURTX) DESC, tp.id
EOF;

  queueEach('recur', $sql);
  queueNext();
}
  
function recur($template) {
  extract(just('id maxStart period periods end duration durations action from to purpose amount', $template));
  $shouldStart = empty($maxStart) ? now() : u\dateIncr($maxStart, $periods, $period);
  if ($end and $end < $shouldStart) return; // don't create anything after template expires
  $erSubs = compact(ray('template shouldStart'));

  if ($action == 'surtx') {
    if ($shouldStart > now() + WEEK_SECS) return; // don't create rules more than a week in advance
    $rule = justNot(ray('id period periods duration durations start end maxStart'), $template);
    $rule['template'] = $id;
    $rule['start'] = $shouldStart;
    $rule['end'] = u\dateIncr($shouldStart, $durations, $duration);
    $ok = db\insert('tx_rules', $rule, 'id');
    if (!$ok) r\tellAdmin(t('Insert rule failed in recur'), $erSubs);

  } elseif (in($action, 'pay charge')) {
    if ($shouldStart > strtotime('tomorrow')) return; // don't create transactions and invoices until the right day
    $fromA = r\acct($from); $toA = r\acct($to);
    if (!$toA->ok) return r\tellAdmin(t('Attempt to pay (or invoice from) an inactive account in recur'), $erSubs);
    if (!$fromA->ok) {
      if ($action == 'pay' or $fromA->closed) return; // keep recurring donations around in case member re-opens account
    }

    if ($period and $period != 'once') {
      $when = r\recurDesc($period, $periods);
      $erSubs += compact(ray('when purpose'));
    } else { $when = t('one-time'); }

    $data = ray('created recurs', $shouldStart, TRUE); // keep this synced with template even if cron fails for a few days
    if ($action == 'pay' and $to == CGID) $data += ['gift' => TRUE]; // assume any recurring payment to CG is a gift
    
    if ($action == 'pay') {
      $ok = $fromA->payApproved($to, $amount, $purpose, FOR_GOODS, $data, $id);
      if (!$ok) return r\tellAdmin(t('Pay failed in recur'), $erSubs);
      r\notify($from, 'recur pay', ray('amount when purpose payee', u\fmtAmt($amount), $when, $purpose, $toA->fullName));
    } else { // charge
      list ($ok, $msg, $args) = be\invoice($toA, $fromA, $amount, $purpose, FOR_GOODS, $data, $id);
      if (!$ok) return r\tellAdmin(t('Invoice failed in recur'), $erSubs + compact('msg', 'args'));
    }
    if ($period == 'once') r\endRule(TRUE, $id);
  } else FAIL(t('bad action in template: ') . pr($template));
  
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
    WHERE $elapsed IN ($delays) AND i.invitee=0 AND u.:IS_OK AND i.nonudge IS NULL
X;

  queueEach('tickleSignup', $sql);

  $delays = '1,4,9,22'; // delays proposed by staff (ultimately every week)
  $elapsed = str_replace('@DATE', 'access', $timeSince);
  $sql = <<< X
    SELECT DISTINCT uid, $elapsed AS elapsed FROM users 
    WHERE ($elapsed IN ($delays) OR ($elapsed>8 AND $elapsed%7=0))
    AND uid>:CANONIC_ACCTS AND NOT activated AND NOT :IS_NONUDGE
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
      WHERE u.:IS_OK AND u.uid>:CANONIC_ACCTS
      AND $sinceActivated>0 AND $sinceActivated % 30 = 0 
      GROUP BY uid
    ) x 
X;

  queueEach('activeMonthly', $sql); // no need for jid here (floors get summed)
  queueEach('activeWeekly', "SELECT DISTINCT uid FROM users WHERE :IS_OK AND uid>:CANONIC_ACCTS AND $sinceActivated>0 AND $sinceActivated % 7 = 0");
  queueNext();
}	

/**
 * Remind invitee to accept invitation.
 */
function tickleSignup($item) {
  extract(just('email inviter code elapsed', $item));
  if (r\acct($email = u\decry('P', $email))) return; // invitee already began to open an account
  $inviterName = r\acct($inviter)->fullName;
  $nudge = t($elapsed < 10 ? 'reminder coming' : 'reminder last');
  r\rMail('nonmember', $email, ray('fullName nudge code noFrame', $inviterName, $nudge, $code, TRUE));
  r\notify($inviter, 'invite languishing', compact('email', 'elapsed'));
}

/**
 * Remind newb to take another step.
 */
function tickleStep($item) {
  extract(just('uid elapsed', $item));
  $a = r\acct($uid);

  if (!$a->member and $step = $a->nextStep()) {
    $msg = ($a->stepCount() == count(ray(r\taskSteps($a->co ? 'co' : 'person'))) - 1) ? 'do step one' : 'take another step';
    r\notify($uid, "$msg|sign in|daily messages");
    if ($elapsed >= 9 and $elapsed < 30) {
      $sql = 'SELECT DISTINCT inviter FROM r_invites i LEFT JOIN users u ON u.uid=i.inviter WHERE invitee=:uid AND u.:IS_OK';
      $q = db\q($sql, compact('uid'));
      while ($row = $q->fetchAssoc()) {
        extract($row);
        r\notify($inviter, 'invitee slow', ray('fullName elapsed step', $a->fullName, $elapsed, $step));
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
  
  // temporarily (until 2019?) limit the rate of change of the floor (while changing to new system)
  $floor = round(min(-$monthly, .9 * $floorX), 2);
  if ($a->isCtty) $floor = -db\sum('amount', 'r_usd t JOIN users u ON u.uid=t.payee', 'u.community=:ctty', ray('ctty', $a->id));
  $a->update('floor', $floor);

  $ratio = $floor ? $floorX / $floor : 1;
  if ($ratio > 1.2 or $ratio < .8) { // big change (or any change in sign)
		$limit = u\fmtAmt(-$a->floor); // (report total to master of joint accounts)
    $msg = 'new floor' . ($a->can(B_DEBT) ? '' : '|no floor effect');
//	  r\tellAdmin('new floor', compact(ray('limit monthly floor floorX ratio msg')));
  // DISABLED FOR now() if (!$a->slave) r\notify($uid, $msg, compact('limit'));
  }
  
  if ($a->cashoutM) $a->cashout();
}

function activeWeekly($item) {
  extract(just('uid', $item));
  $a = r\acct($uid);
  if ($a->cashoutW) $a->cashout();
}

/**
 * Warn members about upcoming annual donation (or lack thereof) -- a week before it's due.
 */
function annualGift() {
  $join = "LEFT JOIN tx_templates tp ON tp.action=:ACT_PAY AND tp.from=u.uid AND tp.to=:CGID AND tp.period='year' AND tp.end IS NULL";
  queueEach('warnAnnual1', annualSql('Donate', 'tp.amount', 'IFNULL(tp.start, u.activated)', $join));
  queueNext();
}

/**  
 * Warn members about upcoming backing renewal date (a week ahead).
 */
function renewBacking() {
  $backMin = isPRODUCTION ? strtotime('7/17/2019') : 0; // because we didn't start renewing on time. Delete this minimum after 7/11/2020
  $sql = annualSql('Backing', 'backing AS amount', "GREATEST(backingDate, $backMin)", '', 'backingNext IS NULL');
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
	$atag = w\atag("/do/do$func~$a->mainQid~$a->emailCode");
  $track = u\cryRay(ray('qid type', $a->mainQid, strtolower($func)));
  $subs = ray('amount when atag track', u\fmtAmt($amount), u\fmtDate(strtotime($target), TRUE), $atag, $track);
  r\notify($uid, $tpls[$func], $subs, TRUE);
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
  // activated is for testing, even though redundant with IS_OK (NO, fix the test)
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
  $time = strftime('%I:%M%P', now());
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
  
  queueEach('whatsup', "SELECT DISTINCT uid, u.:IS_CO as co, MAX(t.created) AS lastTx FROM users u LEFT JOIN tx_entries e USING(uid) JOIN tx_hdrs t USING(xid) WHERE uid>:CANONIC_ACCTS AND u.:IS_OK GROUP BY uid HAVING lastTx IS NULL OR lastTx < $start");
  
  $sql = <<< X
    SELECT DISTINCT u.uid, ROUND(SUM(t.amt)*u.crumbs, 2) AS donate, $end AS date
    FROM users u LEFT JOIN txs_noreverse t ON t.uid2=u.uid
    WHERE t.goods=:FOR_GOODS AND t.created BETWEEN $start AND $end AND u.:IS_OK
    AND (SELECT 1 FROM txs_noreverse WHERE :IS_CRUMBS AND created>=$end AND uid1=u.uid LIMIT 1) IS NULL
    AND (SELECT 1 FROM r_invoices WHERE :IS_CRUMBS AND created>=$end AND payer=u.uid LIMIT 1) IS NULL
    GROUP BY uid HAVING donate>0
X;
  queueEach('crumbs', $sql);
  queueEach('sweep', "SELECT $start AS start,$end AS end"); // must come after all crumbs

	// figure amount to share with each official Common Good Community
// Not until cgc means signed the CG/CGC agreement (and then not retroactive) queueEach('cgc', $cttyGiftsSql($end));
  
  $list = db\lst("CONCAT(fullName,' (',city,')')", 'users', ':IS_OK AND :IS_PAPER');
  r\tellAdmin(t('Send paper statements'), compact('list'), 0); // 0 = suppress report of current account

}

/**
 * Ask the member what's up, if they haven't done anything for a month or more.
 */
function whatsup($item) {
  extract(just('uid lastTx co', $item));
  $msg = 'whats up' . ($co ? '|co promo' : '');
  r\notify($uid, $msg, ray('lastTx', u\fmtDate($lastTx)));
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
  queueEach('taxInfo', "SELECT DISTINCT uid, $year AS year FROM tx_entries JOIN tx_hdrs USING(xid) WHERE created between $start AND $end AND uid NOT :CANONIC ORDER BY uid");
}

/**
 * Notify each user about their 1099-K form.
 */
function taxInfo($item) {
  extract(just('uid year', $item));
  r\notify($uid, 'see tax info', compact('year'));
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
  if ($donate > 0 and $a->ok) $a->payApproved(CG_CRUMBS_UID, $donate, t('crumbs desc', 'pct', u\fmtAmt($a->crumbs, '%')), FOR_NONGOODS, ray('created crumbs gift', $date, TRUE, TRUE));
}

/**
 * Sweep roundups and crumbs contributions into Common Good's account.
 */
function sweep($item) {
  extract(just('end', $item));
  
  $cgA = r\acct(CGID);
  foreach (ray('roundup crumb', CG_ROUNDUPS_UID, CG_CRUMBS_UID) as $type => $uid) {
    $a = r\acct($uid);
    $amt = $a->balance - db\sum('amt', 'txs', 'uid2=:uid AND created>:end', compact(ray('uid end'))); // all but what's too new
    if ($amt > 0) {
      list ($ok, $index, $subs) = be\transfer('payment', $a, $cgA, $amt, $type . t(' donations'), FOR_GOODS, ray('created gift', $end, TRUE));
      if (!$ok) r\tellAdmin($type . t(' sweep failed: ') . t($index, $subs));
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
    $desc = t('50% share of local donations through %dt', 'dt', u\fmtDate($end));
    if ($a->ok) {
      list ($ok, $msg, $subs) = be\transfer('payment', $cgA, $a, $kickBack, $desc, FOR_NONGOODS, ray('created gift', u\monthDay1() - 1, TRUE));
      if (!$ok) r\tellAdmin(t('Failed sharing 50% with community ') . $a->fullName . ': ' . t($msg, $subs));
    }
  }
}  

/**
 * Check accounts for consistency, scams, whatever.
 */
function checkAccts() {
  $sql = <<< X
SELECT DISTINCT u.uid,u.fullName,u.jid,u2.jid AS partnerJid,r.reid,r.other AS relatedUid FROM users u
LEFT JOIN r_relations r ON r.main=u.uid
LEFT JOIN users u2 ON u2.uid=u.jid
WHERE u.uid<>0 AND u.jid AND r.permission=:B_JOINT-:B_RELATED
X;
  queueEach('checkAcct1', $sql);
  queueNext();
}

function checkAcct1($item) {
  extract($item);
  $a = r\acct($uid);
  if (!$reid or !$partnerJid or $partnerJid != $uid or $relatedUid != $jid) r\tellAdmin('bad relation', $item);
  if ($a->hasBank and !$a->masterA->vsecure['bankAccount']) {
    $a->setRisk('hasBank', FALSE);
    r\tellAdmin('hasBank(had) wo bank account', $item);
  } elseif ($a->masterA->vsecure['bankAccount'] and !$a->hasBank) {
    $a->setRisk('hasBank');
    r\tellAdmin('bank account wo (until now) hasBank', $item);
  }
  //look at agreement date and first transaction date and last transaction date. queue appropriate funcs
}

/**
 * Analyze anew how each member relates to other members.
 */
function setNear() {
  db\q('TRUNCATE r_near'); // start from scratch
  $queries = join(' UNION ', [
    'SELECT DISTINCT inviter AS id1, invitee AS id2, 1 AS w FROM r_invites WHERE invitee',
    'SELECT DISTINCT person AS id1, proxy AS id2, 1 AS w FROM r_proxies',
    'SELECT DISTINCT uid1 AS id1, uid2 AS id2, 1 AS w FROM txs WHERE uid1>:CANONIC_ACCTS AND uid2>:CANONIC_ACCTS AND uid1<uid2',
    'SELECT DISTINCT main AS id1, other AS id2, 1 AS w FROM r_relations',
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
  queueEach('acctStats1', 'SELECT uid FROM users WHERE uid>:CANONIC_ACCTS AND :IS_OK');
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
    FROM r_usd WHERE payee=:uid AND completed AND completed<:asof AND amount<>0
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
      WHERE uid>:CANONIC_ACCTS AND (t.created>=:R_LAUNCH OR t.created IS NULL) GROUP BY community ORDER BY -community
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
 * Calculate risks of financial misbehavior for each account.
 * Do companies first, so their risk can be used to help assess individual risk.
 * Do the whole thing twice, to take into account relations with suspicious individuals.
 * @todo: use a temp table instead of newThresholds array OR just a complex SQL
 */
function acctRisk() {
  setv('newThresholds', ray('bigDay bigWeek big7Week bigYear', [], [], [], []));

  db\q('UPDATE users SET risk=NULL'); // avoid unnecessary badConx. don't touch risks though!
  foreach ([1, 2] as $zot) { // do this twice, to take into account connections with scoundrels
    queueEach('acctRisk1', 'SELECT DISTINCT uid FROM users WHERE uid>:CANONIC_ACCTS and :IS_OK ORDER BY :IS_CO DESC');
  }
  queueNext();
}

function acctRisk1($item) {
  include_once __DIR__ . '/../cg-risk.inc';
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
  include_once __DIR__ . '/../cg-risk.inc';
  k\txRisk1(is_array($xid) ? $xid['xid'] : $xid);
}

/**
 * 1. Mark deposited transfers complete USDTX_DAYS business days after deposit (if not already marked complete)
 * 2. Get funds from a connected bank account or drawable CG account, as needed
 * This (in particular getFunds) should be called before invoices and investments, so those ADD to the minimum balance
 */
function getFunds() {
  $sql = 'SELECT DISTINCT txid,channel FROM r_usd WHERE amount>0 AND NOT completed AND :NOW-created>=:HOUR_SECS AND (amount<=:USDTX_FAST OR :NOW-created>=:USDTX_DAYS*:DAY_SECS)'; // this allows us to wait arbitrarily long to deposit chks
  queueEach('completeUsdTx', $sql);
  
	queueEach('getFunds1', 'SELECT uid FROM users WHERE :MASTER AND balance < minimum AND uid > :CANONIC_ACCTS AND :IS_OK AND :IS_REFILL');

  queueNext();
}

function completeUsdTx($item) {
//  r\tellAdmin('cron\\completeUsdTx: ', $item);
  extract(just('txid channel', $item));
  be\completeUsdTx($txid, $channel == TX_CRON);
}

function getFunds1($item) {
  extract(just('uid jid', $item));
  $a = r\acct($uid);
	$amount = ($a->minimum - $a->balance) - $a->getPendingDeposits();
	if ($amount > 0) $a->getFunds($amount, t('to target'));
}

/**
 * Handle pending invoices.
 */
function invoices() {
  $sql = <<< X
    SELECT DISTINCT i.*,i.:IS_FUNDING AS funding 
    FROM r_invoices i JOIN users u1 ON u1.uid=i.payer JOIN users u2 ON u2.uid=i.payee
    WHERE i.status IN (:TX_PENDING,:TX_APPROVED) AND u1.:IS_OK AND u2.:IS_OK
    ORDER BY i.nvid
X;
  queueEach('invoice1', $sql);
  queueNext();
}

function invoice1($item) {
  extract(just('nvid payer payee amount purpose status created funding', $item));
  $a = r\acct($payer);
  $a2 = r\acct($payee);
  $payeeName = $a2->fullName;
  
  if ($status == TX_APPROVED) {
		if (!$a->ok) return; // don't try to pay from an inactive account (we can delay collection by temporarily deactivating)
		$amtFld = 'amount';
    $short = $a->balance > $amount ? 0 : round($a->advances() + $amount - $a->avail(), 2); //	consider credit already used for advances from the bank

    if ($short <= 0) { // pay the invoice if we have enough funds
      list($paid, $msg) = be\payInvoice($nvid);
      if (!$paid) r\tellAdmin(t('invoice payment failed in cron'), compact('msg') + $item);
    } else $paid = FALSE;
		
		if (!$paid or $a->balance < 0) {
      if ($funding and $a->balance + db\sum('amount', 'r_usd', 'payee=:payer AND NOT completed', compact('payer')) < $amount) {
        $funding = FALSE; // fund it again, if necessary
      }
      if (!$funding) {
        if ($a->roundup) $amount = ceil($amount); // take enough for the roundup
        $DBTX = \db_transaction();
        if ($a->getFunds($amount, t('for invoice', compact('nvid')))) {
          db\setFlag('r_invoices', $nvid, B_FUNDING, TRUE);
        } else { // almost always because no bank account is connected
          $msg = 'short invoice' . ($a->hasBank ? '|expect a transfer' : '|when funded|how to fund');
          r\notify($payer, $msg, ray('short payeeName nvid', u\fmtAmt($short), $payeeName, $nvid));
        }
        unset($DBTX);
      }
		}
  } elseif ($status == TX_PENDING) {
    $daysAgo = max(1, round((now() - $created) / DAY_SECS));
    $nudge = $a2->staleNudge;
    if ($nudge !== 0 and $daysAgo % ($nudge ?: STALE_NUDGE) == 0) {
      $subs = ray('daysAgo created amount purpose nvid _a1 payerName payeeName email phone', $daysAgo, u\fmtDate($created, TRUE), u\fmtAmt($amount), $purpose, $nvid, $a->doLink('inv', $nvid), $a->fullName, $payeeName, $a2->email, $a2->phone);
      r\message($payer, 'stale invoice', $subs + ['reply' => $payee]); // tell other party by email about this invoice
      r\message($payee, 'stale invoice report', $subs); // tell the payee we're following up on it
    }
  }
}

/**
 * Handle requests to invest in an investment club.
 */
function invest() {
  queueEach('invest1', 'SELECT stakeid,uid,request FROM r_stakes WHERE request>0');
  queueNext();
}

function invest1($item) {
  extract(just('uid,stakeid,request', $item));
  $a = r\acct($uid);
  if ($a->avail($request)) {
    r\handleInvestmentRequest($stakeid, $msg);
    r\notify($uid, $msg);
  } elseif (!$a->waitingFromBank()) {
    $a->getFunds($request, t('to fulfill your recent investment request'));
  }
}

/**
 * Send daily, weekly, or monthly message digests. (Newbies are always daily)
 */
function notices() {
  global $timeFor; // for testing
  
  $lastStart = getV('last_cron_start', []);
  $day = nni($lastStart, 'day');
  $timeFor_weekly = ($day and strftime('%w', $day) == R_WEEKLY_DAY);
  $timeFor_monthly = ($day and strftime('%d', $day) == R_MONTHLY_DAY);
  if (test() and isset($timeFor)) extract(just('weekly monthly', $timeFor), EXTR_PREFIX_ALL, 'timeFor');

  $where = $dailySql = '((NOT :IS_WEEKLY AND NOT :IS_MONTHLY) OR NOT :IS_MEMBER)'; // daily
  if ($timeFor_weekly) $where .= ' OR :IS_WEEKLY';
  if ($timeFor_monthly) $where .= ' OR :IS_MONTHLY';
  $sql = "SELECT DISTINCT u.uid, :IS_WEEKLY AS weekly, :IS_MONTHLY AS monthly FROM r_notices n JOIN users u ON u.uid=n.uid WHERE IF(activated, u.:IS_OK, NOT u.:IS_NONUDGE) AND NOT sent AND ($where)";

  queueEach('notice1', $sql);
  queueNext();
}

function notice1($item) {
  extract(just('uid daily weekly monthly', $item));

  if (!$args = r\noticeArgs($uid, 'NOT sent', $weekly, $monthly)) return;
  $a = r\acct($uid);
  $args['track'] = u\cryRay(ray('qid type', $a->mainQid, 'notice'));

  if ($a->ok) {
    $link = BASE_URL . "/do/doInvite~$a->mainQid~$a->emailCode~" . (r\rdoMonth() + 12);
    list ($question, $btnText) 
    = ($a->co or $a->shouter) ? ['', t('Invite Someone!')] : [t('If you support %PROJECT, please say so! '), t('Endorse')];
    $style = TEXT_BUTTON_STYLE . ' padding:3px 7px; margin-left:4px;'; // smaller for emails
    $args['shout'] = '<p>' . $question . w\lnk($link, $btnText, ray('stule', $style)) . '</p>';
  } else { $args['shout'] = ''; }
  
  if (!test()) foreach (ray('messages shout') as $k) $args[$k] = str_replace('stule=', 'style=', $args[$k]); // revert in emails (CSP forbids inline styles online)
  if (!r\notify($uid, 'notices', $args, TRUE)) return;
  db\q('UPDATE r_notices SET sent=:NOW WHERE uid=:uid AND NOT sent', compact('uid'));
}

/**
 * Last task queued once the everyDay() function has gotten things started.
 */
function END() {
  global $mya; $mya = NULL; // don't tell Admin the last current member
  if (db\exists('queue')) r\tellAdmin('queue has dingleberries'); else db\q('TRUNCATE queue'); // reset serial field
  $lastCron = getV('last_cron_end', []);
  $lastCron['end'] = now();
  setV('last_cron_end', $lastCron);
  $elapsed = now() - nni($lastCron, 'day', 0);
  $time = strftime('%I:%M%P');
  $msg = t('At %time, after %elapsed secs, daily cron ended successfully.', compact(ray('time elapsed')));
  $topic = t('cron end');
  r\tellAdmin($msg, compact('topic'));
}
