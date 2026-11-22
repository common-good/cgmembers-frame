<?php
namespace CG\Web;
use CG as r;
use CG\Util as u;

require_once __DIR__ . '/tx.inc';

/**
 * /cgpay/photo: the photo of the account the store app scanned (the card code is required, as for /api/idPhoto), with the app's token.
 *   body: {token, actorId, otherId}; 200 {photo: a data URL, or NULL}; 401 {error: bad-token}; 403 {error: bad-actor}; 404 {error: not-found}
 */
function cgpayPhoto() {
  if (($in = appIn()) === FALSE) return;
  if (!$row = appRow(nni($in, 'token')) or !appActor($row, nni($in, 'actorId'))) return;
  if (!$a = getAcct((string) nni($in, 'otherId'))) return appErr('not-found');
  $photo = ($pic = $a->agentA->photo) ? 'data:image/jpeg;base64,' . base64_encode($pic) : NULL;
  return exitJust(u\jsonize(compact('photo')), X_OK, 'photo'); // not the image itself in the log
}
