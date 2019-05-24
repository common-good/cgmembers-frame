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

$bizQ = 'SELECT uid as payee, goods, SUM(amount) AS tot, COUNT(*) AS cnt FROM tx_entries JOIN tx_hdrs USING (xid) WHERE goods=:FOR_GOODS AND created BETWEEN';
$gifts = "r.amount * IF(period='M', 12, IF(period='Q', 4, 1))";

$queries = [
  2 . 'Most Trusted Members' => 
    "SELECT fullName, email AS email, phone, COUNT(b.voter) AS proxiedFor, trust FROM users u LEFT JOIN r_ballots b ON b.proxy=u.uid AND b.created>:REQUEST_TIME-:YEAR_SECS WHERE u.community=:ctty AND u.:IS_OK AND NOT u.:IS_CO GROUP BY IFNULL(b.proxy, u.uid) ORDER BY COUNT(b.voter) DESC, u.trust DESC",
    
  2 . 'Individuals Paid By A Company' =>
    "SELECT DISTINCT main.fullName AS company, other.fullName AS pays, 
            IF(r.:IS_EMPLOYEE, 'employee', IF(r.:IS_OWNER, 'owner', 
                                              IF(r.:IS_CUSTOMER, 'customer', ''))) AS relation
     FROM r_relations r 
          JOIN tx_entries payer ON r.main=payer.uid
          JOIN tx_hdrs t USING (xid)
          JOIN tx_entries payee USING (xid)
          JOIN users main ON payer.uid=main.uid JOIN users other ON payee.uid=main.uid
          LEFT OUTER JOIN tx_hdrs tRev ON tRev.reversesXid=t.xid
     WHERE payer.entryType=':ENTRY_PAYER' AND payee.entryType=':ENTRY_PAYEE'
           AND IFNULL(t.reversesXid,0)=0 AND IFNULL(tRev.xid,0)=0
           AND NOT other.:IS_CO AND other.:IS_OK 
           AND main.community=:ctty AND main.:IS_CO AND main.:IS_OK
     ORDER BY main.community,main.fullName,relation DESC,other.fullName",
  
  2 . 'Transaction Totals to Date' =>
    "SELECT COUNT(*) AS txCount, SUM(ABS(payer.amount)) AS txDollars, 
            SUM(IF(u2.:IS_CO AND (NOT u1.:IS_CO OR channel=:TX_POS), payee.amount, 0)) AS inPersonDollars,
            SUM(IF(u2.:IS_CO AND (NOT u1.:IS_CO OR channel=:TX_POS), .85*.02*payee.amount, 0)) AS ccFeesSaved
    FROM tx_entries payer JOIN tx_hdrs t USING (xid) JOIN tx_entries payee USING (xid)
         JOIN users u1 ON u1.uid=payer.uid JOIN users u2 ON u2.uid=payee.uid
    WHERE payer.entryType=':ENTRY_PAYER' AND payee.entryType=':ENTRY_PAYEE'
          AND (u1.community=:ctty OR u2.community=:ctty)",
    

  B_CO . 'Monthly Business Income (30 day totals)' => 
    "SELECT LEFT(u.fullName, 42) AS company,
            t0.tot AS `total $`, t0.cnt AS `count`,
            t6.tot AS `total $ 6mos ago`, t6.cnt AS `count6`,
            t12.tot AS `total $ 12mos ago`, t12.cnt AS `count12`
     FROM users u 
     LEFT JOIN ($bizQ :NOW-30*:DAY_SECS AND :NOW GROUP BY payee) t0 ON t0.payee=u.uid
     LEFT JOIN ($bizQ :NOW-(180+30)*:DAY_SECS AND :NOW-180*:DAY_SECS GROUP BY payee) t6 ON t6.payee=u.uid
     LEFT JOIN ($bizQ :NOW-:YEAR_SECS-30*:DAY_SECS AND :NOW-:YEAR_SECS GROUP BY payee) t12 ON t12.payee=u.uid
     WHERE u.community=:ctty AND :IS_CO AND :IS_OK AND t0.cnt>0 AND fullName NOT LIKE '%457%' ORDER BY u.fullName",

  'Positive and Negative Balance Totals' => 
    "SELECT c.fullName AS community, 
     SUM(IF(u.balance<0, 1, 0)) AS negativeBalanceCount,
     SUM(IF(u.balance<0, u.balance, 0)) AS negativeBalanceTotal,
     SUM(IF(u.balance>=0, 1, 0)) AS positiveBalanceCount,
     SUM(IF(u.balance>=0, u.balance, 0)) AS positiveBalanceTotal
     FROM users u LEFT JOIN users c ON c.uid=u.community 
     WHERE IFNULL(u.jid,0)=0 AND u.:IS_OK AND u.uid NOT BETWEEN 0 AND (:CANONIC_ACCTS-1)
     GROUP BY c.fullName",
  
  'Company and Member Balances and Credit Lines' =>
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
  'Actual Donations to Common Good and Community' =>
    "SELECT CONCAT_WS(' ', YEAR(FROM_UNIXTIME(t.created)), MONTHNAME(FROM_UNIXTIME(t.created))) AS month,
    SUM(IF(t.:IS_RECURS,payee.amount,0)) AS recurring,
    SUM(IF(payee.uid=:CG_ROUNDUPS_UID,payee.amount,0)) AS roundups,
    SUM(IF(payee.uid=:CG_CRUMBS_UID,payee.amount,0)) AS crumbs,
    SUM(IF(t.:IS_GIFT AND payee.uid NOT IN (:CG_ROUNDUPS_UID, :CG_CRUMBS_UID) AND NOT t.:IS_RECURS,payee.amount,0)) AS other
    FROM tx_hdrs t JOIN tx_entries payer USING (xid) JOIN users u USING (uid) 
         JOIN tx_entries payee USING(xid)
    WHERE payer.entryTYPE=':ENTRY_PAYER' AND payee.entryType=':ENTRY_PAYEE' 
          AND u.community=:ctty AND (t.:IS_GIFT OR payee.uid IN (:CG_ROUNDUPS_UID, :CG_CRUMBS_UID) OR t.:IS_RECURS)
    GROUP BY YEAR(FROM_UNIXTIME(t.created)) DESC, MONTH(FROM_UNIXTIME(t.created)) DESC",

  'Expected Member Donations to CG and Community' =>
    $mGifts =
    "SELECT c.fullName AS community, 
            COUNT(*) AS members,
            SUM($gifts) AS yearly$,
            SUM(IF(u.:IS_ROUNDUP, 1, 0)) AS roundupy,
            SUM(IF(u.crumbs>0, 1, 0)) AS crumby,
            AVG(IF(u.crumbs>0, u.crumbs, NULL)) AS avgCrumbs
    FROM users u LEFT JOIN r_recurs r ON (r.payer=u.uid AND r.ended<>0 AND r.payee=:CGID) LEFT JOIN users c ON c.uid=u.community
    WHERE u.:IS_OK AND NOT u.:IS_CO AND u.uid NOT BETWEEN 0 AND (:CANONIC_ACCTS-1)
    GROUP BY u.community
    ~roundupy = number who do Roundups, crumby = number who do Crumb donations",

  'Expected Company Donations to CG and Community' =>
    strtr($mGifts, ['AS members' => 'AS companies', 'NOT u.:IS_CO' => 'u.:IS_CO']),
  
];

//    IFNULL(u.fullName, REPLACE(SUBSTRING(r.data, 27, LOCATE('\"', SUBSTRING(r.data, 28))), '\"', '')) AS name,
//    SUBSTRING_INDEX(SUBSTRING_INDEX(r.data, '\"', 12), '\"', -1) AS phone,

