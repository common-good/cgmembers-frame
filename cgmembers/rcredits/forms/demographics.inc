<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

function formDemographics($form, &$sta) {
  global $mya;
  $myid = $mya->id;

  $statStart = strtotime('10/23/2014'); // earliest 
  $title = t('Customer Demographics');
  
  $q = db\q('SELECT DISTINCT payer FROM tx_entries WHERE uid=:myid', compact('myid'));
  $count = $q->rowCount();
  if ($count < AGGREGATE_MIN) return w\go('/company-reports', t('There are not enough customers (%count) to show demographics (minimum %AGGREGATE_MIN).', compact('count')));
  
  while ($id = $q->fetchField()) {
    $a = r\acct($id);
    $ages[] = (now() - $a->dob) / YEAR_SECS;
    if ($a->created >= $statStart) {
      $tenures[] = $a->tenure;
      $owns[] = $a->owns ? 1 : 0;
    }
  }
  
  /**/debug('This is unfinished because I don''t understand it.');
  /*
  $tnm = <<< X
    (SELECT ucust.city, COUNT(DISTINCT ucust.uid) count
     FROM tx_hdrs tcust
     JOIN tx_entries payer USING(xid)
     JOIN tx_entries payee USING(xid)
     JOIN users ucust ON ucust.uid=payer.uid
     WHERE payee.uid=:myid AND NOT ucust.:IS_CO AND payer.amount < 0 AND payee.amount > 0
     GROUP BY ucust.state, ucust.city
     ORDER BY count DESC LIMIT 10
    ) x
X;
  $cities = db\rows('*', $tnm, 1, compact('myid'), FALSE);

  $pays = db\rows('*', $tnm, 1, compact('myid'), FALSE);
  */
  $pays = [];
  // calculate medians and avgs
  $count = w\item(number_format(count($ages)), t('Customer Count:'));
  $age = w\item((int) u\median($ages), t('Median Age:'));
  $tenure = w\item(t('%n months at current location', 'n', (int) u\median(@$tenures ?: [])), t('Median Tenure:'));
  $owns = w\item(u\fmtAmt(@$owns ? array_sum($owns) / count($owns) : 0, '%'), t('Owns vs. Rents:'));
  $cities = w\item(makeTable($cities, [t('City/Town'), t('Count')]), t('Top Cities:'));
  $pays = w\item(makeTable($pays, [t('Company'), t('Count')]), t('Also Shop At:'));
  
  return w\cgform(compact(ray('title count age tenure owns cities pays')));
}