<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Get a transaction request (usually pay or charge) and process it
 * @param string $args: all or some parameters for the form (when called from directory or draft form)
 * NOTE: currently handles only pay and charge
 */
function formTx($form, &$sta, $type, $args = '') {
  extract(just('who amount goods purpose', $args));
  global $mya;

  $pay = in($type, 'pay both');
  $charge = in($type, 'charge both');
  $opDesc = $pay ? t('Pay') : t('Charge');
  
  $selling = explode("\n", $mya->selling);
  $purpose = nn($purpose) ?: ($charge ? nni($selling, 0) : '');
  
  focusOn(nn($who) ? 'amount' : 'who');

  $title = $opDesc;
  if ($mya->co) {
    $buttons = $pay ? btn('/tx/pay/payments', t('Upload Payments'), 'default', '', clas('w-pay')) : '';
    if ($pay) $buttons .= btn('/tx/pay/payroll', t('Upload Payroll'), 'default', '', clas('w-pay'));
    if ($charge) $buttons .= btn('/tx/charge/invoices', t('Upload Invoices'), 'default', '', clas('w-charge'));
  } else $buttons = NULL;
  $who = textFld(t('Who:'), [t('name, id, email, or phone')], required(nn($who)));

  $amount = numFld(REQ . t('Amount:'), [t('transaction amount')], dft(nn($amount)) + vmin(.01));
  $purpose = textFld(t('For:'), [t('purpose or description of goods and services')], required(nn($purpose)));
  $start = dateFld(t('Date:'));
  $period = rendA('period', selectFld(BARE, '', dft(''), r\recurRay(FALSE, PERIODS_PLURAL)));
  $periods = intFld(t('Every:'), '', dft(0) + suffix($period));
  $end = dateFld(t('Ending:'), t('Leave blank for "never". You can end it at any time at <%a>History / Recurring Transactions</a>.', '_a', atag('/history/recurring')));
  
  $advanced = item(btn('', t('delay'), 'default', '', 'btn-delay') . btn('', t('repeat'), 'default', '', 'btn-repeat'));
  $submit = t('Submit');

  w\whoFldSubmit('who', $opDesc . t(' %amount to %name?'), ray('allowNonmember', $charge));
  $paying = hidFld($pay);
  
  jsx('tx');
  $form = cgform(compact(ray("title buttons who amount purpose start periods end advanced submit paying")));

  return $form; // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
  global $mya;
  $info = just('who amount purpose start end paying', $sta['input'], NULL);
  extract($info);
  if (mb_strpos($who, '@') and !u\validEmail($email = $who)) return say('bad email', compact('email'), 'who');
  if (!$a = w\whois($who, 'who', $info, 'no self-trading', $paying)) return FALSE;
  $uid = $a->id;
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
  foreach (ray('start end') as $k) if ($$k and $err = u\badDate($$k, $mya->superAdmin ? today() - 180 * DAY_SECS : today())) return say($err, $k);
  u\preray(compact(ray('amount uid start end')), $sta['input']);
}

function formTx_submit($form, &$sta) {
  global $mya;
  $myid = $mya->id;
  extract($info = just('amount purpose period periods start end uid goods paying', $sta['input'], NULL));
//  if (mb_strpos($who, '@')) return w\go('/charge/nonmember/' . u\urlify(serialize($info)));

  u\setDft($goods, FOR_GOODS);
  $DBTX = \db_transaction();
  $delay = ($start > today());
  $repeats = ($periods > 0);
  
  if ($delay) { // delaying, so do nothing now but create a template for the thing (repeating or not)
    list($ok, $msg, $subs) = [TRUE, '', []];
  } else { // no delay
    $data = $period ? ['recurs' => 1] : [];
    if ($start) $data['created'] = $start; // superAdmin chose a date in the past
      
    $z = $paying
    ? $mya->payApproved($uid, $amount, $purpose, $goods, $data)
    : be\invoice($mya, r\acct($uid), $amount, $purpose, $goods, $data);
    extract(just('ok msg subs', $z));
    if (!$ok) $repeats = FALSE; // don't repeat a failure
    if (!$repeats) { // done if we're not repeating (and not delaying)
      unset($DBTX);
      say($msg, $ok ? '' : 'err');
      return go('dashboard');
    }
  }

  // delaying or repeating, so create a rule template
  if (!$end) $end = NULL;
  list ($action, $thing, $way) = $paying ? [ACT_PAY, t('payment'), 'from to'] : [ACT_CHARGE, t('invoice'), 'to from'];
  $info = ray($way, $mya->id, $uid);
 
  if ($repeats) {
    // warn user if they already have a repeating transaction like this with this other account
    $where = "tp.from=:from AND tp.to=:to AND action=:ACT_PAY AND period<>'once' AND IFNULL(end, :NOW + 1) > :NOW";
    if (!$delay and db\exists('tx_templates tp', $where, $info)) {
      $otherName = db\get('fullName', 'users', compact('uid'));
      say(t('Note: You also have another active recurring transaction with %otherName. Go to <%a>History / Recurring Payments</a> to manage your recurring transactions.', 'otherName _a', $otherName, atag('/history/recurring')));
    }
    
    $subs += ray('often', r\recurDesc($period, $periods));
    $msg = u\joins($msg, '|', 'repeats');
  } else {
    list ($periods, $period) = [1, 'once'];
  }
  
  if (!$start) $start = today(); // handle ''
  $info += compact(ray('action amount purpose period periods start end'));
  $id = db\insert('tx_templates', $info, 'id');

  if ($delay) {
    $msg = u\joins($msg, '|', 'thing scheduled');
    $subs += compact('thing');
    r\acct($uid)->tell($msg, $subs, t('payment scheduled'), $paying ? SEND_OUT : SEND_IN);
  } else { // no delay: either a transaction or invoice was created, so set its recursId (pointing to template)
    extract(just('xid nvid', $subs, NULL)); // no other way to know whether payApproved created a tx or an invoice
    list ($tnm, $idFnm) = $xid ? ['tx_hdrs', 'xid'] : ['r_invoices', 'nvid'];
    db\update($tnm, ray("$idFnm recursId", $subs[$idFnm], $id), $idFnm);
  }

  unset($DBTX);

  say(tr($msg, $subs), $ok ? '' : 'err');
  return go('dashboard');
}
