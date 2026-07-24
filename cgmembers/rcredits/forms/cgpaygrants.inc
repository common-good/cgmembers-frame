<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

/**
 * @file
 * Expected-grant CRUD endpoint for the SvelteKit cgpay POC.
 *
 * The POC's grants pages let a sponsored partner list, file, and edit an
 * "expected grant" — a heads-up that a donor is sending funds. PHP owns the
 * storage (`grants` table + grantor contact in `people` table) and validation.
 *
 * Flow:
 *   GET   /cgpay-grants?uid=<uid>   → list that user's expected grants
 *   POST  /cgpay-grants             → create a new expected grant
 *   PATCH /cgpay-grants             → update an existing expected grant
 *
 * All require the same X-CG-Internal-Token shared secret used by /cgpay-sso
 * and /cgpay-lookup (CGPAY_SSO_SECRET, see defs.inc).
 *
 * POST JSON body (create):
 *   {
 *     "uid":      <int>,                       // sponsee account uid
 *     "amount":   <decimal>,                   // required, > 0
 *     "by":       "ach"|"check"|"wire",        // required, payment method
 *     "fund":     <string?>,                   // specific fund within a grant-making organization
 *     "grantorPid": <int?>,                    // if grantor was picked from autocomplete
 *     "fullName": <string>,                    // required, grantor name
 *     "email":    <string?>, "phone": <string?>,
 *     "address":  <string?>, "city": <string?>, "state": <int?>, "zip": <string?>
 *   }
 *
 * PATCH JSON body (update):
 *   { "uid": <int>, "id": <int>, ...same fields... }
 *   All grant + grantor fields optional; only the provided ones change.
 *
 * Grantor-resolution rules (per William):
 *   - No grantorPid provided or pid unknown → new people row
 *   - grantorPid matches, fullName unchanged, missing field being filled → silent update
 *   - grantorPid matches, fullName unchanged, existing field being changed → update people row,
 *     log old→new to changes table, alert admin
 *   - grantorPid matches, fullName changed → new people row (treat as different entity)
 *
 * Returns:
 *   GET   200 {"grants": [{id, amount, by, fund, grantor, documented, received, created}, ...]}
 *   POST  200 {"id": <int>}
 *   PATCH 200 {"id": <int>}
 *   401   bad/missing shared secret
 *   400   malformed input / failed validation (body: {"error": "<why>"})
 *   404   uid/id does not resolve
 */
function cgpayGrants() {
  $method = nni($_SERVER, 'REQUEST_METHOD');
  if (!in_array($method, ['GET', 'POST', 'PATCH'])) return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  if ($method === 'GET')   return cgpayGrantsList();
  if ($method === 'POST')  return cgpayGrantsCreate();
  return cgpayGrantsUpdate();
}

function cgpayGrantsList() {
  $uid = (int) nni($_GET, 'uid');
  if ($uid <= 0) return exitJust(X_SYNTAX);
  $a = r\acct($uid);
  if (!$a) return exitJust(X_NOTFOUND);
  if (!$a->sponsored) return exitJust(X_FORBIDDEN);

  $sql = "SELECT g.id, g.amount, g.by, g.fund, g.documented, g.received, g.created, g.pid, p.fullName AS grantor
          FROM grants g LEFT JOIN people p USING(pid)
          WHERE g.uid=:uid ORDER BY g.created DESC";
  $rows = db\q($sql, compact('uid'))->fetchAll(\PDO::FETCH_ASSOC);

  $grants = array_map(function($r) {
    return [
      'id'         => (int) $r['id'],
      'amount'     => (float) $r['amount'],
      'by'         => $r['by'],
      'fund'       => $r['fund'] ?: '',
      'grantorPid' => $r['pid'] ? (int) $r['pid'] : null,
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

  $grantFlds = cgpayGrantsParseFlds($body, TRUE);
  if (is_string($grantFlds)) return exitJsonError($grantFlds);

  $DBTX = \db_transaction();

  $pid = cgpayGrantsResolveGrantorPid($body, $uid);
  if (is_string($pid)) return exitJsonError($pid);

  $id = db\insert('grants', $grantFlds + [
    'uid'     => $uid,
    'pid'     => $pid,
    'created' => now(),
  ], 'id');

  unset($DBTX);

  return exitJson(['id' => (int) $id], X_OK);
}

function cgpayGrantsUpdate() {
  $body = json_decode(file_get_contents('php://input'), TRUE);
  if (!is_array($body)) return exitJust(X_SYNTAX);

  $uid = (int) nni($body, 'uid');
  $id  = (int) nni($body, 'id');
  if ($uid <= 0 || $id <= 0) return exitJust(X_SYNTAX);
  $a = r\acct($uid);
  if (!$a) return exitJust(X_NOTFOUND);
  if (!$a->sponsored) return exitJust(X_FORBIDDEN);

  $existing = db\row('grants', compact('id'));
  if (!$existing || (int) $existing->uid !== $uid) return exitJust(X_NOTFOUND);

  $grantFlds = cgpayGrantsParseFlds($body, FALSE);
  if (is_string($grantFlds)) return exitJsonError($grantFlds);

  $DBTX = \db_transaction();

  if (cgpayGrantsHasGrantorInput($body)) {
    $pid = cgpayGrantsResolveGrantorPid($body, $uid, (int) $existing->pid);
    if (is_string($pid)) return exitJsonError($pid);
    $grantFlds['pid'] = $pid;
  }

  if ($grantFlds) db\update('grants', compact('id') + $grantFlds, 'id');

  unset($DBTX);

  return exitJson(['id' => $id], X_OK);
}

/**
 * Parse grant-row fields (amount, by, fund) from the request body.
 * @param bool $required  TRUE for create (amount + by required), FALSE for update (all optional).
 * @return array|string   Fields to persist, or an error message string.
 */
function cgpayGrantsParseFlds($body, $required) {
  $out = [];

  if (array_key_exists('amount', $body) || $required) {
    $amount = nni($body, 'amount');
    if ($required || $amount !== null) {
      if ($err = u\badAmount($amount, '>0')) return $err;
      $out['amount'] = $amount;
    }
  }

  if (array_key_exists('by', $body) || $required) {
    $by = strtolower(trim((string) nni($body, 'by')));
    if ($required || $by !== '') {
      if (!in_array($by, ['ach', 'check', 'wire'], TRUE)) return 'bad payment method';
      $out['by'] = $by;
    }
  }

  if (array_key_exists('fund', $body)) {
    $out['fund'] = trim((string) nni($body, 'fund')) ?: null;
  }

  return $out;
}

function cgpayGrantsHasGrantorInput($body) {
  foreach (['fullName', 'email', 'phone', 'address', 'city', 'state', 'zip', 'grantorPid'] as $k) {
    if (array_key_exists($k, $body)) return TRUE;
  }
  return FALSE;
}

/**
 * Resolve the grantor's people-table pid, applying William's edit rules.
 * @param array $body        Full request body (already validated as array).
 * @param int   $sponseeUid  Sponsee account uid (changedBy for changes-table entries).
 * @param int|null $fallbackPid  When updating, the existing grants row's pid (used if body omits grantorPid).
 * @return int|string  The resolved pid, or an error message string.
 */
function cgpayGrantsResolveGrantorPid($body, $sponseeUid, $fallbackPid = null) {
  $fullName = trim((string) nni($body, 'fullName'));
  if ($fullName === '') return 'grantor name required';
  if ($err = u\badName($fullName)) return $err;

  $grantorPid = (int) nni($body, 'grantorPid') ?: $fallbackPid;

  $contact = cgpayGrantsParseContactFlds($body);
  if (is_string($contact)) return $contact;

  if ($grantorPid) {
    $existing = db\row('people', ['pid' => $grantorPid]);
    if (!$existing) $grantorPid = null;
  }

  if (!$grantorPid || strcasecmp(trim((string) $existing->fullName), $fullName) !== 0) {
    // No prior grantor, or the name changed — treat as a new entity.
    return db\insert('people', ['fullName' => $fullName] + $contact, 'pid');
  }

  // Same grantor — apply smart field-update rules.
  $updates = [];
  $changes = [];
  foreach ($contact as $field => $newVal) {
    if ($newVal === '' || $newVal === null) continue;
    $oldVal = (string) ($existing->$field ?? '');
    if ((string) $newVal === $oldVal) continue;
    $updates[$field] = $newVal;
    if ($oldVal !== '') $changes[$field] = ['old' => $oldVal, 'new' => (string) $newVal];
  }

  if ($updates) {
    db\update('people', ['pid' => $grantorPid] + $updates, 'pid');
    foreach ($changes as $field => $vals) {
      db\insert('changes', [
        'table'     => 'people',
        'rid'       => $grantorPid,
        'field'     => $field,
        'oldValue'  => $vals['old'],
        'newValue'  => $vals['new'],
        'changedBy' => $sponseeUid,
      ]);
    }
    if ($changes) {
      r\tellAdmin('grantor-contact-changed', ray(
        'grantor sponsee fields',
        $fullName, $sponseeUid, implode(', ', array_keys($changes))
      ));
    }
  }

  return $grantorPid;
}

/**
 * Parse + validate optional grantor contact fields. Returns array of provided fields or an error string.
 */
function cgpayGrantsParseContactFlds($body) {
  $email   = trim((string) nni($body, 'email'));
  $phone   = trim((string) nni($body, 'phone'));
  $address = trim((string) nni($body, 'address'));
  $city    = trim((string) nni($body, 'city'));
  $state   = nni($body, 'state') === null ? null : (int) nni($body, 'state');
  $zip     = trim((string) nni($body, 'zip'));

  if ($email !== '' && !u\validEmail($email))          return 'bad email';
  if ($phone !== '' && ($err = u\badPhone($phone)))    return $err;
  if ($zip   !== '' && ($err = u\badZip($zip)))        return $err;

  return compact(ray('email phone address city state zip'));
}

function exitJsonError($msg) {
  return exitJson(['error' => (string) $msg], X_SYNTAX);
}
