<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Server-to-server "who is this session" endpoint for the SvelteKit cgpay POC.
 *
 * The cgpay POC's +layout.server.ts calls this on every SSR render so PHP stays
 * the single source of truth for identity. When the user switches accounts on
 * the PHP side (or signs out), the very next node navigation sees the change.
 *
 * Flow:
 *   POC POST /cgpay-whoami
 *     X-CG-Internal-Token: <shared secret>
 *     body: {"ssid": "<value of the SSESS... cookie>"}
 *   200 {"uid": <int>, "name": "<bestName>", "sponsored": <bool>, "menu": [...]}
 *   401 no matching session, or bad/missing shared secret
 *   400 missing ssid
 *
 * Shared secret is CGPAY_SSO_SECRET (config.json `cgpaySsoSecret`); same key as
 * /cgpay-sso and /cgpay-lookup. Fails closed when either side has an empty value.
 */
function cgpayWhoami() {
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  $body = json_decode(file_get_contents('php://input'), TRUE);
  $ssid = isset($body['ssid']) ? trim((string) $body['ssid']) : '';
  if ($ssid === '') return exitJust(X_SYNTAX);

  $uid = db\get('uid', 'sessions', 'ssid=:sid', ['sid' => $ssid]);
  if (!$uid) return exitJust(X_UNAUTH);

  $a = r\acct((int) $uid);
  if (!$a) return exitJust(X_UNAUTH);

  $menu = ['Dashboard', 'History', 'Community', 'Settings'];
  if ($a->co) $menu[] = 'Company';
  if ($a->can(B_ADMIN)) $menu[] = 'Admin';

  return exitJson([
    'uid'       => (int) $a->id,
    'name'      => $a->bestName,
    'sponsored' => (bool) $a->sponsored,
    'menu'      => $menu,
  ], X_OK);
}
