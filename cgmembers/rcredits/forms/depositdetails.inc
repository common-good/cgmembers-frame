<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

/**
 * Show a PDF of a (populated) deposit slip for a particular Dollar Pool transfer group ("deposit") in and/or out.
 */
function formDepositDetails($form, $sta, $args) {
  extract(just('way date total count only', $args, NULL));

  if (!$in = (!isset($way) or strtoupper($way) == 'IN')) $total = str_replace('-', '', $total);

  $activity = t('Bank Transfer');
  $where = 1;
  $b = r\bankData();
  $escrow = $in ? $b->acctIn : $b->acctOut;

  $title = t('Bank Transfer Details');
  $subtext = $date == 1 ? t('(Historical)') : fmtDt($date ?: now());

  $sql = <<<EOF
    SELECT x2.amount, x2.xid, u.uid, u.fullName AS name, x2.pid, e.description
    FROM txs2 x2 
    LEFT JOIN tx_entries e ON e.xid=x2.xid AND e.uid=x2.payee AND e.entryType=:E_OUTER
    LEFT JOIN users u ON u.uid=x2.payee 
    WHERE $where AND x2.deposit=$date AND x2.amount<>0 AND x2.created<=:NOW ORDER BY x2.xid
EOF;

  $q = db\q($sql);
  
  $totIn = $totOut = $cntIn = $cntOut = 0;
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($pid) $name = db\get('fullName', 'people', compact('pid')) . t(' fbo: ') . ($uid == CGID ? PROJECT : str_replace(PROJECT . ' FBO', '', $name));
    if ($only and ($amount > 0 xor $in)) continue;
    if ($amount < 0) {
      $totOut += -$amount;
      $cntOut++;
      $class = ' class="neg"';
    } else {
      $totIn += $amount;
      $cntIn++;
      $class = '';
    }
    $qid = r\Qo::uid2($uid);
    $lines[] = tr('<tr><td>%xid</td><td><%a>%name</a></td><td%class>%amt</td><td>%description</td></tr>', 'xid _a name amt _class description', $xid, atag("/change-account/acct=$qid"), $name, number_format($amount, 2), $class, $description);
  }
  
  $lines[] = t('<tr><td></td><td><b>NET</b> (%count)</td><td>%total</td></tr>', 'count total', $count, u\fmtAmt($total, ''));
  $lines[] = t('<tr><td></td><td><b>IN</b> (%cntIn)</td><td>%totIn</td></tr>', 'cntIn totIn', $cntIn, u\fmtAmt($totIn, ''));
  $lines[] = t('<tr><td></td><td><b>OUT</b> (%cntOut)</td><td>%totOut</td></tr>', 'cntOut totOut', $cntIn, u\fmtAmt($totOut, ''));
  $lines = join("\n", $lines);
  
  $hdrs = t('<tr><th>Tx #</th><th>Name</th><th>Amount</th><th>Description</th></tr>');

  $list = <<<X
  <table>
  $hdrs
  $lines
  </table>
X;

  return cgForm(compact(ray('title subtext list')));  
}
