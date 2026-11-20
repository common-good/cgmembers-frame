<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

require_once __DIR__ . '/token.inc';

const DEMO_ACCOUNTS_PER_HOUR = 20; // per address, so the endpoint cannot be used to fill the database

/**
 * /cgpay/demo: the store app's demo sign-in (the identifier "demo" and password "pass" on its sign-in screen, which store reviewers use).
 * Makes a fresh individual demo account, with some play money and no MFA, and signs in as it.
 *   body: {label, deviceKey, ip}
 *   200 {token, uid, admin, accounts}; 404 {error: not-demo} anywhere but demo; 429 {error: flooding}
 * Accounts nobody has used for a while are purged (see the demo cleanup in the todo list).
 */
function cgpayDemo() {
  if (($in = appIn()) === FALSE) return;
  if (!appDemo()) return appErr('not-demo', X_NOTFOUND);
  foreach (just('label deviceKey', $in, '') as $k => $v) $$k = is_scalar($v) ? (string) $v : '';
  $ip = $_SERVER['REMOTE_ADDR'];
  if (!\flood_is_allowed('cgpay_demo_account', DEMO_ACCOUNTS_PER_HOUR, HOUR_SECS, $ip)) return appErr('flooding', X_TOOMANY);
  \flood_register_event('cgpay_demo_account', HOUR_SECS, $ip);

  $id = strtolower(u\randomString(8, 'lower'));
  $fullName = 'Demo ' . ucfirst(substr($id, 0, 5));
  $info = ray('dob helper floor pin community activated', strtotime('11/1/2002'), 1, -9999, '1234', DEMO_CTTY, now());
  $flags = u\bits('ok member confirmed carded ided debt');
  $email = "$id@example.com";
  $phone = u\fmtPhone('700-' . u\randomString(3, 'digits') . '-' . u\randomString(4, 'digits'));
  list ($address, $city, $state, $zip) = u\parseAddr($postalAddr = '1 Demo St., Demoton, MA 01000');
  $state = r\stateNum($state);
  $info += compact(ray('fullName flags email phone postalAddr address city state zip')) + ['name' => u\shortName($fullName), 'cardCode' => $cardCode = r\cardCode(), 'emailCode' => r\cardCode(), 'pass' => 'pass'];

  if (!$a = new r\Acct($info)) return appErr('failed', X_SYNTAX);
  foreach (ray(S_CARD) as $k) $a->stepDone($k);
  db\q('UPDATE users SET balance=1000 WHERE uid=:uid', ['uid' => $a->id]);
  return appVerify($a->id, NULL, $label, $deviceKey, TRUE);
}
