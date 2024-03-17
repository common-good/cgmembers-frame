<?php

use CG as r;
use CG\Util as u;

/**
 * @file
 * List of queries for member companies.
 * expects $mya not null
 * These special parameters are recognized:
 *  :ctty  the current account's community ID
 *  :myid  the current account's record ID
 * Note: query names cannot contain any of the following characters: /&? (because they confuse the url).
 * For queries with no SQL show here, see the getReport() function in forms/data.inc
 */

$name = u\sqlSerialValue('r.data', 2); // used in Customers query
$phone = u\sqlSerialValue('r.data', 6);
$footnotes = tr('
  <p class="small">
    *&nbsp; = no active %PROJECT account.<br>
    ** = information an account-holder has chosen not to share with you.
  </p>');
$LYdt2 = strtotime('January 1') - 1;
$LYdt1 = strtotime('1 year ago', $LYdt2 + 1);

if (in($mya->coType, [CO_NONPROFIT, CO_COOPERATIVE, CO_CLUB])) {
  $queries = [t('Donations') => 'donations' . t('~<p>Contact information and total donations for each donor.</p>') . $footnotes];
} else $queries = [];

$queries += [
  t('Customers') => 'customers' . t('~<p>A list of your customers (anyone who has paid you). Click an account code for a customer statement.</p>') . $footnotes,

  t('Vendors') => 'vendors' . t('~<p>A list of your vendors (anyone you have paid). Click an account code for a payment and billing history.</p>') . $footnotes,
  
  t('Last Year Payments By Payee') => <<< X
    SELECT uid, u.fullName, SUM(amt) AS total, SUM(IF(for1 LIKE '%labor%', amt, 0)) AS laborTotal
    FROM txs_noreverse x join users u ON u.uid=x.uid2
    WHERE x.created between $LYdt1 AND $LYdt2 AND x.uid1=:myid
    GROUP BY u.uid
    ORDER BY SUM(amt) DESC,u.fullName
X
  . t('~<p>Use this for 1099-NEC amounts.</p>'),
  
  t('Customer Demographics') => 'demographics',
  
  t('%PROJECT Members You Signed Up') =>
  "SELECT IFNULL(uid, (SELECT uid FROM users WHERE fullName=$name)) AS account,
    code AS customer, IFNULL(fullName, $name) AS name, IF(u.activated, '', $phone) AS phone,
    FROM_UNIXTIME(r.created, '%m/%d/%Y') AS created,
    IF(u.uid IS NULL OR u.activated=0,
      IF(u.uid IS NULL OR u.uid=0, 'NOT BEGUN', 'in process'), 
      FROM_UNIXTIME(u.activated, '%m/%d/%Y')) AS completed
  FROM u_relations r LEFT JOIN users u ON u.uid=r.other WHERE r.main=:myid AND r.:IS_CUSTOMER ORDER BY u.activated,r.created"
  . t('~<p>A list of your customers who joined through a shared sign-up, showing progress opening their %PROJECT account.</p>'),

  t('Daily Totals') =>
  "SELECT CONCAT_WS(' ', FROM_UNIXTIME(created, '%m/%d'), LEFT(FROM_UNIXTIME(created, '%W'), 3)) AS `day`,
  SUM(IF(amount>0, amount, 0)) AS `total in`,
  SUM(IF(amount<0, -amount, 0)) AS `total out`
  FROM tx_entries JOIN tx_hdrs USING(xid) JOIN (SELECT xid FROM txs_noreverse) tnr USING(xid)
  WHERE uid=:myid AND created>:REQUEST_TIME-190*:DAY_SECS
  GROUP BY FROM_UNIXTIME(created, '%Y-%m-%d')
  ORDER BY created DESC",
]; // JOIN with txs_noreverse just hides the reversals
