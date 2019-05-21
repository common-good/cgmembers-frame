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
  
  $field = 'DISTINCT payer.uid AS uid';
  $table = 'tx_entries payee JOIN tx_hdrs t USING(xid) JOIN tx_entries payer USING(xid)';
  $where = 'payee.uid=:myid AND payee.entryType=:ENTRY_PAYEE AND payer.entryType=:ENTRY_PAYER';
  $uids = db\col($field, $table, $where, compact('myid'));
  $count = count($uids);
  if ($count < AGGREGATE_MIN) return w\go('/company-reports', t('There are not enough customers (%count) to show demographics (minimum %AGGREGATE_MIN).', compact('count')));
  
  foreach ($uids as $id) {
    $a = r\acct($id);
    $ages[] = (now() - $a->dob) / YEAR_SECS;
    if ($a->created >= $statStart) {
      $tenures[] = $a->tenure;
      $owns[] = $a->owns ? 1 : 0;
    }
  }
  
  $tnm = <<< X
    (SELECT ucust.city, COUNT(DISTINCT ucust.uid) count
     FROM tx_entries payee
          JOIN tx_entries payer USING(xid)
          JOIN users ucust ON ucust.uid=payer.uid
     WHERE payee.uid=:myid AND payee.entryType=:ENTRY_PAYEE AND payer.entryType=:ENTRY_PAYER
           AND NOT ucust.:IS_CO
     GROUP BY ucust.state, ucust.city
     ORDER BY count DESC LIMIT 10
    ) x
X;
  $cities = db\rows('*', $tnm, 1, compact('myid'), FALSE);


  // us is "us", common is common customer, them is the other company
  $sql = <<<X
    SELECT them.fullName AS company, COUNT(DISTINCT payerEntry.uid) AS count
    FROM tx_entries payerEntry
    JOIN tx_entries theirEntry ON (theirEntry.xid=payerEntry.xid AND theirEntry.entryType=:ENTRY_PAYEE)
    JOIN users them ON (them.uid=theirEntry.uid)
    WHERE payerEntry.entryType=:ENTRY_PAYER
    AND payerEntry.uid IN (SELECT DISTINCT commonEntry.uid AS uid
                           FROM users us
                           JOIN tx_entries ourEntry ON (us.uid=ourEntry.uid AND ourEntry.entryType=:ENTRY_PAYEE)
                           JOIN tx_entries commonEntry ON (ourEntry.xid=commonEntry.xid AND commonEntry.entryType=:ENTRY_PAYER)
                           WHERE us.uid=:myid)
    AND them.:IS_CO AND them.uid NOT IN (:myid,:CGID)
    GROUP BY them.fullName
    ORDER BY count DESC, company ASC
    LIMIT 10
X;
  
  $pays = db\q($sql, compact('myid'));
  if (!empty($pays)) $pays=$pays->fetchAll();
  foreach ($pays as $i => $one) $pays[$i] = array_values((array) $one);

  // calculate medians and avgs
  $count = w\item(number_format(count($ages)), t('Customer Count:'));
  $age = w\item((int) u\median($ages), t('Median Age:'));
  $tenure = w\item(t('%n months at current location', 'n', (int) u\median(@$tenures ?: [])), t('Median Tenure:'));
  $owns = w\item(u\fmtAmt(@$owns ? array_sum($owns) / count($owns) : 0, '%'), t('Owns vs. Rents:'));
  $cities = w\item(makeTable($cities, [t('City/Town'), t('Count')]), t('Top Cities:'));
  $pays = w\item(makeTable($pays, [t('Company'), t('Count')]), t('Also Shop At:'));
  return w\cgform(compact(ray('title count age tenure owns cities pays')));
}
