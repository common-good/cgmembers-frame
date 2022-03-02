<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const CC_SHARE = 0.82; // what fraction of consumer transactions are digital
const CC_RATE = 0.02; // rough estimate of average credit card fees for consumer purchases

/**
 * Show statistics.
 */
function formStats($form, &$sta, $args = '') {
  global $mya;
  extract(just('start end', $args, [strtotime('January 1'), strtotime('December 31') + DAY_SECS - 1]));

  $title = t('Statistics');
  $subtext = t('System-wide from %start to %end', 'start end', fmtDt($start), fmtDt($end));

  $when = "BETWEEN :start AND :end";
  $timeSubs = compact(ray('start end'));
  $isNotForProfit = 'IN (SELECT uid FROM users LEFT JOIN u_company USING (uid) WHERE coType=:CO_NONPROFIT XOR (coFlags&(1<<:CO_FLIP))<>0)';

  $flds = <<<X
    COUNT(DISTINCT uid1) AS numDonors,
    COUNT(DISTINCT uid2) AS numOrgsSupported,
    SUM(1) AS numDonations,
    SUM(amt) AS donations,
    SUM(IF(uid2=:UID_ROUNDUPS, 1, 0)) AS numRoundups,
    SUM(IF(uid2=:UID_ROUNDUPS, amt, 0)) AS roundups,
    SUM(IF(uid2=:UID_CRUMBS, 1, 0)) AS numCrumbs,
    SUM(IF(uid2=:UID_CRUMBS, amt, 0)) AS crumbs
X;

  $res = db\get($flds, 'txs_noreverse', "(uid2 $isNotForProfit OR uid2 IN (:UID_ROUNDUPS, :UID_CRUMBS)) AND amt>0 AND created $when", $timeSubs); // includes roundups and crumbs and all donations to CG
  
  $flds = <<<X
    COUNT(DISTINCT uid1) AS numCGDonors,
    SUM(1) AS numCGDonations,
    SUM(amt) AS cgDonations
X;

  $except = "for2 NOT REGEXP('software|sponsorship|loan|repay')";
  $res += db\get($flds, 'txs_noreverse', "uid2=:CGID AND amt>0 AND $except AND uid1 NOT :CANONIC AND created $when", $timeSubs);
  
  $amt = r\annualAmt();
  $flds = <<<X
    SUM($amt) AS cgAnnualGifts,
    SUM(1) AS cgAnnualDonors,
    SUM(IF($amt >= 1000, 1, 0)) AS cgAnnualDonors1k,
    SUM(IF($amt BETWEEN 500 AND 999.99, 1, 0)) AS cgAnnualDonors500,
    SUM(IF($amt BETWEEN 200 AND 499.99, 1, 0)) AS cgAnnualDonors200,
    SUM(IF($amt BETWEEN 100 AND 199.99, 1, 0)) AS cgAnnualDonors100,
    SUM(IF($amt BETWEEN 50 AND 99.99, 1, 0)) AS cgAnnualDonors50,
    SUM(IF($amt BETWEEN 0 AND 49.99, 1, 0)) AS cgAnnualDonorsSmall
X;

  $res += db\get($flds, 'tx_timed tm', 'action=:ACT_PAY AND tm.to=:CGID AND IFNULL(end, NOW()+1)>NOW()');
  
  $flds = <<<X
    COUNT(DISTINCT uid1) AS numBuyers,
    COUNT(DISTINCT uid2) AS numSellers,
    SUM(1) AS numTxs,
    SUM(amt) AS volume
X;

  $res += db\get($flds, 'txs_noreverse', "uid2 NOT $isNotForProfit AND type=:E_PRIME AND amt>0 AND created $when", $timeSubs); // buys
  $res['ccFeesSaved'] = CC_SHARE * CC_RATE * $res['volume'];
  
  say(pr($res));
    
  return cgform(compact(ray('title subtext')));
}

function formStats_validate($form, &$sta) {
  global $mya;
  extract(just('text', $sta['input'], NULL));

  if ($err = u\badAmount($text)) return say($err, 'text');
  u\preRay(compact('text'), $sta['input']);
}

function formStats_submit($form, &$sta) {
  global $mya;
  extract(just('text', $sta['input'], NULL));

  $mya->update(compact('text'));

  return go('dashboard');
}
