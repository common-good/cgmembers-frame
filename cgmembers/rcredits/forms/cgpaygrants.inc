<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

/**
 * @file
 * Expected-grant CRUD endpoint for the SvelteKit cgpay POC.
 *
 * The POC's grants pages let a sponsored partner list and file an "expected
 * grant" — a heads-up that a donor is sending funds. PHP owns the storage
 * (`grants` table + grantor contact in `people` table) and the validation.
 *
 * Flow:
 *   GET  /cgpay-grants?uid=<uid>   → list that user's expected grants
 *   POST /cgpay-grants             → create a new expected grant
 *
 * Both require the same X-CG-Internal-Token shared secret used by /cgpay-sso
 * and /cgpay-lookup (CGPAY_SSO_SECRET, see defs.inc). One secret for the whole
 * server-to-server surface — no point splitting it.
 *
 * POST JSON body:
 *   {
 *     "uid":      <int>,            // sponsee account uid
 *     "amount":   <decimal>,        // required, > 0
 *     "by":       "ach"|"check"|"wire",  // required, payment method
 *     "fullName": <string>,         // required, grantor name
 *     "email":    <string?>, "phone": <string?>,
 *     "address":  <string?>, "city": <string?>, "state": <int?>, "zip": <string?>
 *   }
 *
 * Returns:
 *   GET  200 {"grants": [{id, amount, by, grantor, documented, received, created}, ...]}
 *   POST 200 {"id": <int>}
 *   401  bad/missing shared secret
 *   400  malformed input / failed validation (body: {"error": "<why>"})
 *   404  uid does not resolve to an account
 */
function cgpayGrants() {
  $method = nni($_SERVER, 'REQUEST_METHOD');
  if (!in_array($method, ['GET', 'POST'])) return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  return $method === 'GET' ? cgpayGrantsList() : cgpayGrantsCreate();
}

function cgpayGrantsList() {
  $uid = (int) nni($_GET, 'uid');
  if ($uid <= 0) return exitJust(X_SYNTAX);
  $a = r\acct($uid);
  if (!$a) return exitJust(X_NOTFOUND);
  if (!$a->sponsored) return exitJust(X_FORBIDDEN);

  $sql = "SELECT g.id, g.amount, g.by, g.documented, g.received, g.created, p.fullName AS grantor
          FROM grants g LEFT JOIN people p USING(pid)
          WHERE g.uid=:uid ORDER BY g.created DESC";
  $rows = db\q($sql, compact('uid'))->fetchAll(\PDO::FETCH_ASSOC);

  $grants = array_map(function($r) {
    return [
      'id'         => (int) $r['id'],
      'amount'     => (float) $r['amount'],
      'by'         => $r['by'],
      'grantor'    => $r['grantor'] ?: '',
      'documented' => $r['documented'] ? (int) $r['documented'] : null,
      'received'   => $r['received'] ? (int) $r['received'] : null,
      'created'    => (int) $r['created'],
    ];
  }, $rows);

  return exitJson(['grants' => $grants], X_OK);
}

function cgpayGrantsCreate() {
  $body = json_decode(file_get_contents('php://input'), TRUE);
  if (!is_array($body)) return exitJust(X_SYNTAX);

  $uid = (int) nni($body, 'uid');
  if ($uid <= 0) return exitJust(X_SYNTAX);
  $a = r\acct($uid);
  if (!$a) return exitJust(X_NOTFOUND);
  if (!$a->sponsored) return exitJust(X_FORBIDDEN);

  $fullName = trim((string) nni($body, 'fullName'));
  $amount   = nni($body, 'amount');
  $by       = strtolower(trim((string) nni($body, 'by')));

  if ($err = u\badName($fullName))      return exitJsonError($err);
  if ($err = u\badAmount($amount, '>0')) return exitJsonError($err);
  if (!in_array($by, ['ach', 'check', 'wire'], TRUE)) return exitJsonError('bad payment method');

  $email   = trim((string) nni($body, 'email'));
  $phone   = trim((string) nni($body, 'phone'));
  $address = trim((string) nni($body, 'address'));
  $city    = trim((string) nni($body, 'city'));
  $state   = (int) nni($body, 'state');
  $zip     = trim((string) nni($body, 'zip'));

  if ($email !== '' && !u\validEmail($email)) return exitJsonError('bad email');
  if ($phone !== '' && ($err = u\badPhone($phone))) return exitJsonError($err);
  if ($zip !== ''   && ($err = u\badZip($zip)))    return exitJsonError($err);

  $DBTX = \db_transaction();

  $pid = db\updateOrInsert('people', compact(ray('fullName email phone address city state zip')), 'pid');

  $id = db\insert('grants', [
    'uid'     => $uid,
    'pid'     => $pid,
    'amount'  => $amount,
    'by'      => $by,
    'created' => now(),
  ], 'id');

  unset($DBTX);

  return exitJson(['id' => (int) $id], X_OK);
}

function exitJsonError($msg) {
  return exitJson(['error' => (string) $msg], X_SYNTAX);
}
