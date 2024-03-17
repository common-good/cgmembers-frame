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
function formData($form, &$sta, $args = '', $isco0 = FALSE) {
  global $mya;
  extract(just('code', $args, ''));

  require queryFile($isco0 == 'co');
  
  extract(just('qName sql sqlTitle q isco', u\decryRay($code), NULL));
  u\setDft($isco, $isco0 == 'co');
  
  if ($q) {
    $url = $isco ? '/history/company-reports' : '/community/data';
    w\go("$url/code=" . u\cryRay(ray('qName', $q)));
  }

  if ($sql2 = $sql) {
    $title = $sqlTitle ?: t('MySQL Query');
    try {
      $q = db\adminQ($sql2);
      if ($q and $rows = $q->fetchAll()) {
        $hdrs = array_keys((array) $rows[0]);
        foreach ($rows as $i => $row) $rows[$i] = array_values((array) $row);
        $list = w\makeTable($rows, $hdrs);
      } else $list = t('Zero matching rows found.');
      jsx('query');
      return cgform(compact(ray('title list')), 'wide nolabs');
    } catch (\Exception $e) {
      err($e->getMessage());
      return go('/community/data/code=' . u\cryRay(ray('sql', $sql2)));
    }
  }

  if (!$mya) return softErr(tr('no page permission', 'page', t('Community Data'))); // only members get to see standard queries

  if ($qName and ($sql2 = nni($queries, $qName)) or $sql = nni($queries, substr(nn($qName), 1))) {
    if (substr($sql2, 0, 1) == '/') return go($sql2);
    $title = t('Query Results');
    $subtext = '<h4>' . w\queryPermitted($qName, $sql2, $subtext) . "</h4><p>$subtext</p>"; // alters $sql2
    $report = strhas($sql2, ' ') ? item(w\showRecords(db\q($sql2)->fetchAll())) : getReport($sql2);
    $form = compact(ray('title subtext report'));
  } else {
    $title = $isco ? t('Company Reports') : t('Community and Money Data');
    $subtext = item(t('Click the query of your choice. If you need information not provided here, <%a1>request it</a>!', '_a1', u\emailTag(CGF_EMAIL, t('data request'))), '&nbsp;');
    if ($mya->admRegion) {
      $sql = areaFld(t('SQL:'), [t('MySQL Select Query')], dft(urldecode(nn($sql))) + maxlen(4096));
      $sqlTitle = textFld(t('Title:'), [t('Results Title')], t('Query Results'));
      $submit = submit(t('Go'));
    } else $sql = $sqlTitle = $submit = NULL;

    foreach ($queries as $k0 => $v) {
      if ($k = w\queryPermitted($k0)) $links[$k0] = lnk('/community/data/code=' . u\cryRay(ray('q isco', $k0, $isco)), $k);
    }
    $links = item(u\ul($links), NBSP);
    $form = compact(ray('title subtext sql sqlTitle submit links'));
  }
  return cgform($form, $args ? ($code ? 'wide' : 'wider') . ' nolabs' : '');
}

function formData_validate($form, &$sta) {
  if (op($sta) != 'go') return;
  extract(just('sql', $sta['input']));
  $word1 = mb_strtolower(mb_strstr($sql, ' ', TRUE));
  if (!in($word1, 'select show')) return say(t('Only SELECT and SHOW queries are permitted.'), 'sql');
}

function formData_submit($form, &$sta) {
  extract($info = just('sql sqlTitle', $sta['input'], NULL));

  if ($info) return w\go('/community/data/code=' . u\cryRay($info));
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
    $heads = ray(t('Account,Name,Phone,Email,Net Ever,Unpaid'));
    $colClasses = ray(',,,,alignR,alignR');
    list ($us, $them, $invUs, $invThem) = $nm == 'customers' ? [2, 1, 'payee', 'payer'] : [1, 2, 'payer', 'payee'];
    $whereInv = "$invUs=:myid AND amount<>0 AND status<:TX_PAID";
//    $whereTx = "uid$us=:myid AND amt<>0 AND uid$them NOT :CANONIC GROUP BY uid$them";
    $whereTx = "uid$us=:myid AND amt<>0 AND type IN (:E_PRIME, :E_OUTER)";

    $sql = <<< X
      SELECT u.uid,p.pid,SUM(net0) AS net, SUM(invs0) AS invs FROM (
        SELECT uid$them AS uid, pid, SUM(amt) AS net0, 0 AS invs0
        FROM txs LEFT JOIN txs2 USING (xid) WHERE $whereTx GROUP BY uid$them, pid
        UNION SELECT $invThem AS uid,NULL AS pid,0 AS net0, SUM(amount) AS invs0
        FROM tx_requests WHERE $whereInv GROUP BY $invThem
      ) x LEFT JOIN users u ON u.uid=x.uid LEFT JOIN people p ON p.pid=x.pid GROUP BY u.uid,p.pid ORDER BY IF(p.pid, p.fullName, u.fullName)
X;

    $q = db\q($sql, compact('myid'));
    while ($row = $q->fetchAssoc()) {
      extract($row);
      if ($pid) {
        $vs = db\get('pid AS qid, fullName, phone, email', 'people', compact('pid'));
        extract($vs);
        $qid .= '*'; // mark as no active account
      } else {
        $a = r\acct($uid);
        $qid = w\lnk(tr('/history/customer-statement/cust=%qid&way=%way', 'qid way', $a->mainQid, $nm == 'customers' ? 'IN' : 'OUT'), $a->mainQid);
        if ($a->closed) $qid .= '*'; // mark as no active account
        list ($email, $phone) = $a->nosearch ? ['**', '**'] : [$a->email ?: t('BAD EMAIL'), u\fmtPhone($a->phone, '-')];
        $fullName = $a->fullName;
      }
      $res[] = [$qid, $fullName, $phone, $email, number_format($net, 2), number_format($invs, 2)];
      $classes[] = $invs ? 'hiRow' : '';
    }
    return w\item(w\makeTable(nn($res), $heads, $colClasses, nn($rowClasses)));
    
  } elseif ($nm == 'demographics') {
    w\go('/history/company-reports/demographics');
    
  } elseif ($nm == 'donations') {
    $heads = ray(t('Name,Phone,Email,Address,Member,Total,Count,Since'));
    $colClasses = ray(',,,,alignC,alignR,alignR,');
    $where = "uid2=:myid AND (x.:IS_GIFT OR for2 RLIKE '.*\{donation|gift|grant\}.*')";
    $sql = <<< X
      (SELECT uid AS id, fullName, phone, email, 'yes' AS member, SUM(x.amt) AS total, SUM(1) AS count, MIN(x.created) AS since
      FROM txs x LEFT JOIN users u ON u.uid=x.uid1
      WHERE type=:E_PRIME AND $where
      GROUP BY uid

      UNION
      SELECT pid AS id, fullName, phone, email, 'no' AS member, SUM(x.amt) AS total, SUM(1) AS count, MIN(x.created) AS since
      FROM txs x JOIN txs2 x2 USING(xid) JOIN people p USING(pid)
      WHERE type=:E_OUTER AND $where
      GROUP BY pid)
      
      ORDER BY fullName
X;

    $q = db\q($sql, compact('myid'));
    while ($row = $q->fetchAssoc()) {
      extract($row);
      if ($member == 'yes') {
        $a = r\acct($id);
        if ($a->closed) $fullName .= '*'; // mark as no active account
        $address = $a->postalAddr;
        list ($email, $phone) = $a->nosearch ? ['**', '**'] : [$a->email, $a->phone];
      } else list ($zot, $address) = r\pContactInfo($id);
      
      if ($phone != '**') $phone = u\fmtPhone($phone, '-');
      $res[] = [$fullName, $phone, $email, $address, $member, number_format($total, 2), number_format($count), fmtDt($since)];
    }
    return w\item(w\makeTable(nn($res), $heads, $colClasses, ''));
    
  } elseif ($nm == 'cef-billing') {
    w\go('/sadmin/cef-billing');
  }
}
