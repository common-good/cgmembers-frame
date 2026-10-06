<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Single-user detail endpoint for the SvelteKit admin dashboard.
 *
 * Returns profile + recent transactions + grants for a given uid, scoped to
 * admin callers. Used by the /admin/users/[uid] page on the node side.
 *
 * Flow:
 *   POST /cgpay-admin-user
 *     X-CG-Internal-Token: <shared secret>
 *     body: {"ssid": "<SSESS... value>",
 *            "uid": <int>,
 *            "txLimit": <int 1-200, default 50>,
 *            "grantsLimit": <int 1-200, default 50>}
 *
 *   200 {"user": {uid, qid, name, loginName, fullName, email, phone,
 *                 created, flags, balance, co, sponsored, admin},
 *        "txs":    [{xid, amount, counterparty, description, created}, ...],
 *        "grants": [{id, amount, by, grantor, documented, received, created}, ...]}
 *   401  bad token, no session, or session not admin
 *   400  missing uid
 *   404  uid does not match any account
 */
function cgpayAdminUser() {
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  $body = json_decode(file_get_contents('php://input'), TRUE);
  $ssid = isset($body['ssid']) ? trim((string) $body['ssid']) : '';
  if ($ssid === '') return exitJust(X_SYNTAX);

  $row = db\row('sessions', 'ssid=:sid', ['sid' => $ssid]);
  if (!$row) return exitJust(X_UNAUTH);
  $callerUid = (int) ($row->acct ?? 0) ?: (int) ($row->uid ?? 0);
  if (!$callerUid) return exitJust(X_UNAUTH);
  $me = r\acct($callerUid);
  if (!$me || !$me->can(B_ADMIN)) return exitJust(X_UNAUTH);

  $uid = isset($body['uid']) ? (int) $body['uid'] : 0;
  if ($uid <= 0) return exitJust(X_SYNTAX);

  $txLimit     = isset($body['txLimit'])     ? max(1, min(200, (int) $body['txLimit']))     : 50;
  $grantsLimit = isset($body['grantsLimit']) ? max(1, min(200, (int) $body['grantsLimit'])) : 50;

  $a = r\acct($uid);
  if (!$a) return exitJust(X_NOTFOUND);

  // --- Profile ---
  $userRow = db\row('users', 'uid=:u', ['u' => $uid]);
  $user = [
    'uid'       => (int) $a->id,
    'qid'       => $a->mainQid,
    'name'      => $a->bestName,
    'loginName' => $userRow->name ?? '',
    'fullName'  => $userRow->fullName ?? '',
    'email'     => $userRow->email ?? '',
    'phone'     => $userRow->phone ?? '',
    'created'   => (int) ($userRow->created ?? 0),
    'flags'     => (int) ($userRow->flags ?? 0),
    'balance'   => (float) $a->balance,
    'co'        => (bool) $a->co,
    'sponsored' => (bool) $a->sponsored,
    'admin'     => (bool) $a->can(B_ADMIN),
  ];

  // --- Recent primary transactions (type = 1) ---
  // Direction: amt is signed from uid2's perspective (uid2 received = positive).
  // We shape it uid-of-interest-first: positive = received, negative = paid.
  $txSql = 'SELECT
      xid,
      CASE WHEN uid2 = :u THEN amt   ELSE -amt END  AS amount,
      CASE WHEN uid2 = :u THEN uid1  ELSE uid2 END  AS other_uid,
      CASE WHEN uid2 = :u THEN for2  ELSE for1 END  AS description,
      created
    FROM txs
    WHERE (uid1 = :u OR uid2 = :u) AND type = 1
    ORDER BY created DESC
    LIMIT :lim';
  $txq = db\q($txSql, ['u' => $uid, 'lim' => $txLimit]);

  $txRows = [];
  $otherUids = [];
  while ($row = $txq->fetchAssoc()) {
    $txRows[] = $row;
    if ($row['other_uid']) $otherUids[(int) $row['other_uid']] = TRUE;
  }

  // Resolve counterparty names in one lookup.
  $names = [];
  if ($otherUids) {
    $idList = implode(',', array_map('intval', array_keys($otherUids)));
    $nq = db\q("SELECT uid, fullName, name FROM users WHERE uid IN ($idList)");
    while ($nr = $nq->fetchAssoc()) {
      $names[(int) $nr['uid']] = $nr['fullName'] ?: $nr['name'] ?: ('Member ' . $nr['uid']);
    }
  }

  $txs = [];
  foreach ($txRows as $row) {
    $txs[] = [
      'xid'          => (int) $row['xid'],
      'amount'       => (float) $row['amount'],
      'counterparty' => $names[(int) $row['other_uid']] ?? ('Member ' . $row['other_uid']),
      'description'  => (string) ($row['description'] ?? ''),
      'created'      => (int) $row['created'],
    ];
  }

  // --- Grants (expected + received) for this account ---
  $grantsSql = 'SELECT g.id, g.amount, g.by, g.pid, g.documented, g.received, g.created
                FROM grants g
                WHERE g.uid = :u
                ORDER BY g.created DESC
                LIMIT :lim';
  $gq = db\q($grantsSql, ['u' => $uid, 'lim' => $grantsLimit]);
  $grants = [];
  $pids = [];
  $rawGrants = [];
  while ($row = $gq->fetchAssoc()) {
    $rawGrants[] = $row;
    if ($row['pid']) $pids[(int) $row['pid']] = TRUE;
  }
  $grantors = [];
  if ($pids) {
    $pidList = implode(',', array_map('intval', array_keys($pids)));
    $gq2 = db\q("SELECT pid, fullName FROM people WHERE pid IN ($pidList)");
    while ($gr = $gq2->fetchAssoc()) {
      $grantors[(int) $gr['pid']] = (string) $gr['fullName'];
    }
  }
  foreach ($rawGrants as $row) {
    $grants[] = [
      'id'         => (int) $row['id'],
      'amount'     => (float) $row['amount'],
      'by'         => (string) $row['by'],
      'grantor'    => $grantors[(int) $row['pid']] ?? '',
      'documented' => $row['documented'] === NULL ? NULL : (int) $row['documented'],
      'received'   => $row['received']   === NULL ? NULL : (int) $row['received'],
      'created'    => (int) $row['created'],
    ];
  }

  return exitJson(compact(ray('user txs grants')), X_OK);
}
