<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Show a list of accounts that Common Good owns (including sponsored accounts).
 */
function formOurAccounts($form, &$sta, $args = '') {
  global $mya;
  extract(just('asof', $args, NULL));

  $title = t('Our Accounts');
  $dft = $asof ? strtotime($asof) : now();
  $asof = dateFld(REQ . t('As of:'), '', dft($dft) + suffix(' ' . btn('', t('Go'))));

  $list = item(idsLinked($dft));
  jsx('ourAccts');

  return cgform(compact(ray('title asof list')));
}

function idsLinked($asof) {
  global $mya;

  $seeCanonic = $mya->admSeeCanonic;
  $cgIds = r\cgIncomeUids();
  $ids = db\col('uid', 'users', 'uid IN (:cgIds) ORDER BY uid=:CGID DESC, uid IN :BUCKET_UIDS DESC, fullName', compact('cgIds'));

  $rows = "<br>\n<table>\n";

  foreach ($ids as $uid) {
    $a = r\acct($uid);
    $ours = $a->ourOwn; // maybe show this?
    $since = fmtDt($a->activated);
    $name = $uid == $mya->id ? "<b>$a->bestName</b>" : w\lnk("/change-account/acct=$uid", $a->bestName);
    if (in($uid, BUCKET_UIDS) and $uid != CGID) {
      if ($seeCanonic) $since = ''; else continue;
    }
    $balance = u\fmtAmt($asof > today() ? $a->balance : be\balanceAsOf($asof));
    $rows .= "<tr><td>$a->mainQid</td><td>$since<td>$name</td><td>$balance</td></tr>\n";
  }
  
  $rows .= '</table>';
  
  return $rows;
}
