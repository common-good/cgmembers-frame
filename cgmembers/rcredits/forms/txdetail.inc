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

  $title = t('Transaction #%xid Detail', compact('xid'));
  $toMe = 't.uid2=:myid';

  $sql = <<< X
    SELECT t.created, t.amt, t2.pid, t.for2 AS purpose, c.category, uAgt.fullName AS agt, uAgt.uid AS agtUid,
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

  if ($meId != $myid and !$mya->admin2 and !($mya->cAdmin2 and $community == $mya->community)) return softErr(t('That transaction is not one of yours.'));
  
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
  
  if ($agtUid == $myid) $agt = t('self');

  $ks = ray(t('Date,Amount,From,Postal Addr,For,Category,To Agent,Channel'));
  $vs = [u\fmtDate($created, TRUE), u\fmtAmt($toMe ? $amt : -$amt), $fullName, $addr, $purpose, $category, $agt, ray(TX_CHANNELS)[$channel]];
  
  $form = compact('title');
  foreach ($ks as $i => $k) $form["k$i"] = item($vs[$i], "$k:");
  if (!$toMe and !$mya->anyAdmin) unset($form['k3']); // no addr for people we pay
  
  return cgform($form);
}
