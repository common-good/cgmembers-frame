<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

/**
 * @file
 * Identifier-lookup endpoint for the SvelteKit cgpay POC.
 *
 * The POC's login form lets the user type any of: account code (QID or short),
 * username, full name (handled via shortName), email, or phone number. PHP owns
 * the matching logic — r\loginString() already handles QID/short/email/name
 * (including the encrypted-email path), and this endpoint extends it with phone.
 *
 * Flow:
 *   POC POST /cgpay-lookup with X-CG-Internal-Token: <shared secret>
 *     body: {"identifier": "<whatever the user typed>"}
 *   This endpoint returns:
 *     200 {"uid": <int>}  on match
 *     401  on bad/missing shared secret
 *     400  on missing identifier
 *     404  on no match
 *
 * The POC then SELECTs `users.pass` by uid and verifies the password itself.
 *
 * Shared secret is the same one used by /cgpay-sso (CGPAY_SSO_SECRET, see
 * defs.inc). One secret for both endpoints — no point splitting it.
 */
function cgpayLookup() {
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  $body = json_decode(file_get_contents('php://input'), TRUE);
  $identifier = isset($body['identifier']) ? trim((string) $body['identifier']) : '';
  if ($identifier === '') return exitJust(X_SYNTAX);

  // r\loginString covers: account code (QID + short code), email, and the
  // username/short-name match — including the encrypted-email path.
  $uid = r\loginString($identifier);

  // Fallback: phone number. cgmembers' api.inc has phone matching but it's
  // currently gated off (`FALSE and …`). We enable it here when r\loginString
  // returns nothing AND the input looks phone-shaped.
  if (!$uid && preg_match('/^[\+\(\)\.\- 0-9]{7,}$/', $identifier)) {
    $phone = u\cry('P', u\fmtPhone($identifier, '+n'));
    $uid = db\get('uid', 'users', 'phone=:phone ORDER BY :IS_CO', compact('phone'));
  }

  if (!$uid) return exitJust(X_NOTFOUND);

  return exitJson(['uid' => (int) $uid], X_OK);
}
