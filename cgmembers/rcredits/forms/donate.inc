<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Cron as cr;

/*
  Give the person a chance to donate to CG from the current account.
  If they haven't chosen a recurring donation yet, a recurring donation is the only option
*/
function formDonate($form, &$sta, $args = '') {
  if (!$mya = w\eLinkAcct($sta, $args, 'acct')) return;

  $oftens = [0 => t('How often?')] + array_reverse(ray(OFTENS));
  $giftLevels = array(
  // 10000 => t('Marble .. $10,000'),
  // 5000 => t('Granite ... 5,000'),
    2500 => t('Slate .... $2,500'),
    1000 => t('Glass .... $1,000'),
     500 => t('Iron ....... $500'),
     250 => t('Oak ........ $250'),
     100 => t('Two Bricks . $100'),
      50 => t('One Brick ... $50'),
      25 => t('Half Brick .. $25'),
//       0 => t('None for now . $0'),
      -1 => t('Water ... (other)'),
//      10 => t('Tile ........ $10'),
//       5 => t('Cloth ........ $5'),
  );

	if ($recurs = db\get('id,period,amount', 'r_recurs', 'payer=:payer AND payee=:CGID AND ended=0', ray('payer', $mya->id))) {
		extract($recurs);
		$recurId = hidFld($id);
    $replaces = t('donation replaces', 'period amt', $oftens[$period], u\fmtAmt($amount));
	} else $recurId = NULL;
  
  if (!$mya->activated or !$recurs or !r\acct()) {
		unset($oftens['X']); // no one-time donation unless the account is active, has a recurring donation, and is signed in
		$recurOnly = t(': monthly, quarterly, or yearly');
    $noOneTime = t('<p><small><%aWhy>Why no option for a one-time donation?</a></small></p>', '_aWhy', atag('/help/why-no-single-donation-option'));
	} else $noOneTime = '';
  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of');
   
//  $inSetup = $mya->ok ? '' : t(' The amount you choose will come from your account once it is activated and has enough funds.</p>');
  $title = t('Donate to %PROJECT');
  $amountTip = $mya->co ? '' : t('The most common amounts chosen are $50 and $25.');
  $subtext = t('<p>Make a tax-deductible gift of ZERO OR MORE%recurOnly. %replaces %amountTip</p><p>To donate an amount not listed (including ZERO), choose "Water".</p><p><b class="loud">NOTE: This is a donation, not a deposit.</b> Thank you for your support!</p>', 'recurOnly replaces amountTip', @$recurOnly, @$replaces, $amountTip) . $noOneTime;
  $amtChoice = selectFld(t('Donation:'), '', required($mya->co ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT), $giftLevels);
  $amount = textFld(t('Other amount $'), '');
  $period = selectFld(t('When:'), '', required(0), $oftens);
  $honor = selectFld(t('Honoring:'), '', '', $honors);
  $honored = areaFld('', [t('Honoring whom or what')]);
//    $roundup = $mya->ok ? '' : w\roundupFld($mya),

  jsx('amtChoice');
  return cgform(compact(ray('title subtext amtChoice amount period honor honored recurId')) + w\setupFoot(t('Donate')));
}

function formDonate_validate($form, &$sta) {
  if (!w\checkAmtChoice($sta)) return;
}

function formDonate_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta); 

  extract(just('amount period honor honored roundup recurId', $sta['input']));
  $info = ray('payer payee amount period created lastTx invoiced', $mya->id, CGID, $amount, $period, now(), 0, 0);
  if ($period != 'X' and nn($recurId)) {
		db\update('r_recurs', ray('id ended', $recurId, now()), 'id');
		say(t('Your previous recurring gift has been canceled.'));
	}

  if ($amount > 0) {
    if ($honored) db\insert('r_honors', ray('uid honor honored', $mya->id, $honor, $honored));
    $purpose = t('donation');
    list ($recursId, $periodDesc) = $period == 'X' 
    ? [0, '']
    : [db\insert('r_recurs', ray('payer payee amount period purpose', $mya->id, CGID, $amount, $period, $purpose))
       , ' (' . r\periodDesc($period) . ')'];
//    $count = db\count('r_txs', 'payer=:payer AND payee=:payee AND :IS_RECURS AND created>=:created', compact(ray('payer payee created')));
    $msg = 'gift successful';
    $data = ray('gift recurs', 1, (bool) $recursId);
    if ($mya->payApproved(CGID, $amount, $purpose . $periodDesc, FOR_NONGOODS, $data, $recursId)) {
      r\notify($mya->id, 'gift sent', ray('amount', u\fmtAmt($amount)));
    } else $msg .= '|gift transfer later';
    say($msg);
  }
  if ($mya->ok) r\tellCo('gift', $info);

  return r\acct() ? w\goNextStep('donate', '') : w\doSay();
}
