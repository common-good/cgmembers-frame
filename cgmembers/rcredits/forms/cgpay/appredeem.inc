<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/apptoken.inc';

/**
 * /cgpay/app-redeem: trade a one-time code for the member and the page to open (single use).
 *   body: {code}; 200 {uid, to}; 401 {error: bad-code}
 */
function cgpayAppRedeem() {
  if (($in = appIn()) === FALSE) return;
  $h = appHash(nni($in, 'code'));
  $row = db\row('app_codes', 'codeHash=:h AND usedAt IS NULL AND expires>:now', ['h' => $h, 'now' => now()]);
  if (!$row) return appErr('bad-code', X_UNAUTH);
  db\update('app_codes', ['id' => $row->id, 'usedAt' => now()]);
  return exitJson(['uid' => (int) $row->uid, 'to' => $row->dest], X_OK);
}
