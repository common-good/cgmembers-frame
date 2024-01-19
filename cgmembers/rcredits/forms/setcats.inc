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
 * - bool $reshow: <redisplay the most recent list of transactions that need cat set>
 */
function formSetCats($form, &$sta, $args = '') {
  extract(just('start reshow', $args, NULL));

  $title = t('Set Transaction Categories');
  
  if ($start) {
    $rows = setMostCats($start);
    svar('catsList', $rows);
  } elseif ($reshow) {
    $rows = svar('catsList') ?: [];
  } else {
    $start = dateFld(REQ . t('Starting Date:'), '', dft(getV(VAR_SETCAT_DT0, strtotime('-1 month', u\monthDay1()))));
    $submit = t('Set Cats');
    return cgForm(compact(ray('title start submit')));
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
  extract(just('start', $sta['input'], ''));
  return go("sadmin/set-cats/start=$start");
}

function setMostCats($start) {
  $uids = r\catUids();
  $q = db\q("SELECT DISTINCT xid,flags,recursId FROM tx_entries JOIN tx_hdrs USING(xid) WHERE uid IN (:uids) AND cat IS NULL AND created>=:start ORDER BY created DESC,xid DESC", compact(ray('uids start')));

  $flds = ray('xid type me you purpose');
  $cnt = 0;
  
  while ($tx = $q->fetch()) {
    $e = db\rows('*', 'tx_entries', 'xid=:xid ORDER BY ABS(id), id<0', just('xid', $tx)); // id<0 is essential
    for ($i = 0; $i < count($e); $i++) {
      extract(just('uid cat', (array) $e[$i]), EXTR_PREFIX_ALL, 'x');
      if (!r\hasCats($x_uid)) continue; // ignore entries that don't deserve a cat (don't use $ei here)
      be\setCat($e, $tx, $i); // everything else gets a cat -- automatically or manually
      $ei = $e[$i];
      $ej = $e[be\i2j($i)];

      if ($ei->cat) {
        if ($ei->cat != nn($x_cat)) {
          db\update('tx_entries', just('id cat', (array) $ei));
          $cnt++;
        }
      } else {
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
  
  w\say(t('Set %cnt cats.', compact('cnt')));
  return nn($rows, []);
}
