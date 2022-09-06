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

/**
 * API for interaction with CGPay progressive web app (REST interface) and passthrough for SMP api.
 * @param string $arg: the operation
 */
function api($arg = null) {
  global $channel; $channel = TX_API;

  if (!$arg or is_array($arg)) { // SMP interface uses /api with no explicit endpoints
    include_once R_ROOT . '/forms/smp.inc';
    return smp($arg);
  }
  
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
 *   accounts: an array of assocs: for each saccount accessible from the given account: [cardId, deviceId, name]
 *   where cardId is what will be stored on the device to identify the account associated with it
 *         deviceId is a unique identifier for this device associated with this account
 *         name is the name (fullName) on the account
 */
function accounts() {
  global $nextCode; // tests usurp generated device code, using this instead
  include_once R_ROOT . '/cg-qr.inc';

  foreach (ray('identifier password') as $k) if (!$$k = nni($_GET, $k)) return exitJust(X_UNAUTH);
  if (!$uid = r\loginString($identifier)) return exitJust(X_NOTFOUND);
  
  if (w\flooding($uid, $sta, $err)) return exitJson($err, X_TOOMANY);
  $a = r\acct($uid);
  $success = ($a and $a->passwordOkay($password, 'pass', $err)); // ignore admin pass on dev machine
  w\updateFlood($sta, $success, $err);
  if (!$success) return exitJust($err, X_NOTFOUND);
  if (!$list = be\accountChoices(r\acct($uid))) return exitJust(X_NOTFOUND);

  foreach ($list as $uid => $name) {
    $a = r\acct($uid);
    $cardId = qr\url($a, TRUE);
    $deviceId = nn($nextCode) ?: u\b64encode(u\cryPGP(now() . "/$name", 'private')); // prove to customer who they're dealing with
    $accounts[] = compact(ray('cardId deviceId name'));
  }
  return exitJson(compact('accounts'));
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
 */
function identity() {
  include_once R_ROOT . '/cg-qr.inc';
  foreach (ray('otherId') as $k) if (!$$k = nni($_GET, $k)) return exitJust(X_UNAUTH);
  if (!$a = getAcct($otherId)) return exitJust(X_NOTFOUND);
  
  return exitJson(ray('name agent location', $a->fullName, $a->proSe ? '' : $a->agentA->fullName, $a->location()));
}

/**
 * GET /idPhoto
 * Return an account's id photo.
 *
 * @param string $accountId: part of the abbreviated version of the account associated with the device, just <fmt><region><acct>
 * @param string $code: A cyphertext consisting of the first 50 characters of the device’s unique identifier followed by the otherId (the same as for the identity endpoint), encrypted with the server’s public key then base64-encoded, substituting “-” and “_” for “+” and “/” (342 chars).
 * @return the image (mime type image/jpeg) or an error message (plain text)
 */
function idPhoto() {
  include_once R_ROOT . '/cg-qr.inc';
  foreach (ray('accountId code') as $k) if (!$$k = nni($_GET, $k)) return exitJust(X_UNAUTH);
  if (!$mya = getAcct($accountId, FALSE)) return exitJust(X_NOTFOUND);
  if (!$code = u\decryPGP(u\b64decode($code), 'private')) return exitJust(X_NOTFOUND);
  if (!db\exists('r_boxes', ray('uid code', $mya->id, substr($code, 0, 50)))) return exitJust(X_NOTFOUND);
  if (!$otherId = substr($code, 50) or !$a = qr\acct($otherId, TRUe)) return exitJust(X_NOTFOUND);
  
  return photoReturn($a->agentA->photo);
}

/**
 * Create the requested transaction.
 * @param numeric $amount: amount of transaction, from the actor's point of view
 * @param string $deviceId: the device identifier
 * @param string $actorId: abbreviated identifier for the account associated with the device (the actor)
 * @param string $otherId: abbreviated identifier for the other account
 * @param string $description: a brief description of the transaction
 * @param int $created: Unixtime the transaction was processed on the device
 * @param string $proof: SHA256 hash of actorId + amount (with exactly two digits after the decimal point) + otherId + security code + created.
 * @param bool $offline: <transaction was processed offline and stored for later upload>
 */
function transactions() {
  global $mya, $boxId;
  include_once R_ROOT . '/classes/tx.class';
  extract(just($flds = 'amount deviceId actorId otherId description created proof offline', $_POST, NULL));
  
  // for testing
  $_POST = ray($flds, 123, '', 'K6VMDJI', 'K6VMDJJ', 'stuff', now(), '', FALSE);
  extract($args = just($flds, $_POST, NULL));
  $deviceId = 'qXQL7XsUfsn85DYWIx7lfZIfqTRCqk0_eJoxzIHl4NfbJNyaXHo8sGD1LHq6JC3rvWd0V3UFetNrXG2OHxUybMr2h55K9uP8FANCtyxiDfZ248NJd39czaEWVQohsD82hGouLCdkwpQy2ojV3to8XObSiIj-4MczkbXStq5C4HxJTCJ--st_z8t6EqhGByvuXwTFMeylTlq6agCVvwAhfX6clyS6-TcnmdA9IJs4HnT9vTC1rpqN05zW7S3bi566w7pEfcHh-A28QKsx172XkSiEaHo2cpapYQreUs4QCYXNyLbaq5l-i6m3d0y-RmB9ImFsbGaxhThIh_zLKXyxDw'; // test
  $mya = a('zza'); $a = a('zzb');
  $proof = u\hash("$actorId$amount.00$otherId$a->cardCode$created");
  // end of for testing

  foreach (ray($flds) as $k) if (!isset($$k)) return exitJust($k, X_SYNTAX);
  $version = 100;

  if (!$mya = getAcct($actorId)) return exitJust(X_NOTFOUND);
  r\setAcct($mya);
  if (!$a = getAcct($otherId)) return exitJust(X_NOTFOUND); // proof is proof enough (no cardCode needed)
  $code = $a->cardCode();
  if ($err = u\badAmount($amount, '', 2, MAX_AMOUNT)) return exitJust(X_SYNTAX); // allow negatives

  $amount = str_replace(',', '', number_format($amount, 2)); // make sure both cents digits are included

  $hashOfTx = u\hash($wanted = "$actorId$amount$otherId$code$created");
  if ($proof != $hashOfTx) {
    r\tellAdmin('bad proof', compact(ray('version proof hashOfTx actorId amount otherId code created offline')));
    if (test()) f('t.output', "Should be proof of $wanted");
    return exitJust(X_SYNTAX);
  }

  if (!$device = decodeDevice($deviceId)) return exitJust(X_NOTFOUND);
  $boxId = w\boxId($device, $mya, $version); // find device in the db or put it there

  $goods = $selfHelp = 0;
  $info = ray('created uid1 uid2 amt goods for2', $created, $a->id, $mya->id, $amount, $goods, $description);

  if ($offline and $xid = db\get('xid', 'txs', $info)) { // forcing and transaction exists
    $type = $amount < 0 ? 'payment' : 'charge';
    $z = sendNotifications($mya, $type, Tx::read($xid), 0, FALSE); // fake the message from existing transaction
  } else { // transaction doesn't exist (yet)
    if ($amount < 0 and !$mya->co) list ($type, $amount) = ['payment', -$amount]; else $type = 'charge';
    $z = be\transfer($type, $mya, $a, $amount, $description, $goods, compact(ray('created offline')));
    if (!$z->ok) return badTx($args, $a->fullName, $z->msg, $z->subs);

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
  
  return exitJson($message, X_OK);
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
  
  if ($offline) {
    global $mya;
    $by = $mya->proSe ? '' : t(' (agent %agent)', 'agent', $mya->agentA->fullName);
    $subs += ray('account amount created by', $account, u\fmtAmt($amount), fmtDt($created), $by);
    $mya->tell("bad forced tx|$message", $subs);
  }
  
  return exitJust(compact('message'), X_DENIED);
}

/**
 * Decode the device code using a public key from the appropriate server (just this one for now).
 * @param string $device: the b64encoded encrypted device code received from the device
 * @return: an adequate abbreviation of $device (FALSE if error)
 */
function decodeDevice($device) {
  if (!test() and !u\decryPGP(u\b64decode($device), 'public')) return FALSE;
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
    return $message;
  }

  if (strlen($pic) > $max) header("Content-type: image/jpeg");
  return exitJust($pic, X_GOT);
}
