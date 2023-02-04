<?php
namespace CG\Web;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\QR as qr;
use CG\Txs as x;
use CG\Admin as a;
use CG\Risk as k;
use CG\Web as w;

include_once R_ROOT . '/cg-qr.inc';

/**
 * API for interaction with CGPay progressive web app (REST interface) and passthrough for SMP api.
 * @param string $arg: the operation
 */
function api($arg = null) {
  global $mya, $boxId;
  global $channel;

  if (is_array($arg) or nni($_GET, 'request')) { // SMP interface uses /api with no explicit endpoints
    $channel = TX_API;
    u\loga('in', $arg ?: ($_POST ?: $_GET));
    include_once R_ROOT . '/forms/smp.inc';
    return smp($arg);
  }

  $channel = TX_APP;
  u\loga('in:' . $arg, $_POST ?: $_GET);
  setHeaders();
  if ($deviceId = nni($_POST ?: $_GET, 'deviceId')) { // identify user if deviceId specified
    $device = substr($deviceId, 0, DEVICE_MAXLEN);
    if (!$mya = getA($device)) return exitJust(X_UNAUTH);
    if (!$actorId = nni($_POST ?: $_GET, 'actorId')) return exitJust(X_SYNTAX);
// This should work but doesn't    if (qr\url($mya, TRUE) != $actorId) return exitJust(X_UNAUTH); // each device user should have their own deviceId
    $boxId = w\boxId($device, $mya, 500); // find device in the db or put it there
  } elseif ($arg != 'accounts') return exitJust(X_UNAUTH);
  w\signout(TRUE);

  if (!function_exists($func = '\\CG\\Web\\' . $arg)) return exitJust(X_UNAUTH);
  return $func();
}

/**
 * GET /accounts
 * Return a list of accounts managed by the given user.
 *
 * @param string $identifier: qd (abbreviated qid) or email
 * @param string password: account password
 * @return a JSON hash:
 *   accounts: an array of assocs: for each account accessible from the given account: [accountId, deviceId, name]
 *   where accountId is what will be stored on the device to identify the account associated with it
 *         deviceId is a unique identifier for this device associated with this account
 *         name is the name (fullName) on the account
 */
function accounts() {
  global $nextCode; // tests usurp generated device code, using this instead

  foreach (ray('identifier password') as $k) if (!$$k = nni($_GET, $k)) return exitJust(X_UNAUTH);
  if (!$myid = r\loginString($identifier)) return exitJust(X_NOTFOUND);
  
  if (w\flooding($myid, $sta, $err)) return exitJson($err, X_TOOMANY);
  $a = r\acct($myid);
  $success = ($a and $a->passwordOkay($password, 'pass', $err)); // ignore admin pass on dev machine
  if (!$success) return exitJust($err, X_NOTFOUND);
  w\updateFlood($sta, $success, $err, TRUE);
  if (!$a->ok) return exitJust(X_UNAUTH);
  
  if (!$list = be\accountChoices(r\acct($myid), B_MANAGE)) return exitJust(X_NOTFOUND);

  foreach ($list as $uid => $name) {
    $a = r\acct($uid);
    $accountId = qr\url($a, TRUE);
    $deviceId = nn($nextCode) ?: base64_encode(u\cryPGP(now() . "/$name", 'private')); // prove to customer who they're dealing with
    $qr = ($uid == $myid and !$a->stepsLeft('card')) ? myQr($a) : '';
    $items = explode("\n", $a->selling);
    $accounts[] = compact(ray('accountId deviceId name items'));
  }
  return exitJson(compact('accounts'));
}

function myQr($a) {
  include_once R_ROOT . '/cg-qr.inc';
  $logo = $a->photoSrc(FALSE, TRUE, TRUE);
//  $text = $a->bestName . ", $a->city, $a->st"; // maybe put this at the top, under device-owner name
  $testQr = qr\url($a);
  return base64_encode(qr\generate($testQr, $logo, $a->mainQid));
}

/**
 * GET /identity
 * Return an account's name, agent name, and location.
 *
 * @param string $otherId: abbreviated version of the information from the QR Code for the account to charge, without <type> and <counter>
 *   abbreviated version is <fmt><region><acct><agent><cardCode><type><counter>
 * @return a JSON hash:
 *   name: the fullName of the account
 *   agent: the name of the person acting on behalf of a company (empty for an individual account)
 *   location: the account’s city, state
 *   limit: the maximum amount this account can be charged at this time
 *   items: the device owner's list of transaction descriptions
 */
function identity() {
  global $mya;
  include_once R_ROOT . '/cg-qr.inc';
//  if (!rayhas($_GET, 'otherId')) return idPhoto(); // use /identity as a single endpoint for identity and idPhoto
  foreach (ray('actorId otherId') as $k) if (!$$k = nni($_GET, $k)) return exitJust(X_SYNTAX);
  if (!$mya = getAcct($actorId, FALSE)) return exitJust(X_UNAUTH); // don't just get this from deviceId (could be ambiguous)
  if (!$a = getAcct($otherId) or !$a->ok) return exitJust(X_NOTFOUND);

  $name = $a->fullName;
  $agent = $a->proSe ? '' : $a->agentA->fullName;
  $location = $a->location();
  $limit = round(PHP_INT_MAX - $a->shortfall(FOR_GOODS, PHP_INT_MAX, FALSE, $mya), 2);
  if ($limit > 0) $limit = round($limit - be\handleSurTx($zot, obj(), $a, $mya, $limit, FALSE, TRUE), 2);
  $creditLine = 0 - $a->floor; // "0 -" to prevent -0
  $avgs = r\averageBalance($a->id, today(u\plusMonths(-6))); // based on 6 months even if member is new
  $avgBalance = round($avgs[0], 2);
  $trustRatio = round($a->trust / (db\avg('trust', 'users', ':IS_OK AND NOT :IS_CO') ?: 1), 2);
  $since = (int) $a->activated;
  $items = explode("\n", $mya->selling);

  return exitJson(compact(ray('name agent location limit creditLine avgBalance trustRatio since items')));
}

/**
 * GET /identity (syntax for ID photo image source)
 * Return an account's id photo.
 *
 * @param string $accountId: part of the abbreviated version of the account associated with the device, just <fmt><region><acct>
 * @param string $code: A ciphertext consisting of the first DEVICE_MAXLEN characters of the device’s unique identifier followed by the otherId (the same as for the identity endpoint), encrypted with the server’s public key then base64-encoded, substituting '-' and '_' for '+' and '/' (342 chars).
 * @return the image (mime type image/jpeg) or an error message (plain text)
 */
function idPhoto() {
  global $mya;
  include_once R_ROOT . '/cg-qr.inc';
/*  foreach (ray('accountId code') as $k) if (!$$k = nni($_GET, $k)) return exitJust(X_SYNTAX);
  if (!$mya = getAcct($accountId, FALSE)) return exitJust(X_NOTFOUND);
  if (!$code = u\decryPGP(u\b64decode($code), 'private')) return exitJust(X_NOTFOUND);
  if (!$otherId = substr($code, DEVICE_MAXLEN) or !$a = qr\acct($otherId, TRUe)) return exitJust(X_NOTFOUND);
  */
  foreach (ray('actorId otherId') as $k) if (!$$k = nni($_GET, $k)) return exitJust(X_SYNTAX);
  if (!$mya = getAcct($actorId, FALSE)) return exitJust(X_UNAUTH); // don't just get this from deviceId (could be ambiguous)
  if (!$a = getAcct($otherId)) return exitJust(X_NOTFOUND);

  return photoReturn($a->agentA->photo);
}

/**
 * Create the requested transaction.
 * @param numeric $amount: amount of transaction, from the actor's point of view (negative to reverse a charge)
 * @param string $deviceId: the device identifier
 * @param string $actorId: abbreviated identifier for the account associated with the device (the actor)
 * @param string $otherId: abbreviated identifier for the other account
 * @param string $description: a brief description of the transaction
 * @param int $created: Unixtime the transaction was processed on the device
 * @param string $proof: SHA256 hash of actorId + amount (with exactly two digits after the decimal point) + otherId + security code + created.
 * @param bool $offline: <transaction was processed offline and stored for later upload>
 *
 * NOTE: use only X_OK and X_SYNTAX here.
 */
function transactions() {
  global $mya, $boxId;
  include_once R_ROOT . '/classes/tx.class';

  u\loga('in', $_POST);
  $flds = 'amount deviceId actorId otherId description created proof';
  foreach (ray($flds) as $k) if (!nni($_POST, $k)) return exitJust(t('missing parameter %k', compact('k')), X_SYNTAX);
  extract(just($flds . ' offline', $_POST));
  $version = 100;
  $offline = (int) ($offline == 'true');

  if (!$mya = getAcct($actorId, FALSE)) return postErr(t('This device is not connected to an authorized account.'));
  r\setAcct($mya);
  if (!$a = getAcct($otherId, FALSE) or !$a->ok) return postErr(t('That is not an active account.')); // proof is proof enough (no cardCode needed)
  $cardCode = $a->cardCode();
  if ($err = u\badAmount($amount, '!=0', 2, MAX_AMOUNT)) return exitJust(t('bad amount'), X_SYNTAX); // allow negatives
  if (!is_numeric($created) or $created < now() - 30 * DAY_SECS or $created > now() + 2 * DAY_SECS - 1) return exitJust(t('bad date'), X_SYNTAX);

  $proofAmt = number_format(abs($amount), 2, '.', '');
  if (test()) $proof = u\hash($got = $proof);
  $hashOfTx = u\hash($wanted = "$actorId$proofAmt$otherId$created");
  
  if ($proof != $hashOfTx) {
    r\tellAdmin('bad proof', compact(ray('version proof hashOfTx actorId amount otherId cardCode created offline')));
    if (test()) f('t.output', "Wanted proof: $wanted, got: $got");
    return exitJust(t('bad proof'), X_SYNTAX);
  }
  
  if (!$device = decodeDevice($deviceId)) return postErr(t('This device is not connected to a %PROJECT account.'));
  $boxId = w\boxId(substr($deviceId, 0, DEVICE_MAXLEN), $mya, $version); // find device in the db or put it there

  $goods = 0; $selfhelp = FALSE;
  $info = ray('created uid1 uid2 amt goods for2', $created, $a->id, $mya->id, $proofAmt, $goods, $description);
  
  if ($amount < 0) { // reversing (no effort is made to Undo in real time, so all reversals are offline-agnostic)
    if ($xid = db\get('xid', 'txs', $info)) { // exists
      $z = be\undoTx($mya, $xid, 1, [], TRUE);
      if ($z->ok) r\tellAdmin('reversing tx', ray('xid who created message subs', $xid, "$mya->fullName ($mya->mainQid)", fmtDt($z->subs['created']), $z->msg, $z->subs)); // if ($z->ok and nni($z->subs, 'xid'))
      if (!$z->ok) return badTx($_POST, $a->fullName, $z->msg, $z->subs);
    } else { // does not exist
      exitJson(ray('ok message', TRUE, 'deleted'), X_OK); // message is ignored
    }
  }  

  if ($offline and $xid = db\get('xid', 'txs', $info)) { // forcing and transaction exists
    $type = $amount < 0 ? 'payment' : 'charge';
    $z = be\sendNotifications($mya, $type, r\Tx::read($xid), 0, FALSE); // fake the message from existing transaction
    $message = $z->msg;
  } else { // transaction doesn't exist (yet)
    if ($amount < 0 and !$mya->co) list ($type, $amount) = ['payment', -$amount]; else $type = 'charge';
    $z = be\transfer($type, $mya, $a, $amount, $description, $goods, compact(ray('created offline')));
    if (!$z->ok) return badTx($_POST, $a->fullName, $z->msg, $z->subs);

    if (!$offline and $aa = $a->agentA and $mya->co and !$aa->ided) $aa->setBit(B_IDED);
    $txid = $z->subs['xid'];
    if ($selfhelp) {
      $message = 'selfhelp tx';
    } else {
      $message = $z->msg;
      $forCash = $goods == FOR_USD ? t('for USD') : '';
      $we = $mya->co ? t('We') : t('You');
      $did = tr('just did', $z->subs + compact('forCash', 'we'));
    }
  }
  
  return exitJson(ray('ok message', TRUE, $message), X_OK);
}

/**
 * POST error: return an error message and success status = 0.
 * @param string $msg: the error message
 */
function postErr($msg) {
  u\loga('posterr', $res = ray('ok message', FALSE, $msg));
  exitJson($res, X_OK);
}

/**
 * Return the account corresponding to the given abbreviation.
 * @param string $cardId: the customer abbreviated account code.
 * @param string $code: hash of the customer's card code (formerly the card code itself)
 * @param string $err: (returned) error message, if any
 * @param bool $requireCode: require card code (FALSE for transactions)
 * @return: the customer's acct object (NULL if error)
 */
function getAcct($cardId, $requireCode = TRUE) {
  include_once R_ROOT . '/cg-qr.inc';
  
  if (!$a = qr\acct($cardId, $requireCode)) return r\badCard($cardId, NULL, $err = 'bad card'); 
//  if (r\foreignServer(r\uidRegion($a->agentId))) return !$err = 'wrong server for customer';
  if (!$a->ok) return r\badCard($cardId, NULL, $err = 'card inactive');
  return $a;
}

/**
 * Tell the device owner about a failed background transaction
 */
function badTx($args, $account, $message, $subs = []) {
  extract(just('offline amount created', $args, NULL));
  
  if ($offline == 'true') {
    global $mya;
    $by = $mya->proSe ? '' : t(' (agent %agent)', 'agent', $mya->agentA->fullName);
    $subs += ray('account amount created by', $account, u\fmtAmt($amount), fmtDt($created), $by);
    $mya->tell("bad forced tx|$message", $subs);
    r\tellAdmin("bad forced tx|$message", $subs);
  }
  
  return postErr($message); // offline transactions are always accepted
}

/**
 * Decode the device code using a public key from the appropriate server (just this one for now).
 * @param string $device: the b64encoded encrypted device code received from the device
 * @return: an adequate abbreviation of $device (FALSE if error)
 */
function decodeDevice($device) {
  if (FALSE) if (!test() and !u\decryPGP(u\b64decode($device), 'public')) return FALSE; // temporarily disabled while figuring out how to do ssl public encryption from JS or switch to GPG
  return substr($device, 0, DEVICE_MAXLEN);
}

/**
 * Return the customer's account photo.
 */
function photoReturn($pic) {
  $max = 50; // arbitrary abbreviation length for logging and testing
  $message = strlen($pic) <= $max ? $pic : bin2hex(substr($pic, 0, $max)); // identifies pic in log w/out a big storage hit
  u\loga('out', compact('message'));

  if (test()) {
    $picNone = file_get_contents(R_DFT_PICTURE); // never a company photo
    if (u\starts(bin2hex(substr($picNone, 0, $max)), $message)) $message = 'no photo';
    f('t.output', $message); 
    $pic = $message;
  }

  if (strlen($pic) > $max) header("Content-type: image/jpeg");
  return exitJust($pic, X_GOT);
}

/**
 * Return the company acct.
 * @param string $device: the app user's device ID, first 50 chars.
 * @param string $err: (returned) error message, if any
 * @return: the agent's company acct object (NULL if error)
 */
function getA($device) {
  if (!$uid = db\get('uid', 'r_boxes', 'code=:device ORDER BY access DESC', compact('device'))) return NULL;
  if (!$mya = r\acct($uid)) return NULL; // bad agent
//  if (r\foreignServer(r\uidRegion($mya->agentId))) return NULL; // wrong server for agent
  if (!$mya->ok) return NULL; // company inactive
  if (!$mya->proSe) r\setAcct($mya->agentId); // get agent first, if not proSe
  return r\setAcct($mya);
}

function setHeaders() {
  header('Access-Control-Allow-Origin: *');
  header('Access-Control-Allow-Methods: *'); // GET,HEAD,OPTIONS,POST,PUT
  header('Access-Control-Allow-Headers: Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers');
  header('Access-Control-Expose-Headers: *');
  header('Access-Control-Allow-Credentials: true');

  if (nni($_SERVER, 'REQUEST_METHOD') === 'OPTIONS') { // need preflight here
    header('Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept');
    header('Access-Control-Max-Age: 86400'); // add cache control for preflight cache
    header('Cache-Control: public, max-age=86400'); // @link https://httptoolkit.tech/blog/cache-your-cors/
    header('Vary: origin');
    exit(0);
  }

}
