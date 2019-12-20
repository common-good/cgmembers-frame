<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/invest.inc';

/**
 * List potential or actual investments.
 */
function formInvestList($form, &$sta, $args = '') {
  global $mya;
  extract(just('actual clubqid', $args));
  
  if (!$clubA = clubA($clubqid)) return;
  list ($clubid, $clubqid) = [$clubA->id, $clubA->mainQid];
  
  $title = t('Club Investments');
  $subtext = t('Choose an investment to see details%rate.', 'rate', $clubid == $mya->id ? '' : t(' or to rate it'));
  
  $sql = <<< X
    SELECT DISTINCT i.vestid, u.fullName, i.investment, i.types, i.return, i.soundness AS sound, 
      IF(s.shid IS NULL, 'proposed', 'actual') AS status, AVG(r.patronage) AS patronage, AVG(r.good) AS good, 
      SUM(s.shares*i.price) AS value
    FROM r_investments i 
    LEFT JOIN users u ON u.uid=i.coid 
    LEFT JOIN r_ratings r USING(vestid)
    LEFT JOIN r_shares s USING(vestid)
    WHERE i.clubid=:clubid
    GROUP BY i.vestid
    ORDER BY u.fullName
X;
  $q = db\q($sql, compact('clubid'));
///  debug(compact(ray('sql clubid')));
  $fields = ray('status investment type return value good');
  $hdrs = ray(ucwords(t('status investment type return value good')));
  $hdrs = u\tableRow($hdrs);
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $return = u\fmtAmt($return, '%2');
    $investment = mb_substr("$fullName: $investment", 0, 80);
    $type = (strpos($types, 'I') === FALSE or strpos($types, 'D') !== FALSE) ? t('Equity') : t('Loan');
    $href = BASE_URL . "/invest/rate/vestid=$vestid&clubqid=$clubqid";
    $investment = tags('a', $investment, compact('href'));
    $good = $good ? number_format($good, 1) : '?';
    $value = $value ? number_format($value, 0) : '&mdash;';
    $rows[] = u\tableRow(compact($fields));
  }

  if (@$rows) {
    $rows = join($rows, "\n");
    $list = <<< X
    <table id="investments">
    $hdrs
    $rows
    </table>
X;
  } else $list = t('There are no investments at this time.');

  $buttons = backButton();
  if ($mya->iclub) $buttons .= btn("/invest/propose", t('Propose'));
  
  return cgform(compact(ray('title subtext buttons list')));
}
