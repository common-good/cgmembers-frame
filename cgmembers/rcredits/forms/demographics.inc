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

  $title = t('Customer Demographics');
  
  $q = db\q('SELECT DISTINCT payer FROM r_entries WHERE uid=:myid', compact('myid'));
  $count = $q->rowCount();
  if ($count < AGGREGATE_MIN) return w\go('/company-reports', t('There are not enough customers (%count) to show demographics (minimum %AGGREGATE_MIN).', compact('count')));
  
  while ($id = $q->fetchField()) {
    $a = r\acct($id);
    $ages[] = (time() - $a->dob) / YEAR_SECS;
    $tenures[] = $a->tenure;
    $rents = @$rents + ($a->rents ? 1 : 0);
  }
  
  $tnm = <<< X
    (SELECT ucust.zip, COUNT(ucust.zip) count
     FROM r_tx_hdrs tcust
     JOIN r_entries payer USING(xid)
     JOIN r_entries payee USING(xid)
     JOIN users ucust ON ucust.uid=payer.uid
     WHERE payee.uid=:myid AND NOT ucust.:IS_CO AND payer.amount < 0 AND payee.amount > 0
     GROUP BY ucust.zip
     ORDER BY count LIMIT 5
    ) x
X;
  $zips = db\rows('*', $tnm, 1, compact('myid'), FALSE);
  debug('This is unfinished because I don''t understand it.');
  /*
  $tnm = <<< X
    (SELECT uco.fullName AS company, COUNT(tco.payee) AS count
     FROM r_txs tcust JOIN r_txs tco ON tco.payer=tcust.payer AND tco.payee<>:myid
     JOIN users uco ON uco.uid=tco.payee
     JOIN users ucust ON ucust.uid=tcust.payer
     WHERE tcust.payee=:myid AND uco.:IS_CO AND NOT ucust.:IS_CO
     GROUP BY tco.payee
     ORDER BY count LIMIT 5
    ) x
X;
  $pays = db\rows('*', $tnm, 1, compact('myid'), FALSE);
  */
  $pays = [];
  // calculate medians and avgs
  $age = w\item((int) u\median($ages), t('Median Age:'));
  $tenure = w\item((int) u\median($tenures), t('Median Tenure:'), t('months at current location'));
  $rents = w\item(u\fmtAmt($rents / count($ages), '%'), t('Rents vs. Owns:'));
  $zips = w\item(makeTable($zips, [t('Zip'), t('Count')]), t('Top Zip Codes:'));
  $pays = w\item(makeTable($pays, [t('Company'), t('Count')]), t('Also Shop At:'));
  
  return w\cgform(compact(ray('title age tenure rents zips pays')));
}