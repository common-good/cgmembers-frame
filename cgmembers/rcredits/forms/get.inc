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
  extract(just('cancel', $args));
  if ($txid = @$cancel) {
    if ($xid = db\get('xid', 'r_usd', 'txid=:txid AND payee=:myid and deposit=0', compact('txid', 'myid'))) {
      $crit = compact('xid');
      $DBTX = \db_transaction();
      foreach (ray('tx_entries tx_hdrs r_usd') as $table) db\del($table, $crit);
      unset($DBTX);
      say('transfer canceled', compact('txid'));
      r\tellAdmin("canceled exchange #$txid");
      r\notify($mya->id, t('bank tx canceled', $crit));
      return w\go('/get'); // don't leave the cancelation on the URL (submitting like that cancels the future)
    } else say('That transfer does not exist.');
  }
  
  $title = t('Transfer Funds');
  $subtext = t('<p>When you transfer funds FROM your bank account to your Common Good account, you are buying Common Good credit. Your Common Good credit is backed partly by Dollars in your Common Good Community’s Dollar Pool and partly by collateral, including promises of members and member organizations [<%aBacking>more about backing</a>]. It may take several business days for your bank to send the funds.</p>
  <p>Funds going TO your bank account also may take several days to arrive. If you have an emergency and need funds immediately, talk to us.</p><p>To refill your %PROJECT Account automatically (or not), as needed, adjust your "refills" setting on the <%aSettings>Banking Settings</a> page.</p>', '_aBacking _aSettings', atag('/help/backing-common-good-credits'), atag('/settings/fund')) . t('note|ach warning');

//  $minimum = numFld(t('Minimum:'), t('prompt min'), dft(u\fmtAmt(!$mya->prefsSet ? $mya->minimum : R_MINR_DEFAULT, 's')));
//  $toSpend = u\fmtAmt($mya->avail(FOR_GOODS));
//  $toSpend = u\fmtAmt($mya->r - $mya->committed);
  $balance = u\fmtAmt($mya->balance);
  $credit = u\fmtAmt(-$mya->floor);
  $fromBank = $mya->waitingFromBank();

  $toBank = db\get('amount', 'r_usd', 'payee IN (:myid,:jid) AND amount<0 AND created>:dayAgo', ray('myid jid dayAgo', $myid, $jid, strtotime('-1 day')));
  if (empty($toBank)) { // limit to one withdrawal request per 24 hours
    $avail = $mya->avail(FOR_USD); // drawing from connected accounts not allowed
    foreach (ray(R_EXCHANGE_LIMITS) as $days => $limit) {
      $sofar = $days == 1 ? 0 : db\get('-SUM(amount)', 'r_usd', 'payee=:myid AND amount<0 AND created>:date', ray('myid date', $myid, strtotime("-$days days")));
      $avail = min($avail, $limit - $sofar);
    }
  } else {
    $avail = 0;
  }

  $toBankFmt = u\fmtAmt(-$toBank);
  $fromBankFmt = u\fmtAmt($fromBank);
  $cashoutable = u\fmtAmt(max(0, min($mya->balance, $avail)));
  if ($fromBank != 0) {
    $where = 'payee IN (:myid, :jid) AND completed=0 AND deposit=0 ORDER BY txid DESC';
    if ($rec = db\get('amount, txid', 'r_usd', $where, compact('myid', 'jid'))) {
      extract($rec);
      if ($toBank == 0) {
        $text = t('Cancel');
      } else {
        $text = t('Cancel %fromBankFmt request', compact('fromBankFmt'));
      }
      $link = " <a class=\"buttino\" href=\"$base_url/get/cancel=$txid\">$text</a>";
    } else { $link = ''; }
  } else { $link = ''; }
  
  /* $way = $waiting0 < 0 ? t('to') : ('from'); */
  if ($fromBank == 0) {
    if ($toBank == 0) {
      $waiting = t('You have no pending exchange requests.');
    } else {
      $waiting = t('You have total pending exchange requests of %toBankFmt to your bank account.',
                   compact('toBankFmt'));
    }
  } else {
    if ($toBank = 0) {
      $waiting = t('You have total pending exchange requests of %fromBankFmt from your bank account.',
                   compact('fromBankFmt'));
    } else {
      $waiting = t('You have total pending exchange requests of %toBankFmt to, and %fromBankFmt from, your bank account.',
                   compact('toBankFmt', 'fromBankFmt'));
    }
  }
  $waiting = item($waiting . $link, t('Pending:'));
  /* $waiting = item($waiting == '$0' ? t('You have no pending exchange requests.') : (t('You have total pending exchange requests of %waiting %way your bank account.', compact('waiting', 'way')) . @$link), t('Pending:')); // amount already requested (if any) */
  $avail = hidFld($avail);
  $credit = u\fmtAmt(-$mya->floor);
  $balances = item(t('balances', compact(ray('balance credit cashoutable'))), t('Balance:'));
  $amount = numFld(t('Amount $'), t('Transfer how much?') . ($mya->cAdmin ? t(' (no limit for admin)') : ''));
  $get = submit(BARE . t('From Bank'));
//  $fromBank = fld('checkbox', t('Automate:'), t('(recommended) Refill my %PROJECT Account from my bank account automatically, as needed.'), dft(!$mya->prefsSet ? $mya->can(B_FROM_BANK) : TRUE));
  
//  $maximum = numFld(t('Maximum:'), t('prompt max'), dft(u\fmtAmt(!$mya->prefsSet ? $mya->maximum : R_MAXR_DEFAULT, 's')));
//  $excess = numFld(t('Excess'), '', dft(u\fmtAmt($mya->overMax(), 's')));
//  $toBank = fld('checkbox', t('Automate:'), t('Transfer any excess to my bank account automatically every few days.'), dft(!$mya->prefsSet ? $mya->can(B_TO_BANK) : TRUE));
  $put = submit(BARE . t('To Bank'));
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