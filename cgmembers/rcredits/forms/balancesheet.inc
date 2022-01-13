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
  
  extract(just('ctty', $args, ($mya and $mya->agentId != UID_SUPER) ? $mya->community : 0));
  
  $negReservePct = getv('negReserve', .05);
  $backingReservePct = getv('backingReserve', .50);
  $isMaster = '(uid>jid AND uid NOT :CANONIC)';

  $cttyChoice = cttyFld($ctty);
  jsx('balance-sheet');

  if  ($ctty != 0) { // community only
    // NOTE: uid>jid is normal accounts (in the community) that are independent or half a joint account
    $cttyA = r\acct($ctty);
    $subs = compact('ctty');
    $isCtty = 'community=:ctty';

    $title = t('Community Balance Sheet');
    $subtext = t('For ') . $cttyA->fullName;
    $cgBalance = $cttyA->balance;
    $pool = db\sum('amount', 'txs2 t JOIN users u ON u.uid=t.payee', 'completed AND u.community=:ctty', $subs);
    $poolTable = 'txs_proper JOIN users u1 ON u1.uid=uid1 JOIN users u2 ON u2.uid=uid2'; // NOT canonic?
    $pool += db\sum("IF(u1.$isCtty, -amt, amt)", $poolTable, "u1.$isCtty XOR u2.$isCtty", compact('ctty')); // inter-ctty
    $investments = db\sum('shares*price', 'r_investments i JOIN r_shares s USING (vestid)', 'clubid=:ctty', $subs);
    $invReserve = db\sum('shares*price*reserve', 'r_investments i JOIN r_shares s USING (vestid)', 'clubid=:ctty', $subs);
    $negatives = -db\sum('balance', 'users', "balance<0 AND $isMaster AND community=:ctty AND uid<>:ctty", $subs);
    $negReserve = $negatives * $negReservePct;
    $backing = db\sum('backing', 'users', "$isMaster AND community=:ctty", $subs);
    $backingReserve = $backing * $backingReservePct;
    $invoicesFromUs = db\sum('amount', 'tx_requests', 'payee=:ctty AND status IN (:TX_PENDING,:TX_APPROVED)', $subs);
    
    $balances = db\sum('balance', 'users', "balance>0 AND $isMaster AND community=:ctty AND uid<>:ctty", $subs);
    $invoicesToUs = db\sum('amount', 'tx_requests', 'payer=:ctty AND status IN (:TX_PENDING,:TX_APPROVED)', $subs);
    $otherAssets = $cttyA->otherAssets ?: 0;
    $otherLiabilities = $cttyA->otherLiabilities ?: 0;
    
  } else { // whole region (server) overall
    // NOTE: (uid>jid OR uid<0) AND community<0 is accounts that are independent or half a joint account or roundups or crumbs
    $title = t('%PROJECT Balance Sheet');
    $subtext = t('For ALL participating communities (totals)');
    $cgBalance = NULL;
    $pool = db\sum('amount', 'txs2', 'completed');
    $investments = db\sum('shares*price', 'r_investments i JOIN r_shares s USING (vestid)', 'clubid<0');
    $invReserve = db\sum('shares*price*reserve', 'r_investments i JOIN r_shares s USING (vestid)', 'clubid<0');
    $negatives = -db\sum('balance', 'users', "balance<0 AND ($isMaster OR uid<0) AND community<0");
    $negReserve = $negatives * getv('negReserve', .05);
    $backing = db\sum('backing', 'users', $isMaster);
    $backingReserve = $backing * getv('backingReserve', .50);
    $invoicesFromUs = db\sum('amount', 'tx_requests', 'payee<0 AND status IN (:TX_PENDING,:TX_APPROVED)');

    $balances = db\sum('balance', 'users', "balance>0 AND ($isMaster OR uid<0) AND community<0");
    $invoicesToUs = db\sum('amount', 'tx_requests', 'payer<0 AND payee>0 AND status IN (:TX_PENDING,:TX_APPROVED)');

    $otherAssets = $otherLiabilities = 0;
    foreach ($cttys = db\col('uid', 'users', 'uid<0') as $ctty) {
      $cttyA = r\acct($ctty);
      $otherAssets += $cttyA->otherAssets ?: 0;
      $otherLiabilities += $cttyA->otherLiabilities ?: 0;
    }
  }

  $invReservePct = $investments ? $invReserve / $investments : 0;
  foreach (ray('inv neg backing') as $k) ${$k . 'ReservePct'} = u\fmtAmt(${$k . 'ReservePct'}, '%');
  
  $assetFlds0 = t("|cgBalance:Community %PROJECT Account~Funds in the Community's own account(s); not the total of all funds held in member accounts -- THAT total is held as investments, Dollar Pool bank account balance(s), and other community assets, all of which show up here as separate assets | pool:Dollar Pool~Total transfers from member bank accounts to %PROJECT accounts minus total transfers the other way | investments:Investments~Total current value of the %PROJECT community's investments | negatives:Negative Balance Promises~Total amount members have borrowed from the %PROJECT community by having a negative account balance (promising to bring it up to zero or more within 30 days) | backing:Backing Promises~Total amount members have promised to back (by buying more %PROJECT credit) if there is a cash flow crunch today | invoicesFromUs:Accounts Receivable~Pending %PROJECT invoices from the %PROJECT community to members | assetTotal:Total Assets~");
  /// otherAssets:Other Assets~Liquified value of %PROJECT community assets not tracked within the %PROJECT system (including any other financial account balances) |

  $liabilityFlds0 = t("|balances:%PROJECT Account Balances~Total positive balances held by members in their %PROJECT accounts | invReserve:Investment Loss Reserve (%invReservePct)~Amount held in reserve in case the %PROJECT community's investments lose value | negReserve:Negative Balance Loss Reserve (%negReservePct)~Amount held in reserve in case members fail to repay by bringing their balance up to zero or more | backingReserve:Backing Promise Loss Reserve (%backingReservePct)~Amount held in reserve in case there is a cash crunch and members fail to follow through on their promise to back the system | invoicesToUs:Accounts Payable~Pending %PROJECT invoices to the %PROJECT community from members | liabilityTotal:Total Liabilities~ | netAssets:Net Assets*~Total assets minus total liabilities", compact(ray('invReservePct negReservePct backingReservePct')));
  /// otherLiabilities:Other Liabilities~Community liabilities not tracked within the %PROJECT system |
  
  $netAssets = 0; // just to be defined
  foreach (ray('asset liability') as $k) {
    ${$k . 'Total'} = 0;
    ${$k . 'Flds'} = $ray = ray(${$k . 'Flds0'});
    $keys = array_keys($ray);
    for ($i = count($keys) - 1; $i >= 0; $i--) {
      if (!is_null(${$keys[$i]})) {
        ${$k . 'Total'} += ${$keys[$i]}; // add backwards so as not to re-add totals
      } else {unset($keys[$i]);}
    }
    ${$k . 'Keys'} = ($k . 'Label') . ' ' . join(' ', $keys);
  }
  $netAssets = $assetTotal - $liabilityTotal;
  $posttext = $netAssets < 0 ? t('* Common Good Earth is backing this shortfall (but no new grants or investments) until the Backing Promises have a chance to catch up.') : NULL;
  
  foreach($assetFlds + $liabilityFlds as $k => $v) {
    list ($label, $help) = explode('~', $v);
    $$k = item(u\fmtAmt($$k, '$'), w\popHelp($label, $help));
  }

  $assetLabel = item('', t('ASSETS'));
  $liabilityLabel = item('', t('LIABILITIES'));
///  print_r(compact(ray("title subtext $assetKeys $liabilityKeys")));
///  debug(compact(ray("title subtext $assetKeys $liabilityKeys posttext")));
  
  return cgform(compact(ray("cttyChoice title subtext $assetKeys $liabilityKeys posttext")));
}
