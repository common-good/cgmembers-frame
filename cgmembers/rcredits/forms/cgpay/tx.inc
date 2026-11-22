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
  if (!$row = appRow(nni($in, 'token')) or !appActor($row, nni($in, 'actorId'))) return;
  $p = just('actorId otherId amount description created proof tip', $in, '');
  foreach (ray('offline pending') as $k) $p[$k] = nni($in, $k) ? 'true' : 'false';
  return transactions($p + ['version' => APP_API_VERSION, 'deviceId' => 'token']);
}

/**
 * Act for the account the app names (actorId, as on its transactions) if the token's member may: set $mya and the device box and return TRUE, or reply with the error and return FALSE.
 * The agent in actorId must be the token's member, except for an admin acting for an account they have no relation with (qr\url() then names no agent).
 */
function appActor($row, $actorId) {
  global $mya, $channel;
  include_once R_ROOT . '/forms/api.inc';
  $channel = TX_APP;
  $uid = $row->uid;
  $a = getAcct((string) $actorId, FALSE);
  $agentOk = $a && ($a->agentId == $uid || ($a->proSe && !r\relation(1, $a->id, $uid)));
  if (!$agentOk or !appMayAct($uid, $a->id)) {appErr('bad-actor', X_FORBIDDEN); return FALSE;}
  $mya = $a;
  $a->boxId($row->deviceHash ?: "app$row->id", APP_API_VERSION);
  return TRUE;
}
