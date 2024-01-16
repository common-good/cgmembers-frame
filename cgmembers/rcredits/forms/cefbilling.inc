<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Display a list of links to monthly CEF Billing reports.
 *
 * Before running one of those reports, update our clickup data starting with the earliest month of interest:

  require_once R_ROOT . '/classes/clickup.class';
  $cu = new \Clickup();
  $start = strtotime('1oct2023'); // nobody should be adding or editing any times before that
  $end = NOW;
  $cu->updateTimes($start, $end);
  $cu->updateTasks($start, NOW); // no harm in adding all possible tasks
  $cu->updateTasks('SELECT DISTINCT parent AS task FROM cu_tasks WHERE parent IS NOT NULL'); // add parent tasks
*/
function formCEFBilling($form, &$sta, $month = '') {
  global $mya;
  $myid = $mya->id;

  $title = t('CEF Billing');
  
  if ($month) return showMo($month);
  
  $subtext = t('Choose a month (or quarter or year) to invoice CEF for.');
  return cgform(monthChoices(t('CEF Billing'), 'sadmin/cef-billing', ray('subtext start end', $subtext, -13, -1)));
}

function showMo($month, $dt9 = '') {
  
  if ($dt9) { // alternate syntax: (dt1, dt9) but always whole months
    $dt0 = $month;
    $month = fmtDt($dt0, 'MMM yyyy') . t(' to ') . fmtDt($dt9-1, 'MMM yyyy');
  } else {
    $dt0 = strtotime("1$month");
    $dt9 = u\plusMonths(1, $dt0);
  }
  $sqlTitle = t('CEF Hours By Task ' . $month);
  
  // NOTE: both the subtasks and parent tasks must billable -- so we check for unbillable twice below
  
  $sql = <<<X
  SELECT CONCAT(
    '<', 'a href="https://app.clickup.com/t/', ts.id, '" target="_blank">',
    ts.name, ' (', IFNULL(ts.status, ''), ')', 
    IF(ts.tags='', '', CONCAT(' [', IFNULL(ts.tags, ''), ']')),
    '</a>'
  ) AS TaskName,

  IFNULL(ts.class, ts.class) AS `Class.`,
  ROUND(SUM(secs)/3600, 2) AS Hours,
  ROUND(SUM(secsToDate)/3600, 2) AS ToDate,
  ROUND(ts.estimate/3600, 2) AS Estimate,
  ts.cap AS Cap,
  IF(ROUND(SUM(secsToDate)/3600, 2) > ts.cap, LEAST(ROUND(SUM(secs)/3600, 2), ROUND(SUM(secsToDate)/3600, 2) - ts.cap), '') AS `Over`

  FROM (
    SELECT IFNULL(ts.parent,ts.id) AS tsk, 
    stop-start AS secsToDate,
    IF(stop<$dt0, 0, stop-start) AS secs
    FROM cu_times tm JOIN cu_tasks ts ON ts.id=tm.task
    WHERE stop<$dt9 AND IFNULL(ts.tags, '') NOT LIKE '%unbillable%' 
    AND IFNULL(ts.name, '') NOT LIKE '%Staff and Outreach Partners still cannot create subscribers%'
  ) tsum

  JOIN cu_tasks ts ON ts.id=tsum.tsk
  JOIN cu_lists ls ON ls.id=ts.list 
  JOIN cu_folders f ON f.id=ls.folder 
  WHERE f.name='CEF' 
  AND IFNULL(ts.name, '') NOT LIKE '%Feature #2%'
  AND IFNULL(ts.name, '') NOT LIKE 'Payments Issues%'
  AND IFNULL(ts.tags, '') NOT LIKE '%unbillable%' 
  GROUP BY ts.id
  HAVING Hours<>'0.00'
  ORDER BY TaskName
X;

/*
    FROM_UNIXTIME(MIN(IFNULL(start, :NOW))) AS start0,
    FROM_UNIXTIME(MAX(IFNULL(stop, 0))) AS stop0,
  CONCAT_WS('/', MID(start0, 6, 2), MID(start0, 9, 2)) AS Start,
  CONCAT_WS('/', MID(stop0, 6, 2), MID(start0, 9, 2)) AS Stop,
*/

  $info = compact(ray('sql sqlTitle'));

  return w\go('/community/data/code=' . u\cryRay($info));
}
