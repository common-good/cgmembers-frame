<?php
namespace CG\Web;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Txs as x;
use CG\Admin as a;
use CG\Risk as k;
use CG\Web as w;

/**
 * API for interaction with the SMT system (we hope it's general enough for interaction
 * with other systems.
 * @param string args: a JSON object
 *
 * All requests have the same basic format:
 * { "op": whatever the operation is,
 *   "userId": the userId of the requester,
 *   "password": guess what
 *   "requests": a JSON array of objects; each request has a format based on "op"
 * }
 *
 * All responses have the same basic format:
 * { "op": the incoming request op with "-response" tacked on to the end,
 *   "userId": the userId of the requester (for verification),
 *   "status": 'OK' or 'BAD',
 *   "errors": only present if status=="BAD", an array of strings, explanatory messages
 *   "responses": only present if status=="OK", an array of responses, one for each element
 *                in the original requests array; each response has a format based on "op"
 */
function api($args = null) {
  global $mya;
  global $channel; $channel = TX_API;

  echo 'here1 ';
  $data = (array)json_decode($args);
  print_r($data);
  extract(just('op userId password requests', $data, NULL));
  echo '<br />';
  print_r($op);
  echo '<br />';
  print_r($userId);
  echo '<br />';
  print_r($password);
  echo '<br />';
  print_r($requests);
  echo '<br />';
  echo 'here2 ';
  $mya = r\acct($userId);
  if (! $mya) {
    return errorResponse($op, $userId, ["User id $userId not found"]); // combine these two!!
  }
  echo 'here3 ';
  if (! $mya->passwordOkay($password, 'pass')) {
    return errorResponse($op, $userId, [ "Incorrect password for user $userId" ]);
  }
  echo 'here4 ';
  if (is_null($requests) or ! is_array($requests) or $requests == []) {
    return errorResponse($op, $userId, [ "Improperly formed request" ]);
  }

  echo 'here5 ';
  switch($op) {
  case 'validate-user-ids':  return validateUserIds($userId, $requests);
  case 'sign-up-user':       return signUpUsers($userId, $requests);
  case 'generate-invoices':  return generateInvoices($userId, $requests);
  case 'make-payments':      return makePayments($userId, $requests);
  case 'get-invoice-status': return getInvoiceStatus($userId, $requests);
  default:
    echo 'here6 ';
    return errorResponse($op, $userId, ['Invalid API operation']);
  }
}

function errorResponse($op, $userId, $errors, $status = 'BAD') {
  $op .= '-response';
  $response = compact('op userId errors status');
  header('Access-Control-Allow-Origin: *'); // allow access from web browsers
  return (json_encode($response));
}

function okResponse($op, $userId, $responses, $status = 'OK') {
  $op .= '-response';
  $response = compact(ray('op userId status responses'));
  header('Access-Control-Allow-Origin: *'); // allow access from web browsers
  echo '<br />okResponse returning: ';
  print_r($response);
  echo '<br />thats all';
  return (json_encode($response));
}

function validateUserIds($userId, $requests) {
  echo 'here7 ';
  print_r($requests);
  $responses = [];
  foreach ($requests as $request) {
    $responses[] = validateUserId($userId, $request);
  }
  echo 'here10 ';
  print_r($responses);
  return okResponse('validate-user-ids', $userId, $responses);
}

function validateUserId($userId, $request) {
  echo 'here8 ';
  global $mya;
  extract(just('cgId fullName email phone address city state zipCode', $request, null));
  echo "<br />cgId=$cgId<br />";
  $userA = r\acct($cgId);
  if (! $userA) { return [ 'status' => 'BAD', 'errors' => [ 'Common Good Account not found' ], 'cgId' => $cgId]; }
  if (! u\similar($userA->fullName, $fullName, 5, .5) or !($userA->phone == u\fmtPhone(@$phone, '+n') or $userA->email == $email)) {
    return [ 'status' => 'BAD', 'errors' => [ t('That does not appear to be your correct %PROJECT member ID.') ], 'cgId' => $cgId];
  }
  echo 'here9 ';
  return [ 'status' => 'OK', 'cgId' => $cgId ];
}

function signUpUsers($userId, $requests) {
  $responses = [];
  foreach ($requests as $request) {
    $responses[] = signUpUser($userId, $request);
  }
  echo 'here21 <br />';
  return okResponse('signup', $userId, $responses);
}

function signUpUser($userId, $request) {
  extract(just('fullName email phone address city state zip_code userPassword nonce', $request, null));
  $zip = $zip_code;
  $newa = new r\Acct(compact(ray('fullName email phone address city state zip')));
  $newa->update(['pass' => $userPassword]);
  return ['nonce' => $nonce, 'status' => 'OK', 'cgid' => $mya->mainQid];
}

function generateInvoices($userId, $requests) {
  $responses = [];
  foreach($requests as $request) {
    $responses[] = generateInvoice($request);
  }
  return okResponse('generate-invoices', $userId, $responses);
}

function generateInvoice($request) {
  global $myA;
  extract(just('amount payerId billingDate dueDate smtInvoiceId purpose status'), $request, null);
  $payerA = r\acct($payerId);
  if (! $payerA) { return ray('smtInvoiceId status payerId amount errors', $smtInvoiceId, $status, $payerId, $amount, ['Payer not found']); }
  $existing = db\get(ray('nvid, status', 'r_invoices', ray('amount payer payee purpose created', $amount, $payerA->uid, $myA->uid, $purpose, $billingDate)));
  if ($existing) {
    extract(just('nvid status', $existing));
    $status = statusString($status) . '-DUPLICATE';
    if ($status == 'BAD-DUPLICATE') {
      return ray('smtInvoiceId status payerId amount errors', $smtInvoiceId, 'BAD', $payerId, $amount, ['CG system internal error']);
    }
    return ray('smtInvoiceId status payerId amount cgInvoiceId', $smtInvoiceId, $status, $payerId, $amount, $nvid);
  }
  // Not a duplicate
  list($success, $msg, $subs) = be\invoice($myA, $payerA, $amount, $purpose, FOR_GOODS, ['created' => $billingDate]);
  if ($success) {
    $status = statusString($subs['status']);
    if ($status == 'BAD') {
      return ray('smtInvoiceId status payerId amount errors', $smtInvoiceId, 'BAD', $payerId, $amount, ['CG system internal error']);
    }
    $cgInvoiceId = $subs['nvid'];
    return compact(ray('smtInvoiceId status payerId amount cgInvoiceId'));
  } else {
    $status = 'BAD';
    $errors = [ $msg ];
    return compact(ray('smtInvoiceId status errors payerId amount'));
  }
}

function makePayments($userId, $requests) {
  $responses = [];
  foreach($requests as $request) {
    $responses[] = makePayment($request);
  }
  return okResponse('make-payments', $userId, $responses);
}

function makePayment($request) {
  global $myA;
  extract(just('amount payeeId billingDate purpose smtInvoiceId'), $request, null);
  $payeeA = r\acct($payeeId);
  if (! $payeeA) { return ray('smtInvoiceId status errors', $smtInvoiceId, 'BAD', ['payee account not found']); }
  $existing = db\get('xid', 'txs_prime', 'amt=:amt and uid1=:uid1 and uid2=:uid2 and created=:created and for1=:for1',
                     ray('amt uid1 uid2 created for1', $amount, $myA->uid, $payeeA->uid, $billingDate, $purpose));
  if ($existing) {
    $status = 'DUPLICATE';
    return compact('smtInvoiceId status');
  }
  list($ok, $msg, $subs) = be\transfer('payment', $myA, $payeeA, $amount, $purpose, FOR_GOODS, ['created'=>$billingDate]);
  if ($ok) {
    return ray('smtInvoiceId status', $smtInvoiceId, 'OK');
  }
  $errors = [ t($msg, $subs) ];
  switch ($msg) {
  case 'short from vague':
  case 'short from':
  case 'short to':
    $status = 'NSF';
    break;
  default:
    $status = 'BAD';
  }
  return compact('smtInvoiceId status errors');
}

function getInvoiceStatus($userId, $requests) {
  $responses = [];
  foreach($requests as $request) {
    $responses[] = getOneInvoiceStatus($request);
  }
  return okResponse('get-invoice-status', $userId, $responses);
}

function getOneInvoiceStatus($request) {
  global $myA;
  extract(just('cgInvoiceId amount payerId', $request));
  $payerA = r\acct($payerId);
  if (! $payerA) { return ray('cgInvoiceId amount payerId status errors', $cgInvoiceId, $amount, $payerId, 'BAD', ['payer account missing']); }
  $status = db\get('status', 'r_invoices', ['nvid'=>$cgInvoiceId, 'amount'=>$amount, 'payer'=>$payerA->uid, 'payee'=>$myA->uid]);
  if (! $status) { return ray('cgInvoiceId amount payerId status', $cgInvoiceId, $amount, $payerId, statusString($status)); }
  return ray('cgInvoiceId amount payerId status errors', $cgInvoiceId, $amount, $payerId, 'BAD', ['no invoice found']);
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
