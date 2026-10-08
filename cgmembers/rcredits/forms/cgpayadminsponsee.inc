<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Sponsee detail endpoint for the SvelteKit admin dashboard.
 *
 * Parallel to /cgpay-admin-user but shaped around the sponsee view: the
 * identity block carries legalName + federalId (EIN), grants are the primary
 * ledger, and the relations list surfaces the people who can act on the
 * sponsee's behalf.
 *
 * Flow:
 *   POST /cgpay-admin-sponsee
 *     X-CG-Internal-Token: <shared secret>
 *     body: {"ssid": "<SSESS... value>",
 *            "uid": <int sponsee uid>,
 *            "txLimit": <int 1-200, default 50>,
 *            "grantsLimit": <int 1-200, default 100>}
 *
 *   200 {"sponsee": {uid, qid, name, legalName, federalId, created, flags,
 *                     balance, lastActivity, sponsored, co},
 *        "rollups": {expectedTotal, receivedTotal, expectedCount,
 *                     receivedCount, memberCount},
 *        "relations": [{reid, uid, name, qid, permission, permissionLabel,
 *                        owner, employee, customer, autopay, draw}, ...],
 *        "grants":    [{id, amount, by, grantor, documented, received, created}, ...],
 *        "txs":       [{xid, amount, counterparty, description, created}, ...]}
 *   401  bad token, no session, or caller not admin
 *   400  missing or invalid uid
 *   404  uid not a sponsored account
 */
function cgpayAdminSponsee() {
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  $body = json_decode(file_get_contents('php://input'), TRUE);
  $ssid = isset($body['ssid']) ? trim((string) $body['ssid']) : '';
  if ($ssid === '') return exitJust(X_SYNTAX);

  $sess = db\row('sessions', 'ssid=:sid', ['sid' => $ssid]);
  if (!$sess) return exitJust(X_UNAUTH);
  $callerUid = (int) ($sess->acct ?? 0) ?: (int) ($sess->uid ?? 0);
  if (!$callerUid) return exitJust(X_UNAUTH);
  $me = r\acct($callerUid);
  if (!$me || !$me->can(B_ADMIN)) return exitJust(X_UNAUTH);

  $uid = isset($body['uid']) ? (int) $body['uid'] : 0;
  if ($uid <= 0) return exitJust(X_SYNTAX);

  $txLimit     = isset($body['txLimit'])     ? max(1, min(200, (int) $body['txLimit']))     : 50;
  $grantsLimit = isset($body['grantsLimit']) ? max(1, min(200, (int) $body['grantsLimit'])) : 100;

  $a = r\acct($uid);
  if (!$a) return exitJust(X_NOTFOUND);

  // Must be a fiscally sponsored company account.
  $sponsoredMask = 1 << CO_SPONSORED;
  $co = db\row('u_company', 'uid=:u', ['u' => $uid]);
  if (!$co || !((int) ($co->coFlags ?? 0) & $sponsoredMask)) return exitJust(X_NOTFOUND);

  $userRow = db\row('users', 'uid=:u', ['u' => $uid]);
  $lastTx  = (int) db\get('COALESCE(MAX(created), 0)', 'txs', 'uid1=:u OR uid2=:u', ['u' => $uid]);

  // legalName lives in the serialized users.data blob; federalId is VSECURE
  // (encrypted). Both are exposed via the Acct accessor rather than raw columns.
  $sponsee = [
    'uid'          => (int) $a->id,
    'qid'          => $a->mainQid,
    'name'         => $a->bestName,
    'legalName'    => (string) ($a->legalName ?: ''),
    'federalId'    => (string) ($a->federalId ?: ''),
    'created'      => (int) ($userRow->created ?? 0),
    'flags'        => (int) ($userRow->flags ?? 0),
    'balance'      => (float) $a->balance,
    'lastActivity' => $lastTx,
    'sponsored'    => TRUE,
    'co'           => (bool) $a->co,
  ];

  // --- Grants rollups and list ---
  $expectedTotal = (float) db\get('COALESCE(SUM(amount), 0)', 'grants', 'uid=:u AND received IS NULL',     ['u' => $uid]);
  $receivedTotal = (float) db\get('COALESCE(SUM(amount), 0)', 'grants', 'uid=:u AND received IS NOT NULL', ['u' => $uid]);
  $expectedCount = (int)   db\get('COUNT(*)',                 'grants', 'uid=:u AND received IS NULL',     ['u' => $uid]);
  $receivedCount = (int)   db\get('COUNT(*)',                 'grants', 'uid=:u AND received IS NOT NULL', ['u' => $uid]);

  $gq = db\q(
    'SELECT g.id, g.amount, g.by, g.pid, g.documented, g.received, g.created
       FROM grants g
      WHERE g.uid = :u
      ORDER BY g.created DESC
      LIMIT :lim',
    ['u' => $uid, 'lim' => $grantsLimit]
  );
  $rawGrants = [];
  $pids = [];
  while ($row = $gq->fetchAssoc()) {
    $rawGrants[] = $row;
    if ($row['pid']) $pids[(int) $row['pid']] = TRUE;
  }
  $grantors = [];
  if ($pids) {
    $pidList = implode(',', array_map('intval', array_keys($pids)));
    $pq = db\q("SELECT pid, fullName FROM people WHERE pid IN ($pidList)");
    while ($pr = $pq->fetchAssoc()) {
      $grantors[(int) $pr['pid']] = (string) $pr['fullName'];
    }
  }
  $grants = [];
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

  // --- Related members (people with permission on this sponsee) ---
  $relSql = 'SELECT r.reid, r.main AS uid, r.permission, r.code,
                    u.fullName, u.name
               FROM u_relations r
          LEFT JOIN users u ON u.uid = r.main
              WHERE r.other = :u
           ORDER BY r.permission DESC, u.fullName ASC';
  $rq = db\q($relSql, ['u' => $uid]);
  $permLabels = $GLOBALS['share permissions'];
  $relations = [];
  while ($row = $rq->fetchAssoc()) {
    $perm = (int) $row['permission'];
    $permIndex = max(0, $perm - B_RELATED);
    $permLabel = $permLabels[$permIndex] ?? (string) $perm;
    $code = (int) $row['code'];
    $relUid = (int) $row['uid'];
    $relAcct = $relUid ? r\acct($relUid) : NULL;
    $relations[] = [
      'reid'            => (int) $row['reid'],
      'uid'             => $relUid,
      'qid'             => $relAcct ? $relAcct->mainQid : '',
      'name'            => $row['fullName'] ?: $row['name'] ?: ('Member ' . $relUid),
      'permission'      => $perm,
      'permissionLabel' => (string) $permLabel,
      'owner'           => (bool) ($code & (1 << B_OWNER)),
      'employee'        => (bool) ($code & (1 << B_EMPLOYEE)),
      'customer'        => (bool) ($code & (1 << B_CUSTOMER)),
      'autopay'         => (bool) ($code & (1 << B_AUTOPAY)),
      'draw'            => (bool) ($code & (1 << B_DRAW)),
    ];
  }

  $memberCount = count($relations);

  // --- Recent transactions, shaped sponsee-first (positive = received) ---
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

  $rollups = [
    'expectedTotal' => $expectedTotal,
    'receivedTotal' => $receivedTotal,
    'expectedCount' => $expectedCount,
    'receivedCount' => $receivedCount,
    'memberCount'   => $memberCount,
  ];

  return exitJson(compact(ray('sponsee rollups relations grants txs')), X_OK);
}
