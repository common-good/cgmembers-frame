<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/device: the token's member and their accounts, with device credentials (the app refreshes them on launch).
 *   body: {token}; 200 {uid, admin, accounts} (see appMember()); 401 {error: bad-token}
 */
function cgpayDevice() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token'))) return;
  return exitJson(appMember($row->uid), X_OK);
}
