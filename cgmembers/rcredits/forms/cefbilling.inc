<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

function formCEFBilling($form, &$sta, $month = '') {
  global $mya;
  $myid = $mya->id;

  $title = t('CEF Billing');
  
  if ($month) return showMo($month);
  
  $subtext = t('Choose a month (or quarter or year) to invoice CEF for.');
  return cgform(monthChoices(t('CEF Billing'), 'sadmin/cef-billing', ray('subtext start end', $subtext, -13, -1)));
}

function showMo($month) {
  
  $sqlTitle = t('CEF Hours By Task ' . $month);
  $dt0 = strtotime("1$month");
  $dt9 = u\plusMonths(1, $dt0);
  $dt = fmtDt($dt0, 'Y-m');
  
  $sql = <<<X
    SELECT CONCAT(
    '<', 'a href="https://app.clickup.com/t/', IFNULL(tsp.id, ts.id), '" target="_blank">',
    IFNULL(tsp.name, ts.name), ' (', IFNULL(ts.status, ''), ')', 
    IF(tsp.tags <> '' OR ts.tags <> '', CONCAT(' [', TRIM(CONCAT(IFNULL(tsp.tags, ''), ' ', IFNULL(ts.tags, ''))), ']'), ''),
    '</a>'
  ) AS TaskName,

  IFNULL(ts.class, tsp.class) AS `Class.`,
  ROUND(SUM(secs)/3600, 2) AS Hours,
  ROUND(SUM(toDate)/3600, 2) AS ToDate,
  ROUND(SUM(ts.estimate)/3600, 2) AS Estimate,
  ts.cap AS Cap,
  IF(ROUND(SUM(toDate)/3600, 2) > ts.cap, ROUND(SUM(toDate)/3600, 2) - ts.cap, '') AS `Over`

  FROM (SELECT
    task AS taskid,
    SUM(cu0.stop-cu0.start) AS secs,
    cu2.secs AS toDate
    FROM cu_times  cu0 
    JOIN (SELECT task, SUM(stop-start) AS secs FROM cu_times WHERE stop<$dt9 GROUP BY task) cu2 USING (task)
    WHERE MID(FROM_UNIXTIME(stop), 1, 7)='$dt'
    GROUP BY task
  ) tsum

  JOIN cu_tasks ts ON ts.id=tsum.taskid
  LEFT JOIN cu_tasks tsp ON tsp.id=ts.parent
  JOIN cu_lists ls ON ls.id=ts.list 
  JOIN cu_folders f ON f.id=ls.folder 
  WHERE f.name='CEF' 
  AND IFNULL(tsp.name, '') NOT LIKE '%Feature #2%'
  AND IFNULL(tsp.name, '') NOT LIKE 'Payments Issues%'
  AND IFNULL(ts.name, '') NOT LIKE '%Staff and Outreach Partners still cannot create subscribers%'
  AND CONCAT(IFNULL(tsp.tags, ''), ' ', IFNULL(ts.tags, '')) NOT LIKE '%unbillable%' 
  GROUP BY IFNULL(tsp.id, ts.id)
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
