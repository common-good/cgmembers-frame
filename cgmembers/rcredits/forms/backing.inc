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
  <p>Backing makes it possible for your community to make investments and grants. Whereas bank investments are backed (partially) by the FDIC, Common Good investments and grants are backed by the members.</p>
  
  <p>Please choose a dollar amount you will back, for 12 months. This limited promise we all make to each other assures that any <i>other</i> credit in your account is 100% backed &mdash; by Dollars and/or collateral and/or Backing Promises. This promise comes into play only if there is a <%aCrunch>cash crunch</a>.</p>

  <p>In the event of a cash crunch within one year of today, you agree to increase your balance, within 7 days, by the backing amount you chose and to bring your balance up to that new target at least once a week until the cash crunch ends. If your %PROJECT Community <i>ends</i> instead of recovering, the amount you chose to back becomes a donation (from you to your %PROJECT Community, for all the great things it funded) [<%aExample>example</a>].

  <p>"If there is a cash crunch within the next 12 months, I promise to buy this amount of %PROJECT credit to back the system as described above."</p>
X
  , '_aCrunch _aExample', atag('/help/cashflow'), atag('/help/backing-example'));
  
  $choices0 = ray('10000 2000 500 100 20 1');
  $dft = ($mya->backing + 0) ?: 1;
  if ($dft and !in_array($dft, $choices0)) $choices0[] = $dft;
  foreach ($choices0 as $k) $choices[$k] = u\fmtAmt($k);
  $choices[-1] = t('Other');
  $minDesc = t(' (minimum $1)');
  $amtChoice = selectFld(t('Amount:'), '', required($dft) + suffix($minDesc), $choices);
  $amount = textFld(t('Amount $'), '', suffix($minDesc));
  $signedBy = w\signedBy();
  $date = item(u\fmtDate(NULL, TRUE), t('Date:'));
  $posttext = t('<b>Note:</b> Every year you will be invited to make another backing promise for the next 12 months.');

  jsx('amtChoice');
  return cgform(compact(ray('title subtext amtChoice amount signedBy date')) + w\setupFoot(t('Promise')) + compact('posttext'));
}
  
function formBacking_validate($form, &$sta) {
  if (w\badSignature($sta)) return;
  if (!w\checkAmtChoice($sta, 1)) return;
  extract(just('amount', $sta['input'])); // this must come AFTER checkAmtChoice
  if (strpos($amount, '.')) return say('too fractional', 'amount');
}
  
function formBacking_submit($form, &$sta) {
  global $mya;
  extract(just('amount', $sta['input']));
  if ($amount < $mya->backing) say('backing in effect');
  $mya->update('backing backingDate', $amount, today());
  if ($amount > 1) say(t('Thank you for backing your %PROJECT community!'));
  return w\goNextStep('backing');
}