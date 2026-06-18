<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

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

  // The SSO request itself came in without a session cookie, so Drupal never
  // called session_start() at boot — meaning _drupal_session_write() won't fire
  // automatically at shutdown. Start the session machinery now so the shutdown
  // handler is wired up.
  if (!drupal_session_started()) drupal_session_start();

  // Promote this request to the target user. _drupal_session_write() at
  // shutdown reads $user->uid and writes that into the new sessions row, keyed
  // by session_id().
  global $user;
  $user = (object) ['uid' => $uid];

  // Populate the session vars cgmembers bootstrap reads on the FOLLOWING request
  // (when the browser arrives at the PHP site with our cookie). r\setAcct() with
  // no args reads w\svar('myid') / w\svar('agentId') — which are stored as
  // serialized values under the SVAR_HEADER ('rcredits_') prefix in $_SESSION
  // (see rweb-subs.inc:svar). Without these, $mya is NULL and r\access() fails
  // every permission check, so /history etc. redirect to the signin page.
  $_SESSION[SVAR_HEADER . 'myid'] = serialize($uid);
  $_SESSION[SVAR_HEADER . 'agentId'] = serialize($uid);

  // session_id() now holds the cookie value the browser will send back to demo
  // to authenticate as this user. Drupal writes it to both sid and ssid on
  // HTTPS-only sites.
  $cookieVal = session_id();

  // Create a fresh `box-<QID>` device record + cookie. Acct::boxId() normally
  // does this on a web signin, but its web-channel path reads the existing
  // device code from $_COOKIE — which is empty on this server-to-server call.
  // Inline the new-code path: generate a code, write the r_boxes row, return
  // the cookie name and value so SvelteKit can forward it scoped to the shared
  // parent domain alongside the SSESS cookie. Without this, downstream PHP
  // features that look the device up in r_boxes don't recognise it.
  $boxName = 'box-' . $a->mainQid;
  $boxValue = u\code();
  $boxnum = (db\get('MAX(boxnum)', 'r_boxes', ['uid' => $uid]) ?: 0) + 1;
  db\insert('r_boxes', [
    'uid' => $uid,
    'code' => $boxValue,
    'boxnum' => $boxnum,
    'created' => now(),
    'access' => now(),
    'channel' => TX_WEB,
  ]);

  // Menu categories for this user. Permission rules live in PHP (cg-menu.inc);
  // here we just expose top-level categories the cgpay dashboard renders as links.
  $menu = ['Dashboard', 'History', 'Community', 'Settings'];
  if ($a->co) $menu[] = 'Company';
  if ($a->can(B_ADMIN)) $menu[] = 'Admin';

  return exitJson([
    'cookieName'      => session_name(),
    'sid'             => $cookieVal,
    'ssid'            => $cookieVal,
    'menu'            => $menu,
    'boxCookieName'   => $boxName,
    'boxCookieValue'  => $boxValue,
  ], X_OK);
}
