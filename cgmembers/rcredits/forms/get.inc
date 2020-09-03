<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Exchange USD for rCredits or vice-versa (the Bank tab)
 */
function formGet($form, &$sta, $args = '') {
  global $mya;
  global $base_url;
  
  if (!$mya->hasBank) return w\go('/settings/fund');
  
  \drupal_page_is_cacheable(false); // suppress caching, so buttons work right
  $myid = $mya->id;
  $jid = $mya->jid ?: 0;
  extract(just('cancel', $args, NULL));
  if ($cancel and $mya) return cancelTx($cancel);
  
  $title = t('Transfer Funds');
  $subtext = t('<p>When you transfer funds FROM your bank account to your Common Good account, you are buying Common Good credit. Your Common Good credit is backed partly by Dollars in your Common Good Community’s Dollar Pool and partly by collateral, including promises of members and member organizations [<%aBacking>more about backing</a>]. It may take several business days for your bank to send the funds.</p>
  <p>Funds going TO your bank account also may take several days to arrive. If you have an emergency and need funds immediately, talk to us.</p><p>To refill your %PROJECT Account automatically (or not), as needed, adjust your "refills" setting on the <%aSettings>Banking Settings</a> page.</p>', '_aBacking _aSettings', atag('/help/backing-common-good-credits'), atag('/settings/fund')) . t('note|ach warning');

  $subs = compact(ray('myid jid'));
  $toBank24 = db\get('-amount', 'r_usd', 'payee IN (:myid,:jid) AND amount<0 AND created>:dayAgo', $subs + ['dayAgo' => strtotime('-1 day')]);
  if (empty($toBank24)) { // limit to one withdrawal request per 24 hours
    $avail = $mya->avail(FOR_USD); // drawing from connected accounts not allowed
    foreach (ray(R_EXCHANGE_LIMITS) as $days => $limit) {
      $sofar = $days == 1 ? 0 : db\get('-SUM(amount)', 'r_usd', 'payee=:myid AND amount<0 AND created>:date', ray('myid date', $myid, strtotime("-$days days")));
      $avail = min($avail, $limit - $sofar);
    }
  } else { $avail = 0; }

  foreach (ray('in out other') as $k) {
    $where = 'payee IN (:myid, :jid) AND txid>0 AND ';
    $where .= in($k, 'in other') ? 'amount>0' : 'amount<0';
    $where .= $k == 'other' ? ' AND deposit<>0 AND completed=0' : ' AND deposit=0';
    if ($amt = db\sum('ABS(amount)', 'r_usd', $where, $subs)) {
      $link = $k == 'other' ? t('(not cancelable)') : tr(" <a class=\"buttino\" href=\"$base_url/get/cancel=$k\">%text</a>", 'text', t('Cancel'));
      $text = $k == 'out' ? t('%link %amt from %PROJECT to your bank account.')
      : t('%link %amt from your bank account to %PROJECT.');
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
  $amount = numFld(t('Amount $'), t('Transfer how much?') . ($mya->cAdmin ? t(' (no limit for admin)') : ''));

  $get = submit(BARE . t('From Bank to %PROJECT'));
  $put = submit(BARE . t('From %PROJECT to Bank'));
  $actions = fieldSet('actions', compact('get', 'put'));

  return cgform(compact(ray('title subtext waiting balances amount actions avail')));
}
  
function formGet_validate($form, &$sta) {
  global $mya;
  if ($err = amtErr('amount', $sta)) return say($err, 'amount'); // negative caught by < R_ACHMIN below
  extract(just('amount avail', $sta['input']));
  if (!$mya->cAdmin and $amount < R_ACHMIN) return say('bank too little', 'amount');
// (this is unnecessary)  if ($amount > R_MAX_DAILY_EXCHANGE) return say('max daily usd', 'amount');
  if (op($sta) == 'get') return;
  
  if ($amount > ($mya->cAdmin ? round($mya->balance, 2) : $avail)) {
    $max = u\fmtAmt($avail);
    $extra = $mya->cttyRewardy ? '|short cash help' : '';
    return say('short put' . $extra, compact('max'), 'amount');
  }
  if (round($mya->balance - $amount, 2) < $mya->minimum and $mya->refill) return say('change min first', 'amount');
}
  
function formGet_submit($form, &$sta) {
  global $mya;
  $mya = r\acct();
  u\EXPECT([$mya], 'Acct');
  //  $us = new r\usd($mya = r\acct());
  extract(just('amount', $sta['input']));

  $mya->bank(op($sta) == 'get' ? $amount : -$amount, t('at your request'), [], $why);
  say($why); // expanded message
//  r\tellAdmin('banked' . $extra, $subs);
}

/**
 * Cancel the given transfer.
 * @param int $way: in or out (relative to Common Good)
 */
function cancelTx($way) {
  global $mya; $myid = $mya->id;
  
  $subs = ray('myid jid', $myid, $mya->jid);
  $where = 'payee IN (:myid, :jid) AND deposit=0 AND txid>0 AND ';
  $where .= $way == 'in' ? 'amount>0' : 'amount<0';

  while ($xid = db\get('xid', 'r_usd', $where, $subs)) {
    $crit = compact('xid');
    db\del('r_usd', $crit);
    say('transfer canceled', $crit);
    r\tellAdmin("canceled bank transfer xid #$xid");
    r\notify($myid, t('bank tx canceled', $crit));
  }

  if (!nn($crit)); // ignore if none
  
  return w\go('/get'); // don't leave the cancelation on the URL (submitting like that cancels the future)
}
