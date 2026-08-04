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
 * Validation errors (per William + Jose 2026-07-31):
 *   Instead of returning the FIRST error only, ALL field-level errors are
 *   collected and returned together as {"errors": {"<field>": "<message>", ...}}.
 *   Messages are user-friendly ("That is the wrong format for an email address.").
 *   Frontend uses this shape to highlight all bad fields simultaneously.
 *
 * POST body (create) and PATCH body (update) fields are the same:
 *   { uid, amount, by, fund?, grantorPid?, fullName, email?, phone?,
 *     address?, city?, state?, zip? }
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

  // Validate ALL fields up front so users see every problem at once.
  $errors = [];
  $grantFlds = cgpayGrantsParseFlds($body, TRUE, $errors);
  cgpayGrantsValidateGrantor($body, $errors);
  if ($errors) return exitJsonErrors($errors);

  $DBTX = \db_transaction();
  $pid = cgpayGrantsResolveGrantorPid($body, $uid);
  if (is_string($pid)) return exitJsonErrors(['fullName' => $pid]);

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

  $errors = [];
  $grantFlds = cgpayGrantsParseFlds($body, FALSE, $errors);
  if (cgpayGrantsHasGrantorInput($body)) cgpayGrantsValidateGrantor($body, $errors);
  if ($errors) return exitJsonErrors($errors);

  $DBTX = \db_transaction();
  if (cgpayGrantsHasGrantorInput($body)) {
    $pid = cgpayGrantsResolveGrantorPid($body, $uid, (int) $existing->pid);
    if (is_string($pid)) return exitJsonErrors(['fullName' => $pid]);
    $grantFlds['pid'] = $pid;
  }
  if ($grantFlds) db\update('grants', compact('id') + $grantFlds, 'id');
  unset($DBTX);

  return exitJson(['id' => $id], X_OK);
}

/**
 * Parse grant-row fields (amount, by, fund). Adds any per-field errors to $errors.
 * @param bool $required  TRUE for create (amount + by required), FALSE for update (all optional).
 * @return array          Fields to persist (always returns — errors reported separately).
 */
function cgpayGrantsParseFlds($body, $required, array &$errors) {
  $out = [];

  if (array_key_exists('amount', $body) || $required) {
    $amount = nni($body, 'amount');
    if ($required || $amount !== null) {
      if (u\badAmount($amount, '>0')) {
        $errors['amount'] = t('Please enter an amount greater than zero.');
      } else {
        $out['amount'] = $amount;
      }
    }
  }

  if (array_key_exists('by', $body) || $required) {
    $by = strtolower(trim((string) nni($body, 'by')));
    if ($required || $by !== '') {
      if (!in_array($by, ['ach', 'check', 'wire'], TRUE)) {
        $errors['by'] = t('Please choose a payment method (ACH, check, or wire).');
      } else {
        $out['by'] = $by;
      }
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
 * Validate grantor fields (fullName + optional contact info). Adds per-field errors to $errors.
 * Does NOT write to the DB — pure validation. Actual pid resolution happens in
 * cgpayGrantsResolveGrantorPid() after validation passes.
 */
function cgpayGrantsValidateGrantor($body, array &$errors) {
  $fullName = trim((string) nni($body, 'fullName'));
  if ($fullName === '') {
    $errors['fullName'] = t('Please enter the grantor\'s name.');
  } elseif (u\badName($fullName)) {
    $errors['fullName'] = t('Please enter a valid name for the grantor.');
  }

  // Address is required (per William 2026-07-31: we send acknowledgments to the grantor
  // and may need the mailing address for grant records). City / state / zip are all
  // part of a complete address, so all four required together.
  $address = trim((string) nni($body, 'address'));
  if ($address === '') $errors['address'] = t('Please enter the grantor\'s street address.');

  $city = trim((string) nni($body, 'city'));
  if ($city === '') $errors['city'] = t('Please enter the grantor\'s city.');

  $state = nni($body, 'state');
  if ($state === null || (int) $state <= 0) {
    $errors['state'] = t('Please select the grantor\'s state.');
  }

  $zip = trim((string) nni($body, 'zip'));
  if ($zip === '') {
    $errors['zip'] = t('Please enter the grantor\'s zip code.');
  } elseif (u\badZip($zip)) {
    $errors['zip'] = t('That is the wrong format for a zip code.');
  }

  // Email + phone remain optional but validated if provided.
  $email = trim((string) nni($body, 'email'));
  if ($email !== '' && !u\validEmail($email)) {
    $errors['email'] = t('That is the wrong format for an email address.');
  }

  $phone = trim((string) nni($body, 'phone'));
  if ($phone !== '' && u\badPhone($phone)) {
    $errors['phone'] = t('That is the wrong format for a phone number.');
  }
}

/**
 * Resolve the grantor's people-table pid, applying William's edit rules.
 * Called AFTER cgpayGrantsValidateGrantor() so inputs are already valid.
 *
 * @param array $body        Full request body (already validated as array).
 * @param int   $sponseeUid  Sponsee account uid (changedBy for changes-table entries).
 * @param int|null $fallbackPid  When updating, the existing grants row's pid (used if body omits grantorPid).
 * @return int|string  The resolved pid, or an error message string.
 */
function cgpayGrantsResolveGrantorPid($body, $sponseeUid, $fallbackPid = null) {
  $fullName = trim((string) nni($body, 'fullName'));
  $grantorPid = (int) nni($body, 'grantorPid') ?: $fallbackPid;
  $contact = cgpayGrantsExtractContactFlds($body);

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
 * Extract grantor contact fields from the body (no validation — validation done separately).
 */
function cgpayGrantsExtractContactFlds($body) {
  $email   = trim((string) nni($body, 'email'));
  $phone   = trim((string) nni($body, 'phone'));
  $address = trim((string) nni($body, 'address'));
  $city    = trim((string) nni($body, 'city'));
  $state   = nni($body, 'state') === null ? null : (int) nni($body, 'state');
  $zip     = trim((string) nni($body, 'zip'));
  return compact(ray('email phone address city state zip'));
}

/**
 * Return per-field error response with a top-level summary suitable for the form banner.
 * Also keeps `error` for backward compatibility with any older client that only reads that.
 */
function exitJsonErrors(array $errors) {
  $summary = t('There is an error in the information you typed — fix the highlighted field(s) and try again.');
  $body = [
    'message' => $summary,
    'errors'  => $errors,
    'error'   => $summary,   // backward-compat: older clients read `error`
  ];
  return exitJson($body, X_SYNTAX);
}

// Deprecated single-error helper — kept for compatibility with any remaining single-message callers.
function exitJsonError($msg) {
  return exitJson(['error' => (string) $msg, 'message' => (string) $msg], X_SYNTAX);
}
