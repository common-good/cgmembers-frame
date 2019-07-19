<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Buy or sell in investment club investment.
 */
function formInvestBuySell($form, &$sta, $args = '') {
  global $mya;
  
  extract(just('vestid', $args));
  
  $q = db\q('SELECT i.coid, i.clubid, i.types, i.price, COUNT(shid) AS `count`, SUM(shares) AS shares, SUM(pending) AS pending FROM r_investments i LEFT JOIN r_shares s ON s.vestid=i.vestid WHERE i.vestid=:vestid', compact('vestid'));
  if (!$res = $q->fetchAssoc()) return w\go('/page-not-found');

  extract($res);

  $equity = ($types[0] == 'D');
  $pending += 0; $shares += 0; // convert to numeric
  $clubqid = r\qid($clubid);
  list ($what, $pendingLabel, $buyLabel, $sellLabel, $buyOrSell, $amtToBuy) = $equity // equity investment?
  ? ray(t('shares in, Pending Sales, Buy Shares, Sell Shares, Buy or Sell, Shares to Buy'))
  : ray(t('loan to, Pending Repayment Request, Lend More, Request Repayment, Lend or Reclaim, Amount to Lend'));
  
  $title = t('Buy or Sell Investment #%vestid', compact('vestid'));
  
  $subtext = t(<<< X
  <p>Complete this form to increase or decrease the club's %what %coName.</p>
  <p>Once you have finished all your club's planned selling and buying for this cycle, click <%a>Distribute Profits</a> here or on the main Investment page, to share the club's profits with club members and %PROJECT.</p>
X
    , 'what coName _a', $what, r\acct($coid)->fullName, atag('/invest/distribute'));

  $ifSelling = $shares ? t('If you reduce your investment, it will take effect when the company pays the invoice.') : '';
  $ifSellingMore = $pending ? t('  Whatever reduction you request here will be added to your current pending requests.') : '';
  $amtTip = $equity ? [t('Number of Shares'), $ifSelling . $ifSellingMore] : [t('Amount')];

  $selling = $shares 
  ? item($equity ? -$pending : u\fmtAmt(-$pending), "$pendingLabel:") 
  : NULL;
  $buy = submi($buyLabel, '', 'md');
  $sell = $shares ? submi($sellLabel, '', 'md') : NULL;
  $submit = fieldSet('submit2', compact('buy', 'sell'), ' ');
  if ($equity) {
    $ourShares = item(t('%shares (at %price)', 'shares price', number_format($shares), u\fmtAmt($price)), t('Current Shares:'));
  } else $ourShares = item(u\fmtAmt($shares), t('Current Loan'));
  $shares = textFld($shares ? "$buyOrSell:" : "$amtToBuy:", $amtTip, required());
  $old = hidFld(serialize(compact(ray('vestid coid clubqid types count'))));

  return cgform(compact(ray('title subtext ourShares selling shares submit old')));
}

function formInvestBuySell_validate($form, &$sta) {
  extract(just('shares', $sta['input']));
  if ($err = u\badAmount($shares, '>0', 0)) return say($err, 'shares');
  u\preray(compact('shares'), $sta['input']); // update with any format corrections
}

function formInvestBuySell_submit($form, &$sta) {
  extract(just('shares old', $sta['input']));
  extract(just('vestid coid clubqid types count', unserialize($old)));
  
  $equity = ($types[0] == 'D');
  $type = $equity ? t('investment') : t('loan');
  $price = db\get('price', 'r_investments', compact('vestid'));
  $total = round($shares * $price, 2);
  $sharesDpy = number_format($shares, 0);
  $priceDpy = u\fmtAmt($price);
  $clubA = r\acct($clubqid);
  $coA = r\acct($coid);
  
  $DBTX = \db_transaction();
  if (op($sta) == 'buy') {
    list ($index, $subs) = be\transfer('payment', $clubA, $coA, $total, $type, FOR_USD, ray('investment', 1));
    $success = @$subs['success'];
    if ($equity) {
      $extraMsg = $success ? t('purchased shares', 'amt co price', $sharesDpy, $coA->fullName, $priceDpy) : t('No shares were purchased.');
    } else $extraMsg = $success ? t('loaned', 'amount co', u\fmtAmt($shares), $coA->fullName) : t('Nothing was loaned.');
    if ($success) db\insert('r_shares', ray('vestid shares when', $vestid, $shares, now()), 'shid');
  } else { // sell
    $shid = db\insert('r_shares', ray('vestid shares pending when', $vestid, 0, -$shares, now()), 'shid');
    if ($equity) {    
      $for = t('redeeming investment: %amt shares at %price', 'amt price', $sharesDpy, $priceDpy);
    } else $for = t('loan repayment');
    list ($index, $subs) = be\invoice($clubA, $coA, $total, $for, FOR_USD, ray('investment shid', 1, $shid));
//    if (@$count + 0 == 1) { // the usual case -- one shares record
    $extraMsg = $equity ? t('investment sale pending') : t('repayment request pending');
    $extraMsg = nn($subs['success']) ? tr($extraMsg, 'co', $coA->fullName) : '';
  }
  unset($DBTX);

  say("$index|$extraMsg", $subs, @$subs['success'] ? NULL : 'err');
  return w\go("/invest/list/actual=1&clubqid=$clubqid");
}
