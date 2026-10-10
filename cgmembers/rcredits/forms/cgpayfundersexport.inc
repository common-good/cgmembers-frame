<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;

require_once R_ROOT . '/forms/cgpaypeopleautocomplete.inc'; // for cgpaySharedSecretOk()

/**
 * @file
 * Funder export endpoint for the SvelteKit cgpay POC.
 *
 * Exposes:
 *   GET /cgpay-funders-export?format=<csv|mailerlite>
 *     → downloadable CSV of all funders (everyone who has ever been a grantor).
 *       `mailerlite` format uses the column headers MailerLite expects for imports.
 *
 * Requires the same X-CG-Internal-Token shared secret used by other /cgpay-*
 * endpoints (CGPAY_SSO_SECRET, see defs.inc).
 */
function cgpayFundersExport() {
  if (nni($_SERVER, 'REQUEST_METHOD') !== 'GET') return exitJust(X_SYNTAX);
  if (!cgpaySharedSecretOk()) return exitJust(X_UNAUTH);

  $format = strtolower(trim((string) nni($_GET, 'format'))) ?: 'csv';
  if (!in_array($format, ['csv', 'mailerlite'], TRUE)) return exitJust(X_SYNTAX);

  $rows = fundersExportRows();
  fundersEmitCsv($rows, $format);
  exit;
}

/**
 * All funders (everyone who's ever been a grantor), with aggregate stats:
 *   pid, fullName, email, phone, address, city, state, zip, grantCount, totalGranted, lastGrantAt
 */
function fundersExportRows() {
  $sql = "SELECT p.pid, p.fullName, p.email, p.phone, p.address, p.city, p.state, p.zip,
                 COUNT(g.id) AS grantCount, SUM(g.amount) AS totalGranted, MAX(g.created) AS lastGrantAt
          FROM people p
          JOIN grants g USING (pid)
          WHERE g.pid IS NOT NULL
          GROUP BY p.pid
          ORDER BY p.fullName";
  return db\q($sql)->fetchAll(\PDO::FETCH_ASSOC);
}

/**
 * Stream funder rows as CSV to the browser and exit.
 * @param array  $rows    Rows from fundersExportRows().
 * @param string $format  'csv' (raw columns) or 'mailerlite' (MailerLite-import headers).
 */
function fundersEmitCsv($rows, $format) {
  $stamp = date('Y-m-d');
  $filename = "funders-{$format}-{$stamp}.csv";
  header('Content-Type: text/csv; charset=utf-8');
  header('Content-Disposition: attachment; filename="' . $filename . '"');

  $out = fopen('php://output', 'w');
  if ($format === 'mailerlite') {
    // MailerLite import headers - email is the only required column.
    fputcsv($out, ['email', 'name', 'phone', 'city', 'state', 'zip', 'grant_count', 'total_granted', 'last_grant_at']);
    foreach ($rows as $r) {
      if (empty($r['email'])) continue; // MailerLite requires email
      fputcsv($out, [
        $r['email'],
        $r['fullName'],
        $r['phone'],
        $r['city'],
        $r['state'],
        $r['zip'],
        (int) $r['grantCount'],
        (float) $r['totalGranted'],
        $r['lastGrantAt'] ? date('Y-m-d', (int) $r['lastGrantAt']) : '',
      ]);
    }
  } else {
    fputcsv($out, ['pid', 'fullName', 'email', 'phone', 'address', 'city', 'state', 'zip', 'grantCount', 'totalGranted', 'lastGrantAt']);
    foreach ($rows as $r) {
      fputcsv($out, [
        (int) $r['pid'],
        $r['fullName'],
        $r['email'],
        $r['phone'],
        $r['address'],
        $r['city'],
        $r['state'],
        $r['zip'],
        (int) $r['grantCount'],
        (float) $r['totalGranted'],
        $r['lastGrantAt'] ? date('Y-m-d', (int) $r['lastGrantAt']) : '',
      ]);
    }
  }
  fclose($out);
}
