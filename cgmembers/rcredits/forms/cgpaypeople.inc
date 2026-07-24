<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * People-table endpoints for the SvelteKit cgpay POC.
 *
 * Currently exposes:
 *   GET /cgpay-people-autocomplete?uid=<sponsee_uid>&q=<query>&limit=<n>
 *     → list of candidate grantors matching <query>, drawn from people who have
 *       ever been a grantor OR who have been a prior donor to this sponsee.
 *       Returns fields the caller can use to auto-populate the grants form.
 *
 * Requires the same X-CG-Internal-Token shared secret used by other /cgpay-*
 * endpoints (CGPAY_SSO_SECRET, see defs.inc).
 *
 * Returns:
 *   GET 200 {"people": [{pid, fullName, email, phone, address, city, state, zip}, ...]}
 *   401 bad/missing shared secret
 *   400 malformed input
 *   404 uid does not resolve to a sponsee
 */
function cgpayPeopleAutocomplete() {
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'GET') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  $uid = (int) nni($_GET, 'uid');
  if ($uid <= 0) return exitJust(X_SYNTAX);
  $a = r\acct($uid);
  if (!$a) return exitJust(X_NOTFOUND);
  if (!$a->sponsored) return exitJust(X_FORBIDDEN);

  $q = trim((string) nni($_GET, 'q'));
  if (strlen($q) < 2) return exitJson(['people' => []], X_OK);

  $limit = (int) nni($_GET, 'limit');
  if ($limit <= 0 || $limit > 50) $limit = 20;

  // Filter: anyone who has ever been a grantor (has a row in grants). This is a
  // strict superset of "prior donor to this sponsee" — the ORDER BY promotes the
  // sponsee-specific matches to the top of the list for relevance.
  $like = '%' . $q . '%';
  $sql = "SELECT p.pid, p.fullName, p.email, p.phone, p.address, p.city, p.state, p.zip
          FROM people p
          WHERE p.fullName LIKE :like
          AND p.pid IN (SELECT DISTINCT pid FROM grants WHERE pid IS NOT NULL)
          ORDER BY
            CASE WHEN p.pid IN (SELECT DISTINCT pid FROM grants WHERE uid = :uid AND pid IS NOT NULL) THEN 0 ELSE 1 END,
            p.fullName
          LIMIT $limit";
  $rows = db\q($sql, compact('like', 'uid'))->fetchAll(\PDO::FETCH_ASSOC);

  $people = array_map(function($r) {
    return [
      'pid'      => (int) $r['pid'],
      'fullName' => (string) $r['fullName'],
      'email'    => (string) ($r['email']   ?? ''),
      'phone'    => (string) ($r['phone']   ?? ''),
      'address'  => (string) ($r['address'] ?? ''),
      'city'     => (string) ($r['city']    ?? ''),
      'state'    => $r['state'] !== null ? (int) $r['state'] : null,
      'zip'      => (string) ($r['zip']     ?? ''),
    ];
  }, $rows);

  return exitJson(['people' => $people], X_OK);
}
