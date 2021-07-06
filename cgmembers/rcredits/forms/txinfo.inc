<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once R_ROOT . '/tx.class';

/**
 * Show information about a specific transaction and its risks.
 */
function formTxInfo($form, &$sta, $xid = '') {
  include R_ROOT . '/rweb/risk-descs.inc';
  global $base_url;
  $x = r\Tx::read($xid);
  list ($e1, $e2) = [$x->payerEntry, $x->payeeEntry];
//  extract($x->ray);
//  extract(just('xfee force undoes undoneBy', $x->data));
  
  $title = item(t('Transaction #') . $xid);
  $when = item(u\fmtDate($x->created) . t(' at ') . u\fmtTime($x->created) . ($x->is('offline') ? t(' (created offline)') : ''), t('Date/Time:'));
  $amountDesc = item(u\fmtAmt($e1->amount), t('Amount:'));
  $from = item(txInfoParty($x->payerId, $e1->agentUid), t('From:'));
  $to = item(txInfoParty($x->payeeId, $e2->agentUid), t('To:'));
  $same = ($e1->description == $e2->description);
  $goodsDesc = ' (' . ray(R_WHYS)[$x->goods] . ')';
  $for1 = item($e1->description . $goodsDesc, $same ? t('For:') : t('Payer Desc:'));
  $for2 = $same ? NULL : item($e2->description, t('Payee Desc:'));
  $by = item($x->actorId == $x->payeeId ? t('by payee') : t('by payer'), t('Initiated:'));
  $channels = ray(TX_CHANNELS);
  $where = item($channels[$x->channel] . t(' (Device #%box)', 'box', $x->boxId), t('Method:'));
  // related transactions (xfer/undoes/undoneBy
  if ($x->reversesXid) $rel[] = t('undoes tx#') . txLink($x->reversesXid);
  if ($x->reversedByXid) $rel[] = t('undone by tx#') . txLink($x->reversedByXid);
// THIS DOESN'T WORK YET  if ($x->xfee) $rel[] = u\fmtAmt($x->xfee) . ' xfee';

  $related = nn($rel) ? item(join(', ', $rel), t('Related Txs:')) : NULL;
  $riskDesc = $x->risk < K_RED ? t('low') : ($x->risk < 2 * K_RED ? t('moderate') : t('high'));
  $risks = r\riskList($risks0 = $x->risks, K_TX_RISKS); 
  $risk = item(round($x->risk) . ' ' . button(t('Recalculate'), "$base_url/rerisk/tx/$x->xid/1") . " ($riskDesc): $risks", t('Risk:'));
  $adminOk = boolFld(t('Risk Ok?'), '', $risks0 & 1);
  $submit = submit();
  return cgform(compact(ray('title when amountDesc from to for1 for2 by where risk adminOk related submit')));
}

function txInfoParty($uid, $agent) {
  $desc = r\acct($uid)->linkedDesc();
  $by = $agent == $uid ? t('pro se') : (t('by ') . r\acct($agent)->linkedDesc());
  return "$desc $by";
}
  
//function txLink($xid) {global $base_url; return "<a href=\"$base_url/sadmin/tx-info/$xid\">$xid</a>";}
