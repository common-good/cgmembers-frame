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
  
  $uids = db\col('DISTINCT uid1', 'txs_prime', 'uid2=:myid', compact('myid'));
  $count = count($uids);
  if ($count < AGGREGATE_MIN) return w\go('/history/company-reports', t('There are not enough customers (%count) to show demographics (minimum %AGGREGATE_MIN).', compact('count')));
  
  $ages = [];
  foreach ($uids as $id) {
    $a = r\acct($id);
    if ($a->hasDob) $ages[] = date('Y') - $a->yob;
    if ($a->created >= $statStart) {
      $tenures[] = $a->tenure;
      $owns[] = $a->owns ? 1 : 0;
    }
  }
  
  $tnm = <<< X
    (SELECT uCust.city, COUNT(DISTINCT uCust.uid) count
     FROM txs_prime t JOIN users uCust ON uCust.uid=t.uid1
     WHERE t.uid2=:myid
     GROUP BY uCust.state, uCust.city
     ORDER BY count DESC, uCust.state, uCust.city LIMIT 10
    ) x
X;
// (used to limit this to individual customers, not businesses, with: WHERE NOT uCust.:IS_CO
  $cities = db\rows('*', $tnm, 1, compact('myid'), FALSE);

/*  // us is "us", them is the other company
  $sql = <<<X
    SELECT them.fullName AS company, COUNT(DISTINCT toThem.uid1) AS count
    FROM txs toThem JOIN users them ON (them.uid=toThem.uid2)
    WHERE toThem.uid1 IN (
      SELECT DISTINCT toUs.uid1 FROM users us JOIN txs toUs ON toUs.uid2=us.uid WHERE us.uid=:myid
    ) AND them.:IS_CO AND them.uid NOT IN (:myid,:CGID)
    GROUP BY them.fullName
    ORDER BY count DESC, company ASC
    LIMIT 10
X;
*/

  // uCo is another company, tToUs is payments to this company, tToOther is payments to uCo by this company's customers (uCust)
  $sql = <<< X
    SELECT DISTINCT uCo.fullName AS company, COUNT(DISTINCT tToOther.uid1) AS count
     FROM txs_prime tToUs LEFT JOIN txs_prime tToOther ON tToOther.uid1=tToUs.uid1 AND tToOther.uid2 NOT IN (:myid,:CGID)
     LEFT JOIN users uCust ON uCust.uid=tToUs.uid1
     LEFT JOIN users uCo ON uCo.uid=tToOther.uid2
     WHERE tToUs.uid2=:myid AND uCo.:IS_CO AND NOT uCust.:IS_CO
     GROUP BY tToOther.uid2
     ORDER BY count DESC, company ASC LIMIT 10
X;
  
  $pays = db\rows('company, count', "($sql) x", 1, compact('myid'));
  foreach ($pays as $i => $one) $pays[$i] = array_values((array) $one);

  // calculate medians and avgs
  $count = w\item(number_format(count($ages)), t('Customer Count:'));
  $age = w\item((int) u\median($ages), t('Median Age:'));
  $tenure = w\item(t('%n months at current location', 'n', (int) u\median(nn($tenures) ?: [])), t('Median Tenure:'));
  $owns = w\item(u\fmtAmt(nn($owns) ? array_sum($owns) / count($owns) : 0, '%'), t('Owns vs. Rents:'));
  $cities = w\item(makeTable($cities, [t('City/Town'), t('Count')]), t('Top Cities:'));
  $pays = w\item(makeTable($pays, [t('Company'), t('Count')]), t('Also Shop At:'));
  return w\cgform(compact(ray('title count age tenure owns cities pays')));
}
