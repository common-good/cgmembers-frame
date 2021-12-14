<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Testing as t;

/**
 * Approve, Pay (in part or in full), cancel, deny, or reverse a pending payment, possibly by clicking a link without signing in (from rDo).
 * If no user is signed in, $args must include the do code (to prevent mischief)
 * @param mixed $args: http query with parameters:
 *   int $nvid: request/invoice number
 *   string $ret: URL to return to
 *   bool $webpay: <this is confirmation after clicking a "Pay with Common Good" button>
 *   string $code: the docode, including op='inv' and v=nvid for verification
 */
function formHandleInvoice($form, &$sta, $args = '') {
  global $mya;

  extract(just('nvid ret webpay code', $args, NULL));
  $ret = $ret ? u\deurlify($ret) : 'empty'; // was empty/1

  if (!$inv = db\get('*', 'tx_requests', 'nvid=:nvid', compact('nvid'))) return w\go($ret, t('That link is no longer valid. To approve this payment request, sign in at :CG_DOMAIN and click History / Pending FROM You.'), 'err');
  if ($err = r\badUnpaidInv($inv)) return w\go($ret, $err, 'ERR');
  extract(just('nvid amount payer payee purpose created data status flags', $inv));
  
  if ($mya and !in($mya->id, [$payer, $payee])) w\signout(TRUE); // make sure we're signed out of any possibly different acct
  if (!$mya) { // not signed in (might have changed in previous line)
    if (test() and $code == 'TESTDOCODE') $code = t\doCode(); // most recently created doCode
    if (!$code) return go('/signin'); // probably admin browser refresh (but timed out)
    if (!$info = u\decryRay($code)) return hack(t('fake doCode in handleInvoice'), compact('args'));

    extract(just('op v', $info, NULL));
    u\EXPECT($op == 'inv' and $v == $nvid, 'legacy handleInvoice fake');
  }

  $subs = ray('amount what date', u\fmtAmt($amount), $purpose, u\fmtDate($created, TRUE));
  $toMe = ($mya and $mya->id == $payee); // toMe means payment is to me

  if ($toMe) { // cancel or reverse a payment request TO me (reversing currently has no effect, because already-paid invoices just get an error message)
    list ($verb, $explain) = $status < TX_PAID ? [t('Cancel'), ''] : [t('Reverse'), t(' (by creating an offsetting negative invoice and paying it back)')];
    $title = $verb . '?';
    $subs += ray('who explain', r\acct($payer)->fullName, $explain);
    $subtext = $verb . t(' %date invoice charging %who %amount for %what%explain?', $subs);
    $cancel = submit(t('Cancel Invoice'), '', '', suffix(lnk('', t('Go back'), clas('btn-back'))));
    jsx('back-button');
  } else { // approving or denying a requested payment FROM me
    $a = r\acct($payer);
    $title = t('Confirm Payment');
    $subs += ray('who', r\acct($payee)->fullName);
    $amount = be\invRemains($nvid);

    if ($a->hasBank) {
      $_warning = t('<p><b>IMPORTANT:</b> To avoid insufficient funds charges, be sure there are enough funds in your bank account to cover any transfer that may result from approving payment of this invoice (and future invoices, if you check the "Auto" box).</p>');
      if (!$webpay) $always = boxFld('always', t('Auto:'), t('Pay all invoices from %who automatically.', $subs), 0);
    } elseif ($a->avail() < $amount) {
      $_warning = t('<p><b>NOTE:</b>You cannot approve this invoice until you have enough money in your account. Sign in at %CG_DOMAIN and click "Bank" on the menu to transfer funds from (or to) your bank account. Then click History / Pending FROM You and select this invoice to approve it.</p>');
      $disabled = w\disabled();
    } else $_warning = '';

    $payAmount = numFld(t('Amount to Pay:'), [t('Amount')], dft($amount, 2));
    $amount = u\fmtAmt($amount);
    $remains = $amount == $subs['amount'] ? '' : t(' (%amount remaining)', compact('amount'));
    $subtext = t('<p>Pay %amount to %who for %what%remains.</p>%warning', $subs + compact(ray('_warning remains')));
    if (u\getBit($flags, B_SELF) and $status == TX_APPROVED) {
      $cancel = submit(t('Cancel Invoice'), '', '', suffix(lnk('', t('Go back'), clas('btn-back'))));
    } else {
      $link = '&nbsp; &nbsp;' . w\lnk('', t('dispute'), ray('id', 'dispute-it'));
      $pay = submit(t('Pay Now'), '', '', w\suffix($link) + (nn($disabled) ?: []));
    
      $whyNot = textFld(t('Reason for dispute:'));
      $deny = submit(t('Dispute'));
      $denySet = fieldSet('denySet', compact('whyNot', 'deny'), BARE);
    }
  }
  
  $toMe = hidFld($toMe);
  $hid = hidFlds(compact('ret') + $inv);
  w\jsx('dispute');

  $info = []; foreach (ray('title subtext payAmount always pay denySet cancel toMe hid') as $k) if (isset($$k)) $info[$k] = $$k;
  $info['#validate'] = array('CG\\Web\\formHandleInvoice_validate');
  $info['#submit'] = array('CG\\Web\\formHandleInvoice_submit');
  
  return cgform($info);
}

function formHandleInvoice_validate($form, &$sta) {
  extract(just('whyNot toMe payAmount', $sta['input'], NULL));
  extract(just('amount', hidFlds($sta, 'hid'), NULL));
  $deny = (op($sta) == 'deny');
  if (!$toMe and $deny and !$whyNot) return say('say why not', 'whyNot');
  if ($toMe and isset($payAmount) and $err = u\badAmount($payAmount, '>', 2, $amount)) return say($err, 'payAmount');
}

function formHandleInvoice_submit($form, &$sta) {
  $hidFlds = 'ret nvid payer payee purpose created data amount';
  extract(just('whyNot always amount payAmount toMe ', $sta['input'], NULL));
  extract(just($hidFlds, hidFlds($sta, 'hid')));

  list ($fromA, $toA) = [r\acct($payer), r\acct($payee)];
  $payeeName = $toA->fullName;
  $subs = ray('amount payerName payeeName created purpose reason', u\fmtAmt($amount), $fromA->fullName, $payeeName, u\fmtDate($created), $purpose, $whyNot);

  $op = op($sta);
  $ok = TRUE; // messages default to status, rather than error

  if ($op == 'pay' and !$whyNot) {
    if ($fromA->ok) {
      extract(just('ok msg', be\payInvoice($nvid, $payAmount))); // marks it approved if insufficient funds
    } else {
      db\update('tx_requests', ray('nvid status', $nvid, TX_APPROVED), 'nvid'); // in case it wasn't marked approved already
      $msg = t('finish signup|when funded');
    }
    
    if ($always) {
      r\acct($payee, $payer)->setRel('customer autopay', TRUE, TRUE);
      $msg .= '|' . t('auto pay on', compact('payeeName'));
    }
  } elseif ($op == 'cancel') {
    if (db\get('status', 'tx_requests', compact('nvid')) >= TX_PAID) return go('history/pending-to', tr('inv already paid'), 'err'); // bad timing
    db\del('tx_requests', 'nvid', $nvid);
    if ($toMe) $fromA->tellOut('invoice withdrawn', $subs);
    $msg = 'invoice cancel done';
  } elseif ($op == 'deny' or $whyNot) {
    $data = serialize(compact('whyNot') + ($data ? unserialize($data) : []));
    db\update('tx_requests', ray('status data nvid', TX_DENIED, $data, $nvid), 'nvid');
    $toA->tell('invoice denied', $subs);
    $msg = 'denial done';
  } else $msg = ''; // default: canceled

  say($msg ?: 'nothing done', $ok ? '' : 'ERR');
  return w\go($ret);
}
