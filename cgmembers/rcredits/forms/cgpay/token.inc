<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Db as db;
use CG\Util as u;

/**
 * @file
 * Shared code of the store app's token sign-in endpoints (/cgpay/signin, mfa, device, account, enter, redeem, unlock, revoke).
 *
 * The app signs in once and keeps a revocable token (only its sha256 is stored, in app_tokens). Every call
 * comes from the node server (X-CG-Internal-Token, like /cgpay/sso), never from the browser. Errors are JSON
 * {"error": "<code>"} so the app can word them itself.
 */

const APP_TOKEN_DAYS = 90; // a token not used for this long stops working
const APP_CODE_SECS = 60; // life of a one-time code for opening a live page
const APP_MFA_SECS = 600; // life of the second sign-in step
const APP_API_VERSION = 40206; // the legacy app's version, so the device id has the current format

/**
 * Check the method and shared secret, then return the request fields, or FALSE after replying with the error.
 */
function appIn() {
  global $channel; $channel = TX_API;
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') {exitJust(X_SYNTAX); return FALSE;}
  $given = (string) nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (CGPAY_SSO_SECRET === '' or !hash_equals(CGPAY_SSO_SECRET, $given)) {exitJust(X_UNAUTH); return FALSE;}
  $in = json_decode(test() ? nni($GLOBALS, 'testBody') : file_get_contents('php://input'), TRUE) ?: [];
  if (filter_var(nni($in, 'ip'), FILTER_VALIDATE_IP)) {
    $_SERVER['REMOTE_ADDR'] = $in['ip']; // the caller is the node server, so flood control must see the member's address, not its own
    \drupal_static_reset('ip_address'); // ip_address() may have cached the node server's address already
  }
  return $in;
}

/**
 * Reply with an error code (and the HTTP status that goes with it).
 */
function appErr($code, $status = X_NOTFOUND) {return exitJson(['error' => $code], $status);}

function appHash($s) {return hash('sha256', (string) $s);}

/**
 * Finish a sign-in: flood control, the password, the account checks, then a token or (with MFA) a handle for /cgpay/mfa.
 * @param string $password: the member's password, or NULL when a scanned card was enough (demo only, see /cgpay/card)
 * @param bool $skipMfa: no emailed passcode (demo only)
 * @param int $actFor: the account the app should act for first (a company card's company), or NULL
 */
function appVerify($uid, $password, $label, $deviceKey, $skipMfa = FALSE, $actFor = NULL) {
  $sta = [];
  if (w\flooding($uid, $sta, $err)) return appErr('flooding', X_TOOMANY);
  $a = r\acct($uid);
  if (!is_null($password)) {
    $success = ($a and $a->passwordOkay($password, 'pass', $err));
    w\updateFlood($sta, $success, $err);
    if (!$success) return appErr('bad-login', X_UNAUTH);
  }
  if (!$a->ok) return appErr('not-ok', X_FORBIDDEN);
  if (isPRODUCTION and $a->admin and !u\strongPass((string) $password, u\pwAvoid($a))) return appErr('weak-admin-password', X_FORBIDDEN);

  if ($skipMfa or !$a->canMfa or appTrusted($uid, $deviceKey)) return appIssue($uid, $label, $deviceKey, FALSE, $actFor);

  $nonce = u\randomString(8, 'digits') . '';
  r\rMail('confirm-signin', $uid, compact('nonce'));
  $expires = now() + APP_MFA_SECS;
  $uid .= '';
  return exitJson(['mfa' => u\cryRay(compact(ray('uid nonce expires label deviceKey actFor')))], X_OK);
}

/**
 * Return whether this server is demo. A developer's own machine can pretend to be (testops 'demo', and the feature tests), so the store app's env switching can be tested against one server; an online server never can.
 */
function appDemo() {global $testDemo; return (bool) (isDEMO or (test() and $testDemo) or (isLOCAL and !test() and getv('testDemo', 0)));}

/**
 * Return this server's tag for its tokens, so the app can tell which env a token is for and no env takes another's.
 * beta shares main's config (and database), so its tokens are main's.
 */
function appEnv() {return (appDemo() ? 'demo' : STAGE) . '.';} // a developer's machine acting as demo tags its tokens as demo's, as the app expects

/**
 * Make a token for the member, record it, and reply with it and the device credentials the transaction screens need.
 * @param bool $trust: this device skips MFA from now on
 * @param int $actFor: the account the app should act for first, or NULL
 */
function appIssue($uid, $label, $deviceKey, $trust, $actFor = NULL) {
  $token = appEnv() . bin2hex(random_bytes(32));
  $created = $lastUsed = now();
  $expires = $created + APP_TOKEN_DAYS * DAY_SECS;
  $tokenHash = appHash($token);
  $deviceHash = $deviceKey ? appHash($deviceKey) : NULL;
  $trusted = $trust ? 1 : 0;
  $label = mb_substr(trim((string) $label), 0, 60);
  db\insert('app_tokens', compact(ray('uid tokenHash deviceHash label trusted created lastUsed expires')));
  return exitJson(['token' => $token] + appMember($uid) + ($actFor ? ['account' => (int) $actFor] : []), X_OK);
}

/**
 * Return what the app keeps about the member: uid, whether they switch accounts by typing (admins), and the accounts they may act for.
 */
function appMember($uid) {
  $accounts = [];
  foreach (be\accountChoices(r\acct($uid), B_BUY, 'u.:IS_OK', TRUE) ?: [] as $id => $name) if ($one = appAccount($id, $uid, $name)) $accounts[] = $one;
  return ['uid' => (int) $uid, 'admin' => (bool) r\acct($uid)->admSeeAccts, 'accounts' => $accounts];
}

/**
 * Return one account as the app keeps it: uid, name, isCo, photo (a data URL of the thumbnail, so it shows offline, or NULL),
 * and device (the credentials the transaction screens use, or NULL if the account can't use the app yet).
 */
function appAccount($id, $agent, $name = '') {
  include_once R_ROOT . '/forms/api.inc';
  if (!$a = r\acct($id, $agent)) return NULL;
  $photo = $a->thumb ? 'data:image/jpeg;base64,' . base64_encode($a->thumb) : NULL;
  return ['uid' => (int) $id, 'name' => $name ?: $a->bestName, 'isCo' => (bool) $a->co, 'photo' => $photo, 'device' => accountInfo($a, $a->fullName, APP_API_VERSION)];
}

/**
 * Return whether the member may act for the account: a relation with permission, or an admin's reach (the rules of change-account, see changeWho()).
 */
function appMayAct($uid, $id) {
  if ($id == $uid) return TRUE;
  if (!$a = r\acct($id, $uid)) return FALSE;
  return $a->can() or ($a->admin and ($a->admRegion or $a->community == $a->agentA->community));
}

/**
 * Return the live token row for the token, slid forward; or FALSE after replying that the token is not good.
 */
function appRow($token) {
  $now = now();
  $row = u\starts((string) $token, appEnv()) ? db\row('app_tokens', 'tokenHash=:h AND revoked IS NULL AND expires>:now', ['h' => appHash($token), 'now' => $now]) : NULL;
  if (!$row) {appErr('bad-token', X_UNAUTH); return FALSE;}
  db\update('app_tokens', ['id' => $row->id, 'lastUsed' => $now, 'expires' => $now + APP_TOKEN_DAYS * DAY_SECS]);
  return $row;
}

/**
 * Mark a one-time value used (kept with the one-time codes, by hash) and return whether this call was the first to use it.
 */
function appClaim($s, $uid, $expires) {
  return (bool) db\q('INSERT IGNORE INTO app_codes SET uid=:uid, account=:uid, codeHash=:h, dest=:dest, expires=:expires, usedAt=:now', ['uid' => $uid, 'h' => appHash($s), 'dest' => '', 'expires' => $expires, 'now' => now()])->rowCount();
}

/**
 * Return the uid the identifier names (an account code, email, phone number or name), or NULL.
 */
function appFind($identifier) {
  if ($uid = r\loginString($identifier)) return $uid;
  if (!preg_match('/^[\+\(\)\.\- 0-9]{7,}$/', $identifier)) return NULL;
  $phone = u\cry('P', u\fmtPhone($identifier, '+n'));
  return db\get('uid', 'users', 'phone=:phone ORDER BY :IS_CO', compact('phone')) ?: NULL;
}

/**
 * Return whether this installation is trusted for the member (signed in with MFA before, and not revoked everywhere since).
 */
function appTrusted($uid, $deviceKey) {
  return $deviceKey and db\exists('app_tokens', 'uid=:uid AND deviceHash=:h AND trusted=1', ['uid' => $uid, 'h' => appHash($deviceKey)]);
}
