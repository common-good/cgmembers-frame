<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Transfer from bank to CG or vice versa.
 */
function formGet($form, &$sta, $args = '') {
  global $mya;
  global $base_url;
  
  if (!$mya->hasBank) return w\go('/settings/fund');
  
  \drupal_page_is_cacheable(false); // suppress caching, so buttons work right
  extract(just('cancel expedite', $args, NULL));
  if ($cancel and $mya) return cancelTx($cancel);
  if ($expedite and $mya and $mya->admRegion) {
    be\completeUsdTx($expedite);
    be\usdTxFollowup($xid);
    return sayEmpty(t('Bank transfer successfully expedited.'));
  }
  
  $title = t('Transfer Funds');
  $subtext = t('<p>When you transfer funds FROM your bank account, you are buying Common Good credit. That credit is backed partly by Dollars in your Common Good Community\'s Dollar Pool and partly by collateral, including promises of members and member organizations [<%aBacking>more about backing</a>].</p>
  <p>It may take up to 4 business days to transfer funds (3 if you submit your request before 2pm Eastern US Time). If you have an emergency and need funds immediately, talk to us.</p><p>To refill your %PROJECT Account automatically (or not), as needed, adjust your "refills" setting on the <%aSettings>Banking Settings</a> page.</p>', '_aBacking _aSettings', atag('/help/backing-common-good-credits'), atag('/settings/fund'));

  $avail = $mya->avail(FOR_USD); // drawing from connected accounts not allowed
  foreach (ray(R_EXCHANGE_LIMITS) as $days => $limit) {
    $sofar = $days == 1 ? 0 : db\get('-SUM(amount)', 'txs2_bank', 'payee=:myid AND amount<0 AND created>:date', ray('myid date', $mya->id, strtotime("-$days days")));
    $avail = min($avail, $limit - $sofar);
  }

  $subs = ray('uids', $mya->uids);
  foreach (ray('in out other') as $k) {
    $where = 'payee IN (:uids) AND txid>0 AND ';
    $where .= in($k, 'in other') ? 'amount>0' : 'amount<0';
    $where .= $k == 'other' ? ' AND deposit<>0 AND completed=0' : ' AND deposit=0';
    if ($amt = db\sum('ABS(amount)', 'txs2_bank', $where, $subs)) {
      $xid = ($mya->admRegion and in($k, 'in other')) ? db\get('xid', 'txs2_bank', $where . ' AND completed=0 AND amount=' . $amt, $subs) : '';
      $expedite = $xid ? ' ' . buttino('/get/expedite=' . $xid, t('Expedite')) : '';
      $link = $k == 'other' ? t('(not cancelable)') : buttino("/get/cancel=$k", t('Cancel'));
      $text = $k == 'out' ? t('%link %amt from %PROJECT to your bank account.')
      : (t('%link %amt from your bank account to %PROJECT.') . $expedite);
      $wait = tr($text, 'amt _link', u\fmtAmt($amt), $link);
      $waiting[] = "<span>$wait</span>";
    }
  }

  $waiting = nn($waiting)
  ? t('You have bank transfer requests in process: <br>') . join('<br>', $waiting)
  : t('You have no pending bank transfer requests.');
  $waiting = item($waiting, t('Pending:'));


  $avail = hidFld($avail);
  $balance = u\fmtAmt($mya->balance);
  $credit = u\fmtAmt(-$mya->floor);
  $cashoutable = u\fmtAmt(max(0, min($mya->balance, $avail)));
 
  $balances = item(t('balances', compact(ray('balance credit cashoutable'))), t('Balance:'));
  $amount = numFld(t('Amount $'), t('Transfer how much?') . ($mya->admin ? t(' (no limit for admin)') : ''));

  $arrow = '<span class="glyphicon glyphicon-arrow-right"></span>';
  $get = submit(BARE . t('Bank %m CG', '_m', $arrow));
  $put = submit(BARE . t('CG %m Bank', '_m', $arrow));
  $actions = fieldSet('actions', compact('get', 'put'));

  return cgform(compact(ray('title subtext waiting balances amount actions avail')));
}
  
function formGet_validate($form, &$sta) {
  global $mya;
  if ($err = amtErr('amount', $sta)) return say($err, 'amount'); // negative caught by < R_ACHMIN below
  extract(just('amount avail', $sta['input']));
  if (!$mya->admin and $amount < R_ACHMIN) return say('bank too little', 'amount');
// (this is unnecessary)  if ($amount > R_MAX_DAILY_EXCHANGE) return say('max daily usd', 'amount');
  if (op($sta) == 'get') return;
  
  if ($amount > ($mya->admin ? round($mya->balance, 2) : $avail)) {
    $max = u\fmtAmt($avail);
    $extra = $mya->cttyRewardy ? '|short cash help' : '';
    return say('short put' . $extra, compact('max'), 'amount');
  }
  if (round($mya->balance - $amount, 2) < $mya->minimum and $mya->refill) return say('change min first', 'amount');
}
  
function formGet_submit($form, &$sta) {
  global $mya;

  extract(just('amount', $sta['input']));

  list ($xid, $amtDone, $report) = $mya->bank(op($sta) == 'get' ? $amount : -$amount, t('as soon as possible'), ray('explicit', TRUE));
  say($report); // expanded message
  return go('get'); // required for testing
}

/**
 * Cancel the given transfer.
 * @param int $way: in or out (relative to Common Good)
 */
function cancelTx($way) {
  global $mya;
  
  $where = 'payee IN (:uids) AND deposit=0 AND txid>0 AND ';
  $where .= $way == 'in' ? 'amount>0' : 'amount<0';

  while ($xid = db\get('xid', 'txs2_bank', $where, ray('uids', $mya->uids))) {
    $crit = compact('xid');
    db\del('txs2_bank', $crit);
    say('bank tx canceled', $crit);
    r\tellAdmin("canceled bank transfer xid #$xid: $mya->fullName ($mya->mainQid)");
    $mya->tellInOut($way == 'in', t('bank tx canceled', $crit));
  }

  if (!nn($crit)); // ignore if none
  
  return w\go('/get'); // don't leave the cancelation on the URL (submitting like that cancels the future)
}
