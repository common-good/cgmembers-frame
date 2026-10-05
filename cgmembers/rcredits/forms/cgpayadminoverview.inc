<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Aggregate-counts endpoint for the SvelteKit admin dashboard overview.
 *
 * The node side (`cgpay` /admin) needs a handful of aggregate counts (members,
 * sponsees, recent transactions, pending grants). Doing those reads with the
 * cgweb_ro DB user runs into permission gaps on some tables (notably
 * `u_company`), so we route the reads through PHP - which has the DB user with
 * full access - and hand back a small JSON payload.
 *
 * Flow:
 *   POC POST /cgpay-admin-overview
 *     X-CG-Internal-Token: <shared secret>
 *     body: {"ssid": "<value of the SSESS... cookie>"}
 *
 *   200 { members: {total, active},
 *         sponsees: {total},
 *         transactions: {last30d},
 *         grants: {pendingCount} }
 *   401  bad/missing shared secret, no matching session, or session not admin
 *   400  missing ssid
 *
 * Shared secret is CGPAY_SSO_SECRET (config.json `cgpaySsoSecret`) - same key
 * as /cgpay-sso, /cgpay-lookup, /cgpay-whoami. Fails closed when either side
 * has an empty value.
 *
 * Defense-in-depth: even though the node caller already gates /admin behind an
 * admin check, this endpoint independently verifies the session belongs to an
 * admin (B_ADMIN) before returning aggregates.
 */
function cgpayAdminOverview() {
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

  $a = r\acct((int) $uid);
  if (!$a || !$a->can(B_ADMIN)) return exitJust(X_UNAUTH);

  $thirtyDaysAgo = now() - 30 * 24 * 60 * 60;

  $membersTotal  = (int) db\get('COUNT(*)', 'users', 'uid > 1');
  $membersActive = (int) db\get('COUNT(*)', 'users', 'uid > 1 AND (flags & :ok) > 0', ['ok' => B_OK]);
  $sponseesTotal = (int) db\get('COUNT(*)', 'u_company', '(coFlags & (1 << :sponsored)) > 0', ['sponsored' => CO_SPONSORED]);
  $txs30d        = (int) db\get('COUNT(*)', 'txs', 'type=:t AND created >= :since', ['t' => 1, 'since' => $thirtyDaysAgo]);
  $pendingGrants = (int) db\get('COUNT(*)', 'grants', 'received IS NULL');

  return exitJson([
    'members'      => ['total' => $membersTotal, 'active' => $membersActive],
    'sponsees'     => ['total' => $sponseesTotal],
    'transactions' => ['last30d' => $txs30d],
    'grants'       => ['pendingCount' => $pendingGrants],
  ], X_OK);
}
