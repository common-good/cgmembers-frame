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
function formBacking($form, &$sta, $args = '') {
  global $mya;
  
  $title = t('Backing Promise');
  $subtext = t(<<< X
  <p>Please choose a dollar amount you will back, for one year. This limited promise we all make to each other assures that any <i>other</i> credit in your account is 100% backed &mdash; by Dollars and/or collateral and/or Backing Promises.</p>

  <p>In the event of a <%aCrunch>cash crunch</a> within one year of today, you agree to increase your balance by the backing amount you chose, within 7 days and to bring your balance up to that new target at least once a week until the cash crunch ends. If your %PROJECT Community ends instead of recovering, you would likely forfeit the amount you promised to back [<%aExample>example</a>].

  <p>"If there is a cash crunch within the next 12 months, I promise to buy this amount of %PROJECT credit to back the system as described above."</p>
X
  , '_aCrunch _aExample', atag('/help/cashflow'), atag('/help/backing-example'));
  
  foreach (ray('10000 2000 500 100 20 1') as $k) $choices[$k] = u\fmtAmt($k);
  $choices[-1] = t('Other');
  $minDesc = t(' (minimum $1)');
  $amtChoice = selectFld(t('Amount:'), '', required(100) + suffix($minDesc), $choices);
  $amount = textFld(t('Amount $'), '', suffix($minDesc));
  $signedBy = w\signedBy();
  $date = item(u\fmtDate(NULL, TRUE), t('Date:'));
  $posttext = t('<b>Note:</b> Every year you will be invited to make another backing promise for the next 12 months.');

  jsx('amtChoice');
  return cgform(compact(ray('title subtext amtChoice amount signedBy date')) + w\setupFoot(t('Promise'), t('Promise')) + compact('posttext'));
}
  
function formBacking_validate($form, &$sta) {
  if (w\badSignature($sta)) return;
  if (!w\checkAmtChoice($sta)) return;
}
  
function formBacking_submit($form, &$sta) {
  global $mya;
//  $us = new r\usd($mya = r\acct());
  extract(just('amount', $sta['input']));

	$why = t('at your request');
  $mya->bank(op($sta) == 'get' ? $amount : -$amount, $why);
  say($why); // expanded message
//  r\tellAdmin('banked' . $extra, $subs);
}