<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

//include_once R_ROOT . '/qo.class';

/**
 * Handle a scanned Common Good card.
 */
function formCard($form, &$sta, $dom = '', $code = '') {
  global $mya;
  
  if (!$mya) return w\softErr(t('You are not signed in. Sign in and consider setting "Stay signed in for scans" in Advanced Settings.') . $code);
  $dom = strtoupper($dom); // correct a fluke in the server's rewrite
  if (!$res = r\Qo::qr2("$dom.RC2.ME/$code") or !$a = r\acct($res[0]) or $a->cardCode != $res[1]) return softErr(t('That is not a valid %PROJECT card.'));
  $title = t('You: ') . $mya->fullName;
  $photo = item($a->photoHtml(FALSE, TRUE));
  $subtext = ($a->co ? "<small>$a->agentA->fullName</small><br>" : '') . "<b>$a->fullName</b><br>$a->city, $a->st";

  $amount = numFld(t('Amount: '), [t('Amount')]);
  if ($mya->co) { // we are a company
    $choices = explode("\n", $mya->selling);
    $choice0Count = count($choices);
    $choices = u\joinRays($choices, ray(t('Refund to customer, %you receives cash, Customer buys cash from %you', 'you', $mya->fullName)));
  } else { // we are an individual
    $choices = $a->co ? explode("\n", $mya->selling) : []; // if other is a company assume pay -- an individual cannot charge a company this way
    $choice0Count = count($choices);
  }
  
  $choices[] = $otherDpy = t('Other:');
  
  $for = selectFld(REQ . t('For:'), '', '', $choices);
  $desc = textFld(REQ . t('For:'), [t('Description')], (count($choices) == 1 ? dft($otherDpy) : []) + clas('invisible'));
  $a = hidFld(serialize($a));
  jsx('card', compact(ray('choice0Count otherDpy')));

  $submit = t('Charge');
  $pay = $choices ? NULL : submit(t('Pay'));
  
  return cgform(compact(ray('title photo subtext amount for desc submit pay a')));
}

function formCard_validate($form, &$sta) {
  global $mya;
  extract(just('amount', $sta['input']));

  if ($err = u\badAmount($amount)) return say($err, 'amount');
  u\preRay(compact('amount'), $sta['input']);
}

function formCard_submit($form, &$sta) {
  global $mya;
  extract(just('amount desc uid a', $sta['input']));

  $a = unserialize($a);
  $goods = !strhas($desc, t('cash'));
  $type = (op($sta) == 'charge' ? 'charge' : 'pay');
  list ($ok, $msg, $subs) = be\transfer($type, $mya, $a, $amount, $desc, $goods);

  $msg = tr($msg, $subs);
  return $ok ? go('empty', $msg) : softErr($msg);
}