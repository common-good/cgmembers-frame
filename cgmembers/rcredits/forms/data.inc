<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Present a choice of data queries
 */
function formData($form, &$sta, $args = '', $isco = FALSE) {
  $isco = ($isco == 'co');
  require_once queryFile($isco);
  extract(just('qName', $args));
  
  if ($qName0 = urldecode(@$qName) and $sql = @$queries[$qName0] and $qName = w\queryPermitted($qName0, $sql, $subtext)) {
    $title = $qName;
    $subtext = $subtext ?: '&nbsp;';
    $report = strpos($sql, ' ') ? item(w\showRecords(db\q($sql)->fetchAll())) : getReport($sql);
    $form = compact(ray('title subtext report'));
  } else {
    $title = $isco ? t('Company Reports') : t('Community and Money Data');
    $subtext = item(t('Click the query of your choice. If you need information not provided here, <%a1>request it</a>!', ['_a1' => u\emailTag(CGF_EMAIL, t('data request'))]), '&nbsp;');
    $isco = hidFld($isco ?: 0);
    $form = compact(ray('title subtext isco'));
    
    foreach ($queries as $k0 => $v) {
      if ($k = w\queryPermitted($k0)) $form[$k0] = submit($k, 'default', 'xs');
    }
  }
  return cgform($form);
}

function formData_submit($form, &$sta) {
  extract(just('isco', $sta['input']));
  require_once queryFile($isco);
  $op = w\opText($sta);
  $d = substr(op($sta), 0, 1);
  if (strpos('0123456789', $d) !== FALSE) $op = $d . $op; // leading digit, if any
  $url = $isco ? '/company-reports' : '/community/data';
  w\go("$url/qName=" . urlencode($op));
}

function queryFile($isco) {return R_ROOT . ($isco ? '/rweb/queries-co.inc' : '/rweb/queries.inc');}

/**
 * Return a report form-field for the specified report.
 * @param string $nm: report name
 */
function getReport($nm) {
  global $mya;

  print_r(debug_backtrace());
  die('unfinished');

  $myid = $mya->id;
  
  if ($nm == 'customers' or $nm == 'vendors') {
    $heads = ray(t('Account,Name,Phone,Email,Net Ever'));
    $colClasses = ray(',,,,num');
    list ($us, $them) = $nm == 'customers' ? ['payee', 'payer'] : ['payer', 'payee'];
    $whereInv = "$us=:myid AND amount<>0 GROUP BY $them";
    $whereTx = "$us.uid=:myid AND amount<>0 GROUP BY $them.uid";
    $sql = <<< X
      SELECT u.uid,SUM(net0) AS net, SUM(invs0) AS invs FROM (
        SELECT $them AS uid, SUM(amount) AS net0, 0 AS invs0
        FROM r_tx_hdrs JOIN r_entries payer USING(xid) JOIN r_entries payee USING(xid)
        WHERE $whereTx
        UNION SELECT $them AS uid,0 AS net0, SUM(amount) AS invs0
        FROM r_invoices WHERE $whereInv AND status<:TX_PAID
      ) x LEFT JOIN users u ON u.uid=x.uid GROUP BY u.uid ORDER BY fullName
X;
    $q = db\q($sql, compact('myid'));
    while ($row = $q->fetchAssoc()) {
      extract($row);
      $a = r\acct($uid);
      $qid = w\lnk(tr('/history/customer-statement/cust=%qid&way=%way', 'qid way', $a->mainQid, $nm == 'customers' ? 'IN' : 'OUT'), $a->mainQid);
      list ($email, $phone) = $a->nosearch ? ['*', '*'] : [$a->email, u\fmtPhone($a->phone, '-')];
      $res[] = [$qid, $a->fullName, $phone, $email, number_format($net, 2)];
      $classes[] = $invs ? 'hiRow' : '';
    }
    return w\item(w\makeTable(@$res, $heads, $colClasses, @$rowClasses));
  } elseif ($nm == 'demographics') {
    w\go('/company-reports/demographics');
  }
}
