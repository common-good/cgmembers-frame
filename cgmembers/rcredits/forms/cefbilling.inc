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
  $dt = fmtDt(strtotime("1$month"), 'Y-m');
  
  $sql = <<<X
    SELECT CONCAT(
    '<', 'a href="https://app.clickup.com/t/', IFNULL(tsp.id, ts.id), '" target="_blank">',
    IFNULL(tsp.name, ts.name), ' (', IFNULL(ts.status, ''), ')', 
    IF(tsp.tags <> '' OR ts.tags <> '', CONCAT(' [', TRIM(CONCAT(IFNULL(tsp.tags, ''), ' ', IFNULL(ts.tags, ''))), ']'), ''),
    '</a>'
  ) AS TaskName,

  IFNULL(ts.class, tsp.class) AS `Class.`,
  REPLACE(MID(FROM_UNIXTIME(MIN(IFNULL(start0, :NOW))), 1, 10), '-', '') AS Start,
  REPLACE(MID(FROM_UNIXTIME(MAX(IFNULL(stop9, 0))), 1, 10), '-', '') AS Stop,
  ROUND(SUM(hours), 2) AS Hours,
  ROUND(SUM(ts.estimate)/3600, 2) AS Estimate,
  ROUND(2 * SUM(ts.estimate)/3600, 2) AS MinCap

  FROM (SELECT
    task AS taskid,
    MIN(IFNULL(start, :NOW)) AS start0,
    MAX(IFNULL(stop, 0)) AS stop9,
    SUM(stop-start)/3600 AS hours
    FROM cu_times
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
  ORDER BY Start
X;

  $info = compact(ray('sql sqlTitle'));

  return w\go('/community/data/code=' . u\cryRay($info));
}
