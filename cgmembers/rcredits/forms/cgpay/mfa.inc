<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Db as db;
use CG\Util as u;

require_once __DIR__ . '/token.inc';

/**
 * /cgpay/mfa: the store app's second sign-in step.
 *   body: {mfa (the handle from /cgpay/signin), code (the passcode we emailed), trust (skip this step on this device from now on)}
 *   200 {token, uid, admin, accounts, and account (to act for) if the sign-in was by card}; 401 {error: bad-handle | bad-code | expired}; 429 {error: flooding}
 * Wrong codes count as failed sign-ins (flood control), so the 8-digit code cannot be guessed.
 */
function cgpayMfa() {
  if (($in = appIn()) === FALSE) return;
  extract(just('mfa code trust', $in, ''));
  if (!$ray = u\decryRay($mfa)) return appErr('bad-handle', X_UNAUTH);
  extract(just('uid nonce expires label deviceKey actFor', $ray, ''));
  if (now() > $expires) return appErr('expired', X_UNAUTH);

  $sta = [];
  if (w\flooding($uid, $sta, $err)) return appErr('flooding', X_TOOMANY);
  $success = hash_equals((string) $nonce, trim($code));
  w\updateFlood($sta, $success, $err);
  if (!$success) return appErr('bad-code', X_UNAUTH);
  if (!appClaim('mfa:' . $mfa, $uid, $expires)) return appErr('bad-handle', X_UNAUTH); // each handle signs in once
  return appIssue($uid, $label, $deviceKey, $trust, $actFor ?: NULL);
}
