<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Grantor autocomplete endpoint for the SvelteKit cgpay POC.
 *
 * Exposes:
 *   GET /cgpay-people-autocomplete?uid=<sponsee_uid>&search=<query>&limit=<n>
 *     → list of candidate grantors matching <query>. See grantorAutocompleteRows().
 *
 * Requires the same X-CG-Internal-Token shared secret used by other /cgpay-*
 * endpoints (CGPAY_SSO_SECRET, see defs.inc). Also holds cgpaySharedSecretOk(),
 * which forms/cgpayfundersexport.inc loads from here.
 */
function cgpayPeopleAutocomplete() {
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'GET') return exitJust(X_SYNTAX);
  if (!cgpaySharedSecretOk()) return exitJust(X_UNAUTH);

  $uid = (int) nni($_GET, 'uid');
  if ($uid <= 0) return exitJust(X_SYNTAX);
  $a = r\acct($uid);
  if (!$a) return exitJust(X_NOTFOUND);
  if (!$a->sponsored) return exitJust(X_FORBIDDEN);

  $q = trim((string) nni($_GET, 'search')); // not 'q': Drupal reads $_GET['q'] as the page path
  if (strlen($q) < 2) return exitJson(['people' => []], X_OK);

  $limit = (int) nni($_GET, 'limit');
  if ($limit <= 0 || $limit > 50) $limit = 20;

  $people = grantorAutocompleteRows($uid, $q, $limit);
  return exitJson(['people' => $people], X_OK);
}

/**
 * Verify the shared-secret header used by all /cgpay-* server-to-server endpoints.
 */
function cgpaySharedSecretOk() {
  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  return !empty($expected) && !empty($provided) && hash_equals($expected, $provided);
}

/**
 * Autocomplete-shaped rows: matching people who have ever been a grantor,
 * with prior donors to $sponseeUid promoted to the top.
 * Shared by /cgpay-people-autocomplete (server-to-server) and the admin ajax
 * `suggestGrantor` op in forms/ajax.inc.
 *
 * @param int    $sponseeUid  Used only for ORDER BY (prior-donor-to-this-sponsee ranks first).
 * @param string $q           Query string, >= 2 chars (caller enforces).
 * @param int    $limit       Max rows to return (caller-bounded).
 * @return array              List of [pid, fullName, email, phone, address, city, state, zip].
 */
function grantorAutocompleteRows($sponseeUid, $q, $limit = 20) {
  $sponseeUid = (int) $sponseeUid;
  $limit = max(1, min(50, (int) $limit));
  $like = '%' . $q . '%';

  // Filter: anyone who has ever been a grantor (has a row in grants). Strict
  // superset of "prior donor to this sponsee" — ORDER BY promotes matches for
  // the current sponsee first.
  $sql = "SELECT p.pid, p.fullName, p.email, p.phone, p.address, p.city, p.state, p.zip
          FROM people p
          WHERE p.fullName LIKE :like
          AND p.pid IN (SELECT DISTINCT pid FROM grants WHERE pid IS NOT NULL)
          ORDER BY
            CASE WHEN p.pid IN (SELECT DISTINCT pid FROM grants WHERE uid = :uid AND pid IS NOT NULL) THEN 0 ELSE 1 END,
            p.fullName
          LIMIT $limit";
  $rows = db\q($sql, ['like' => $like, 'uid' => $sponseeUid])->fetchAll(\PDO::FETCH_ASSOC);

  return array_map(function($r) {
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
}
