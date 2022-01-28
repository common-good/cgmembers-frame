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

  $scannerA = getScannerA(); // allow even friends to use the device, without changing standard scanner

  if (!r\cook('scanner')) {
    if (!$mya) return goFirst('signin'); // sign in if no scanner
    $accts = db\keyed('uid,fullName', 'u_relations r JOIN users u ON u.uid=r.main', 'other=:myid AND permission>=:scan', ray('myid scan', $mya->id, r\perm(B_SCAN)));
    if ($accts) return chooseScanner($accts, $dom ? "$dom/$code" : '');
    $scannerA = $mya;
    r\setCook('scanner', $mya->mainQid); // proSe with no other accounts, first time, so set scanner and stay signed in
  } elseif (!$scannerA) return go('app'); // scanner account no longer exists 

  if (!$dom) { // no parameters specified -- just running the app
    return startup($scannerA);
  } elseif ($dom == 'selfServe') { // same (run the app), but with a message about selfServe on or off
    say(t('Self-Serve Mode is now ') . ($code ? t('ON') : t('OFF')));
    if ($code) signout(TRUE); // self-serve mode has no effect when signed in
    return startup($scannerA);
  } elseif ($dom == 'done') {    
    if (!$z = u\decryRay($code)) return hack('done with no code');
    return cardDone($z);
  } elseif ($dom == 'disconnect') {
    r\setCook('scanner', NULL);
    return go('/app', t('The CGPay<sup>&trade;</sup> app is no longer associated with any account, on this device.'));
  } elseif (in($func = $dom, 'receipt undo tip')) {
    extract(just('xid tip', $code, NULL));
    $boxes = db\col('id', 'r_boxes', ray('code', $box)) ?: -1;
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
  
  $myName = $scannerA->fullName;
  $amount = numFld(t('Amount: '), [t('Amount')]);
  $uid = hidFld($a->id);
  $mode = hidFld('normal');
  $photo = item($a->agentA->photoHtml(FALSE, TRUE));
  $agentName = $a->agentA->fullName;
  $subtext = ($a->co ? "<small>$agentName</small><br>" : '') . "<b>$a->fullName</b><br>$a->city, $a->st";

  if (selfServe()) {
    $title = t('Pay: ') . $myName;
    $choices = $choices0 = explode("\n", $scannerA->selling);
    $pay = submit(BARE . t('Pay'));
    $buttons = fieldSet('buttons', compact('pay'));
    
  } else { // not selfServe
    $title = t('You: ') . $myName;
  
    if ($scannerA->co) { // we are a company
      $choices0 = $scannerA->selling ? explode("\n", $scannerA->selling) : [];
      $choices = u\joinRays($choices0, ray(t('Refund to customer, %you receives cash, Customer buys cash from %you', 'you', $scannerA->fullName)));
    } else { // we are an individual
      $choices = $choices0 = $a->co ? explode("\n", $scannerA->selling) : []; // if other is a company assume pay -- an individual cannot charge a company this way
    }
    
    $mobileCo = ($scannerA->co and u\isMobile());
    $charge = submit(BARE . t('Charge'));
    $pay = (($mya or r\cook('trust')) and !$mobileCo) ? submit(BARE . t('Pay')) : NULL;
    $go = submit(BARE . t('Go'));
    $buttons = fieldSet('buttons', compact(ray('charge pay go')));
  }
  
  $choices[] = t('Other:');
  $for = selectFld(REQ . t('For:'), '', '', $choices);
  $desc = textFld(t('For:'), [t('Description')]);
  jsx('card', 'choice0Count', count($choices0));

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
  global $mya, $box;
  
  extract(just('mode account tail amount desc uid', $sta['input'], NULL));

  if ($mode == 'choose') { // chose an account to scan from
    $scannerA = r\acct($account);
    if ($scannerA->id != $mya->id) {
      w\signout(TRUE);
      w\boxId($box, $scannerA); // create boxid if it doesn't exist yet
    }
    r\setCook('scanner', $scannerA->mainQid); // not ->qid
    return go($tail ? "/card/$tail" : '/app');
  }

  // mode = 'normal'    
  list ($actorA, $a) = u\order(selfServe(), r\acct($uid), getScannerA());
  $op = op($sta);
  $goods = strhas($desc, t('cash')) ? FOR_USD : FOR_GOODS;
  $type = ($op == 'charge' ? 'charge' : 'payment');
  $z = be\transfer($type, $actorA, $a, $amount, $desc, $goods);

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
 * Show startup screen, having already chosen an associated account.
 */
function startup($actorA) {
  global $mya;

  $title = (selfServe() ? '' : t('You: ')) . $actorA->bestName;
  $subtext = ($mya and !in_array(r\cook('scanner'), [$mya->mainQid, $mya->qid])) ? t('<b class="loud">NOTE:</b> You are signed in, so you will be transacting on behalf of the current account, not the device\'s default scanning account.') : NULL;
  $lnkShowQr = $mya ? btn('/show-qr', t('Show My QR'), '', 'md', clas('btn-done')) : '';
  $lnkScanQr = btn('/scan-qr', ($mya and !$mya->co) ? t('Scan a CG Card') : t('Scan a Customer Card'), '', 'md', clas('btn-done'));
  $buttons = "$lnkShowQr$lnkScanQr";

  return cgform(compact(ray('title subtext buttons')));
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

  $title = $actorA ? ((selfServe() ? t('Pay: ') : t('You: ')) . $actorA->bestName) : t('Scanner');
  $lnkShowQr = $mya ? btn('/show-qr', t('Show My QR'), '', 'md', clas('btn-done')) : '';
  $lnkScanQr = btn('/scan-qr', t('Scan a CG Card'), '', 'md', clas('btn-done'));
  
  $buttons = "$lnkShowQr$lnkScanQr";
  if ($xid and $tx = r\Tx::read($xid) and !$tx->reversesXid and !$tx->reversedByXid) {
    if (!selfServe()) { // omit most buttons in selfServe mode
      foreach (ray(t('receipt:Receipt, undo:Undo, tip:Tip')) as $k => $desc) $$k = btn("/card/$k/xid=$xid", $desc, 'default', 'sm', clas("btn-done btn-$k"));
      if (!$actorA->coCan(CO_TIP) or tipAmt($tx)) $tip = '';
      $buttons = "$undo<br>$buttons<br>$tip$receipt";
    }
    $msg = t('Reverse your %amt charge to %who (%dt)?', 'amt who dt', u\fmtAmt($tx->amt2), $tx->a1->fullName, u\fmtDate($tx->created));
    jsx('cardDone', compact(ray('msg')));
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
  $amt = item(u\fmtAmt($tx->amt2, '$'), t('Amount:'));
  if ($tipAmt = tipAmt($tx)) {
    $tip = item(u\fmtAmt($tipAmt, '$'), t('Tip:'));
    $total = item(u\fmtAmt($tx->amt2 + $tipAmt, '$'), t('Total:'));
  } else $tip = $total = NULL;
  $for = item($tx->e2->description, t('For:'));
  $xid = item($tx->xid, t('Tx#:'));
  $agt = $tx->e2->agentUid == $tx->e2->uid ? NULL : item(r\acct($tx->e2->agentUid)->mainQid, t('Agent:'));
  $goBack = item('<br>' . btn('', t('Print'), '', 'xs', clas('btn-print')), '<br>' . btn('', '<span>&laquo;</span>&nbsp;' . t('Back'), '', 'xs', clas('btn-goback')));
  
  jsx('receipt');
  
  return cgForm(compact(ray('logo title dt nm acct to for xid agt amt tip total goBack')), 'tight', 'left');
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
      if (tipAmt($tx)) return cardDone(ray('ok msg xid', FALSE, t('You already have a tip connected with this purchase.'), $tx->xid));
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

function tipAmt($tx) {return db\get('amt', 'txs', ray('xid uid1 uid2 type', $tx->xid, $tx->uid1, $tx->uid2, E_AUX));}

/**
 * Return the account to scan from. Also set $boxId.
 */
function getScannerA() {
  global $mya, $box, $boxId;
  
  if ($mya) return $mya;
  $scanner = r\cook('scanner');
  $scannerA = r\acct($scanner);
  if ($scanner and (!$scannerA or !w\boxIdCheck($box, $scannerA))) return r\setCook('scanner', NULL);
  $boxId = $scannerA ? w\boxId($box, $scannerA) : NULL;
  return $scannerA;
}

function selfServe() {global $mya; return (r\cook('selfServe') and !$mya);}
