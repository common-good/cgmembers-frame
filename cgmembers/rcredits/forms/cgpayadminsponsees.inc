<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Paginated sponsees list endpoint for the SvelteKit admin dashboard.
 *
 * Returns one page of fiscally sponsored accounts with the fields the admin
 * Sponsees table renders: identity, EIN, balance, expected/received grants
 * totals, last activity.
 *
 * Flow:
 *   POST /cgpay-admin-sponsees
 *     X-CG-Internal-Token: <shared secret>
 *     body: {"ssid": "<SSESS... value>",
 *            "limit": <int 1-100, default 25>,
 *            "offset": <int >= 0, default 0>}
 *
 *   200 {"items": [{uid, qid, name, legalName, federalId, balance,
 *                   expectedGrantsTotal, receivedGrantsTotal, lastActivity}, ...],
 *        "total": <int>,
 *        "limit": <int>,
 *        "offset": <int>}
 *   401  bad token, no matching session, or session not admin
 */
function cgpayAdminSponsees() {
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  $body = json_decode(file_get_contents('php://input'), TRUE);
  $ssid = isset($body['ssid']) ? trim((string) $body['ssid']) : '';
  if ($ssid === '') return exitJust(X_SYNTAX);

  $uid = db\get('uid', 'sessions', 'ssid=:sid', ['sid' => $ssid]);
  if (!$uid) return exitJust(X_UNAUTH);
  $me = r\acct((int) $uid);
  if (!$me || !$me->can(B_ADMIN)) return exitJust(X_UNAUTH);

  $limit  = isset($body['limit'])  ? max(1, min(100, (int) $body['limit'])) : 25;
  $offset = isset($body['offset']) ? max(0, (int) $body['offset']) : 0;

  $sponsoredMask = 1 << CO_SPONSORED;
  $where  = '(c.coFlags & :mask) > 0';
  $params = ['mask' => $sponsoredMask];

  $total = (int) db\get('COUNT(*)', 'u_company c', $where, $params);
  $sql = "SELECT c.uid, c.legalName, c.federalId
          FROM u_company c
          WHERE $where
          ORDER BY c.uid ASC
          LIMIT :lim OFFSET :off";
  $qres = db\q($sql, $params + ['lim' => $limit, 'off' => $offset]);

  $items = [];
  while ($row = $qres->fetchAssoc()) {
    $sponseeUid = (int) $row['uid'];
    $a = r\acct($sponseeUid);
    if (!$a) continue;

    // Grants roll-ups: sum of amounts, split by whether received date is set.
    $expectedTotal = (float) db\get('COALESCE(SUM(amount), 0)', 'grants', 'uid=:u AND received IS NULL', ['u' => $sponseeUid]);
    $receivedTotal = (float) db\get('COALESCE(SUM(amount), 0)', 'grants', 'uid=:u AND received IS NOT NULL', ['u' => $sponseeUid]);
    $lastTx        = (int) db\get('COALESCE(MAX(created), 0)', 'txs', 'uid1=:u OR uid2=:u', ['u' => $sponseeUid]);

    $items[] = [
      'uid'                 => $sponseeUid,
      'qid'                 => $a->mainQid,
      'name'                => $a->bestName,
      'legalName'           => $row['legalName'] ?: '',
      'federalId'           => $row['federalId'] ?: '',
      'balance'             => (float) $a->balance,
      'expectedGrantsTotal' => $expectedTotal,
      'receivedGrantsTotal' => $receivedTotal,
      'lastActivity'        => $lastTx,
    ];
  }

  return exitJson(compact(ray('items total limit offset')), X_OK);
}
