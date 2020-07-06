<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Join an investment club or manage membership therein.
 */
function formInvest($form, &$sta, $args = '') {
  global $mya; $uid = $mya->id;

  $title = t('Investment Club Overview');

  $clubid = $mya->iclub? $uid : db\get('uid', 'users', 'fullName LIKE :club AND :zip REGEXP zip AND :IS_OK', ray('club zip', t('%Investment Club%'), $mya->zip));
  
  if (!$clubid) {
    $subtext = t('There is no active investment club in your community at this time. Consider starting one! Contact %PROJECT staff for help getting started (see contact information below).');
    return cgform(compact(ray('title subtext')));
  } elseif (!$mya->can(B_SELL)) return w\go('summary', 'no receive invest');

  $clubA = r\acct($clubid);
  $clubqid = $clubA->mainQid;
  $clubVal = investmentsVal($clubid, $investments, $liquid, $reserve, $invCount);
  
  if ($mya->iclub) { // the club itself
    if ($res = db\get('-SUM(request) AS reqTot, SUM(IF(request<0, 1, 0)) AS reqCount, SUM(stake) AS stakes', 'r_stakes', compact('clubid'))) {
      extract($res);
      $reqCount = number_format($reqCount);
    } else $reqTot = $reqCount = $stakes = 0;
    
    $btnList = btn("/invest/list/clubqid=$clubqid", t('List'));
    $btnPropose = btn('/invest/propose', t('Propose'));
    $btnCashout = btn('/invest/cashout', t('Cash Out'));
    $btnDividends = btn('/invest/dividends', t('Issue Dividends'));
    $btnSave = btn('', t('Save'));
    $members = number_format(db\get('COUNT(DISTINCT uid)', 'r_stakes', compact('clubid')));

		$liquid = item(u\fmtAmt($liquid), t('Liquid:'));
    $requests = item(u\fmtAmt($reqTot), t('Requests to Cash Out (%reqCount):', compact('reqCount')), '', suffix($btnCashout));
		$investments = item(u\fmtAmt($investments), t('Investments (%invCount):', compact('invCount')), '', suffix($btnList . $btnPropose));
		$lossReserve = item(u\fmtAmt($reserve - $clubA->minimum), t('Loss Reserve:'));
		$expenseReserve = numFld(t('Expense Reserve:'), '', required(u\fmtAmt($clubA->minimum)) + suffix($btnSave));
    $avail = item(u\fmtAmt($clubVal - $stakes), t('Available for Dividends:'), '', suffix($btnDividends));
		$clubVal = item(u\fmtAmt($clubVal), t('Club Net Value:'));
    $stakes = item(u\fmtAmt($stakes), t('Total Member Stakes (%members):', compact('members')));
    $fields = 'title liquid requests investments lossReserve expenseReserve clubVal stakes avail';
    jsx('invest');
  } elseif ($isMember = $res = db\get('request,requestedOut,stake,stakeid', 'r_stakes', compact(ray('uid clubid')))) { // member!
    extract($res);
    $old = hidFld(serialize(compact(ray('stakeid clubid stake request requestedOut'))));
    $request += 0; // enable if($request)
    $stake += 0;
    $buttons = btn("/invest/list/clubqid=$clubqid", t('List Investments'), 'success');
    $amount = numFld($stake ? t('Buy or sell:') : t('Invest:'), [t('Amount'), t('If you reduce your investment amount, that request will take effect when funds become available.') . $request ? t('  Whatever amount you request here will be in addition to your current request.') : ''], required()); // must be before request item
    $myRequest = $request ? item(u\fmtAmt($request), t('Change request:'), t('Your pending request to %change your investment', 'change', $request < 0 ? t('reclaim part of') : t('increase'))) : NULL;
//    $togAttrs = ray('on off offstyle', t('Sell'), t('Buy'), 'warning');
//    $buyorsell = togFld('buysell', t('Buy or Sell:'), '', 1, $togAttrs);
    $buy = submi(t('Invest MORE'), '', 'md');
    $sell = ($stake or $request) ? submi(t('Invest LESS'), '', 'md') : NULL;
    //$myStake = item(u\fmtAmt($stake, ''), t('Your investment:'));
		$pct = myShareOfClub($uid, $clubid);
		$myVal = $pct * $clubVal;
		$clubVal = item(t('%val (%liquid liquid)', 'val liquid', u\fmtAmt($clubVal), u\fmtAmt($liquid)), t('Club Value:'));
		$myShare = item(tr('%amt (%pct)', 'amt pct', u\fmtAmt($myVal), u\fmtAmt($pct, '%')), t('Your Share:'));
    $submit = fieldSet('submit2', compact(ray('buy sell')), ' ');
    $fields = 'title buttons old clubVal myShare myRequest amount submit';
  } else { // not a member yet
    $old = hidFld(serialize(compact('clubid')));
    $title = t('Join Your Local Investment Club');
    $subtext = t(<<< X
  <p>Yes, I want to join the %cttyName Investment Club (the Club). I agree to the Club's <%a>Partnership Agreement</a>. In particular:</p>
  <ol><li>I recognize my investment is at risk. I may not be able to reclaim it right away and may <i>lose</i> some or all of it.</li>
  <li>I agree to participate actively in overseeing the Club's investments &mdash; at a minimum by voting on the proposed investments each calendar quarter (voting directly, not by proxy).</li>
  </ol>
X
    , 'cttyName _a', $mya->cttyA->fullName, atag('http://partnershipLink'));
    $signedBy = w\signedBy();
    $submit = t('Join');
    $fields = 'title subtext signedBy old submit';
  }

  return cgform(compact(ray($fields)));
}

function formInvest_validate($form, &$sta) {
  global $mya;

  extract(just('signedBy amount old expenseReserve', $sta['input']));

  if (isset($signedBy)) return !w\badSignature($sta);
  
  if (isset($expenseReserve)) {
    if ($err = u\badAmount($expenseReserve)) return say($err, 'expenseReserve');
    $sta['input']['expenseReserve'] = $expenseReserve;
    return;
  }
  
 // buy or sell
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
  extract(just('request stake', unserialize($old)));
  u\setDft($request, 0);

  if (op($sta) == 'buy') {
    if ($request + $amount > 0 and $short = $mya->shortfall(FOR_USD, $request + $amount)) {
      if (!$mya->hasBank) return say(t('You are %short short for that investment.', 'short', u\fmtAmt($short)), 'amount');
      $sta['input']['short'] = $short;
    }
  } elseif ($stake + $request - $amount < 0) return say(t('investment oversell'), 'amount');
  $sta['input']['amount'] = $amount;
}

function formInvest_submit($form, &$sta) {
  global $mya;
  extract(just('signedBy old amount short reserve', $sta['input'], NULL));
  
  if ($signedBy) {
    extract(just('clubid', unserialize($old)));
    db\insert('r_stakes', ray('uid clubid joined', $mya->id, $clubid, now()), 'stakeid');
    return say(t('now a member'));
  }

  if (isset($reserve)) {
    u\EXPECT($mya->id == $mya->iclubid, 'setting reserve when not club');
    $mya->update('minimum', $reserve);
    return say(t('info saved'));
  }
  
  // buy or sell  
  extract(just('stakeid clubid stake request requestedOut', unserialize($old)));
  if (test() and !nn($stakeid)) return say('no stakeid -- invest module may be disabled', 'err'); // keep tests from dying

  $buy = (op($sta) == 'buy');
  $request = round($request + ($buy ? $amount : -$amount), 2);
  $info = compact(ray('stakeid request'));
	if (!$buy) $info['requestedOut'] = now();
  db\update('r_stakes', $info, 'stakeid');
  
  if ($buy and $request > 0) {
    if ($short) {
      $msg = t('You do not currently have enough funds in your %PROJECT account for that investment, so %short will be drawn automatically from your bank account. Once that transfer is complete, your requested investment will be made automatically.', 'short', u\fmtAmt($short));
      $mya->getFunds($short, t('for investment'));
    } else r\handleInvestmentRequest($stakeid, $msg);
  } elseif (!$buy and $request < 0) { // sell
		$msg = t('redemption request', 'request', u\fmtAmt(-$request));
	} else $msg = t('Your request has been adjusted accordingly.');

  say($msg);
}

/**
 * Return the investment club's total current value.
 * @param int $clubid: the club's record ID
 * @param numeric $val: (RETURNED) value of club's investments
 * @param numeric $liquid: (RETURNED) club's cash assets (including reserve)
 * @param numeric $reserve: (RETURNED) club's cash reserve (loss reserve plus expense reserve)
 * @param int $cnt: (RETURNED) number of (actual) club investments
 */
function investmentsVal($clubid, &$val = 0, &$liquid = 0, &$reserve = 0, &$cnt = 0) {
	$icA = r\acct($clubid);
  if ($val = db\sum('shares*price', 'r_investments JOIN r_shares USING(vestid)', compact('clubid'))) {
	  $res = db\get('SUM(1) AS cnt, SUM(reserve) AS lossReserves', 'r_investments i', 'clubid=:clubid AND (SELECT 1 FROM r_shares s WHERE s.vestid=i.vestid LIMIT 1)', compact('clubid'));
    extract($res);
  } else $val = $cnt = $lossReserves = 0;
  
	list ($liquid, $reserve) = [$icA->balance, $icA->minimum + $lossReserves];
	return round($liquid + $val - $reserve, 2); // cash plus investments less reserve
}

/**
 * Return the fraction of the club owned by the current account.
 * A CG member who is not a club member can represent their spouse at a club meeting, but there is no joint investment account.
 * The member's fair share is calculated as the sum of products of stakes held by the member times number of seconds held,
 * as compared to the total of all such sums of products (for all club members)
 * @param int $clubid: the club's record ID
 * @param numeric $icVal: club's total current value
 *
 */
function myShareOfClub($myid, $clubid, &$myStake = '', &$totalStakes = '', $totalFairShares = '') {
	$memberAmt = 'IF(uid1=:clubid,-amt,amt)';
	$memberFairShare = "(:NOW-created)*$memberAmt";
	$isMe = ':myid IN (uid1,uid2)';
  
	$sql = <<<X
	  SELECT SUM($memberAmt) AS totalStakes,
		       SUM($memberFairShare) AS totalFairShares,
		       SUM(IF($isMe, $memberAmt, 0)) AS myStake,
					 SUM(IF($isMe, $memberFairShare, 0)) AS memberFairShare
	  FROM txs WHERE :clubid IN (uid1,uid2) AND :IS_STAKE
X;
  if (!$res = db\q($sql, compact(ray('myid clubid')))->fetchAssoc()) return 0;
  extract($res);
	return abs($totalFairShares) > .005 ? round($memberFairShare / $totalFairShares, 2) : 0;
}

/**
 * Return a club account.
 * @param string $clubqid: (OPTIONAL) the club's qid
 */
function clubA($clubqid = NULL) {
  global $mya;
  if ($clubqid) {
    if ($clubA = r\acct($clubqid) and $clubA->iclub) return $clubA;
  } else {
    if ($mya->iclub) return $mya;
    if ($clubid = $mya->iClubId) return r\acct($clubid);
  }
  
  return w\go('/invest/overview', t('no page permission'));
}
