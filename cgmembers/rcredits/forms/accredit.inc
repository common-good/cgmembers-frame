<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Give a customer store credit
 */
function formAccredit($form, &$sta, $args = '') {
  $title = t('Give a Customer Store Credit');
  $subtext = t('<p>Give a customer some credit. Then whenever they pay you through Common Good, they automatically get a 100% rebate until the credit is used up.</p><p>If the customer is <i>buying</i> the credit or someone is buying store credit as a gift to someone else, you will probably want to create a <%a>CGPay button</a> instead (for your website, newsletters, or emails).', '_a', atag('/co/cgbutton'));

  $customer = textFld(t('Customer:'), [t('Name, id, email, or phone')]);
  $amount = numFld(t('Credit:'), [t('Amount')]);
  w\whoFldSubmit('customer', t('Grant %amount credit to %name?'), 'self-credit', ':IS_OK');
  $submit = t('Save');
  
  return cgform(compact(ray('title subtext customer amount submit')));
}

function formAccredit_validate($form, &$sta) {
  if ($err = u\badAmount($sta['input']['amount'])) return say($err, 'amount');
}

function formAccredit_submit($form, &$sta) {
  global $mya;
  extract(just('amount', $sta['input']));

  $uid = whoId($sta);
  $a = r\acct($uid);
  be\advanceCredit(SELF_CREDIT, $a, $mya, $amount);

  $msg = 'store grant|to redeem';
  if ($a->stepsLeft('card')) $msg .= '|want a card|finish get card';
  $amt = u\fmtAmt($amount);
  $a->tellIn($msg, ray('co amount', $mya->fullName, $amt), t('You got credit!'));

  say($msg = 'you gave credit', $subs = ray('nm amount', $a->fullName, $amt));
  $mya->tellOut($msg, $subs, t('You gave a customer credit')); // make sure management sees what credit the staff handed out
  
  return w\go('dashboard');
}
