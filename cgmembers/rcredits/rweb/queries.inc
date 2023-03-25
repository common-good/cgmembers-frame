<?php

use CG as r;
use CG\Util as u;

/**
 * @file
 * List of queries for the Community and Money Data page on the Community menu (for members only).
 * Queries beginning with a digit are for admins or companies only: 0=admV 1=admRegion 2=admin B_CO=companies
 * These special parameters are recognized:
 *  :ctty  the current account's community ID
 *  :myid  the current account's record ID
 * Note: query names cannot contain any of the following characters: /&? (because they confuse the url).
 * For queries with no SQL show here, see the getReport() function in forms/data.inc
 */

global $mya;

$bizQ = 'SELECT uid as payee, goods, SUM(IF(amount>0,amount,0)) AS sales, SUM(IF(amount<0,amount,0)) AS payments, COUNT(*) AS cnt FROM tx_entries JOIN tx_hdrs USING (xid) WHERE goods=:FOR_GOODS AND created BETWEEN';
$gifts = r\annualAmt();
$fsFeeCat = r\nick2cat('FS-FEE');

if ($mya and $mya->admin) {
  $source = "TRIM(REPLACE(REPLACE(source,'knows: self ()', ''), '-', ' '))";
  $fy9 = strtotime('July 1') - 1; if ($fy9 > now()) $fy9 = strtotime('-1 year', $fy9);
  $fy0 = strtotime('-1 year', $fy9) + 1;
  $y9 = strtotime('January 1') - 1;
  $y0 = strtotime('-1 year', $y9) + 1;
  $cgUids = join(',', r\cgUids());
  $justCgUids = join(',', r\justCgUids());
  $qtrDt1 = u\qtrDt($qtrDt2 = u\qtrDt() - 1);
  $perMo = ':MONTH_SECS/LEAST(:NOW-u.activated, 6*:MONTH_SECS)';
  
  $queries = [
  1 . t('Dollar Pool Liability') => 
    'SELECT SUM(IF(uid NOT IN :EXTERNAL_UIDS, IF(jid, balance/2, balance), 0)) AS balTotal,
     SUM(IF(uid IN :EXTERNAL_UIDS, balance, 0)) AS netIn,
     SUM(IF(jid, balance/2, balance)) AS discrepancy
     FROM users',
     
  3 . t('Company participation') =>
    "SELECT LEFT(FROM_UNIXTIME(activated), 10) AS since, fullName, CONCAT(city, ', ', abbreviation) AS location,
       IF(crumbs=0, '-', CONCAT(ROUND(100*crumbs, 2), '%')) AS crumbs,
       ROUND($perMo * SUM(IF(h.created < :NOW-6*:MONTH_SECS, 0, GREATEST(e.amount, 0))), 2) AS `sales/mo`, 
       ROUND($perMo * SUM(IF(h.created < :NOW-6*:MONTH_SECS, 0, GREATEST(-e.amount, 0))), 2) AS `payments/mo`,
       ROUND($perMo * COUNT(IF(h.created < :NOW-6*:MONTH_SECS, 0, ABS(e.amount))), 2) AS `tx/mo`, 
       ROUND($perMo * SUM(IF(h.created < :NOW-6*:MONTH_SECS, 0, IF(other.entryType=:E_REBATE, -e.amount, 0))), 2) AS `rebates/mo`,
       ROUND($perMo * SUM(IF(h.created < :NOW-6*:MONTH_SECS, 0, IF(other.cat=$fsFeeCat, GREATEST(-e.amount, 0), 0))), 2) AS `FS fees/mo`,
       ROUND($perMo * SUM(IF(h.created < :NOW-6*:MONTH_SECS, 0, IF(otherUid IN ($justCgUids), GREATEST(-e.amount, 0), 0))), 2) AS `to CG/mo`,
       ROUND(SUM(IF(otherUid IN ($justCgUids), GREATEST(-e.amount, 0), 0)), 2) AS `to CG ever`
       FROM tx_hdrs h JOIN tx_entries e USING (xid) 
         JOIN users u USING (uid) 
         LEFT JOIN r_states s ON s.id=u.state
         JOIN (SELECT id, uid AS otherUid, entryType, cat FROM tx_entries) other ON other.id=-e.id
       WHERE u.:IS_CO AND u.:IS_OK AND u.uid NOT IN ($justCgUids, :UID_OUTER, :UID_BANK)
       GROUP BY e.uid
       ORDER BY `to CG/mo` DESC, `sales/mo` DESC",
  
  1 . t('Community Fund Balance By Community') => 
    'SELECT c.fullName AS community, SUM(IF(u.jid, u.balance/2, u.balance)) AS balance 
     FROM users u LEFT JOIN users c ON c.uid=u.community 
     WHERE u.uid NOT IN :EXTERNAL_UIDS
     GROUP BY u.community
     ORDER BY c.fullName',
  
  1 . t('Income & Expense Report') => '/sadmin/iereport',
  
  1 . t('Major Gifts') => 
    "SELECT IF(p.pid IS NULL, u.fullName, p.fullName) AS name, u2.name AS `to`,
      SUM(IF(t.:IS_GIFT, amt, 0)) AS gifts,
      SUM(IF(t.:IS_GIFT, 0, amt)) AS nonGifts, u.:IS_INFLUENCER AS marked, p.pid IS NULL AS member,
      IF(p.pid IS NULL, CONCAT(u.city, ', ', s.abbreviation, ' ', u.zip), CONCAT(IFNULL(p.address, ''), ', ', p.city, ', ', s.abbreviation, ' ', p.zip)) AS address
      FROM txs t LEFT JOIN txs2 t2 USING (xid) 
      LEFT JOIN users u ON u.uid=t.uid1 
      LEFT JOIN users u2 ON u2.uid=t.uid2
      LEFT JOIN people p ON p.pid=t2.pid AND p.pid>0
      LEFT JOIN r_states s ON s.id=IF(p.pid IS NULL, u.state, p.state)
      WHERE t.created BETWEEN $y0 AND $y9 AND t.uid2 IN ($justCgUids) AND t.uid1 NOT IN ($justCgUids)
      AND t.cat2 NOT BETWEEN 1150 AND $fsFeeCat
      GROUP BY name
      HAVING gifts>=99
      ORDER BY gifts DESC
    ". t('~<p>Total gifts specifically to %PROJECT in its most recently ended calendar year from top donors.'),

  1 . t('Gifts From "Disqualified Persons"') => 
    "SELECT IF(p.pid IS NULL, u.fullName, p.fullName) AS name, SUM(IF(t.:IS_GIFT, amt, 0)) AS giftTotal, 
      SUM(IF(amt>=1000 AND t.:IS_GIFT, amt, 0)) AS over999,
      SUM(IF(t.:IS_GIFT, 0, amt)) AS nonGift, u.:IS_INFLUENCER AS marked, p.pid IS NULL AS member,
      IF(p.pid IS NULL, CONCAT(u.city, ', ', s.abbreviation, ' ', u.zip), CONCAT(IFNULL(p.address, ''), ', ', p.city, ', ', s.abbreviation, ' ', p.zip)) AS address
      FROM txs t LEFT JOIN txs2 t2 USING (xid) 
      LEFT JOIN users u ON u.uid=t.uid1 
      LEFT JOIN people p ON p.pid=t2.pid AND p.pid>0
      LEFT JOIN r_states s ON s.id=IF(p.pid IS NULL, u.state, p.state)
      WHERE t.created BETWEEN $fy0 AND $fy9 AND t.uid2 IN ($cgUids) AND t.uid1 NOT IN ($cgUids)
      GROUP BY name
      HAVING over999>=:SUBSTANTIAL_GIFT OR marked
      ORDER BY giftTotal DESC
    ". t('~<p>Total gifts in %PROJECT\'s most recently ended fiscal year (July to June) from Influencers or that exceed $5,000 &mdash; ignoring separate gifts of less than $1,000 as allowed for IRS Form 990 Schedule B: "Include all separate and independent gifts that are $1,000 or more Gifts that are less than $1,000 may be disregarded."</p><p>"marked" means the person is marked in the database as an influencer &mdash; that is, someone who might have significant influence over the affairs of the organization because of their position (including being a board member, former board member, key employee, or relative of such a person). Otherwise the person\'s "influence" is because they donated a lot of money.'),
    // in advance: $gift and (($recurs ? db\get(r\annualAmt(), 'tx_timed tm', ray('id', $recurs)) : amt) >= SUBSTANTIAL_GIFT)
    
  1 . t('Food Fund Contributions') =>
    "SELECT COUNT(uid) AS participants, SUM(amt) AS monthly FROM (
      SELECT uid, SUM(food*amt) AS amt, IF(SUM(amt)=0, 0, 1) AS active  
      FROM (SELECT uid,food FROM users WHERE food>0) u 
      LEFT JOIN (SELECT uid1 AS uid, amt FROM txs_noreverse WHERE :T_FOODY AND created>:NOW-:MONTH_SECS) t USING(uid)
      GROUP BY uid
    ) x",
    
  1 . t('New States Last Quarter (for bank)') =>
    "SELECT DISTINCT s.abbreviation FROM users u JOIN r_states s ON u.state=s.id
      WHERE activated BETWEEN $qtrDt1 AND $qtrDt2
      AND state NOT IN (SELECT DISTINCT state FROM users WHERE activated AND activated<$qtrDt1)",
  
  1 . t('New Members Last Quarter (for bank)') =>
    "SELECT SUM(IF(:IS_CO, 0, 1)) AS members,
            SUM(IF(:IS_CO, 1, 0)) AS companies
      FROM users u WHERE activated BETWEEN $qtrDt1 AND $qtrDt2",
      
  2 . t('Positive and Negative Balance Totals') => 
    "SELECT c.fullName AS community, 
     SUM(IF(u.balance<0, 1, 0)) AS negCount,
     SUM(IF(u.balance<0, u.balance, 0)) AS negativeBalTotal,
     SUM(IF(u.balance>=0, 1, 0)) AS posCount,
     SUM(IF(u.balance>=0, u.balance, 0)) AS positiveBalTotal
     FROM users u LEFT JOIN users c ON c.uid=u.community 
     WHERE IFNULL(u.jid,0)=0 AND u.:IS_OK AND u.uid NOT :CANONIC AND u.uid NOT IN ($cgUids)
     GROUP BY c.fullName",
  
  2 . t('Most Trusted Members') => 
    "SELECT fullName, email AS email, phone, COUNT(b.voter) AS proxiedFor, trust FROM users u LEFT JOIN r_ballots b ON b.proxy=u.uid AND b.created>:REQUEST_TIME-:YEAR_SECS WHERE u.community=:ctty AND u.:IS_OK AND NOT u.:IS_CO GROUP BY IFNULL(b.proxy, u.uid) ORDER BY COUNT(b.voter) DESC, u.trust DESC, fullName",
    
  2 . 'Individuals Paid By A Company' =>
    "SELECT DISTINCT main.fullName AS company, other.fullName AS pays, 
            IF(r.:IS_EMPLOYEE, 'employee', IF(r.:IS_OWNER, 'owner', 
                                              IF(r.:IS_CUSTOMER, 'customer', ''))) AS relation
     FROM u_relations r
       JOIN txs_noreverse t ON t.uid1=r.main
       JOIN users main ON main.uid=r.main 
       JOIN users other ON other.uid=r.other
     WHERE NOT other.:IS_CO AND other.:IS_OK 
           AND main.community=:ctty AND main.:IS_CO AND main.:IS_OK
     ORDER BY main.community,main.fullName,relation DESC,other.fullName",
  
  2 . 'Transaction Totals to Date' =>
    "SELECT COUNT(*) AS txCount, SUM(ABS(amt)) AS txDollars, 
            SUM(IF(u2.:IS_CO AND (NOT u1.:IS_CO OR channel=:TX_APP), amt, 0)) AS inPersonDollars,
            SUM(IF(u2.:IS_CO AND (NOT u1.:IS_CO OR channel=:TX_APP), .85*.02*amt, 0)) AS ccFeesSaved
    FROM txs t JOIN users u1 ON u1.uid=uid1 JOIN users u2 ON u2.uid=uid2
    WHERE :ctty IN (u1.community, u2.community)",
    
  2 . 'Most Active Company Accounts' => <<< X
    SELECT DISTINCT c.fullName AS company, p.fullName AS contact, c.phone, c.email, c.city, c.zip, MID(FROM_UNIXTIME(lastTx), 1, 10) AS lastTx, IF(c.:IS_OK, 'open', 'NO') AS open
    FROM (SELECT uid, MAX(created) AS lastTx FROM tx_entries JOIN tx_hdrs USING(xid) GROUP BY uid) t
    JOIN users c ON c.uid=t.uid
    JOIN u_relations r ON r.main=c.uid
    JOIN users p ON p.uid=r.other
    WHERE c.:IS_CO GROUP BY c.uid
    ORDER BY lastTx DESC
X
  ,
  
    2 . 'Most Active Individual Accounts' => <<< X
    SELECT DISTINCT fullName AS name, phone, email, city, zip, MID(FROM_UNIXTIME(lastTx), 1, 10) AS lastTx, IF(u.:IS_OK, 'open', 'NO') AS open
    FROM (SELECT uid, MAX(created) AS lastTx FROM tx_entries JOIN tx_hdrs USING(xid) GROUP BY uid) t
    JOIN users u USING(uid)
    WHERE NOT u.:IS_CO ORDER BY lastTx DESC
X
  ,
  
  2 . t('Where Do Our Members Hear About CG') => <<< X
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
      SUM(IF(u.:IS_OK, 1, 0)) AS member,
      SUM(IF(u.:IS_OK OR uid IS NULL, 0, 1)) AS stuck,
      SUM(IFNULL(tm.amount, 0) * perYear(tm.period, tm.periods))
        / SUM(IF(u.:IS_OK OR data LIKE '%"donate";b:1%', 1, 0)) AS avgYearlyGiftDollars,
      SUM(IF(u.:IS_ROUNDUP, 1, 0))
        / SUM(IF(u.:IS_OK OR data LIKE '%"prefs";b:1%', 1, 0)) AS avgRoundup,
      SUM((SELECT SUM(1) FROM r_invites i WHERE i.inviter=u.uid))
        / SUM(IF(u.:IS_OK, 1, 0)) AS eachInvited
    FROM users u
    LEFT JOIN tx_timed tm ON tm.from=u.uid AND tm.to=:CGID AND tm.end IS NULL
    LEFT JOIN (SELECT uid AS helper,fullName AS helperName FROM users) h USING(helper)
    WHERE u.uid>0 AND u.uid NOT :CANONIC
    GROUP BY origin
    ORDER BY member
X
  . '~For each origin, members = the number of members that came from there; stuck = the number of people who started to join but stopped; avgRoundup = the average number who chose roundups; eachInvited = the average number of people members from that origin have invited. Origins in quotes are what the member told us in the "Referred by" field.',
  ];
}

$queries = array_merge(nn($queries, []), [
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
  
  t('Company and Member Balances and Credit Lines') =>
    "SELECT c.fullName AS community,
            SUM(IF(u.:IS_CO, 0, u.balance)) AS memBals,
            SUM(IF(u.:IS_CO, 0, -u.floor)) AS memCredit, 
            SUM(IF(u.:IS_CO, 0, IF(u.:IS_REFILL, u.minimum, 0))) AS memTargetBals,
            SUM(IF(u.:IS_CO, u.balance, 0)) AS coBals,
            SUM(IF(u.:IS_CO, -u.floor, 0)) AS coCredit, 
            SUM(IF(u.:IS_CO, IF(u.:IS_REFILL, u.minimum, 0), 0)) AS coTargetBals
     FROM users u LEFT JOIN users c ON c.uid=u.community
     WHERE IFNULL(u.jid,0)=0 AND c.uid<0 AND u.uid NOT :CANONIC GROUP BY c.fullName",

//     SUM(IF(t.:IS_GIFT OR t.:IS_ROUNDUPS OR t.:IS_CRUMBS OR t.recursId,0,amount)) AS `payments to CG`
  t('Actual Donations to CG and Community') =>
    "SELECT CONCAT_WS(' ', YEAR(FROM_UNIXTIME(t.created)), MONTHNAME(FROM_UNIXTIME(t.created))) AS month,
    SUM(IF(uid2=:UID_REGULARS,amt,0)) AS recurring,
    SUM(IF(uid2=:UID_ROUNDUPS,amt,0)) AS roundups,
    SUM(IF(uid2=:UID_CRUMBS,amt,0)) AS crumbs,
    SUM(IF(t.:IS_GIFT AND uid2 NOT IN (:UID_REGULARS, :UID_ROUNDUPS, :UID_CRUMBS) AND t.recursId IS NULL, amt, 0) AND cat2<>$fsFeeCat) AS `stepups/other`,
    SUM(IF(cat2=$fsFeeCat,amt,0)) AS `FS fees`
    FROM txs t JOIN users u ON u.uid=t.uid1
    WHERE u.community=:ctty 
    AND ((uid2=:CGID AND t.:IS_GIFT) OR uid2 IN (:UID_REGULARS, :UID_ROUNDUPS, :UID_CRUMBS) OR cat2=$fsFeeCat)
    GROUP BY YEAR(FROM_UNIXTIME(t.created)) DESC, MONTH(FROM_UNIXTIME(t.created)) DESC",

  t('Expected Member Donations to CG and Community') =>
    $mGifts =
    "SELECT c.fullName AS community, 
            COUNT(*) AS members,
            SUM($gifts) AS yearly$,
            SUM(IF(u.:IS_ROUNDUP, 1, 0)) AS roundupy,
            SUM(IF(u.crumbs>0, 1, 0)) AS crumby,
            AVG(IF(u.crumbs>0, u.crumbs, NULL)) AS avgCrumbs
    FROM users u LEFT JOIN tx_timed tm ON (tm.from=u.uid AND tm.end IS NOT NULL AND tm.to=:CGID) LEFT JOIN users c ON c.uid=u.community
    WHERE u.:IS_OK AND NOT u.:IS_CO AND u.uid NOT :CANONIC
    GROUP BY u.community
    ~roundupy = number who do Roundups, crumby = number who do Crumb donations",

  t('Expected Company Donations to CG and Community') =>
    strtr($mGifts, ['AS members' => 'AS companies', 'NOT u.:IS_CO' => 'u.:IS_CO']),
  
]);

//    IFNULL(u.fullName, REPLACE(SUBSTRING(r.data, 27, LOCATE('\"', SUBSTRING(r.data, 28))), '\"', '')) AS name,
//    SUBSTRING_INDEX(SUBSTRING_INDEX(r.data, '\"', 12), '\"', -1) AS phone,

/**
 * Ideas for other queries

Inviters
select fullName, CONCAT(city, ', ', abbreviation) AS location, COUNT(*) AS count, IF(:IS_OK, '', 'NO') AS active FROM r_invites i join users u ON u.uid=i.inviter LEFT JOIN r_states s ON s.id=u.state where uid<>26742000000001 GROUP by uid order by fullName

// when people signed up recently
eachA(function ($a) {
  $dt = fmtDt($a->created);
  $status = $a->ok ? '' : ' (in process)';
///  echo "$dt $a->fullName$status\n";
}, 'created > :NOW - 3 * :MONTH_SECS ORDER BY created DESC');

// show food fund percentages and estimated monthly donations
$rows = db\rows('fullName', 'users u', ':U_FOODY=1');
$sum = db\sum('amt', 'txs', ':T_FOODY AND (SELECT food FROM users WHERE uid=uid1)>0 AND created>:NOW-:MONTH_SECS');

// Who still owes CG a phone deposit
SELECT fullName, nvid AS invoice, i.purpose FROM users u 
LEFT JOIN (SELECT uid1 AS uid FROM txs_prime t0 WHERE t0.uid2=:CGID AND amt>=20) t USING(uid)
LEFT JOIN (SELECT nvid,payer AS uid,purpose FROM tx_requests i0 
  WHERE i0.payee=:CGID AND amount>=20 AND status<:TX_PAID) i USING(uid)
WHERE u.:IS_CO AND t.uid IS NULL ORDER BY community

// Individual Cards to Print
  eachA(function ($a) {
    $gift = $a->giftsDesc();
    $bank = $a->hasBank ? '' : t('(NO BANK)');
/** / echo "$a->mainQid $a->fullName: $gift -- $a->postalAddr $bank<" . 'br><' . 'br>';

//    list ($address, $city, $state, $zip) = u\parseAddr($a->postalAddr);
///    echo "$a->fullName;$address;$city, $state $zip<" . 'br>';
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
