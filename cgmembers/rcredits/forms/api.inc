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
use CG\Test as t;

include_once R_ROOT . '/cg-qr.inc';

/**
 * API for interaction with CGPay progressive web app (REST interface) and passthrough for SMP api.
 * @param string $arg: the operation
 */
function api($arg = null) {
  global $mya;
  global $channel;

  $params = $_POST ?: $_GET; // don't use $_REQUEST unless we fix the tests to handle it

  if (is_array($arg) or nni($params, 'request')) { // SMP interface uses /api with no explicit endpoints
    $channel = TX_API;
    u\loga('in', $arg ?: ($params));
    include_once R_ROOT . '/forms/smp.inc';
    return smp($arg);
  }

  /**
   * Endpoints (always POST): accounts, identity, idPhoto, latest, info, transactions, comments, lostPassword, test
   */
  $channel = TX_APP;
  unset($params['q']); // Drupal adds this
  u\loga('in:' . $arg, $params);
  setHeaders();
  if ($arg == 'test') $arg = 'testops'; // avoid collision with our regular testing framework
  if (!function_exists($func = '\\CG\\Web\\' . $arg)) return exitJust(X_SYNTAX);

  if ($these = nni($_POST, 'these')) { // process an array with no response needed other than "ok"
    foreach (json_decode($these) as $one) {
      fixVersion($one);
      if (!$ok = authOk($arg, $one)) return; // return is for testing -- any error triggers exitJust()
      $func($one);
    }
    return exitJson('', X_OK);
  }


  fixVersion($params);
  if (!$ok = authOk($arg, $params)) return; // return is for testing -- any error triggers exitJust()
  return $func($params); // functions ignore $params unless they optionally accept a POSTed array
}

/**
 * POST /accounts
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
function accounts($params) {
  global $nextCode; // tests usurp generated device code, using this instead

  foreach (ray('identifier password') as $k) if (!$$k = nni($params, $k)) return exitJust(X_UNAUTH);
  if (strlen($identifier) == 1) $identifier = 'newzp' . $identifier;
  if (!$myid = r\loginString($identifier)) return exitJust(X_NOTFOUND);
  
  if (w\flooding($myid, $sta, $err)) return exitJson($err, X_TOOMANY);
  $a = r\acct($myid);
  $success = ($a and $a->passwordOkay($password, 'pass', $err)); // ignore admin pass on dev machine
  if (!$success) return exitJust($err, X_NOTFOUND);
  w\updateFlood($sta, $success, $err, TRUE);
  if (!$a->ok) return exitJust(X_FORBIDDEN);
  
  if (!$list = be\accountChoices(r\acct($myid), B_MANAGE, 'u.:IS_OK', TRUE)) return exitJust(X_NOTFOUND);

  foreach ($list as $uid => $name) {
    $a = r\acct($uid, $myid);
    $isCo = $a->co;
    if ($a->stepsLeft($isCo ? 'coapp' : 'app')) continue; // no using app without card permission
    $accountId = qr\url($a, 'main'); // r\acct($uid)
    $deviceId = (isDEV or isDEMO) ? (nn($nextCode) ?: 'dev' . $a->fullName[0]) : base64_encode(u\cryPGP(now() . "/$name", 'private')); // prove to customer who they're dealing with
//    $qr = ($uid == $myid and !$a->stepsLeft('card')) ? myQr($a) : NULL;
    $qr = myQr($a);
    $selling = $isCo ? explode("\n", $a->selling) : NULL;
    $accounts[] = compact(ray('accountId deviceId qr isCo name selling'));
  }
  if (!nn($accounts)) return exitJust(X_NOTFOUND);
  
  return exitJson(compact(ray('accounts')), X_OK);
}

function myQr($a) {
  include_once R_ROOT . '/cg-qr.inc';
  $logo = $a->agentA->photoSrc(FALSE, TRUE, TRUE);
//  $text = $a->bestName . ", $a->city, $a->st"; // maybe put this at the top, under device-owner name
  $testQr = qr\url($a, 'card');
  return qr\generate($testQr, $logo, t('Account: %qid (%city, %st)', 'qid city st', $a->mainQid, $a->city, $a->st));
}

/**
 * POST /identity
 * Return an account's name, agent name, and location.
 *
 * @param string $otherId: abbreviated version of the information from the QR Code for the account to charge, without <type> and <counter>
 *   abbreviated version is <fmt><region><acct><agent><cardCode><type><counter>
 * @return a JSON hash:
 *   name: the fullName of the account
 *   agent: the name of the person acting on behalf of a company (empty for an individual account)
 *   location: the account’s city, state
 *   limit: the maximum amount this account can be charged at this time
 *   selling: the device owner's list of transaction descriptions
 */
function identity($params) {
  global $mya;
  include_once R_ROOT . '/cg-qr.inc';
//  if (!rayhas($_POST, 'otherId')) return idPhoto(); // use /identity as a single endpoint for identity and idPhoto
  foreach (ray('otherId') as $k) if (!$$k = nni($params, $k)) return exitJust(X_SYNTAX);
  if (!$a = getAcct($otherId, FALSE) or !$a->ok) return exitJust(X_NOTFOUND);

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
  $selling = explode("\n", $mya->selling);

  return exitJson(compact(ray('name agent location limit creditLine avgBalance trustRatio since selling')));
}

/**
 * POST /idPhoto (syntax for ID photo image source)
 * Return an account's id photo.
 *
 * @param string $otherId: abbreviated info from QR code for the other account ID
 * @return the image (mime type image/jpeg) or an error message (plain text)
 */
function idPhoto($params) {
  global $mya;
  include_once R_ROOT . '/cg-qr.inc';
  
  foreach (ray('otherId') as $k) if (!$$k = nni($params, $k)) return exitJust(X_SYNTAX);
  if (!$a = getAcct($otherId)) return exitJust(X_NOTFOUND);

  return photoReturn($a->agentA->photo);
}

/**
 * POST /latest (app relB depends on this, so keep it around until release C takes effect for all users)
 * Return very short information about latest information of interest to the app.
 * @param int $lastTx: Unixtime of last tx initiated by the device
 * @return {version, txCount, msg}
 */
function latest($params) {
  $version = APP_VERSION;
  $txCount = 0;
  $msg = NULL; // a way to communicate with app user(s)
  return exitJson(compact(ray('version txCount msg')));
}

/**
 * POST /confirms
 * Confirm or deny an invoice
 */
function confirms($params) {
  global $mya;
  require_once R_ROOT . '/forms/handleinvoice.inc';
  extract(just('id yesno whyNot', $params, NULL));

  if (!$id or !isset($yesno)) return exitJust(X_SYNTAX);
  if (!$inv = db\get('*', 'tx_requests', 'nvid=:id', compact('id'))) return exitJust(X_NOTFOUND);
  if ($inv['payer'] != $mya->id) return exitJust(X_UNAUTH);
  
  if (!$ok = handleInvoice($yesno ? 'pay' : 'deny', $inv['amount'], NULL, $whyNot, $inv)) return exitJust(X_DENIED);
  exitJson('', X_OK);
}

/**
 * Create the requested transaction.
 * @param int $version: app version number
 * @param string $deviceId: the device identifier
 * @param string $actorId: abbreviated identifier for the account associated with the device (the actor)
 * @param string $otherId: abbreviated identifier for the other account
 * @param numeric $amount: amount of transaction, from the actor's point of view (negative to reverse a charge)
 * @param string $description: a brief description of the transaction
 *               last character="!" means don't complain about duplicate transactions (crucial for testing)
 * @param int $created: Unixtime the transaction was processed on the device
 * @param string $proof: SHA256 hash of actorId + amount (with exactly two digits after the decimal point) + otherId + security code + created.
 * @param bool $offline: <transaction was processed offline and stored for later upload>
 * @param numeric $tip: amount of tip or empty
 * @return {$xid, $message}
 *
 * NOTE: use only X_OK and X_SYNTAX here.
 */
function transactions($params) {
  global $allowDupTxs;
  global $scanned; $scanned = true;
  
  global $mya, $boxId, $allowDupTxs;
  include_once R_ROOT . '/classes/tx.class';

  $flds = 'version deviceId actorId otherId amount description created offline';
  foreach (ray($flds) as $k) if (!$$k = nni($params, $k)) return exitJust(t('missing parameter "%k"', compact('k')), X_SYNTAX);
  extract(just('proof tip', $params, NULL));
  if (substr($description, -1, 1) == '!') $allowDupTxs = TRUE; // avoid preventing reruns of tests
  $params['offline'] = $offline = ($offline == 'true'); // make offline bool, modify params for error
  if (test()) $proof = $proof ? u\hash($gotProof = $proof) : $proof;

  if (!$a = getAcct($otherId, FALSE, $err) or !$a->ok) {
    return $offline ? badTx($a, $params, obj('msg', $err)) : postErr($err); // proof is proof enough (no cardCode needed)
  }
  $cardCode = $a->cardCode();
  if ($err = u\badAmount($amount, '!=0', 2, MAX_AMOUNT)) return exitJust(t('bad amount'), X_SYNTAX); // allow negatives
  if (!is_numeric($created) or $created < now() - 30 * DAY_SECS or $created > now() + 2 * DAY_SECS - 1) return exitJust(t('bad date'), X_SYNTAX);

  foreach (['', 'Undo'] as $uq) { // for the tx and its Undo (proof of Undo tx matches the original transaction's proof)
    $proofAmt = number_format($uq ? -$amount : $amount, 2, '.', '');
    $proofWanted = u\hash(${"testProofWanted$uq"} = $actorId . $proofAmt . $otherId . $cardCode . $created); // cardCode is not a parameter, but it is hashed into the proof
    ${"proofOk$uq"} = ($proof == $proofWanted);
///    debug(compact(ray('proofAmt proofWanted gotProof proof ' . "testProofWanted$uq proofOk$uq")));
  }

  if (!$proofOk and !$proofOkUndo) { // bad proof
    r\tellAdmin('bad proof', compact(ray('version testProofWanted testProofWantedUndo actorId amount otherId cardCode created offline')));
    if (test()) f('t.output', "Wanted proof: $testProofWanted or $testProofWantedUndo, got: $gotProof");
    return exitJust(t('bad proof'), X_SYNTAX);
  }
  
  $goods = FOR_GOODS; $selfhelp = FALSE;
  $info = ray('created uid1 uid2 amt goods for2', $created, $a->id, $mya->id, $amount, $goods, $description);
  
  if ($proofOkUndo) { // reversing (no effort is made to Undo in real time, so all reversals are offline-agnostic)
    if ($amount > 0) {
      $info = ray('uid1 uid2', $mya->id, $a->id) + $info; // correct the point of view if undoing a payment
    } else $info['amt'] = -$amount; // look for original amount (the correction of point of view above also reverses the sign)
    if ($xid = db\get('xid', 'txs', $info)) { // see if original transaction exists
      $z = be\posUndo($mya, $xid, TRUE);
      debug($z);
      if (!$z->ok) return badTx($a, $params, $z);
    } // else nothing to do (pretend we deleted it)
    return exitJson(ray('ok message', TRUE, $amount > 0 ? t('reversal invoiced') : t('deleted')), X_OK); // message is ignored by app
  }  

  if ($amount < 0) list ($type, $amount) = ['payment', -$amount]; else $type = 'charge';

  // new transaction
  if ($xid = db\get('xid', 'txs', $info)) { // transaction exists
    $z = be\sendNotifications($mya, $type, r\Tx::read($xid), 0, FALSE); // fake the message from existing transaction
    $message = $z->msg;
  } else { // transaction doesn't exist (yet)
    $DBTX = \db_transaction();
    $z = be\transfer($type, $mya, $a, $amount, $description, $goods, compact(ray('created offline')));
    $message = $selfhelp ? 'selfhelp tx' : $z->msg;
    if ($z->ok) {
      if (!$offline and $aa = $a->agentA and $mya->co and !$aa->ided) $aa->setBit(B_IDED);
    } else { // failed to create transaction
      if ($offline) return badTx($a, $params, $z);
    }

    if ($tip > 0) {
      include_once R_ROOT . '/classes/tx.class';
      $pct = number_format(100 * $tip / $amount, 1);
      $tx = r\Tx::read($z->subs['xid']);
      list ($a1, $a2) = u\order($type == 'payment', $mya, $a);
      be\txEntryPair($entries, $tx, $tip, [$a1, $a2], t('tip (%pct%)', compact('pct')), E_AUX);
      be\insertTxes($entries, $tx->xid); // no balance checking (make this always go through!!)
    }  
    unset($DBTX);
  }
  
  return exitJson(ray('ok message', $z->ok, $message), X_OK);
}

/**
 * POST /info (CQP, count)
 *   -> {balance, surtxs, txs: [{xid, amount, accountId, name, description, created}, …]}
 *
 * Retrieve recent transactions and related data from the server.
 *
 */
function info($params) {
  global $mya;
  
  foreach (ray('count') as $k) if (!$$k = nni($params, $k)) return exitJust(X_SYNTAX);
  if (!is_numeric($count) or $count < 1) return exitJust(X_SYNTAX);
  
  r\launchWebsocket();

  foreach (ray('balance crumbs roundup') as $k) $$k = $mya->$k;
  $surtxs = (array) db\get('SUM(amount) AS amount, SUM(`portion`) AS `portion`', 'tx_rules', '`from`=:myid AND action=:ACT_SURTX AND payeeType=:REF_ANYCO', ray('myid', $mya->id));
  $surtxs += compact(ray('crumbs roundup'));

  $myUids = $mya->uids;
  $toMe = 'uid2 IN (:myUids)';
  $fromMe = 'uid1 IN (:myUids)';
  $flds = "xid, IF($toMe, amt, -amt) AS amount, IF($toMe, uid1, uid2) AS other, IF($toMe, agt1, agt2) AS agent, IF($toMe, for2, for1) AS description, created";
  $txs = db\rows($flds, 'txs', "($toMe OR $fromMe) ORDER BY created DESC LIMIT $count", compact(ray('myUids count'))); // somehow :count doesn't work in LIMIT

  foreach ($txs as $i => $tx) {
    extract((array) $tx);
    $a = r\acct($tx->other, $tx->agent);
    $accountId = qr\url($a, 'main');
    $name = $a->fullName;
//    $agent = $a->proSe ? '' : $a->agentA->fullName;
//    $location = $a->location();
//    $photo = $a->hasPhoto ? $a->photo : file_get_contents($a->co ? R_DFT_PICTURE_WIDE : R_DFT_PICTURE);
//    die(u\alterImg(imagecreatefromstring($photo), $a->co ? 48 : 27, 36));
//    $avatar = ''; // dies    $avatar = base64_encode(u\alterImg(imagecreatefromstring($photo), $a->co ? 48 : 27, 36)); // always 36 high (best if height is a multiple of 12)
//    $since = $a->created;
//    $txs[$i] = (array) $txs[$i];
    $txs[$i] = compact(ray('xid, amount, accountId, name, description, created'));
  }
  
  return exitJson(compact(ray('balance surtxs txs')), X_OK);
}  

/**
 * POST /comments
 *
 * Send a comment to an administrator.
 *
 * @param string $deviceId: the device identifier
 * @param string $accountId: the user's account ID, including cardCode
 * @param string $text: the comment
 * @param int $created: Unixtime the comment was submitted
 */
function comments($params) {
  global $mya;

  foreach (ray('text created') as $k) if (!$$k = nni($params, $k)) return exitJust(X_SYNTAX);
  $created = fmtDt($created);
  $qid = $mya->qid;
  $topic = t('%who (%qid %dt)', 'who qid dt', $mya->fullName, $qid, $created);
  list ($text, $topic) = u\starts($text, '[dev]') ? ["$topic<br>$text", '[DEV]'] : [$text, t('CGPay comment from ') . $topic];
  r\tellPromo($text, $topic, $mya->nameAndEmail());
  
  return exitJson('', X_OK);
}

/**
 * POST /test
 * 
 * Put or get test data.
 *
 * @param string $op: the test operation (see below)
 * plust other parameters depending on the operation:
 *   initialize (): creates default data (especially for Release A)
 *   clear (): removes all test data
 *   insert (table, [{keyedValues}]) where the first keyed value is the record key
 *   update (table, {keyedValues}) where the first keyed value is the record key
 *   rows (fieldList, table) -> [[values]]
 *     where fieldList is '*' --  eventually maybe a comma-separated list of fields or aggregates such as COUNT(*) or SUM(amt)
*/
function testops($params) {
  if (!!isPRODUCTION or !!isSTAGE) return exitJust(X_FORBIDDEN);

  if (!$op = nni($params, 'op')) return exitJust(X_SYNTAX);
   
  require_once(R_ROOT . '/cg-testing.inc');
  require_once(R_ROOT . '/admin/admin.inc');
  require_once(R_ROOT . '/classes/qo.class');

  $uidMin = r\qo('NEWZPA')->id; $uidMax = r\qo('NEWZPI')->id;
  $uidCrit = "BETWEEN $uidMin AND $uidMax";

  switch ($op) {
    case 'initialize': 
      //t\clear(TX_APP); 
      a\makeTestAccounts(); // only in release A, reset to default accounts instead of clearing all test data
      db\q('DELETE h,e FROM tx_hdrs_all h INNER JOIN tx_entries_all e USING (xid) WHERE uid ' . $uidCrit); // remove any txs that have been deleted by makeTestAccounts()
      break;
      
    case 'rows':
      foreach (ray('fieldList table') as $k) if (!$$k = nni($params, $k)) return exitJust(X_SYNTAX);
      if (!in($table, 'txs')) return exitJust(X_SYNTAX);
      try {
        $rows = db\rows('*', 'txs', "uid1 $uidCrit AND uid2 $uidCrit ORDER BY xid");
        $rows = json_decode(str_replace('actorId', 'actorUid', json_encode($rows))); // we really ought to rename the field
        return exitJson($rows, X_OK);
      } catch (\Exception $er) {
        return exitJust(X_SYNTAX);
      }
      
    case 'update':
      foreach (ray('table keyedValues') as $k) if (!$$k = nni($params, $k)) return exitJust(X_SYNTAX);
      if (!in($table, 'users')) return exitJust(X_SYNTAX);
      if (!$ray = json_decode($keyedValues)) return exitJust(X_SYNTAX);
      foreach ($ray as $i => $row) db\update($table, (array) $row, 'uid');
      break;
      
    default: return exitJust(X_SYNTAX);
  }
  return exitJson('', X_OK);
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
function getAcct($cardId, $requireCode = TRUE, &$err = '') {
  include_once R_ROOT . '/cg-qr.inc';
  
  if (!$a = qr\acct($cardId, $requireCode) or !$a->ok) {
    r\badCard($cardId, NULL);
    $err = t('That is not an active %PROJECT account.');
    return FALSE;
  }
//  if (r\foreignServer(r\uidRegion($a->agentId))) return !$err = 'wrong server for customer';
  return $a;
}

/**
 * Tell the device owner about a failed background transaction
 */
function badTx($a, $info, $z) {
  global $mya;
  extract(just('offline amount created', $info, NULL));
  
  $badFlds = 'version deviceId actorId otherId amount description created proof offline problem';
  $info['offline'] = (int) $offline;  
  $info['problem'] = $z->msg;
  db\insert('tx_bads', just($badFlds, $info));
  
  $by = $mya->proSe ? '' : t(' (agent %agent)', 'agent', $mya->agentA->fullName);
  $dpy = ray('name amount created by', $a ? $a->fullName : t('UNKNOWN'), u\fmtAmt($amount), fmtDt($created), $by);
//    $mya->tell("bad forced tx|$message", $info);
  r\tellAdmin("bad forced tx|$z->msg", $info + $dpy);
  
  return postErr($z->msg); // offline transactions are always accepted
}

/**
 * Decode the device code using a public key from the appropriate server (just this one for now).
 * @param string $device: the b64encoded encrypted device code received from the device
 * @return: an adequate abbreviation of $device (FALSE if error)
 */
function decodeDevice($device) {
  if (test() and $device == 'whatever') return FALSE;
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
  return exitJust($pic, X_OK);
}

/**
 * Check the validity of a specified deviceId and actorId.
 * @param string $arg: the endpoint
 * @param assoc $params: the parameters
 * @param bool $noBox: if true, don't bother with a boxId (important for appSocket)
 * @return <ok>
 */
function authOk($arg, $params, $noBox = FALSE) {
  global $mya, $boxId;

  if ($deviceId = nni($params, 'deviceId')) { // identify user if deviceId specified
    if (!$actorId = nni($params, 'actorId')) return exitJust(X_SYNTAX);
    if (!$mya = getAcct($actorId, FALSE)) return exitJust(X_UNAUTH);
    if ($deviceId == 'dev' . $mya->fullName[0] and !isPRODUCTION) { // for example devA
      // accept it when testing
    } else {
      if (!$nm = decodeDevice($deviceId)) return exitJust(X_UNAUTH);
    }
    $device = substr($deviceId, 0, DEVICE_MAXLEN);
    if (!$noBox) $boxId = w\boxId($device, $mya, nni($params, 'version', -1)); // find device in the db or put it there
  } elseif (!in($arg, 'accounts lostPassword testops')) return exitJust(X_UNAUTH);

  return TRUE;
}

/**
 * Convert the version to a number.
 */
function fixVersion(&$params) {
  extract(just('version', $params, NULL));
  if (!$version) exitJust(X_SYNTAX);
  $vv = explode('.', $version);
  if (count($vv) == 3) $params['version'] = $vv[0] * 10000 + $vv[1] * 100 + $vv[2] * 1;
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
