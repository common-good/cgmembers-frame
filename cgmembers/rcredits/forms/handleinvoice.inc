<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Testing as t;

const ACT_CANCEL = 1; // radio button to cancel invoice

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
  
  if (!$inv = db\get('*', 'tx_requests', 'nvid=:nvid', compact('nvid'))) return w\go($ret, t('That link is no longer valid. To approve this payment request, sign in at %CG_DOMAIN and click History / Pending From You.'), 'ERR');
  if ($err = r\badUnpaidInv($inv)) return w\go($ret, $err, 'ERR');
  extract(just('nvid amount payer payee purpose created data status flags', $inv));

  if ($code) {
    if (test() and $code == 'TESTDOCODE') $code = t\doCode(); // most recently created doCode
    if (!$info = u\decryRay($code)) return hack(t('fake doCode in handleInvoice'), compact(ray('args code info')));
    extract(just('op v', $info, NULL));
    u\EXPECT($op == 'inv' and $v == $nvid, t('handleInvoice fake') . pr(compact(ray('args code info'))));
//    if (!w\itsMyBox($payer)) return w\go($ret, t('You can pay an invoice only on a device you have signed in from.'), 'ERR');
  }
  
  $toMe = ($mya and in($payee, $mya->uids)); // toMe means payment is to me (possible only when signed in)
  $fromMe = ($mya and in($payer, $mya->uids)); // fromMe means signed in and paying

  if ($mya and !$toMe and !$fromMe) w\signout(TRUE); // make sure we're signed out of any possibly different acct
  if (!$mya and !$code) return go('/signin'); // not signed in (might have changed in previous line); probably admin browser refresh (but timed out)

  $subs = ray('nvid amount what date', $nvid, u\fmtAmt($amount), $purpose, fmtDt($created));
  $data = unserialize(nn($data)) ?: [];
  $reasonDft = nni($data, 'reason') ?: nni($data, 'whyNot'); // whyNot is deprecated (maybe convert it?)

  if ($toMe and in($status, [TX_PAID, TX_CANCELED])) { // manually closed invoiced
    $title = t('Invoice Was Closed Manually');
    $subs += ray('who', r\acct($payer)->fullName);
    $subtext = t('Reopen invoice #%nvid (%date) charging %who %amount for %what?', $subs);
    $because = item($reasonDft, t('Closed Because:'));
    $reopen = submit(t('Reopen'), '', '', suffix(lnk('', t('Go back'), clas('btn-back'))));
    jsx('back-button');
    
  } elseif ($toMe and $status < TX_PAID) { // unpaid invoice (cancel or close)
    $title = t('Unpaid Invoice');
    $subs['who'] = r\acct($payer)->fullName;
    $subtext = t('Handle invoice #%nvid (%date) charging %who %amount for %what.', $subs);
    $action = radiosFld(REQ . t('Action:'), '', dft(0), ray(t('mark paid, cancel')));

    $because = textFld(REQ . t('Reason:'), [t('Reason'), t('If the invoice was paid some other way, describe it here.')], dft($reasonDft));

    $close = submit(t('Close'));
    jsx('back-button');
    
  } elseif ($toMe) { // handle an already-paid request for payment TO me
    return go($ret, t('Request #%nvid is already paid.', compact('nvid')));
  /*
    list ($verb, $explain) = [t('Reverse'), t(' (by creating an offsetting negative invoice and paying it back)')];
    $title = $verb . '?';
    $subs += ray('who explain', r\acct($payer)->fullName, $explain);
    $subtext = $verb . t(' %date invoice charging %who %amount for %what%explain?', $subs);
    $cancel = submit(t('Cancel Invoice'), '', '', suffix(lnk('', t('Go back'), clas('btn-back'))));
    jsx('back-button');
    */
  } else { // approving or denying a requested payment FROM me
    $a = r\acct($payer);
    $title = t('Confirm Payment');
    $subs += ray('who', r\acct($payee)->fullName);
    $remaining = be\invRemains($nvid);

    if ($a->hasBank) {
      $_warning = t('<p><b>IMPORTANT:</b> To avoid insufficient funds charges, be sure there are enough funds in your bank account to cover any transfer that may result from approving payment of this invoice (and future invoices, if you check the "Auto" box).</p>');
      if (!$webpay) $always = boxFld('always', t('Auto:'), t('Pay all invoices from %who automatically.', $subs), 0);
    } elseif ($a->avail() < $remaining) {
      $_warning = t('<p><b>NOTE:</b>You cannot approve this invoice until you have enough money in your account. Sign in at %CG_DOMAIN and click "Bank" on the menu to transfer funds from (or to) your bank account. Then click History / Pending FROM You and select this invoice to approve it.</p>');
      $disabled = w\disabled();
    } else $_warning = '';

    $payAmount = numFld(t('Amount to Pay:'), [t('Amount')], dft($remaining, 2));
    $remains = u\fmtAmt($remaining);
    $remains = $remains == $subs['amount'] ? '' : t(' (%remains remaining)', compact('remains'));
    $subtext = t('<p>Pay %amount to %who for %what%remains.</p>%warning', $subs + compact(ray('_warning remains')));
    $remaining = hidFld($remaining);

    if (u\getBit($flags, B_SELF) and $status == TX_APPROVED) {
      $cancel = submit(t('Cancel Payment'), '', '', suffix(lnk('', t('Go back'), clas('btn-back'))));
    } else {
      if ($status >= TX_PAID) return w\go($ret, t('inv already paid'), 'ERR');
      if (u\getBit($flags, 'funding')) return w\go($ret, t('inv already funding'), 'ERR');
      $link = '&nbsp; &nbsp;' . w\lnk('', t('dispute'), ray('id', 'dispute-it'));
      $choices = [
        t('Pull this entire amount from your bank account.'),
        t('Pay first from your Common Good balance; then from your bank account as needed.'),
      ];
      $auto = $a->hasBank ? radiosFld(REQ . NBSP, dft(0), [], $choices) : hidFld(1);
      $pay = submit(t('Pay Now'), '', '', w\suffix($link) + (nn($disabled) ?: []));
    
      $reason = textFld(t('Reason for dispute:'));
      $deny = submit(t('Dispute'));
      $denySet = fieldSet('denySet', compact('reason', 'deny'), BARE);
    }
  }
  
  $toMe = hidFld($toMe);
  $hid = hidFlds(compact('ret') + $inv);
  w\jsx('dispute');

  $info = []; foreach (ray('title subtext payAmount always auto pay action because denySet close cancel reopen toMe remaining hid') as $k) if (isset($$k)) $info[$k] = $$k;
  $info['#validate'] = array('CG\\Web\\formHandleInvoice_validate');
  $info['#submit'] = array('CG\\Web\\formHandleInvoice_submit');
  
  return cgform($info);
}

function formHandleInvoice_validate($form, &$sta) {
  extract(just('reason toMe payAmount remaining', $sta['input'], NULL));
  extract(just('amount', hidFlds($sta, 'hid'), NULL));
  $deny = (op($sta) == 'deny');
  if (!$toMe and $deny and !$reason) return say('say why not', 'reason');
  if (isset($payAmount) and $err = u\badAmount($payAmount, '>0', 2, $remaining)) return say($err, 'payAmount');
}

function formHandleInvoice_submit($form, &$sta) {
  extract(just('payAmount', $input = $sta['input'], NULL));

  $hidInfo = hidFlds($sta, 'hid');
  handleInvoice(op($sta), $payAmount, $hidInfo + just('always reason action auto', $input));
  return w\go($hidInfo['ret']);
}

function handleInvoice($op, $payAmount, $info) {
  extract(just('nvid payer payee purpose created data amount always reason action auto', $info, NULL));
  list ($fromA, $toA) = [r\acct($payer), r\acct($payee)];
  $subs = ray('amount payerName payeeName created purpose reason', u\fmtAmt($amount), $fromA->fullName, $toA->fullName, fmtDt($created), $purpose, $reason);
  $ok = TRUE; // messages default to type "status", rather than "error"
  $data = ($data ? unserialize($data) : []);
  
  if ($op == 'pay' and !$reason) {
    $flags = db\get('flags', 'tx_requests', compact('nvid'));
    db\update('tx_requests', ray('nvid status', $nvid, TX_APPROVED), 'nvid'); // in case it wasn't marked approved already
    if ($fromA->ok) { // if payer account is active
      extract(just('ok msg', $z = be\payInvoice($nvid, $auto, $payAmount)));
      if ($payAmount < $amount) { // don't let the funding flag prevent payer from completing payment!
        $flags = u\setBit($flags, 'funding', FALSE);
        db\update('tx_requests', compact(ray('nvid flags')), 'nvid');
      }
    } else { // if payer account is not active yet
      $msg = t('finish signup|when funded');
    }
    if ($always) {
      r\acct($payee, $payer)->setRel('customer autopay', TRUE, TRUE);
      $msg .= '|' . t('auto pay on', just('payeeName', $subs));
    }
    
  } elseif ($op == 'close') { // only payee can cancel
    if (db\get('status', 'tx_requests', compact('nvid')) >= TX_PAID) return go('history/pending-to', tr('inv already paid'), 'err'); // bad timing
    list ($status, $done) = $action == ACT_CANCEL 
    ? [TX_CANCELED, t('canceled')] 
    : [TX_PAID, t('marked PAID')];
    $data = serialize(compact('reason') + $data);
    db\update('tx_requests', compact(ray('status data nvid')), 'nvid');
    $fromA->tellOut('invoice withdrawn', $subs + compact('done'));
    $msg = tr('invoice done', compact('done'));
    
  } elseif ($op == 'reopen') {
    unset($data['reason']);
    db\update('tx_requests', ray('status data nvid', TX_PENDING, serialize($data), $nvid), 'nvid');
    $link = $fromA->doLink('inv', $nvid);
    $subs += ray('otherName otherEmail otherPhone _a1 reply', 
      $toA->fullName, $toA->email, u\fmtPhone($toA->phone), $link, $payee);
    $fromA->tell('invoice reopened|invoiced you', $subs);
    $msg = tr('inv reopened');
    
  } elseif ($op == 'deny' or $reason) {
    $data = serialize(compact('reason') + $data);
    db\update('tx_requests', ray('status data nvid', TX_DENIED, $data, $nvid), 'nvid');
    $toA->tell('invoice denied', $subs);
    u\tellApp('denied', $fromA, $toA, $amount, $purpose, $created, $reason);
    $msg = 'denial done';
    
  } else $msg = ''; // default: canceled

  say($msg ?: 'nothing done', $ok ? '' : 'ERR');
  return $ok;
}
