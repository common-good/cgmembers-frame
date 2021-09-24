<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Handle a scanned Common Good card.
 * @param string $dom: 
 *   the compressed region code OR
 *   undo, tip, or receipt
 * @param string $code:
 *   the account's compressed qid, cardCode, and possibly count OR
 *   URL parameters for undo, tip, or receipt (xid, pct)
 */
function formCard($form, &$sta, $dom = '', $code = '') {
  global $mya, $box, $boxId;

  if (!$scanner = r\cook('scanner')) {
    if (!$mya) return goFirst('signin'); // sign in if no scanner
    $accts = db\keyed('uid,fullName', 'u_relations r JOIN users u ON u.uid=r.main', 'other=:myid AND permission>=:scan', ray('myid scan', $mya->id, r\perm(B_SCAN)));
    if ($accts) return chooseScanner($accts, "$dom/$code");
    $scanner = $mya->mainQid;
  }
  $scannerA = $mya ?: r\acct($scanner); // allow even friends to use the device, without changing standard scanner

  if ($dom == 'done') {    
    if (!$z = u\decryRay($code)) return hack('done with no code');
    return cardDone($z);
  } elseif ($dom == 'disconnect') {
    r\setCook('scanner', NULL);
    return cardDone(ray('ok msg', TRUE, t('This device is no longer associated with any account.')));
  } elseif (in($func = $dom, 'receipt undo tip')) {
    extract(just('xid tip', $code, NULL));
    $boxes = db\col('id', 'r_boxes', ray('code', $box));
    if (!$scannerA or !$xid or !$tx = r\Tx::lastTx($scannerA->id, 'xid=:xid and boxId IN (:boxes)', compact(ray('xid boxes'))) or $tx->uid2 != $scannerA->id) return softErr(t('That is not a payment to you.'));
    $func = "\\CG\\Web\\$func";
    return $func($tx, $tip); // only tip uses $tip
  }

  // no special $dom parameter -- must be region code
  if (!$subdom = strtoupper($dom) or !$code) return exit('fail'); // correct a fluke in the server's rewrite (empty subdomain is a hacker -- give no information, don't even tell admin)
  $qrDom = ray(QR_DOMS)[(bool) isPRODUCTION];
  $res = r\Qo::qr2("$subdom.$qrDom/$code");
  if (!$res or !$a = r\acct($res[0]) or $a->cardCode() != $res[1]) return softErr(t('That is not a valid %PROJECT card.'));
    
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
//  foreach (ray('dom code') as $k) $$k = hidFld($$k);
  
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
  $scannerA = getScannerA();
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
function cardDone($z) {
  global $mya;

  if (!$actorA = getScannerA()) return softErr(t('Sign in or scan a customer card to continue.'));
  extract(just('ok msg xid subs', $z, NULL));
  if (!$xid and $subs) extract(just('xid', $subs, NULL));
  if ($msg) say($msg, $ok ? '' : 'err');

  $title = t('You: ' . ($actorA ? $actorA->bestName : t('Scanner')));
  $lnkShowQr = $mya ? btn('/show-qr', t('Show My QR'), '', 'md', clas('btn-done')) : '';
  $lnkScanQr = btn('/scan-qr', t('Scan Another QR'), '', 'md', clas('btn-done'));
  
  $buttons = "$lnkShowQr$lnkScanQr";
  if ($xid and $tx = r\Tx::read($xid) and !$tx->reversesXid and !$tx->reversedByXid) {
    foreach (ray(t('receipt:Receipt, undo:Undo, tip:Tip')) as $k => $desc) $$k = btn("/card/$k/xid=$xid", $desc, 'default', 'sm', clas("btn-done btn-$k"));
    if (hasTip($tx)) $tip = '';
    $receipt = ''; // disable until this prints properly on mobile
    $buttons = "$undo<br>$buttons<br>$tip$receipt";
    $buttons = item($buttons);
    $tx = r\Tx::read($xid);
    $msg = t('Reverse your %amt charge to %who (%dt)?', 'amt who dt', u\fmtAmt($tx->amt2), $tx->a1->fullName, u\fmtDate($tx->created));
    jsx('cardDone', compact('msg'));
  }

  return cgform(compact(ray('title buttons')));
}

/**
 * Print a receipt for a transaction.
 */
function receipt($tx) {
  $bizA = $tx->actorA;
  $logo = logoFld($bizA->logo ? $bizA : r\acct(CGID)); // use CG logo if company has none
  $title = t('Purchase Receipt');
  $dt = item(u\fmtDate($tx->created, TRUE), t('Date:'));
  $nm = item($tx->a1->fullName, t('From:'));
  $acct = item($tx->a1->mainQid, t('Account:'));
  $to = item($bizA->fullName, t('Paid To:'));
  $amt = item(u\fmtAmt($tx->amt2), t('Amount:'));
  $for = item($tx->e2->description, t('For:'));
  $xid = item($tx->xid, t('Tx#:'));
  $agt = $tx->e2->agentUid == $tx->e2->uid ? NULL : item(r\acct($tx->e2->agentUid)->mainQid, t('Agent:'));
  
  jsx('receipt', 'code', u\cryRay(ray('ok msg xid', TRUE, '', $tx->xid)));
  
  return cgForm(compact(ray('logo title dt nm acct to amt for xid agt')), 'tight', 'left');
}

/**
 * Undo a transaction.
 */
function undo($tx) {
  $z = be\posUndo($tx->actorA, $tx->xid, $tx->amt2 < 0 ? (now() - $tx->created < FORCE_WINDOW) : TRUE); // forced reversal of payments is time-sensitive
  return cardDone($z);
}

/**
 * Ask the customer for a tip.
 * tip=3 means $3, tip=3! means 3% (an actual percent sign screws up the browser)
 */
function tip($tx, $tip) {
  if (isset($tip)) {
    list ($tip, $isPct) = substr($tip, -1, 1) == '!' ? [strstr($tip, '!', TRUE), TRUE] : [$tip, FALSE];
    if ($err = u\badAmount($tip, '>=0')) return go("/card/tip/xid=$tx->xid", $err, 'err');
    jsx('cardTipDone'); // must precede cardDone (including errors)
    if ($isPct) {
      $desc = t('tip (%tip%)', compact('tip'));
      $tip *= $tx->amt2 / 100;
    } else $desc = t('tip');
    if ($tip = round($tip, 2)) {
      if (hasTip($tx)) return cardDone(ray('ok msg xid', FALSE, t('You already have a tip connected with this purchase.'), $tx->xid));
      be\txEntryPair($entries, $tip, [$tx->uid1, $tx->uid2], $desc, NULL, E_AUX);
      be\insertTxes($entries, $tx->xid); // no balance checking (make this always go through!!)
    }
    return cardDone(ray('ok msg xid', TRUE, t('Thank you for your purchase!'), $tx->xid));
  } else {
    $tips[0] = t('No Tip');
    foreach (TIP_DFTS as $i => $tip) {
      $amt = number_format($tip * $tx->amt2 / 100, 2);
      $tips[$i + 1] = "<big>$tip%</big> " . t('Tip') . "<br><small>$$amt</small>";
    }
    $tips['NP'] = t('Custom %');
    $tips['ND'] = t('Custom $');

    foreach ($tips as $pct => $desc) ${"btn$pct"} = btn("/card/tip/xid=$tx->xid&tip=$pct!", $desc, 'primary', 'md', clas("btn-tip btn$pct"));
    $buttons = item("$btn0<br>$btn1$btn2$btn3<br>$btnNP$btnND");
    $tipP = numFld(t('Tip:'), '', suffix('% ' . btn('', 'Go', 'primary', 'sm')) + vmin(0));
    $tipD = numFld(t('Tip: $'), '', suffix(btn('', 'Go', 'primary', 'sm')) + vmin(0));
    jsx('cardTip', 'xid msg title amt', $tx->xid, t('Total charge including tip is %total. Okay?'), t('Confirm %tip tip'), $tx->amt2);

    return cgForm(compact(ray('buttons tipP tipD')), '', 'narrow left');
  }  
}

function hasTip($tx) {return db\exists('txs', ray('xid uid1 uid2 type', $tx->xid, $tx->uid1, $tx->uid2, E_AUX));}

/**
 * Return the account to scan from. Also set $boxId.
 */
function getScannerA() {
  global $mya, $box, $boxId;
  
  if ($mya) return $mya;
  $scannerA = r\acct(r\cook('scanner'));
  $boxId = w\boxId($box, $scannerA->id);
  return $scannerA;
}
