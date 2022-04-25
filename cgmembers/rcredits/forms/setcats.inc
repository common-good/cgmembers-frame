<?php
/**
 * @file
 * Ad hoc functions
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
 * - bool $overwrite: redo the cat field of all transactions in the range
 * - bool $reshow: <redisplay the most recent list of transactions that need cat set>
 */
function formSetCats($form, &$sta, $args = '') {
  extract(just('start overwrite reshow', $args, NULL));

  $title = t('Set Transaction Categories');
  
  if ($start) {
    $rows = setMostCats($start, $overwrite);
    svar('catsList', $rows);
  } elseif ($reshow) {
    $rows = svar('catsList') ?: [];
  } else {
    $start = dateFld(REQ . t('Starting Date:'));
    $overwrite = boolFld(t('Overwrite:'), FALSE);
    $submit = t('Set Cats');
    return cgForm(compact(ray('title start overwrite submit')));
  }

  $return = u\pry('/sadmin/set-cats/reshow=1');
  $list = item(showRecords(nn($rows, []), NULL, NULL, "/history/transaction/return=$return&xid"));
  return cgForm(compact(ray('title list')), 'wider');
}

function formSetCats_validate($form, &$sta) {
  extract(just('start', $sta['input'], NULL));
  u\badDate($start);
  u\preray(compact('start'), $sta['input']);
}

function formSetCats_submit($form, &$sta) {
  extract(just('start overwrite', $sta['input'], ''));
  return go("sadmin/set-cats/start=$start&overwrite=$overwrite");
}

function setMostCats($start, $overwrite) {
  $uids = r\cgUids(FALSE);
  $catCrit = $overwrite ? '1' : 'cat IS NULL';
  $q = db\q("SELECT DISTINCT xid,flags,recursId FROM tx_entries JOIN tx_hdrs USING(xid) WHERE uid IN (:uids) AND $catCrit AND created>=:start ORDER BY created DESC,xid DESC", compact(ray('uids start')));

  $flds = ray('xid type me you purpose');
  while ($tx = $q->fetch()) {
    $e = db\rows('*', 'tx_entries', 'xid=:xid ORDER BY ABS(id), id>0', just('xid', $tx));
    for ($i = 0; $i < count($e); $i++) {
      $ei = $e[$i];
      if (!r\isCGs($ei->uid)) continue;
      be\setCat($e, $tx, $i);
      if ($ei->cat) {
        db\update('tx_entries', just('id cat', (array) $ei));
      } else {
        $j = $i + (($i & 1) ? -1 : 1); // point to the other entry in the pair
        $ej = $e[$j];
        $iA = r\acct($ei->uid);
        $jA = r\acct($ej->uid);
        $xid = $tx->xid;
        list ($me, $you) = [$iA->fullName, $jA->fullName];
        $purpose = $ei->description;
        $type = ray(E_TYPES)[$ei->entryType];
//        $rows[] = [$xid, $type, $me, $you, $purpose];
        $rows[$ei->id] = compact($flds);
      }
    }
  }
  
  return nn($rows, []);
}