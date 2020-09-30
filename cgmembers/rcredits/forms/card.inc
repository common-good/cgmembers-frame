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
  
  if (!$mya) {
    extract(just('trust scanner', $_COOKIE), NULL);
    $qid = r\cryptCook('qid');
    if (!$trust or !$scanner or !$qid or !$mya = r\acct($scanner, $qid)) return w\softErr(t('You are not signed in. Sign in and consider checking "Trust this device".'));
  }
  if (!$mya->can(B_SCAN)) return w\softErr(t('You do not have permission to scan a customer card for the current account.'));
  
  $dom = strtoupper($dom); // correct a fluke in the server's rewrite
  if (!$res = r\Qo::qr2("$dom.RC2.ME/$code") or !$a = r\acct($res[0]) or $a->cardCode != $res[1]) return softErr(t('That is not a valid %PROJECT card.'));
  $title = t('You: ') . $mya->fullName;
  $photo = item($a->photoHtml(FALSE, TRUE));
  $subtext = ($a->co ? "<small>$a->agentA->fullName</small><br>" : '') . "<b>$a->fullName</b><br>$a->city, $a->st";

  $amount = numFld(t('Amount: '), [t('Amount')]);
  if ($mya->co) { // we are a company
    $choices = $mya->selling ? explode("\n", $mya->selling) : [];
    $choice0Count = count($choices);
    $choices = u\joinRays($choices, ray(t('Refund to customer, %you receives cash, Customer buys cash from %you', 'you', $mya->fullName)));
  } else { // we are an individual
    $choices = $a->co ? explode("\n", $mya->selling) : []; // if other is a company assume pay -- an individual cannot charge a company this way
    $choice0Count = count($choices);
  }
  
  $choices[] = t('Other:');
  
  $for = selectFld(REQ . t('For:'), '', '', $choices);
  $desc = textFld(t('For:'), [t('Description')]);
  $myid = hidFld($mya->id);
  $uid = hidFld($a->id);
  jsx('card', compact(ray('choice0Count')));

  $charge = submit(BARE . t('Charge'));
  $pay = $choices ? NULL : submit(BARE . t('Pay'));
  $go = submit(BARE . t('Go'));
  $buttons = fieldSet('buttons', compact(ray('charge pay go')));
  
  return cgform(compact(ray('title photo subtext amount for desc buttons myid uid')));
}

function formCard_validate($form, &$sta) {
  extract(just('amount', $sta['input']));

  if ($err = u\badAmount($amount)) return say($err, 'amount');
  u\preRay(compact('amount'), $sta['input']);
}

function formCard_submit($form, &$sta) {
  extract(just('amount desc uid myid uid', $sta['input'], NULL));
  $mya = r\acct($myid);
  $a = r\acct($uid);
  $goods = strhas($desc, t('cash')) ? FOR_USD : FOR_GOODS;
  $type = (op($sta) == 'charge' ? 'charge' : 'pay');
  list ($ok, $msg, $subs) = be\transfer($type, $mya, $a, $amount, $desc, $goods);

  $msg = tr($msg, $subs);
  return $ok ? go('empty', $msg) : softErr($msg);
}