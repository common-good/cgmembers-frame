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
  global $mya;
  $isco = ($isco == 'co');
  require queryFile($isco);
  extract(just('qName sql q d', $args, NULL));
  
  if ($q) {
    $d = substr($q, 0, 1);
//    if (strhas('0123456789', $d)) $q = $d . $q; // leading digit, if any
    $url = $isco ? '/history/company-reports' : '/community/data';
    w\go("$url/qName=" . urlencode($q));
  }
  
  $sql0 = urldecode(nn($sql));
  $qName0 = nn($qName);

  $btn = w\submi('Go', 'success', 'xs', ['parents' => ['go']]);
  $sql = $mya->admin ? areaFld(t('SQL:'), [t('MySQL Select Query')], dft($sql0) + maxlen(4096) + suffix(\render($btn))) : NULL;

  if ($mya->admin and $sql0) {
    $title = t('MySQL Query');
    try {
      $q = db\adminQ($sql0);
      if ($q and $rows = $q->fetchAll()) {
        $hdrs = array_keys((array) $rows[0]);
        foreach ($rows as $i => $row) $rows[$i] = array_values((array) $row);
        $list = w\makeTable($rows, $hdrs);
      } else $list = t('Zero matching rows found.');
      $form = compact(ray('title list'));
    } catch (\Exception $e) {
      say($e->getMessage(), 'sql');
      $form = compact(ray('title sql'));
    }
  } elseif (!empty($qName0)) {
    $sql = nn($queries[$qName0]);
    if (!empty($sql)) {
      $title = t('Query Results');
      $subtext = '<h4>' . w\queryPermitted($qName0, $sql, $subtext) . "</h4><p>$subtext</p>"; // alters $sql
      $report = strhas($sql, ' ') ? item(w\showRecords(db\q($sql)->fetchAll())) : getReport($sql);
      $form = compact(ray('title subtext report'));
    }
  } else {
    $title = $isco ? t('Company Reports') : t('Community and Money Data');
    $subtext = item(t('Click the query of your choice. If you need information not provided here, <%a1>request it</a>!', '_a1', u\emailTag(CGF_EMAIL, t('data request'))), '&nbsp;');
    $isco = hidFld($isco ?: 0);

    foreach ($queries as $k0 => $v) {
      if ($k = w\queryPermitted($k0)) $links[$k0] = lnk("/community/data/q=$k0", $k);
    }
    $links = item(u\ul($links), NBSP);
    $form = compact(ray('title subtext sql isco links'));
  }
  return cgform($form, '', 'above');
}

function formData_validate($form, &$sta) {
  if (op($sta) != 'go') return;
  extract(just('sql', $sta['input']));
  $word1 = mb_strtolower(mb_strstr($sql, ' ', TRUE));
  if (!in($word1, 'select show')) return say(t('Only SELECT and SHOW queries are permitted.'), 'sql');
}

function formData_submit($form, &$sta) {
  extract(just('sql', $sta['input']));

  if (op($sta) == 'go') return w\go('/community/data/sql=' . urlencode($sql));
}

function queryFile($isco) {return R_ROOT . ($isco ? '/rweb/queries-co.inc' : '/rweb/queries.inc');}

/**
 * Return a report form-field for the specified report.
 * @param string $nm: report name
 */
function getReport($nm) {
  global $mya;
  $myid = $mya->id;
  
  if ($nm == 'customers' or $nm == 'vendors') {
    $heads = ray(t('Account,Name,Phone,Email,Net Ever'));
    $colClasses = ray(',,,,num');
    list ($us, $them, $invUs, $invThem) = $nm == 'customers' ? [2, 1, 'payee', 'payer'] : [1, 2, 'payer', 'payee'];
    $whereInv = "$invUs=:myid AND amount<>0 GROUP BY $invThem";
    $whereTx = "uid$us=:myid AND amt<>0 AND uid$them NOT :CANONIC GROUP BY uid$them";
    $sql = <<< X
      SELECT u.uid,SUM(net0) AS net, SUM(invs0) AS invs FROM (
        SELECT uid$them AS uid, SUM(amt) AS net0, 0 AS invs0
        FROM txs_prime WHERE $whereTx
        UNION SELECT $invThem AS uid,0 AS net0, SUM(amount) AS invs0
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
    return w\item(w\makeTable(nn($res), $heads, $colClasses, nn($rowClasses)));
  } elseif ($nm == 'demographics') {
    w\go('/history/company-reports/demographics');
  }
}
