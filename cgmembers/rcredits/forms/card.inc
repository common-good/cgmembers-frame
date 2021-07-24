<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Handle a scanned Common Good card.
 * @param string $dom: the compressed region code
 * @param string $code: the account's compressed qid, cardCode, and possibly count
 */
function formCard($form, &$sta, $dom = '', $code = '') {
  global $mya;

  if ($dom == 'done') {    
    if (!$z = u\decryRay($code)) return hack('done with no code');
    extract(just('ok msg', $z, NULL));
    return cardDone($ok, $msg);

  } elseif ($dom == 'disconnect') {
    r\setCook('scanner', NULL);
    return cardDone(TRUE, t('This device is no longer associated with any account.'));
  }

  if (!$subdom = strtoupper($dom) or !$code) return exit('fail'); // correct a fluke in the server's rewrite (empty subdomain is a hacker -- give no information, don't even tell admin)
  $qrDom = ray(QR_DOMS)[(bool) isPRODUCTION];
  $res = r\Qo::qr2("$subdom.$qrDom/$code");
  if (!$res or !$a = r\acct($res[0]) or $a->cardCode() != $res[1]) return softErr(t('That is not a valid %PROJECT card.'));
  
  if (!$scanner = r\cook('scanner')) {
    if (!$mya) return goFirst('signin'); // sign in if no scanner
    $accts = $mya ? db\keyed('uid,fullName', 'r_relations r JOIN users u ON u.uid=r.main', 'other=:myid AND permission>=:scan', ray('myid scan', $mya->id, r\perm(B_SCAN))) : NULL;
    if ($accts) return chooseScanner($accts, "$dom/$code");
    $scanner = $mya->mainQid;
  }
  
  $scannerA = $mya ?: r\acct($scanner); // allow even friends to use the device, without changing standard scanner
  if ($scannerA->id == $a->id) return softErr(t('You cannot transact with yourself, so there is no point in scanning your own card.'));
  
  r\setCook('scanner', $scanner);
  $trust = r\cook('trust');
  $myName = $scannerA->fullName;
  $title = t('You: ') . $myName;
  
  $photo = item($a->agentA->photoHtml(FALSE, TRUE));
  $agentName = $a->agentA->fullName;
  $subtext = ($a->co ? "<small>$agentName</small><br>" : '') . "<b>$a->fullName</b><br>$a->city, $a->st";

  $amount = numFld(t('Amount: '), [t('Amount')]);
  if ($scannerA->co) { // we are a company
    $choices = $scannerA->selling ? explode("\n", $scannerA->selling) : [];
    $choice0Count = count($choices);
    $choices = u\joinRays($choices, ray(t('Refund to customer, %you receives cash, Customer buys cash from %you', 'you', $scannerA->fullName)));
  } else { // we are an individual
    $choices = $a->co ? explode("\n", $scannerA->selling) : []; // if other is a company assume pay -- an individual cannot charge a company this way
    $choice0Count = count($choices);
  }
  
  $choices[] = t('Other:');
  
  $for = selectFld(REQ . t('For:'), '', '', $choices);
  $desc = textFld(t('For:'), [t('Description')]);
  jsx('card', compact(ray('choice0Count')));

  $charge = submit(BARE . t('Charge'));
  $pay = ($mya or $trust) ? submit(BARE . t('Pay')) : NULL;
  $go = submit(BARE . t('Go'));
  $buttons = fieldSet('buttons', compact(ray('charge pay go')));
  $uid = hidFld($a->id);
  $mode = hidFld('normal');
  
  return cgform(compact(ray('title photo subtext for desc amount buttons uid mode')));
}

function formCard_validate($form, &$sta) {
  extract(just('mode amount', $sta['input'], NULL));
  if ($mode == 'choose') return TRUE;
  
  // mode = 'normal'    
  if ($err = u\badAmount($amount)) return say($err, 'amount');
  u\preRay(compact('amount'), $sta['input']);
}

function formCard_submit($form, &$sta) {
  global $mya;
  
  extract(just('mode account tail amount desc uid', $sta['input'], NULL));
  if ($mode == 'choose') { // chose an account to scan from
    $scannerA = r\acct($account);
    r\setCook('scanner', $scannerA->mainQid); // not ->qid
    $page = u\urlify("/card/$tail");
    if (test()) $page = "/card/$tail"; // because changeWho simulates urlify for tests
    return changeWho('', "page=$page&acct=$scannerA->id", $sta);
  }

  // mode = 'normal'    
  $scannerA = $mya ?: r\acct(r\cook('scanner'));
  $a = r\acct($uid);
  $op = op($sta);
  
  $goods = strhas($desc, t('cash')) ? FOR_USD : FOR_GOODS;
  $type = ($op == 'charge' ? 'charge' : 'payment');
  $z = be\transfer($type, $scannerA, $a, $amount, $desc, $goods);

//  say($z->msg, $z->ok ? '' : 'err');
  return go('/card/done/' . u\cryRay($z));
}

/**
 * Don't stay signed in after scanning a card (so employees can use the device to scan customer cards also).
 * Display a message of success or failure on the "empty" page.
 *//*
function cardSignout($msg, $err = '') {
  
  $sid = session_id();
  db\del('sessions', 'ssid', $sid);
  session_unset();
  
  r\setAcct(NULL);
  return sayEmpty($msg, $err);
}*/

/**
 * Choose which account "owns" this device for scanning purposes.
 * @param assoc $choices: list of account names keyed by uid
 * @param string $tail: the original query
 */
function chooseScanner($choices, $tail) {
  global $mya;
  
  jsx('cardChoose');

  $title = t('Choose Account');
  $subtext = t('Scan From Which Account?');
  $account = radiosFld(NBSP, '', '', ray($mya->id, $mya->fullName) + $choices);
  $mode = hidFld('choose');
  $tail = hidFld($tail);
  $submit = submit();
  return cgform(compact(ray('title subtext account mode tail submit')));
}

/**
 * Report a result, showing useful links.
 */
function cardDone($ok, $msg) {
  global $mya;

  say($msg, $ok ? '' : 'err');

  $nm = ($scanner = r\cook('scanner') and $scannerA = r\acct($scanner)) ? $scannerA->fullName : ($mya ? $mya->fullName : '');
  $title = $nm ? t('You: ' . $nm) : t('Scanner');
  $lnkShowQr = $mya ? btn('/show-qr', t('Show My QR'), '', 'md') : '';
  $lnkScanQr = btn('/scan-qr', t('Scan Another QR'), '', 'md');
  $subtext = item("$lnkShowQr$lnkScanQr");

  return cgform(compact(ray('title subtext')));
}
