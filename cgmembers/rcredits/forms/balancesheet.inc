<?php

namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * @file
 * Display a balance sheet for the account's community.
 */

function formBalanceSheet($form, &$sta, $args = '') {

  global $mya;
  $serverId = r\serverUid();

  $negReservePct = getv('negReserve', .05);
  $backingReservePct = getv('backingReserve', .50);

  if  ($mya and $mya->community != $serverId and $mya->id != 1) {
    $cttyA = $mya->cttyA;
    $ctty = $mya->community;
    $subs = compact('ctty');

    $title = t('Community Balance Sheet');
    $subtext = t('for ') . $cttyA->fullName;
    $cgBalance = $cttyA->balance;
    $pool = db\sum('amount', 'r_usd t JOIN users u ON u.uid=t.payee', 'u.community=:ctty', $subs);
    $investments = db\sum('shares*price', 'r_investments i JOIN r_shares s USING (vestid)', 'clubid=:ctty', $subs);
    $invReserve = -db\sum('shares*price*reserve', 'r_investments i JOIN r_shares s USING (vestid)', 'clubid=:ctty', $subs);
    $negatives = -db\sum('balance', 'users', 'uid<>:ctty AND balance<0 AND community=:ctty AND IFNULL(jid,0)=0', $subs);
    $negReserve = -$negatives * $negReservePct;
    $backing = db\sum('backing', 'users', 'community=:ctty', $subs);
    $backingReserve = -$backing * $backingReservePct;
    $invoicesFromUs = db\sum('amount', 'r_invoices', 'payee=:ctty AND status IN (:TX_PENDING,:TX_APPROVED)', $subs);
    
    $balances = db\sum('balance', 'users', 'uid<>:ctty AND balance>0 AND community=:ctty AND IFNULL(jid,0)=0', $subs);
    $invoicesToUs = db\sum('amount', 'r_invoices', 'payer=:ctty AND status IN (:TX_PENDING,:TX_APPROVED)', $subs);
    $otherAssets = $cttyA->otherAssets;
    $otherLiabilities = $cttyA->otherLiabilities;
  } else {

    $title = t('%PROJECT Balance Sheet');
    $subtext = t('for all participating communities (totals)');
    $cgBalance = db\sum('balance', 'users', 'uid<0');
    /* $pool = db\sum('amount', 'r_usd'); */
    $pool = 0-db\sum('balance', 'users', 'uid BETWEEN :MINIMUM_BANK_UID AND :MAXIMUM_BANK_UID');
    $investments = db\sum('shares*price', 'r_investments i JOIN r_shares s USING (vestid)', 'clubid<0');
    $invReserve = -db\sum('shares*price*reserve', 'r_investments i JOIN r_shares s USING (vestid)', 'clubid<0');
    $negatives = -db\sum('balance', 'users', 'uid>0 AND uid NOT BETWEEN :MINIMUM_BANK_UID AND :MAXIMUM_BANK_UID AND balance<0 AND IFNULL(jid,0)=0');
    $negReserve = -$negatives * getv('negReserve', .05);
    $backing = db\sum('backing', 'users', 'IFNULL(jid,0)=0');
    $backingReserve = -$backing * getv('backingReserve', .50);
    $invoicesFromUs = db\sum('amount', 'r_invoices', 'payee<0 AND status IN (:TX_PENDING,:TX_APPROVED)');

    $balances = db\sum('balance', 'users', 'uid>0 AND uid NOT BETWEEN :MINIMUM_BANK_UID AND :MAXIMUM_BANK_UID AND balance>0 AND IFNULL(jid,0)=0');
    $invoicesToUs = db\sum('amount', 'r_invoices', 'payer<0 AND payee>0 AND status IN (:TX_PENDING,:TX_APPROVED)');

    $otherAssets = $otherLiabilities = 0;
    foreach ($cttys = db\col('uid', 'users', 'uid<0') as $ctty) {
      $cttyA = r\acct($ctty);
      $otherAssets += $cttyA->otherAssets;
      $otherLiabilities += $cttyA->otherLiabilities;
    }

  }

  $invReservePct = $investments ? $invReserve / $investments : 0;
  foreach (ray('inv neg backing') as $k) ${$k . 'ReservePct'} = u\fmtAmt(${$k . 'ReservePct'}, '%');
  
  $assetFlds0 = t("|cgBalance:Community %PROJECT Account~Funds in the Community's own account(s); not the total of all funds held in member accounts -- THAT total is held as investments, Dollar Pool bank account balance(s), and other community assets, all of which show up here as separate assets | pool:Dollar Pool~Total transfers from member bank accounts to %PROJECT accounts minus total transfers the other way | investments:Investments~Total current value of the %PROJECT community's investments | invReserve:Investment Loss Reserve (%invReservePct)~Amount held in reserve in case the %PROJECT community's investments lose value | negatives:Negative Balance Promises~Total amount members have borrowed from the %PROJECT community by having a negative account balance (promising to bring it up to zero or more within 30 days) | negReserve:Negative Balance Loss Reserve (%negReservePct)~Amount held in reserve in case members fail to repay by bringing their balance up to zero or more | backing:Backing Promises~Total amount members have promised to back (by buying more %PROJECT credit) if there is a cash flow crunch today | backingReserve:Backing Promise Loss Reserve (%backingReservePct)~Amount held in reserve in case there is a cash crunch and members fail to follow through on their promise to back the system | invoicesFromUs:Accounts Receivable~Pending %PROJECT invoices from the %PROJECT community to members | assetTotal:Total Assets~", compact(ray('invReservePct negReservePct backingReservePct')));
  /// otherAssets:Other Assets~Liquified value of %PROJECT community assets not tracked within the %PROJECT system (including any other financial account balances) |

  $liabilityFlds0 = t("|balances:%PROJECT Account Balances~Total positive balances held by members in their %PROJECT accounts | invoicesToUs:Accounts Payable~Pending %PROJECT invoices to the %PROJECT community from members | liabilityTotal:Total Liabilities~ | netAssets:Net Assets~Total assets minus total liabilities");
  /// otherLiabilities:Other Liabilities~Community liabilities not tracked within the %PROJECT system |
  
  $netAssets = 0; // just to be defined
  foreach (ray('asset liability') as $k) {
    ${$k . 'Total'} = 0;
    ${$k . 'Flds'} = $ray = ray(${$k . 'Flds0'});
    $keys = array_keys($ray);
    for ($i = count($keys) - 1; $i >= 0; $i--) ${$k . 'Total'} += ${$keys[$i]}; // add backwards so as not to re-add totals
    ${$k . 'Keys'} = ($k . 'Label') . ' ' . join(' ', $keys);
  }
  $netAssets = $assetTotal - $liabilityTotal;
  
  foreach($assetFlds + $liabilityFlds as $k => $v) {
    list ($label, $help) = explode('~', $v);
    $$k = item(u\fmtAmt($$k, '$'), w\popHelp($label, $help));
  }

  $assetLabel = item('', t('ASSETS'));
  $liabilityLabel = item('', t('LIABILITIES'));
  ///  print_r(compact(ray("title subtext $assetKeys $liabilityKeys")));
  
  return cgform(compact(ray("title subtext $assetKeys $liabilityKeys")));
}