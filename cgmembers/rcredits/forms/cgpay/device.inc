<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/device: the device credentials for the token's member (the app refreshes them on launch).
 *   body: {token}; 200 {uid, device} (device is null if the account can't use the app yet); 401 {error: bad-token}
 */
function cgpayDevice() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token'))) return;
  return exitJson(['uid' => (int) $row->uid, 'device' => appDevice($row->uid)], X_OK);
}
