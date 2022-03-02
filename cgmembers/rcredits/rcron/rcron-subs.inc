<?php
namespace CG\Cron;
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Risk as k;
use CG\Cron as cr;

/**
 * @file
 * Subroutines for Cron activities.
 
 f('cr.zot', arg1, ...);
*/

function queueEach($op, $sql, $subs = []) {
  $result = db\q($sql, $subs);
//  $sql = u\SUBS($sql);

  while ($row = $result->fetchAssoc()) {
    cr\queue($op, $row);
  }
}

/**
 * Run the next daily function.
 */
function queueNext($task = NULL) {
  global $cronOp; 

  if ($cronOp) {
    cr\queue('END');
    while (db\exists('queue')) cr\run();
    return; // doing just one, so do nothing more
  } 
    
  u\setDft($task, str_replace(__NAMESPACE__ . '\\', '', caller()));
  u\loga('queueNext', u\jsonize(compact('task')));
  cr\queue(u\nextAfter($task, ray(DAILY_OPS)));
//  if (isDEV) while (db\exists('queue')) cr\run();
}

/**
 * Say whether it's time to run a periodic function.
 * @param int $time: current time (or time to test)
 * @param string $period: name of period type (day, hour, etc.)
 * @param int $chosen: at which point should we do the function ('' means now)
 * @return <it's now a different period than when the function was last run AND we're at or past the chosen point>
 */
//define('TIME_CODES', 'year:Ym,month:md,week:Ua,day:dH,hour:HM');
const TIME_CODES = 'year:YM,month:Md,week:We,day:dH,hour:Hm';

function timeFor($time, $period, $chosen) {
  global $timeFor; if (test() and isset($timefor)) return nni($timefor, $period);

  $lastStart = getV('last_cron_start', []);
  if ($lastStart and now() - nni($lastStart, $period, 0) < HOUR_SECS) return FALSE;
  $lastCron = getV('last_cron_end') ?: [];
  $lastTime = nni($lastCron, $period, 0);
  list ($periodCode, $withinCode) = str_split(ray(TIME_CODES)[$period]);
  $point = fmtDt($time, $withinCode); // where are we within the period
  if ($withinCode == 'a') $point = u\day2n($point); // %u and %w fail on dev machine (among others)
  list ($periodNum, $lastPeriodNum) = [fmtDt($time, $periodCode), fmtDt($lastTime, $periodCode)];
  return (!nn($lastTime) or ($periodNum != $lastPeriodNum and ($chosen === '' or $point >= $chosen)));
}

/**
 * Add a task to the cron queue.
 * @param string $op: the function name in rcron.inc
 * @param assoc $item: a single assoc argument to that function, containing parameters
 * @param bool $always: <queue it even when running tests> (normally FALSE)
 */
function queue($op, $item = [], $always = FALSE) {
  global $cronOp; // this is set when calling cron() from PHP window on test server

  $item = compact('op') + $item;
  if ((nn($cronOp) or test()) and !$always) {
    cr\doTask($item);
  } else {
    $item = serialize($item0 = $item);
    $dup = in($op, 'acctRisk1 geocode') ? FALSE : db\exists('queue', compact('item')); // allow dups just in this case
    if ($dup and in($op, 'payRequests getFunds1')) return; // not an error, but don't duplicate (payRequests was 5-minute getFunds followup on balChange)
    u\EXPECT(!$dup, 'item already in queue: ' . u\jsonize($item0));
    db\insert('queue', ray('item created', $item, now()));
  }
}
