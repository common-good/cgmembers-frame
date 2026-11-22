<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/tx: the store app sends a transaction (or the undo of one) with its token, instead of the old API's device id.
 *   body: {token, actorId, otherId, amount, description, created, proof, offline, pending, tip}; the fields and the proof are those of /api/transactions.
 *   200 {ok, message} as from /api/transactions; 400 (plain text) a malformed transaction; 401 {error: bad-token}; 403 {error: bad-actor}
 */
function cgpayTx() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token'))) return;
  $p = just('actorId otherId amount description created proof tip', $in, '') + ['version' => APP_API_VERSION, 'deviceId' => 'token'];
  foreach (ray('offline pending') as $k) $p[$k] = nni($in, $k) ? 'true' : 'false';
  if (!appActor($row, $p['actorId'], $p)) return;
  return transactions($p);
}

/**
 * Act for the account the app names (actorId, as on its transactions) if the token's member may: set $mya and the device box and return TRUE, or reply with the error and return FALSE.
 * The agent in actorId must be the token's member: an admin with no relation to the account may not transact for it.
 * A refused transaction ($tx) taken offline is kept and told to the token's member and to staff (see keepBadTx).
 */
function appActor($row, $actorId, $tx = NULL) {
  global $mya, $channel;
  include_once R_ROOT . '/forms/api.inc';
  $channel = TX_APP;
  $uid = $row->uid;
  $a = getAcct((string) $actorId, FALSE);
  if (!$a or $a->agentId != $uid or !appMayAct($uid, $a->id)) {
    if (nni($tx, 'offline') == 'true') {
      $mya = r\acct($uid);
      keepBadTx(getAcct((string) $tx['otherId'], FALSE, FALSE), ['offline' => TRUE] + $tx, t('You may not act for that account.'));
    }
    appErr('bad-actor', X_FORBIDDEN); return FALSE;
  }
  $mya = $a;
  $a->boxId($row->deviceHash ?: "app$row->id", APP_API_VERSION);
  return TRUE;
}
