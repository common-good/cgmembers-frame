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
  
  $data = (array)json_decode($args);
  extract(just('op userId password requests', $data, NULL));
  $mya = r\acct($userId);
  if (! $mya) {
    errorResponse($op, $userId, ["User id $userId not found"]);
  }
  if (! $mya->passwordOkay($password, 'pass')) {
    errorResponse($op, $userId, [ "Incorrect password for user $userId" ]);
  }
  if (is_null($requests) or ! is_array($requests) or $requests = []) {
    errorResponse($op, $userId, [ "Improperly formed request" ]);
  }
  
  switch($op) {
  case 'validate-code':
    return validateCodes($userId, $requests);
  case 'signup':
    return signups($userId, $requests);
  case 'generate-invoices':
    return generateInvoices($userId, $requests);
  case 'pay':
    /* return pay($userId, $requests); */
  case 'invoice-status':
    /* return invoiceStatus($userId, $requests); */
  default:
    return '';
  }
}

function errorResponse($op, $userId, $errors, $status = 'BAD') {
  $response = ['op' => $op . "-response", 'userId' => $userId, 'status' => $status,
               'errors' => $errors];
  header('Access-Control-Allow-Origin: *'); // allow access from web browsers
  exit(json_encode($response));
}

function okResponse($op, $userId, $responses, $status = 'OK') {
  $response = ['op' => $op . "-response", 'userId' => $userId, 'status' => $status,
               'responses' => $responses];
  header('Access-Control-Allow-Origin: *'); // allow access from web browsers
  exit(json_encode($response));
}

function validateCodes($userId, $requests) {
  $responses = [];
  foreach ($requests as $request) {
    extract(just('cgId fullName email phone address city state zipCode', $request, null));
    $userA = r\acct($cgId);
    if (! $userA) {
      $responses[] = [ 'status' => 'BAD', 'errors' => [ 'Common Good Account not found' ], 'cgId' => $cgId];
      continue;
    }
    if (! u\similar($userA->fullName, $fullName, 5, .5) or !($userA->phone == u\fmtPhone(@$phone, '+n') or $userA->email == $email)) {
      $responses[] = [ 'status' => 'BAD', 'errors' => [ t('That does not appear to be your correct %PROJECT member ID.') ], 'cgId' => $cgId];
      continue;
    }
    $responses[] = [ 'status' => 'OK', 'cgId' => $cgId ];
  }
  okResponse('validate-codes', $userId, $responses);
}

function signups($userId, $requests) {
  $responses = [];
  foreach ($requests as $request) {
    extract(just('firstName lastName email phone address1 address2 city state zip_code userPassword nonce', $request, null));
    $fullName = "$firstName $lastName";
    $zip = $zip_code;
    $address = "$address1, $address2";
    $newa = new r\Acct(compact(ray('fullName email phone address city state zip')));
    $newa->update(['pass' => $userPassword]);
    $responses[] = ['nonce' => $nonce, 'status' => 'OK', 'cgid' => $mya->mainQid];
  }
  okResponse('signup', $userId, $responses);
}

function generateInvoices($userId, $data) {
  $responses = [];
  foreach($requests as $request) {
    generateInvoice($request);
  }
  okResponse('generate-invoices', $userId, $responses);
}

function generateInvoice($request) {
  extract(just('amount payerId billingDate dueDate smtInvoiceId purpose'), $request, null);
  list($success, $msg, $subs) = be\invoice($myA, r\acct($payerId), $amount, $purpose);
  if ($success) {
    
    return compact(ray('smtInvoiceId status errors payer amount cgInvoiceId'));
}

  /* global $mya; */
  
  /* $responses = []; */
  /* $requests = $data['requests']; */
  /* foreach ($requests as $request) { */
  /*   extract(just('amount payer billingDate dueDate smtInvoiceId purpose', $request, NULL)); */
  /*   $payerA = r\acct($payer); */
  /*   list($ok, $msg, $args, ?, $nvid) = be\invoice($mya, $payer, $amount, $purpose, FOR_NONGOODS); */
  /*   if ($ok) { */
}
