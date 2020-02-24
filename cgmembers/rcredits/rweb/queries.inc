<?php

use CG as r;
use CG\Util as u;

/**
 * @file
 * List of queries for the Community and Money Data page on the Community menu (for members only).
 * Queries beginning with a digit are for admins or companies only: 0=superAdmin 1=admin 2=cAdmin or cAdmin2 B_CO=companies
 * These special parameters are recognized:
 *  :ctty  the current account's community ID
 *  :myid  the current account's record ID
 * Note: query names cannot contain any of the following characters: /&? (because they confuse the url).
 */

$bizQ = 'SELECT uid as payee, goods, SUM(IF(amount>0,amount,0)) AS sales, SUM(IF(amount<0,amount,0)) AS payments, COUNT(*) AS cnt FROM tx_entries JOIN tx_hdrs USING (xid) WHERE goods=:FOR_GOODS AND created BETWEEN';
$gifts = "r.amount * IF(period='M', 12, IF(period='Q', 4, 1))";
$source = "TRIM(REPLACE(REPLACE(source,'knows: self ()', ''), '-', ' '))";

$queries = [
  1 . t('Food Fund Percentage Contributions') =>
    "SELECT COUNT(uid) AS participants, SUM(amt) AS monthly FROM (
      SELECT uid, SUM(food*amt) AS amt, IF(SUM(amt)=0, 0, 1) AS active  
      FROM (SELECT uid,food FROM users WHERE food>0) u 
      LEFT JOIN (SELECT uid1 AS uid, amt FROM txs_noreverse WHERE :T_FOODY AND created>:NOW-:MONTH_SECS) t USING(uid)
      GROUP BY uid
    ) x",
// SUM(active) AS active, 

  2 . t('Most Trusted Members') => 
    "SELECT fullName, email AS email, phone, COUNT(b.voter) AS proxiedFor, trust FROM users u LEFT JOIN r_ballots b ON b.proxy=u.uid AND b.created>:REQUEST_TIME-:YEAR_SECS WHERE u.community=:ctty AND u.:IS_OK AND NOT u.:IS_CO GROUP BY IFNULL(b.proxy, u.uid) ORDER BY COUNT(b.voter) DESC, u.trust DESC, fullName",
    
  2 . 'Individuals Paid By A Company' =>
    "SELECT DISTINCT main.fullName AS company, other.fullName AS pays, 
            IF(r.:IS_EMPLOYEE, 'employee', IF(r.:IS_OWNER, 'owner', 
                                              IF(r.:IS_CUSTOMER, 'customer', ''))) AS relation
     FROM r_relations r
       JOIN txs_noreverse t ON t.uid1=r.main
       JOIN users main ON main.uid=r.main 
       JOIN users other ON other.uid=r.other
     WHERE NOT other.:IS_CO AND other.:IS_OK 
           AND main.community=:ctty AND main.:IS_CO AND main.:IS_OK
     ORDER BY main.community,main.fullName,relation DESC,other.fullName",
  
  2 . 'Transaction Totals to Date' =>
    "SELECT COUNT(*) AS txCount, SUM(ABS(amt)) AS txDollars, 
            SUM(IF(u2.:IS_CO AND (NOT u1.:IS_CO OR channel=:TX_POS), amt, 0)) AS inPersonDollars,
            SUM(IF(u2.:IS_CO AND (NOT u1.:IS_CO OR channel=:TX_POS), .85*.02*amt, 0)) AS ccFeesSaved
    FROM txs t JOIN users u1 ON u1.uid=uid1 JOIN users u2 ON u2.uid=uid2
    WHERE :ctty IN (u1.community, u2.community)",
    

  t('Company Income and Payments - 30 day totals') => 
    "SELECT LEFT(u.fullName, 42) AS company,
            t0.sales AS `sales $`, t0.payments AS `payments $`, t0.cnt AS `count`,
            t6.sales AS `sales $ 6mos ago`, t6.payments AS `payments $ 6mos ago`, t6.cnt AS `count6`,
            t12.sales AS `sales $ 12mos ago`, t12.payments AS `payments $ 12mos ago`, t12.cnt AS `count12`
     FROM users u 
     LEFT JOIN ($bizQ :NOW-30*:DAY_SECS AND :NOW GROUP BY payee) t0 ON t0.payee=u.uid
     LEFT JOIN ($bizQ :NOW-(180+30)*:DAY_SECS AND :NOW-180*:DAY_SECS GROUP BY payee) t6 ON t6.payee=u.uid
     LEFT JOIN ($bizQ :NOW-:YEAR_SECS-30*:DAY_SECS AND :NOW-:YEAR_SECS GROUP BY payee) t12 ON t12.payee=u.uid
     WHERE u.community=:ctty AND :IS_CO AND :IS_OK AND t0.cnt>0 AND fullName NOT LIKE '%457%' ORDER BY t0.sales DESC",

  t('Positive and Negative Balance Totals') => 
    "SELECT c.fullName AS community, 
     SUM(IF(u.balance<0, 1, 0)) AS negativeBalanceCount,
     SUM(IF(u.balance<0, u.balance, 0)) AS negativeBalanceTotal,
     SUM(IF(u.balance>=0, 1, 0)) AS positiveBalanceCount,
     SUM(IF(u.balance>=0, u.balance, 0)) AS positiveBalanceTotal
     FROM users u LEFT JOIN users c ON c.uid=u.community 
     WHERE IFNULL(u.jid,0)=0 AND u.:IS_OK AND u.uid NOT BETWEEN 0 AND (:CANONIC_ACCTS-1)
     GROUP BY c.fullName",
  
  t('Company and Member Balances and Credit Lines') =>
    "SELECT c.fullName AS community,
            SUM(IF(u.:IS_CO, 0, u.balance)) AS memBalances,
            SUM(IF(u.:IS_CO, 0, -u.floor)) AS memCredit, 
            SUM(IF(u.:IS_CO, 0, IF(u.:IS_REFILL, u.minimum, 0))) AS memTargetBalances,
            SUM(IF(u.:IS_CO, u.balance, 0)) AS coBalances,
            SUM(IF(u.:IS_CO, -u.floor, 0)) AS coCredit, 
            SUM(IF(u.:IS_CO, IF(u.:IS_REFILL, u.minimum, 0), 0)) AS coTargetBalances 
     FROM users u LEFT JOIN users c ON c.uid=u.community
     WHERE IFNULL(u.jid,0)=0 AND c.uid<0 GROUP BY c.fullName",

//     SUM(IF(t.:IS_GIFT OR t.:IS_ROUNDUPS OR t.:IS_CRUMBS OR t.:IS_RECURS,0,amount)) AS `payments to CG`
  t('Actual Donations to Common Good and Community') =>
    "SELECT CONCAT_WS(' ', YEAR(FROM_UNIXTIME(t.created)), MONTHNAME(FROM_UNIXTIME(t.created))) AS month,
    SUM(IF(t.:IS_RECURS,amt,0)) AS recurring,
    SUM(IF(uid2=:CG_ROUNDUPS_UID,amt,0)) AS roundups,
    SUM(IF(uid2=:CG_CRUMBS_UID,amt,0)) AS crumbs,
    SUM(IF(t.:IS_GIFT AND uid2 NOT IN (:CG_ROUNDUPS_UID, :CG_CRUMBS_UID) AND NOT t.:IS_RECURS,amt,0)) AS other
    FROM txs t JOIN users u ON u.uid=t.uid1
    WHERE u.community=:ctty AND (t.:IS_GIFT OR uid2 IN (:CG_ROUNDUPS_UID, :CG_CRUMBS_UID) OR t.:IS_RECURS)
    GROUP BY YEAR(FROM_UNIXTIME(t.created)) DESC, MONTH(FROM_UNIXTIME(t.created)) DESC",

  t('Expected Member Donations to CG and Community') =>
    $mGifts =
    "SELECT c.fullName AS community, 
            COUNT(*) AS members,
            SUM($gifts) AS yearly$,
            SUM(IF(u.:IS_ROUNDUP, 1, 0)) AS roundupy,
            SUM(IF(u.crumbs>0, 1, 0)) AS crumby,
            AVG(IF(u.crumbs>0, u.crumbs, NULL)) AS avgCrumbs
    FROM users u LEFT JOIN r_recurs r ON (r.payer=u.uid AND r.ended<>0 AND r.payee=:CGID) LEFT JOIN users c ON c.uid=u.community
    WHERE u.:IS_OK AND NOT u.:IS_CO AND u.uid NOT :CANONIC
    GROUP BY u.community
    ~roundupy = number who do Roundups, crumby = number who do Crumb donations",

  t('Expected Company Donations to CG and Community') =>
    strtr($mGifts, ['AS members' => 'AS companies', 'NOT u.:IS_CO' => 'u.:IS_CO']),
    
  2 . t('Where Do Our Members Hear About %PROJECT') => <<< X
    SELECT 
      IF(helper in (0,1), 
        IF(source LIKE '%presentation%' OR source LIKE '%talk%', '"Presentation"',
        IF(source LIKE '%internet%' OR source LIKE '%web%' OR source LIKE '%google%', '"Web"',
        IF(source LIKE '%friend%' OR source LIKE '%mouth%', '"Word of Mouth"',
        IF(source LIKE '%facebook%', '"Facebook"',
        IF(source LIKE '%twitter%', '"Twitter"',
        IF(source LIKE '%ujima%', '"Ujima Project"',
        IF(source LIKE '%various%' OR source LIKE '%multiple%', '"Various"',
        IF(source LIKE '%article%' OR source LIKE '%gazette%' OR source LIKE '%recorder%' OR source LIKE '%news%', '"News"',
        IF(1, '"(Other)"', LEFT($source, LOCATE(' ', CONCAT($source, ' ')) - 1))
        )))))))),
        IF((SELECT SUM(1) FROM users h WHERE h.helper=u.helper) > 4, helperName, 'Invited by Other')
      ) AS origin,
      SUM(IF(:IS_OK, 1, 0)) AS member,
      SUM(IF(:IS_OK OR uid IS NULL, 0, 1)) AS stuck,
      SUM(IFNULL(r.amount, 0) * IF(r.period='Y', 1, IF(r.period='Q', 4, IF(r.period='M', 12, IF(r.period='W', 52, 0)))))
        / SUM(IF(:IS_OK OR data LIKE '%"donate";b:1%', 1, 0)) AS avgYearlyGiftDollars,
      SUM(IF(:IS_ROUNDUP, 1, 0))
        / SUM(IF(:IS_OK OR data LIKE '%"prefs";b:1%', 1, 0)) AS avgRoundup,
      SUM((SELECT SUM(1) FROM r_invites i WHERE i.inviter=u.uid))
        / SUM(IF(:IS_OK, 1, 0)) AS eachInvited
    FROM (
      SELECT uid,helper,source,flags,data,created FROM users
      UNION SELECT NULL AS uid, NULL AS helper, source, 0 aS flags, '' AS data, created FROM signup s0 
        WHERE (SELECT 1 FROM users u0 WHERE u0.fullName=s0.fullName LIMIT 1) IS NULL
    ) u
    LEFT JOIN r_recurs r ON r.payer=u.uid AND r.payee=:CGID AND r.ended=0
    LEFT JOIN (SELECT uid AS helper,fullName AS helperName FROM users) h USING(helper)
    GROUP BY origin
    ORDER BY member
X
  . '~For each origin, members = the number of members that came from there; stuck = the number of people who started to join but stopped; avgRoundup = the average number who chose roundups; eachInvited = the average number of people members from that origin have invited. Origins in quotes are what the member told us in the "Referred by" field.',
  
];

//    IFNULL(u.fullName, REPLACE(SUBSTRING(r.data, 27, LOCATE('\"', SUBSTRING(r.data, 28))), '\"', '')) AS name,
//    SUBSTRING_INDEX(SUBSTRING_INDEX(r.data, '\"', 12), '\"', -1) AS phone,

/**
 * Ideas for other queries

// when people signed up recently
eachA(function ($a) {
  $dt = u\fmtDate($a->created, TRUE);
  $status = $a->ok ? '' : ' (in process)';
///  echo "$dt $a->fullName$status\n";
}, 'created > :NOW - 3 * :MONTH_SECS ORDER BY created DESC');

// show food fund percentages and estimated monthly donations
$rows = db\rows('fullName', 'users u', ':U_FOODY=1', ['iid' => '2']);
$sum = db\sum('amt', 'txs', ':T_FOODY AND (SELECT food FROM users WHERE uid=uid1)>0 AND created>:NOW-:MONTH_SECS');

// Who still owes CG a phone deposit
SELECT fullName, nvid AS invoice, i.purpose FROM users u 
LEFT JOIN (SELECT uid1 AS uid FROM txs_prime t0 WHERE t0.uid2=:CGID AND amt>=20) t USING(uid)
LEFT JOIN (SELECT nvid,payer AS uid,purpose FROM r_invoices i0 
  WHERE i0.payee=:CGID AND amount>=20 AND status<:TX_PAID) i USING(uid)
WHERE u.:IS_CO AND t.uid IS NULL ORDER BY community

// Individual Cards to Print
  eachA(function ($a) {
    $gift = $a->giftsDesc();
    $bank = $a->hasBank ? '' : t('(NO BANK)');
/** / echo "$a->mainQid $a->fullName: $gift -- $a->postalAddr $bank<" . 'br><' . 'br>';

//    list ($address, $city, $state, $zip) = u\parseAddr($a->postalAddr);
//    echo "$a->fullName;$address;$city, $state $zip<" . 'br>';
  }, 'NOT :IS_CO AND NOT :IS_CARDED AND :IS_MEMBER ORDER BY fullName');
  
  // duplicated by Monthly Company Income and Payments
  $recentSecs = 'GREATEST(1, LEAST(6*:MONTH_SECS, :NOW-u.activated))'; // for company monthly sales and payments (past 6 months)
  t('Company Monthly Sales and Payments') =>
    "SELECT fullName AS company, 
      sum(IF(uid2=uid, amt2, 0))*MONTH_SECS/$recentSecs as sales, 
      sum(IF(uid1=uid, amt1, 0))*MONTH_SECS/$recentSecs AS payments
    FROM users u JOIN txs t ON uid IN (uid1, uid2) 
    WHERE u.IS_CO AND u.activated AND t.created>:NOW-$recentSecs
    ORDER BY sales DESC",  
*/
