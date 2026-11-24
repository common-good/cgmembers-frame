<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/redeem: trade a one-time code for the member, the account they act for and the page to open (single use).
 *   body: {code}; 200 {uid, account, to, tokenId (the app token, for /cgpay/sso)}; 401 {error: bad-code}
 */
function cgpayRedeem() {
  if (($in = appIn()) === FALSE) return;
  $h = appHash(nni($in, 'code')); $now = now();
  if (!db\q('UPDATE app_codes SET usedAt=:now WHERE codeHash=:h AND usedAt IS NULL AND expires>:now', compact('h', 'now'))->rowCount()) return appErr('bad-code', X_UNAUTH); // one atomic claim, so two calls with the same code cannot both win
  $row = db\row('app_codes', 'codeHash=:h', compact('h'));
  return exitJson(['uid' => (int) $row->uid, 'account' => (int) $row->account, 'to' => $row->dest, 'tokenId' => (int) $row->tokenId], X_OK);
}
