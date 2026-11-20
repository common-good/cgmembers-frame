<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

require_once __DIR__ . '/token.inc';

const DEMO_ACCOUNTS_PER_HOUR = 20; // per address
const DEMO_ACCOUNTS_PER_DAY = 500; // in all, so changing addresses cannot fill the database either
const DEMO_SOURCE = 'store app demo'; // marks the accounts for the purge of unused ones
const DEMO_PURGE_DAYS = 7; // a demo account nobody has used for this long is deleted (an app review can take several days)

/**
 * /cgpay/demo: the store app's demo sign-in (the identifier "demo" and password "pass" on its sign-in screen, which store reviewers use).
 * Makes a fresh individual demo account, with some play money and no MFA, and signs in as it.
 *   body: {label, deviceKey, ip}
 *   200 {token, uid, admin, accounts}; 404 {error: not-demo} anywhere but demo; 429 {error: flooding}
 * The accounts carry source DEMO_SOURCE, so a purge can find the unused ones.
 */
function cgpayDemo() {
  if (($in = appIn()) === FALSE) return;
  if (!appDemo()) return appErr('not-demo', X_NOTFOUND);
  foreach (just('label deviceKey', $in, '') as $k => $v) $$k = is_scalar($v) ? (string) $v : '';
  $ip = $_SERVER['REMOTE_ADDR'];
  if (!\flood_is_allowed('cgpay_demo_account', DEMO_ACCOUNTS_PER_HOUR, HOUR_SECS, $ip) or !\flood_is_allowed('cgpay_demo_accounts', DEMO_ACCOUNTS_PER_DAY, DAY_SECS, 'all')) return appErr('flooding', X_TOOMANY);
  \flood_register_event('cgpay_demo_account', HOUR_SECS, $ip);
  \flood_register_event('cgpay_demo_accounts', DAY_SECS, 'all');

  $id = strtolower(u\randomString(8, 'lower'));
  $fullName = 'Demo ' . ucfirst(substr($id, 0, 5));
  $info = ray('dob helper floor pin community activated', strtotime('11/1/2002'), 1, -9999, '1234', DEMO_CTTY, now());
  $flags = u\bits('ok member confirmed carded ided debt');
  $email = "$id@example.com";
  list ($address, $city, $state, $zip) = u\parseAddr($postalAddr = '1 Demo St., Demoton, MA 01000');
  $state = r\stateNum($state);
  $info += compact(ray('fullName flags email postalAddr address city state zip')) + ['source' => DEMO_SOURCE] + ['name' => u\shortName($fullName), 'cardCode' => $cardCode = r\cardCode(), 'emailCode' => r\cardCode(), 'pass' => 'pass'];

  if (!$a = new r\Acct($info)) return appErr('failed', X_SYNTAX);
  foreach (ray(S_CARD) as $k) $a->stepDone($k);
  db\q('UPDATE users SET balance=1000 WHERE uid=:uid', ['uid' => $a->id]);
  return appVerify($a->id, NULL, $label, $deviceKey, TRUE);
}

/**
 * Delete the demo accounts made by /cgpay/demo that nobody has used for DEMO_PURGE_DAYS (older ones whose app tokens are idle that long).
 * Runs daily on demo (see everyDay()). Returns how many were deleted.
 */
function purgeDemoAccounts() {
  if (!appDemo()) return 0;
  require_once R_ROOT . '/admin/admin.inc';
  $old = now() - DEMO_PURGE_DAYS * DAY_SECS;
  $source = DEMO_SOURCE;
  $uids = db\col('uid', 'users u', 'source=:source AND created<:old AND NOT EXISTS (SELECT 1 FROM app_tokens t WHERE t.uid=u.uid AND t.lastUsed>=:old)', compact('source', 'old'));
  foreach ($uids as $uid) {db\del('app_tokens', 'uid=:uid', compact('uid')); \CG\Admin\deleteUid($uid);}
  return count($uids);
}
