<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Testing as t;

/**
 * Pay or deny an invoice, possibly by clicking a link without signing in (from rDo).
 * If no user is signed in, $args must include the do code (to prevent mischief)
 * @param mixed $args: http query with parameters:
 *   int $nvid: invoice number
 *   string $ret: URL to return to
 *   bool $webpay: <this is confirmation after clicking a "Pay with Common Good" button>
 *   string $code: the docode, including op='inv' and v=nvid for verification
 */
function formHandleInvoice($form, &$sta, $args = '') {
  global $mya;

  extract(just('nvid ret webpay code', $args, NULL));
  $ret = $ret ? u\deurlify($ret) : 'empty'; // was empty/1

  if (!$inv = db\get('*', 'r_invoices', 'nvid=:nvid', compact('nvid'))) return w\go($ret, t('That link is no longer valid. To pay the invoice, sign in at :CG_DOMAIN and click History / Invoice To You.'), 'err');
  if ($err = r\badUnpaidInv($inv)) return w\go($ret, $err, 'ERR');
  extract(just('nvid amount payer payee purpose created data', $inv));
  
  if ($mya and !in($mya->id, [$payer, $payee])) w\signout(TRUE); // make sure we're signed out of any possibly different acct
  if (!$mya) { // not signed in (might have changed in previous line)
    if (test() and $code == 'TESTDOCODE') $code = t\doCode(); // most recently created doCode
    if (!$code or !$info = u\decryRay($code)) return hack(t('fake doCode in handleInvoice'), compact('args'));

    extract(just('op v', $info, NULL));
    if (!isset($v)) { // legacy (delete after 10/1/2020)
      $doInfo = db\get('data,uid', 'r_do', ray('doid', nni($info, 'op')));
      $doData = unserialize($doInfo['data']);
      extract(just('v', $doData, NULL));
      if ($doData['op'] != 'inv') $op = ''; // error
    }
    u\EXPECT($op == 'inv' and $v == $nvid, 'legacy handleInvoice fake');
  }

  $subs = ray('amount what date', u\fmtAmt($amount), $purpose, u\fmtDate($created, TRUE));
  $toMe = ($mya and $mya->id == $payee); // toMe means payment is to me

  if ($toMe) { // confirming delete by originator
    $title = t('Delete?');
    $subs += ray('who', r\acct($payer)->fullName);
    $subtext = t('Delete %date invoice charging %who %amount for %what?', $subs);
    $delete = submit(t('Delete'));
    $cancel = submit(t('Cancel'));
    $delSet = fieldSet('delSet', compact('delete', 'cancel'), BARE); // separate it from subtext

  } else { // approving or denying
    $a = r\acct($payer);
    $title = t('Confirm Payment');
    $subs += ray('who', r\acct($payee)->fullName);
    $amount = be\invRemains($nvid);

    if ($a->hasBank) {
      $_warning = t('<p><b>IMPORTANT:</b> To avoid insufficient funds charges, be sure there are enough funds in your bank account to cover any transfer that may result from approving payment of this invoice (and future invoices, if you check the "Auto" box).</p>');
      if (!$webpay) $always = boxFld('always', t('Auto:'), t('Pay all invoices from %who automatically.', $subs), 0);
    } elseif ($a->avail() < $amount) {
      $_warning = t('You cannot approve this invoice until you have enough money in your account. Sign in at %CG_DOMAIN and click "Bank" on the menu to transfer funds from (or to) your bank account. Then click History / Invoices TO You and select this invoice to approve it.');
      $disabled = w\disabled();
    } else $_warning = '';

    $payAmount = numFld(t('Amount to Pay:'), [t('Amount')], dft(number_format($amount, 2)));
    $amount = u\fmtAmt($amount);
    $remains = $amount == $subs['amount'] ? '' : t(' (%amount remaining)', compact('amount'));
    $subtext = t('<p>Pay %amount to %who for %what%remains.</p>%warning', $subs + compact(ray('_warning remains')));
    $link = '&nbsp; &nbsp;' . w\lnk('', t('dispute'), ray('id', 'dispute-it'));
    $pay = submit(t('Pay Now'), '', '', w\suffix($link) + (nn($disabled) ?: []));
    
    $whyNot = textFld(t('Reason for dispute:'));
    $deny = submit(t('Dispute'));
    $denySet = fieldSet('denySet', compact('whyNot', 'deny'), BARE);
  }
  
  $toMe = hidFld($toMe);
  $hid = hidFlds(compact('ret') + $inv);
  w\jsx('dispute');

  $info = []; foreach (ray('title subtext payAmount always pay denySet delSet toMe hid') as $k) if (isset($$k)) $info[$k] = $$k;
  $info['#validate'] = array('CG\\Web\\formHandleInvoice_validate');
  $info['#submit'] = array('CG\\Web\\formHandleInvoice_submit');
  
  return cgform($info);
}

function formHandleInvoice_validate($form, &$sta) {
  extract(just('whyNot toMe payAmount hid', $sta['input'], NULL));
  extract(just('amount', hidFlds($hid), NULL));
  $deny = (op($sta) == 'deny');
  if (!$toMe and $deny and !$whyNot) return say('say why not', 'whyNot');
  if ($toMe and $err = u\badAmount($payAmount, '>', 2, $amount)) return say($err, 'payAmount');
}

function formHandleInvoice_submit($form, &$sta) {
  $hidFlds = 'ret nvid payer payee purpose created data';
  extract(just('hid whyNot always payAmount ' . $hidFlds, $sta['input'], NULL)); // hidFlds here is for testing
  extract(just($hidFlds, hidFlds($hid)));

  list ($fromA, $toA) = [r\acct($payer), r\acct($payee)];
  $payeeName = $toA->fullName;
  $subs = ray('amount payerName payeeName created purpose reason', u\fmtAmt($payAmount), $fromA->fullName, $payeeName, u\fmtDate($created), $purpose, $whyNot);

  $op = op($sta);
  $ok = TRUE; // messages default to status, rather than error

  if ($op == 'pay' and !$whyNot) {
    $short = round($payAmount - $fromA->avail(), 2);
    if ($fromA->ok and $short <= 0) {
      list($ok, $msg) = be\payInvoice($nvid, $payAmount);
    } else {
      db\update('r_invoices', ray('nvid status', $nvid, TX_APPROVED), 'nvid');
      if ($fromA->ok) {
        $msg = $fromA->hasBank ? 'expect a transfer' : 'when funded|how to fund';
        $msg = t('short invoice', ray('short payeeName nvid', u\fmtAmt($short), $payeeName, $nvid)) . "|$msg";
      } else $msg = t('finish signup|when funded');
    }
    
    if ($always) {
      r\acct($payee, $payer)->setRel('customer autopay', TRUE, TRUE);
      $msg .= '|' . t('auto pay on', compact('payeeName'));
    }
  } elseif ($op == 'delete') {
    db\del('r_invoices', 'nvid', $nvid);
    $fromA->tellOut('invoice withdrawn', $subs);
    $msg = 'invoice delete done';
  } elseif ($op == 'deny' or $whyNot) {
    $data = serialize(compact('whyNot') + ($data ? unserialize($data) : []));
    db\update('r_invoices', ray('status data nvid', TX_DENIED, $data, $nvid), 'nvid');
    $toA->tellIn('invoice denied', $subs);
    $msg = 'denial done';
  } else $msg = ''; // default: canceled

  return w\go($ret, $msg ?: 'nothing done', $ok ? '' : 'ERR');
}