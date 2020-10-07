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
  if (!$subdom = strtoupper($dom)) return exit('fail'); // correct a fluke in the server's rewrite (empty subdomain is a hacker -- give no information, don't even tell admin)
  $qrDom = ray(QR_DOMS)[(bool) isPRODUCTION];
  $res = r\Qo::qr2("$subdom.$qrDom/$code");
  if (!$res = r\Qo::qr2("$subdom.$qrDom/$code") or !$a = r\acct($res[0]) or $a->cardCode() != $res[1]) return softErr(t('That is not a valid %PROJECT card.'));

  if (r\acct()) signout(TRUE); // use scanner cookie, not whatever we're signed in to
  
  $no = submit(BARE . t('No'), '', 'lg');
  $yes = submit(BARE . t('Yes'), '', 'lg');
  $buttons = fieldSet('buttons', compact(ray('no yes')));
  $uid = hidFld($a->id);
  $nm = $a->fullName;
  
  if (!$scanner = r\cook('scanner') or !r\acct($scanner)) { // first time scanning a card, so ask whether to use it to set up this device
    $title = t('Device Owner Setup');
    $subtext = t('Associate this device with <b>%nm</b>, to charge or pay through %PROJECT?', compact('nm'));
    $way = hidFld('up');
    return cgform(compact(ray('title subtext buttons way uid')));
  }

  $mya = r\acct($scanner);
  $myName = $mya->fullName;
  $title = t('You: ') . $myName;
    
  if ($mya->id == $a->id) {
    $subtext = t('Do you want to disconnect this device from %myName?', compact('myName'));
    $way = hidFld('down');
    return cgform(compact(ray('title subtext buttons way')));
  }
  
  $photo = item($a->photoHtml(FALSE, TRUE));
  $subtext = ($a->co ? "<small>$agentName</small><br>" : '') . "<b>$a->fullName</b><br>$a->city, $a->st";

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
  jsx('card', compact(ray('choice0Count')));

  $charge = submit(BARE . t('Charge'));
  $pay = $choices ? NULL : submit(BARE . t('Pay'));
  $go = submit(BARE . t('Go'));
  $buttons = fieldSet('buttons', compact(ray('charge pay go')));
  
  return cgform(compact(ray('title photo subtext amount for desc buttons uid')));
}

function formCard_validate($form, &$sta) {
  extract(just('amount', $sta['input']));
  if (!isset($amount)) return;
  if ($err = u\badAmount($amount)) return say($err, 'amount');
  u\preRay(compact('amount'), $sta['input']);
}

function formCard_submit($form, &$sta) {
  extract(just('amount desc uid way', $sta['input'], NULL));
  $mya = r\acct(r\cook('scanner'));
  $a = r\acct($uid);
  $op = op($sta);
  
  if ($way == 'up') { // setting up device
    if ($op == 'no') return cardSignout(tr('scan co card first'), 'err');
    r\setCook('scanner', $a->qid); // scan QRs with last account used
    r\setAcct(NULL); // 
    return cardSignout(tr('This device is now associated with %nm.', 'nm', $a->fullName));
  }
  
  if ($way == 'down') { // setting up device
    if ($op == 'no') return cardSignout(tr('no self-trading'), 'err');
    r\setCook('scanner', NULL); // scan QRs with last account used
    r\setAcct(NULL); // 
    return cardSignout(tr('This device is no longer associated with %nm.', 'nm', $mya->fullName));
  }
  
  $goods = strhas($desc, t('cash')) ? FOR_USD : FOR_GOODS;
  $type = ($op == 'charge' ? 'charge' : 'pay');
  list ($ok, $msg, $subs) = be\transfer($type, $mya, $a, $amount, $desc, $goods);

  $msg = tr($msg, $subs);
  return cardSignout($msg, $ok ? '' : 'err');
}

/**
 * Don't stay signed in after scanning a card (so employees can use the device to scan customer cards also).
 * Display a message of success or failure on the "empty" page.
 */
function cardSignout($msg, $err = '') {
  
  $sid = session_id();
  db\del('sessions', 'ssid', $sid);
  session_unset();
  
  r\setAcct(NULL);
  say($msg, $err);
  return go('empty');
}
