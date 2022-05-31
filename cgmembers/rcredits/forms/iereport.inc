<?php
/**
 * @file
 * Income and Expense Report
*/
namespace CG\Web;
 
use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Admin as a;
use CG\Web as w;

/**
 * Set transaction categories for export to QBO.
 * @param string $args: URL parameters -- may include:
 * - int $start: starting date
 * - bool $end: ending date
 */
function formIEReport($form, &$sta, $args = '') {
  extract(just('start end', $args, NULL));

  $title = t('CG Income & Expense Report');
  
  if ($start) {
    $subtext = fmtDt($start) . ' - ' . fmtDt($end);
    $list = item(iereport($start, $end));
    return cgForm(compact(ray('title subtext list')), 'nolabs');
  } else {
    $start = dateFld(REQ . t('Starting Date:'));
    $end = dateFld(REQ . t('Ending Date:'), '', dft(NOW));
    $submit = t('Go');
    return cgForm(compact(ray('title start end submit')));
  }
}

function formIEReport_validate($form, &$sta) {
  extract(just('start end', $sta['input'], NULL));
  u\badDate($start); u\badDate($end);
  u\preray(compact(ray('start end')), $sta['input']);
}

function formIEReport_submit($form, &$sta) {
  extract(just('start end', $sta['input'], ''));
  $end = strtotime('tomorrow', $end) - 1;
  return go("sadmin/iereport/start=$start&end=$end");
}

/**
 * Return a table of categorized financial history.
 * @param date $start, $end: start and end dates
 */
function iereport($start, $end) {
  $catIds = db\keyed('category,id', 'tx_cats', "(category RLIKE '^[IE]' OR category LIKE '%To Be Paid Through CG%') AND `show` ORDER BY id");
  $cats = array_fill_keys(array_keys($catIds), 0);
  
  $uids = join(', ', r\catUids(FALSE));

  $sql = <<< X
    SELECT cat, e.id > 0 AS toMe, SUM(amount) AS amount, COUNT(e.id) AS cnt,
    IFNULL(category, IF(e.id > 0, 'I: Uncategorized Income', 'E: Uncategorized Expense')) AS catName
    FROM tx_entries e JOIN tx_hdrs h USING(xid) LEFT JOIN tx_cats c ON c.id=e.cat
    WHERE uid IN ($uids) AND created BETWEEN :start AND :end 
    AND ((category IS NULL AND entryType<>:E_BANK) OR LEFT(category, 1) IN ('I', 'E') OR category LIKE '%To Be Paid Through CG%')
    GROUP BY category
    ORDER BY e.id < 0, category
X;
  $q = db\q($sql, compact(ray('start end'))); // 14900 is payroll ("To Be Paid Through CG"
  while ($row = $q->fetchAssoc()) {
    extract($row); // cat, catName, amount, cnt
    $cats[$catName] = $amount;
  }

  $hdr = "<tr class=\"hdr\"><td colspan=\"2\">%hdr</td><td></td></tr>\n";
  $tot = "<tr class=\"tot\"><td>%tot</td><td>%amt</td><td></td></tr>\n";
  $res = tr($hdr, 'hdr', t('INCOME'));
  $prevParts = NULL;
  $indent = '<div class="indent"></div>';
  $total = 0;
  $subtotal = [];
  $income = TRUE;
  foreach ($cats as $catName => $amount) {
    $cat = $catIds[$catName] ?: 'NULL';
    if (strhas($catName, 'Uncategorized')) $cat = 'NULL';
    if (!$income) $amount = -$amount;
    $parts = explode(':', $catName);
    $pCnt = count($parts);
    if (!$prevParts or $parts[1] != $prevParts[1]) {
      if (count($subtotal) > 1) $res .= tr("<tr class=\"subtotal\"><td></td><td></td><td>%subtotal</td></tr>\n", 'subtotal', u\fmtAmt(array_sum($subtotal), ''));
      $subtotal = [];
      if ($prevParts and $parts[0] != $prevParts[0]) { // done with Income
        $res .= tr($tot, 'tot amt', t('TOTAL INCOME'), u\fmtAmt($total, '$'));
        $res .= tr($hdr, 'hdr', t('EXPENSE'));
        $total = 0;
        $income = FALSE;
      }
      $prevParts = $parts;
    }

    $sql = <<< X
      SELECT CONCAT('<a class="xid">', h.xid, '</a>') AS xid, FROM_UNIXTIME(h.created, '%m/%d/%Y') AS created,
        u1.fullName AS `from`, u2.fullName AS `to`, e1.amount, e1.description, e1.rule, h.recursId, h.reversesXid
      FROM tx_entries e1 JOIN tx_hdrs h USING(xid) JOIN tx_entries e2 ON e2.id=-e1.id
      LEFT JOIN users u1 ON u1.uid=e1.uid LEFT JOIN users u2 ON u2.uid=e2.uid
      WHERE e1.cat=%cat AND h.created BETWEEN %start AND %end
      AND e1.uid IN ($uids)
      ORDER BY h.created,h.xid,e1.id
X;
    $sql = tr(trim($sql), compact(ray('start end cat'))); // oddly this fails without trim()
    $sql = str_replace('=NULL', ' IS NULL', $sql);

    $lnk = '/community/data/code=' . u\cryRay(compact('sql'));
    $catName = trim($parts[$pCnt - 1]);
    $catName = str_repeat($indent, $pCnt - 1) . $catName;
    $subtotal[] = $amount;
    $total += $amount;
    $res .= tr("<tr><td>$catName</td><td><%a>%amount</a></td><td></td></tr>\n", '_a amount', atag($lnk), u\fmtAmt($amount, ''));
  }
  $res .= tr($tot, 'tot amt', t('TOTAL EXPENSE'), u\fmtAmt($total, '$'));
  
  return "<table>\n$res</table>";
}
