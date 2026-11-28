<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/tx: the store app sends a transaction (or the undo of one) with its token, instead of the old API's device id.
 *   body: {token, actorId, otherId, amount, description, created, proof, offline, pending, tip, manager, managerProof}; the fields and the proof are those of /api/transactions.
 *   dupOk: the app asked whether a repeat of a recent transaction (same account, same amount) was meant, and the member said yes.
 *   manager, managerProof: a locked cash register's payment, made after a manager's card unlocked it offline: the card's account id, and the proof made with that card's code instead of the other's.
 *   200 {ok, message} as from /api/transactions; 400 (plain text) a malformed transaction; 401 {error: bad-token}; 403 {error: bad-actor | locked}
 */
function cgpayTx() {
  global $mya, $allowDupTxs;
  if (($in = appIn()) === FALSE) return;
  if (nni($in, 'dupOk')) $allowDupTxs = TRUE; // the app warned of a likely duplicate and the member went ahead
  if (!$row = appRow(nni($in, 'token'))) return;
  $p = just('actorId otherId amount description created proof tip', $in, '') + ['version' => APP_API_VERSION, 'deviceId' => 'token'];
  foreach (ray('offline pending') as $k) $p[$k] = nni($in, $k) ? 'true' : 'false';
  if (!appActor($row, $p['actorId'], $p)) return;
  if ($by = appSigner($row, $p, $in)) appAs($row, r\acct($mya->id, $by)); // the manager is the payment's agent
  if (($co = appLocked($row)) and !appRegisterMay($co, $p, $in)) {
    $msg = t('This device is a cash register. Only a manager can pay from it.');
    if ($p['offline'] == 'true') keepBadTx(getAcct((string) $p['otherId'], FALSE, FALSE), ['offline' => TRUE] + $p, $msg);
    return appErr('locked', X_FORBIDDEN);
  }
  return transactions($p);
}

/**
 * Whether a locked cash register (for company $co) may send this transaction: a charge, the undo of a charge (its proof is the charge's), the undo of a payment,
 * or a payment a manager of the company made after unlocking it with their card offline ($in's manager and managerProof, see cgpayTx).
 */
function appRegisterMay($co, $p, $in) {
  extract(just('amount proof', $p));
  if ($amount > 0) return TRUE;
  if (!$other = getAcct((string) $p['otherId'], FALSE, FALSE)) return FALSE;
  if (hash_equals(appProof($p, -$amount, $other->cardCode()), (string) $proof)) return TRUE;
  if (!$m = getAcct((string) nni($in, 'manager'), FALSE) or !appManages($m->agentId, $co)) return FALSE;
  return appManagerProofOk($p, $in, $m);
}

/**
 * The proof of a transaction ($p) for amount $amt, made with card code $code, as /api/transactions makes it.
 */
function appProof($p, $amt, $code) {
  $s = $p['actorId'] . number_format($amt, 2, '.', '') . $p['otherId'] . $code . $p['created'];
  return test() ? $s : u\hash($s);
}

function appManagerProofOk($p, $in, $m) {return hash_equals(appProof($p, $p['amount'], $m->cardCode()), (string) nni($in, 'managerProof'));}

/**
 * The manager to record as the agent of a cash register's payment: the one whose card proof came with it, else the one whose unlock is on (0 if none).
 */
function appSigner($row, $p, $in) {
  if (!$row->lockedTo or $p['amount'] >= 0) return 0;
  if ($m = getAcct((string) nni($in, 'manager'), FALSE) and appManages($m->agentId, $row->lockedTo) and appManagerProofOk($p, $in, $m)) return (int) $m->agentId;
  return appLocked($row) ? 0 : (int) $row->unlockedBy;
}

/**
 * Act for the account the app names (actorId, as on its transactions) if the token's member may: set $mya and the device box and return TRUE, or reply with the error and return FALSE.
 * The agent in actorId must be the token's member: an admin with no relation to the account may not transact for it.
 * A cash register (see appOnly) acts only for its company.
 * A refused transaction ($tx) taken offline is kept and told to the token's member and to staff (see keepBadTx).
 */
function appActor($row, $actorId, $tx = NULL) {
  global $mya, $channel;
  include_once R_ROOT . '/forms/api.inc';
  $channel = TX_APP;
  $uid = $row->uid;
  $a = getAcct((string) $actorId, FALSE);
  if (!$a or $a->agentId != $uid or !appMayAct($uid, $a->id) or (($only = appOnly($row)) and $a->id != $only)) {
    if (nni($tx, 'offline') == 'true') {
      $mya = r\acct($uid);
      keepBadTx(getAcct((string) $tx['otherId'], FALSE, FALSE), ['offline' => TRUE] + $tx, t('You may not act for that account.'));
    }
    appErr('bad-actor', X_FORBIDDEN); return FALSE;
  }
  appAs($row, $a);
  return TRUE;
}

function appAs($row, $a) {
  global $mya;
  $mya = $a;
  $a->boxId($row->deviceHash ?: "app$row->id", APP_API_VERSION);
}
