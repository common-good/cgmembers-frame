<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Cross-app SSO endpoint for the SvelteKit cgpay POC.
 *
 * The cgpay POC (https://cgpay-poc.commongood.earth) verifies a user's password
 * against `users.pass` and now also needs to sign that user in on the PHP side,
 * so menu links from the dashboard can land on the existing member site without
 * a second login. The flow:
 *
 *   1. Browser → POST /api/login on cgpay POC
 *   2. cgpay POC verifies password (existing)
 *   3. cgpay POC calls POST /cgpay-sso on this server (server-to-server)
 *      with X-CG-Internal-Token: <shared secret> and JSON body {"uid": <int>}
 *   4. This endpoint:
 *        - verifies the shared secret
 *        - creates a fresh row in `sessions` for that uid
 *        - returns the cookie name + sid/ssid for the cgpay POC to forward
 *   5. cgpay POC responds to the browser with Set-Cookie for that session,
 *      scoped to Domain=.commongood.earth so the cookie reaches the PHP site
 *
 * Shared secret lives in config.json under key `cgpaySsoSecret` and is exposed at
 * boot time as the CGPAY_SSO_SECRET constant (see defs.inc). It is rejected unless
 * both ends present a non-empty matching value.
 *
 * This endpoint is NOT called from the browser — only from the cgpay server.
 * No CORS headers, no session/login UI.
 */
function cgpaySso() {
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  // Shared-secret auth. Refuse if config is missing — fail closed, not open.
  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  // Parse JSON body
  $raw = file_get_contents('php://input');
  $body = json_decode($raw, TRUE);
  $uid = isset($body['uid']) ? (int) $body['uid'] : 0;
  if (!$uid) return exitJust(X_SYNTAX);

  // Verify the target account exists and is allowed to use the system
  $a = r\acct($uid);
  if (!$a) return exitJust(X_NOTFOUND);
  if (!$a->ok) return exitJust(X_FORBIDDEN);

  // Fresh session IDs — distinct from the (ephemeral) session of THIS request.
  $sid = drupal_random_key();
  $ssid = drupal_random_key();

  // Direct INSERT — the codebase's db\insert() helper expects an 'id' primary-key
  // column that this table doesn't have (sessions uses sid/ssid as the keys).
  $row = [
    'uid' => $uid,
    'acct' => $uid,              // viewing own account
    'sid' => $sid,
    'ssid' => $ssid,
    'hostname' => ip_address(),
    'timestamp' => REQUEST_TIME,
    'cache' => 0,
    'session' => '',             // Drupal hydrates on the user's next request
  ];
  db\q('INSERT INTO sessions SET uid=:uid, acct=:acct, sid=:sid, ssid=:ssid, hostname=:hostname, timestamp=:timestamp, cache=:cache, session=:session', $row);

  // Menu categories for this user. Permission rules live in PHP (cg-menu.inc);
  // here we just expose top-level categories the cgpay dashboard renders as links.
  $menu = ['Dashboard', 'History', 'Community', 'Settings'];
  if ($a->co) $menu[] = 'Company';
  if ($a->can(B_ADMIN)) $menu[] = 'Admin';

  return exitJson([
    'cookieName' => session_name(),
    'sid'        => $sid,
    'ssid'       => $ssid,
    'menu'       => $menu,
  ], X_OK);
}
