<?php
namespace CG\Web;
use CG as r;

require_once __DIR__ . '/tx.inc';

/**
 * /cgpay/identity: what the store app shows about the account it scanned, as from /api/identity, with the app's token.
 *   body: {token, actorId, otherId}; 200 {name, agent, location, limit, ...} (see identity()); 401 {error: bad-token}; 403 {error: bad-actor}; 404 not a member card
 */
function cgpayIdentity() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token')) or !appActor($row, nni($in, 'actorId'))) return;
  return identity(['otherId' => (string) nni($in, 'otherId')]);
}
