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
    
  u\setDft($task, str_replace(__NAMESPACE__ . '\\', '', u\caller()));
  u\loga('queueNext', json_encode(compact('task')));
  cr\queue(u\nextAfter($task, ray(DAILY_TASKS)));
//  if (isDEV) while (db\exists('queue')) cr\run();
}

/**
 * Say whether it's time to run a periodic function.
 * @param int $time: current time (or time to test)
 * @param string $period: name of period type (day, hour, etc.)
 * @param int $chosen: at which point should we do the function ('' means now)
 * @return <it's now a different period than when the function was last run AND we're at or past the chosen point>
 */
define('TIME_CODES', 'year:Ym,month:md,week:Ua,day:dH,hour:HM');

function timeFor($time, $period, $chosen) {
  global $timeFor; if (u\test() and isset($timefor)) return @$timefor[$period];

  $lastStart = getV('last_cron_start', []);
  if (now() - nn($lastStart[$period], 0) < HOUR_SECS) {
    r\tellAdmin(t('Tried to run cron twice in an hour for period ') . $period);
    return FALSE;
  }

  $lastCron = \variable_get('r_last_cron') ?: [];
  $lastTime = @$lastCron[$period];
  $codes = ray(TIME_CODES);
  list ($periodCode, $withinCode) = str_split($codes[$period]);
  $point = strftime("%$withinCode", $time);
  if ($withinCode == 'a') $point = u\day2n($point); // %u and %w fail on dev machine (among others)
  list ($periodNum, $lastPeriodNum) = [strftime("%$periodCode", $time), strftime("%$periodCode", $lastTime)];
  $isTime = ($periodNum != $lastPeriodNum and ($chosen === '' or $point >= $chosen));
  if ($isTime) {
    $lastStart[$period] = now();
    setV('last_cron_start', $lastStart); // remember that this periodic activity has been done
  }
  
  return $isTime;
}

function queue($op, $item = []) {
  global $cronOp; // this is set when calling cron() from PHP window on test server

  $item = compact('op') + $item;
  if (@$cronOp or u\test()) {
    cr\doTask($item);
  } else {
    $item = serialize($item0 = $item);
    if ($op != 'acctRisk1') u\EXPECT(!db\exists('queue', compact('item')), 'item already in queue: ' . json_encode($item0));
    db\insert('queue', ray('item created', $item, now()));
  }
}
