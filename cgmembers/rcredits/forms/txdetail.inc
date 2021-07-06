<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Display details about one transaction.
 */
function formTxDetail($form, &$sta, $args = '') {
  global $mya; $myid = $mya->id;
  global $channel;
  
  extract(just('xid', $args, NULL));

  $toMe = 't.uid2=:myid';

  $sql = <<< X
    SELECT t.created, t.amt, t2.pid, t.for2 AS purpose, t.cat, c.category, uAgt.fullName AS agt, uAgt.uid AS agtUid,
    uYou.uid AS youId, uYou.fullName, uMe.uid AS meId, uMe.community, $toMe AS toMe
    FROM txs t LEFT JOIN txs2 t2 USING(xid)
    LEFT JOIN users uMe ON uMe.uid=IF($toMe, t.uid2, t.uid1)
    LEFT JOIN users uYou ON uYou.uid=IF($toMe, t.uid1, t.uid2)
    LEFT JOIN people p ON p.pid=t2.pid
    LEFT JOIN users uAgt ON uAgt.uid=IF($toMe, t.agt2, t.agt1)
    LEFT JOIN budget_cats c ON c.id=t.cat
    WHERE xid=:xid
    ORDER BY t.type LIMIT 1
X;

  if (!$tx = db\q($sql, compact(ray('xid myid')))->fetchAssoc()) return softErr('no such tx'); // for now just show main transaction
  extract($tx);

  if ($meId != $myid) return softErr(t('That transaction is not one of yours.'));
  
  if ($pid) {
    if (!$info = db\get('fullName,address,city,state,zip,phone,email', 'people', compact('pid'))) return FAIL(t('Missing non-member information in transaction!'));
    extract($info);
    $fullName .= t(' (non-member)');
    $state = r\realState($state);
    $addr = "$address, $city, $state $zip";
  } else {
    $a = r\acct($youId);
    $addr = $a->postalAddr;
  }

  $title = t('Transaction #%xid Detail', compact('xid'));
  $created = editIf($mya->superAdmin, 'date', t('Date:'), $created, u\fmtDate($created, TRUE));
  $amt = editIf($mya->superAdmin, 'num', t('Amount:'), $amt, u\fmtAmt($toMe ? $amt : -$amt), vmin(-99999));
  $who = item($fullName, $toMe ? t('From:') : t('To:'));
  $addr = ($toMe or $mya->anyAdmin) ? item($addr, t('Postal Addr:')) : NULL; // no addr for people we pay
  $for = editIf($mya->admin, '', t('For:'), $purpose);
  $cat = $mya->admin ? catFld($cat, '') : ($category ? item($category, t('Category:')) : NULL);
  $agt = item($agtUid == $myid ? t('self') : $agt, t('To Agent:'));
  $chan = item(ray(TX_CHANNELS)[$channel], t('Channel:'));
  $submit = $mya->admin ? t('Update') : NULL;
  $back = item(w\backButton());

  $toMe = hidFld($toMe);
  $xid = hidFld($xid);

  return cgform(compact(ray('title created amt who addr for cat agt chan submit back toMe xid')));
}

function formTxDetail_validate($form, &$sta) {
  global $mya;
  extract(just('created amt', $sta['input']), NULL);

  if ($created and $err = u\badDate($created, now() - YEAR_SECS, tomorrow())) return say($err, 'created');
  if ($amt and $err = u\badAmount($amt, '!=0')) return say($err, 'amt');
  u\preRay(compact(ray('created amt')), $sta['input']);
}

function formTxDetail_submit($form, &$sta) {
  global $mya;

  $xidSub = just('xid', $input = $sta['input']);
  if ($info = just('created', $input)) db\update('tx_hdrs_all', $info + $xidSub, 'xid');
  if ($info = just('amt for cat', $input)) db\q('UPDATE tx_entries_all SET amount=IF(id<0, -:amt, :amt), description=:for, cat=:cat WHERE xid=:xid AND entryType IN (:E_PRIME, :E_BANK, :E_OUTER)', $info + $xidSub);

  return w\go('history/transactions');
}
