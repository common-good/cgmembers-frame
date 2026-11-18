<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/account: an account the member typed (admins, who may act for accounts not in their list), as the app keeps it.
 *   body: {token, who (account code, email, phone or name)}; 200 {account} (see appAccount()); 401 {error: bad-token}; 404 {error: not-found}; 403 {error: not-allowed}
 */
function cgpayAccount() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token'))) return;
  if (!$id = appFind(trim((string) nni($in, 'who')))) return appErr('not-found');
  if (!appMayAct($row->uid, $id)) return appErr('not-allowed', X_FORBIDDEN);
  return exitJson(['account' => appAccount($id, $row->uid)], X_OK);
}
