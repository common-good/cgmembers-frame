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
 * API for interaction with CGPay progressive web app or with the SMT system (we hope it's general enough for interaction
 * with other systems.
 *
 * For CGPay PWA (REST interface):
 * @param string $arg: the operation (accounts or transactions)
 *
 * For SMT system:
 * @param string $arg: NULL
 * @param $_GET['request']: a JSON object
 *
 * All requests have the same basic format:
 * { "op": whatever the operation is
 *   "userId": the userId of the requester (a company)
 *   "password": password
 *   "requests": a JSON array of objects; each request has a format based on "op"
 * }
 *
 * All responses have the same basic format:
 * { "op": the incoming request op with "-response" tacked on to the end,
 *   "userId": the userId of the requester (for verification),
 *   "status": 'OK' or 'BAD',
 *   "error": only present if status=="BAD", an explanatory message
 *   "responses": only present if status=="OK", an array of responses, one for each element
 *                in the original requests array; each response has a format based on "op"
 */
function api($arg = null) {
  global $mya;
  global $channel; $channel = TX_API;

  if ($arg) return call_user_func('\\CG\\Web\\' . $arg);

  $data = u\dejsonize(nni($arg ?: $_GET, 'request'));
  extract(just('op userId password requests', $data, NULL));
  $mya = r\acct($userId);
  if (!$mya or !$mya->co or $password != $mya->emailCode) {
    return errorResponse($op, $userId, "Company id $userId not found or wrong password");
  }
  if (is_null($requests) or ! is_array($requests) or $requests == []) {
    return errorResponse($op, $userId, "Improperly formed request");
  }

  switch($op) {
  case 'validate-user-ids':  return validateUserIds($userId, $requests);
  case 'sign-up-users':      return signUpUsers($userId, $requests);
  case 'generate-invoices':  return generateInvoices($userId, $requests);
  case 'make-payments':      return makePayments($userId, $requests);
  case 'get-invoice-status': return getInvoiceStatus($userId, $requests);
  default:
    return errorResponse($op, $userId, 'Invalid API operation');
  }
}

function errorResponse($op, $userId, $error, $status = 'BAD') {
  $op .= '-response';
  $response = compact(ray('op userId error status'));
  header('Access-Control-Allow-Origin: *'); // allow access from web browsers
  return exitJust(u\jsonize($response));
}

function okResponse($op, $userId, $responses, $status = 'OK') {
  $op .= '-response';
  $response = compact(ray('op userId status responses'));
  header('Access-Control-Allow-Origin: *'); // allow access from web browsers
  return exitJust(u\jsonize($response));
}

function validateUserIds($userId, $requests) {
  $responses = [];
  foreach ($requests as $request) {
    $responses[] = validateUserId($userId, $request);
  }
  return okResponse('validate-user-ids', $userId, $responses);
}

function validateUserId($userId, $request) {
  global $mya;
  extract(just('cgId fullName email phone address city state zipCode', $request, null));
  $userA = r\acct($cgId);
  if (! $userA) { return [ 'status' => 'BAD', 'error' => 'Common Good Account not found', 'cgId' => $cgId]; }
  if (! u\similar($userA->fullName, $fullName, 5, .5) or !($userA->phone == u\fmtPhone(nn($phone), '+n') or $userA->email == $email)) {
    return [ 'status' => 'BAD', 'error' => t('That does not appear to be your correct %PROJECT member ID.'), 'cgId' => $cgId];
  }
  return [ 'status' => 'OK', 'cgId' => $cgId ];
}

function signUpUsers($userId, $requests) {
  $responses = [];
  foreach ($requests as $request) {
    $responses[] = signUpUser($userId, $request);
  }
  return okResponse('sign-up-users', $userId, $responses);
}

function signUpUser($userId, $request) {
  global $mya;
  extract(just('fullName email phone address city state zip_code userPassword nonce', $request, null));
  $zip = $zip_code;
  $newa = new r\Acct(compact(ray('fullName email phone address city state zip')));
  $newa->update(['pass' => $userPassword]);
  return ['nonce' => $nonce, 'status' => 'OK', 'cgId' => $newa->mainQid];
}

function generateInvoices($userId, $requests) {
  $responses = [];
  foreach($requests as $request) $responses[] = generateInvoice($request);
  return okResponse('generate-invoices', $userId, $responses);
}

function generateInvoice($request) {
  global $mya;

  extract(just('amount payerId billingDate dueDate nonce purpose status', $request, null));
  $payerA = r\acct($payerId);
  if (! $payerA) { return ray('nonce status payerId amount error', $nonce, 'BAD', $payerId, $amount, 'Payer not found'); }
  $existing = db\get('nvid, status', 'tx_requests', ray('amount payer payee purpose created', $amount, $payerA->uid, $mya->uid, $purpose, $billingDate));
  if ($existing) {
    extract(just('nvid status', $existing));
    $status = statusString($status) . '-DUPLICATE';
    if ($status == 'BAD-DUPLICATE') {
      return ray('nonce status payerId amount error', $nonce, 'BAD', $payerId, $amount, 'CG system internal error');
    }
    return ray('nonce status payerId amount cgInvoiceId', $nonce, $status, $payerId, $amount, $nvid);
  }
  // Not a duplicate
  $z = be\invoice($mya, $payerA, $amount, $purpose, FOR_GOODS, ['created' => $billingDate]);
  if ($z->ok) {
    $status = statusString($z->subs['status']);
    if ($status == 'BAD') {
      return ray('nonce status payerId amount error', $nonce, 'BAD', $payerId, $amount, 'CG system internal error');
    }
    $cgInvoiceId = $z->subs['nvid'];
    return compact(ray('nonce status payerId amount cgInvoiceId'));
  } else {
    $status = 'BAD';
    $error = [ $z->index ];
    return compact(ray('nonce status error payerId amount'));
  }
}

function makePayments($userId, $requests) {
  foreach($requests as $request) $responses[] = makePayment($request);
  return okResponse('make-payments', $userId, nn($responses));
}

function makePayment($request) {
  global $mya;
  extract(just('amount payeeId billingDate purpose nonce', $request, null));
  $payeeA = r\acct($payeeId);
  $status = 'BAD';
  $error = 'payee account not found';
  if (! $payeeA) { return compact(ray('nonce status payeeId amount error')); }
  $existing = db\get('xid', 'txs_prime', 'amt=:amt and uid1=:uid1 and uid2=:uid2 and created=:created and for1=:for1',
                     ray('amt uid1 uid2 created for1', $amount, $mya->uid, $payeeA->uid, $billingDate, $purpose));
  if ($existing) {
    $status = 'DUPLICATE';
    return compact(ray('nonce status payeeId amount'));
  }
  $z = be\transfer('payment', $mya, $payeeA, $amount, $purpose, FOR_GOODS, ['created'=>$billingDate]);
  if ($z->ok) return ray('nonce status payeeId amount', $nonce, 'OK', $payeeId, $amount);

  switch ($z->index) {
  case 'short from vague':
  case 'short from':
  case 'short to':
    $status = 'NSF';
    break;
  default:
    $status = 'BAD';
  }
  
  $error = $z->msg;
  return compact(ray('nonce status payeeId amount error'));
}

function getInvoiceStatus($userId, $requests) {
  $responses = [];
  foreach($requests as $request) {
    $responses[] = getOneInvoiceStatus($request);
  }
  return okResponse('get-invoice-status', $userId, $responses);
}

function getOneInvoiceStatus($request) {
  global $mya;
  extract(just('cgInvoiceId amount payerId', $request));
  $payerA = r\acct($payerId);
  if (! $payerA) { return ray('cgInvoiceId amount payerId status error', $cgInvoiceId, $amount, $payerId, 'BAD', 'payer account missing'); }
  $result = db\get('status, payee', 'tx_requests', ['nvid'=>$cgInvoiceId, 'amount'=>$amount, 'payer'=>$payerA->uid, 'payee'=>$mya->uid]);
  if ($result == FALSE) {
    return ray('cgInvoiceId amount payerId status error', $cgInvoiceId, $amount, $payerId, 'BAD', 'no invoice found');
  }
  return ray('cgInvoiceId amount payerId status', $cgInvoiceId, $amount, $payerId, statusString($result['status']));
}

function statusString($status) {
  if ($status >= TX_PAID) return 'PAID';
  switch ($status) {
  case TX_DELAYED: return('DELAYED');
  case TX_DENIED: return('DENIED');
  case TX_PENDING: return('PENDING');
  case TX_APPROVED: return('APPROVED');
  default: return('BAD');
  }
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
