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

  $oftens = [0 => t('How often?')] + array_reverse(r\recurRay(TRUE));
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

	if ($recurs = db\get('id,period,amount', 'tx_templates t', 'action=:ACT_PAY AND t.from=:payer AND t.to=:CGID AND end IS NULL', ray('payer', $mya->id))) {
		extract($recurs);
		$recursId = hidFld($id);
    $replaces = t('donation replaces', 'period amt', $oftens[$period], u\fmtAmt($amount));
	} else $recursId = NULL;
  
  if (!$mya->activated or !$recurs or !r\acct()) {
		unset($oftens['once']); // no one-time donation unless the account is active, has a recurring donation, and is signed in
    unset($oftens[0]);
		$recurOnly = t(': monthly, quarterly, or yearly');
    $noOneTime = t('<p><small><%aWhy>Why no option for a one-time donation?</a></small></p>', '_aWhy', atag('/help/why-no-single-donation-option'));
	} else $noOneTime = '';
  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of');
   
//  $inSetup = $mya->ok ? '' : t(' The amount you choose will come from your account once it is activated and has enough funds.</p>');
  $title = t('Donate to %PROJECT');
  $amountTip = $mya->co ? '' : t('The most common amounts chosen are $50 and $25.');
  $subtext = t('<p>Make a tax-deductible gift of ZERO OR MORE%recurOnly. %replaces %amountTip</p><p>To donate an amount not listed (including ZERO), choose "Water" (and fill in the amount).</p><p><b class="loud">NOTE: This is a donation, not a deposit.</b> Thank you for your support!</p>', 'recurOnly replaces amountTip', nn($recurOnly), nn($replaces), $amountTip) . $noOneTime;
  $amtChoice = selectFld(REQ . t('Donation:'), '', dft($mya->co ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT), $giftLevels);
  $amount = numFld(t('Donation:'), [t('Amount')]);
  $period = selectFld(REQ . t('When:'), '', dft('year'), $oftens);
  $honored = rendA('honored', textFld(BARE, [t('(optional)')]));
  $honor = selectFld(t('Honoring:'), '', suffix($honored), $honors);

  jsx('amtChoice');
  return cgform(compact(ray('title subtext amtChoice amount period honor recursId')) + w\setupFoot(t('Donate')));
}

function formDonate_validate($form, &$sta) {
  if (!w\checkAmtChoice($sta)) return;
}

function formDonate_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta); 

  extract(just('amount period honor honored roundup recursId', $sta['input']));
  $info = ray('payer payee amount period created lastTx invoiced', $mya->id, CGID, $amount, $period, now(), 0, 0);
  if ($period != 'once' and nn($recursId)) {
    $was = (object) db\get('amount, period, periods', 'tx_templates', ray('id', $recursId));
    $was = u\fmtAmt($was->amount) . ' ' . r\recurDesc($was->period, $was->periods);
    r\endRule(TRUE, $recursId);
		say(t('Your previous recurring gift has been canceled.'));
	} else $was = t('no previous recurring donation');

  if ($amount > 0) {
    if ($honored) db\insert('r_honors', $honored = ray('uid honor honored', $mya->id, $honor, $honored));
    $purpose = t('donation');
    $recursId = $period == 'once' ? 0
    : db\insert('tx_templates', ray('action from to amount period purpose duration', ACT_PAY, $mya->id, CGID, $amount, $period, $purpose, 'once'));
    $msg = 'gift successful';
    $data = ray('gift recurs', 1, (bool) $recursId);
    if ($mya->ok and $z = $mya->payApproved(CGID, $amount, $purpose, FOR_NONGOODS, $data, $recursId) and nni($z->subs, 'xid')) {
      $mya->tellOut('gift sent', ray('amount', u\fmtAmt($amount)));
    } else $msg .= '|gift transfer later';
    say($msg);
  } else $was = $honored = '';
  
  $nm = $mya->fullName;
  if ($mya->ok) r\tellCAdmin('gift', $info + compact(ray('nm was honored')));

  return r\acct() ? w\goNextStep('donate', '') : w\doSay();
}
