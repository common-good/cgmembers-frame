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
  $subtext = t('<p>This list of accounts owned by %PROJECT shows end-of-day balances.</p>');
  $dft = $asof ? strtotime($asof) : now();
  $asof = dateFld(REQ . t('As of:'), '', dft($dft) + suffix(' ' . btn('', t('Go'))));

  $list = item(idsLinked(tomorrow($dft)));
  jsx('ourAccts');

  return cgform(compact(ray('title subtext asof list')), 'wider');
}

function idsLinked($asof) {
  global $mya;

  $seeCanonic = $mya->admSeeCanonic;
  $cgIds = r\cgIncomeUids();
  $ids = db\col('uid', 'users', 'uid IN (:cgIds) ORDER BY uid=:CGID DESC, uid IN :BUCKET_UIDS DESC, fullName', compact('cgIds'));

  $rows = "<br>\n<table>\n";
  $hdrs = ray(t('Since, ID, Name, Balance'));
  $rows .= '<tr><td>' . join('</td><td>', $hdrs) . '</td></tr>';

  foreach ($ids as $uid) {
    $a = r\acct($uid);
    $ours = $a->ourOwn; // maybe show this?
    $since = fmtDt($a->activated);
    if (in($uid, BUCKET_UIDS) and $uid != CGID) {
      $bestName = $a->name;
      if ($seeCanonic) $since = ''; else continue;
    } else $bestName = $a->bestName;
    $name = $uid == $mya->id ? "<b>$bestName</b>" : w\lnk("/change-account/acct=$uid", $bestName);

    $balance = !$a->activated ? t('pending')
    : (!$a->ok ? (($tx = $a->lastTx and $dt = $tx->created) ? t('closed ') . fmtDt($dt) : t('no txs yet'))
      : u\fmtAmt($asof > today() ? $a->balance : be\balanceAsOf($a->uids, $asof)) );

    $row = [$since, $a->mainQid, $name, $balance];
    $rows .= '<tr><td>' . join('</td><td>', $row) . '</td></tr>';
  }
  
  $rows .= '</table>';
  
  return $rows;
}
