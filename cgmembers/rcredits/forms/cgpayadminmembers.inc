<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Paginated members list endpoint for the SvelteKit admin dashboard.
 *
 * Returns one page of accounts (uid > 1) with the fields the admin Members
 * table renders: uid, QID, display name, join date, balance, flags.
 *
 * Flow:
 *   POST /cgpay-admin-members
 *     X-CG-Internal-Token: <shared secret>
 *     body: {"ssid": "<SSESS... value>",
 *            "limit": <int 1-100, default 25>,
 *            "offset": <int >= 0, default 0>,
 *            "q": "<optional name/qid filter>"}
 *
 *   200 {"items": [{uid, qid, name, created, balance, flags}, ...],
 *        "total": <int>,  // total matching across all pages
 *        "limit": <int>,
 *        "offset": <int>}
 *   401  bad token, no matching session, or session not admin
 *   400  malformed limit/offset
 */
function cgpayAdminMembers() {
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
  $q      = isset($body['q']) ? trim((string) $body['q']) : '';

  $where  = 'uid > 1';
  $params = [];
  if ($q !== '') {
    $where .= ' AND (fullName LIKE :q OR name LIKE :q)';
    $params['q'] = '%' . $q . '%';
  }

  $total = (int) db\get('COUNT(*)', 'users', $where, $params);
  $sql = "SELECT uid, name, fullName, created, balance, flags FROM users WHERE $where ORDER BY uid ASC LIMIT :lim OFFSET :off";
  $qres = db\q($sql, $params + ['lim' => $limit, 'off' => $offset]);

  $items = [];
  while ($row = $qres->fetchAssoc()) {
    $a = r\acct((int) $row['uid']);
    $items[] = [
      'uid'     => (int) $row['uid'],
      'qid'     => $a ? $a->mainQid : '',
      'name'    => $a ? $a->bestName : ($row['fullName'] ?: $row['name']),
      'created' => (int) $row['created'],
      'balance' => (float) ($row['balance'] ?? 0),
      'flags'   => (int) $row['flags'],
    ];
  }

  return exitJson(compact(ray('items total limit offset')), X_OK);
}
