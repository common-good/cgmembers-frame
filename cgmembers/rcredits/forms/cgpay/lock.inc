<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/lock: make the app a company's cash register, or stop.
 *   body: {token, account}: lock the token to that company (the member must manage it); {token, account: 0}: unlock it for good; {token}: end a manager's unlock.
 *   200 {uid, admin, accounts, lockedTo} as for /cgpay/device; 401 {error: bad-token}; 403 {error: not-allowed | locked}
 * Changing or ending the lock of a locked token needs an unlock by the token's member first (/cgpay/unlock), or a cashier (or another manager) could free the device
 * and reach the member's other accounts.
 */
function cgpayLock() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token'))) return;
  $account = nni($in, 'account', NULL);
  if (is_null($account)) {
    $lockedTo = $row->lockedTo;
  } else {
    if (appOnly($row)) return appErr('locked', X_FORBIDDEN); // changing or ending the lock needs the token's member's own unlock
    $lockedTo = (int) $account ?: NULL;
    if ($lockedTo and (!$a = r\acct($lockedTo) or !$a->co or !appManages($row->uid, $lockedTo))) return appErr('not-allowed', X_FORBIDDEN);
  }
  $unlockedUntil = $unlockedAt = $unlockedBy = NULL;
  db\update('app_tokens', ['id' => $row->id] + compact(ray('lockedTo unlockedUntil unlockedAt unlockedBy')));
  return exitJson(appMember($row->uid, db\row('app_tokens', 'id=:id', ['id' => $row->id])), X_OK);
}
