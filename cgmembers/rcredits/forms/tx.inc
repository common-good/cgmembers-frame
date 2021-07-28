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
 */
function formTx($form, &$sta, $type, $args = '') {
  extract(just('who amount goods purpose', $args));
  global $mya;

  $fbo = $mya->sponsored;
  $pay = in($type, 'pay both');
  $charge = (in($type, 'charge both') and (!$fbo or $mya->admin)); // don't show charge button for sponsored org, except to admin
  list ($payDesc, $chargeDesc) = [t('Pay'), t('Charge')];
  $opDesc = $pay ? $payDesc : $chargeDesc;
  
  $selling = explode("\n", $mya->selling);
  $purpose = nn($purpose) ?: ($charge ? nni($selling, 0) : '');
  
  focusOn(nn($who) ? 'amount' : 'who');

//  $mem = rent(radiosFld('', '', ray('inline', 1), ray(t('Member Non-member')))); // fails
  $mem = rentRadios('mem', 0, ray(t('Member Non-member')));

  $title = $fbo ? item($opDesc, NBSP, '', suffix($mem)) : $opDesc;

  if ($mya->co) {
    $buttons = $pay ? btnOpt('/tx/pay/payments', t('Upload Payments'), 'w-pay') : '';
    if ($pay) $buttons .= btnOpt('/tx/pay/payroll', t('Upload Payroll'), 'w-pay');
    if ($charge) $buttons .= btnOpt('/tx/charge/invoices', t('Upload Invoices'), 'w-charge');
  } else $buttons = NULL;
  
  $who = textFld(t('Who:'), [t('Name, id, email, or phone')], dft(nn($who)));

  $fullName = textFld(t('Full Name:'), [t('Full name properly capitalized')]);
  $address = textFld(t('Postal Addr:'), [t('Number and street')]);
  $city = textFld(t('City:'), [t('Postal city')]);
  $state = stateFld(NULL, '');
  $zip = textFld(t('Zip:'), [t('Postal code')]);
  $phone = phoneFld(t('Phone:'), [t('Phone number')]);
  $email = emailFld(t('Email:'), [t('Email address')]);
  $addr = fieldSet('addr', compact(ray('fullName address city state zip phone email')), BARE);

  $amount = numFld(REQ . t('Amount:'), [t('Transaction amount')], dft(nn($amount)) + vmin(.01));
  $purpose = textFld(REQ . t('For:'), [t('Purpose or description of goods and services')], dft(nn($purpose)));
  $cat = $fbo ? catFld() : NULL; // can't have different budgetCats, so start with Income (it's short)
  $start = dateFld(t('Date:'));
  $period = rendA('period', selectFld(BARE, '', dft(''), r\recurRay(FALSE, PERIODS_PLURAL)));
  $periods = intFld(t('Every:'), '', dft(0) + suffix($period));
  $end = dateFld(t('Ending:'), t('Leave blank for "never". You can end it at any time at <%a>History / Automated Payments</a>.', '_a', atag('/history/recurring')));
  
  $advanced = item(btnOpt('', t('delay'), 'btn-delay') . btnOpt('', t('repeat'), 'btn-repeat'));
  $submit = t('Submit');

  list ($field, $question, $selfErr) = ['who', t(' %amount to %name?'), 'self-tx'];
  // duplicate scraps.js's question, restrict, and allowNonmember in whoFldSubmit() for testing
  w\whoFldSubmit($field, $opDesc . $question, $selfErr, $pay ? ':IS_OK' : '', ray('allowNonmember custom', $charge, 1));
  
  $admin = $mya->admin;
  jsx('tx', compact(ray('field question selfErr payDesc chargeDesc fbo admin')));
  
  $paying = hidFld($pay);
  $fbo = hidFld($fbo);
  $form = cgform(compact(ray("title buttons who addr amount purpose cat start periods end advanced submit paying fbo")));

  return $form;
}

function formTx_validate($form, &$sta) {
  global $mya;
  $info = just('amount purpose start end paying fbo who fullName address city state zip phone email', $sta['input'], NULL);
  extract($info);
/*  if (mb_strpos($who, '@') and !u\validEmail($email = $who)) return say('bad email', compact('email'), 'who');
  if (!$a = w\whois($who, 'who', $info, 'self-tx', $paying)) return FALSE;
  $uid = $a->id; */
  if (!$who) {
    if (!$paying and !$mya->admin) return say(t('Only %PROJECT admins can charge non-members.'), 'err');
    if ($err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');
    if ($zip and $err = u\badZip($zip)) return say($err, 'zip');
    if ($phone and $err = u\badPhone($phone)) return say($err, 'phone');
    if ($email and !u\validEmail($email)) return say('bad email', 'email');
  }
  
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
  foreach (ray('start end') as $k) if ($$k and $err = u\badDate($$k, $mya->superAdmin ? today() - 180 * DAY_SECS : today())) return say($err, $k);
  u\preray(compact(ray('fullName zip amount start end')), $sta['input']);
}

function formTx_submit($form, &$sta) {
  global $mya;

  extract($input = just('amount purpose period periods start end goods paying fbo who fullName address city state zip phone email cat', $sta['input'], NULL));
//  if (mb_strpos($who, '@')) return w\go('/charge/nonmember/' . u\urlify(serialize($input)));

  $DBTX = \db_transaction();

  if ($who) { // normal account
    $uid = whoId($sta);
    $pid = NULL;
  } else { // FBO (sponsored) account
    u\EXPECT($fbo and ($paying or $mya->admin), 'fbo income by non-admin');
    $uid = UID_OUTER;
    if (!$pid = db\get('pid', 'people', $email ? compact('email') : compact(ray('fullName address zip')))) {
      $pid = db\insert('people', compact(ray('fullName address city state zip phone email')), 'pid');
    }
    $info = ray('purpose pid completed deposit explicit cat', $purpose, $pid, now(), $mya->admin ? now() : 0, TRUE, $cat);
    $xid = be\createUsdTransfer($mya, $paying ? -$amount : $amount, $info);
    u\EXPECT($xid, 'outer transfer failed');
    say('info saved');
    $res = be\thanks(FALSE, $pid, $amount, PER_1, 0, $mya->id, TRUE);
    unset($DBTX);
    return $res; // no need to continue (outer transactions never delay or repeat)
  }
  
  u\setDft($goods, FOR_GOODS);
  $delay = ($start > today());
  $repeats = ($periods > 0);
  
  if ($delay) { // delaying, so do nothing now but create a template for the thing (repeating or not)
    list($ok, $msg, $subs) = [TRUE, '', []];
  } else { // no delay
     
    $data = $period ? ['recurs' => 1] : [];
    if ($start) $data['created'] = $start; // superAdmin chose a date in the past
    if ($fbo) $data += compact('cat');

    $z = $paying
    ? $mya->payApproved($uid, $amount, $purpose, $goods, $data, $period ? [$period, $periods] : NULL)
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
    $where = "tm.from=:from AND tm.to=:to AND action=:ACT_PAY AND period<>:PER_1 AND IFNULL(end, :NOW + 1) > :NOW";
    if (!$delay and db\exists('tx_timed tm', $where, $info)) {
      $otherName = db\get('fullName', 'users', compact('uid'));
      say(t('Note: You also have another active recurring transaction with %otherName. Go to <%a>History / Recurring Payments</a> to manage your recurring transactions.', 'otherName _a', $otherName, atag('/history/recurring')));
    }
    
    $subs += ray('often', r\recurDesc($period, $periods));
    $msg = u\joins($msg, '|', 'repeats');
  } else {
    list ($periods, $period) = [1, PER_1];
  }
  
  if (!$start) $start = today(); // handle ''
  $info += compact(ray('action amount purpose period periods start end'));
  $id = db\insert('tx_timed', $info, 'id');

  if ($delay) {
    $msg = u\joins($msg, '|', 'thing scheduled');
    $subs += compact('thing');
    r\acct($uid)->tell($msg, $subs, t('payment scheduled'), $paying ? SEND_OUT : SEND_IN);
  } else { // no delay: a recurring transaction or invoice was created, so set its recursId (pointing to tx_template record)
    extract(just('xid nvid', $subs, NULL)); // no other way to know whether payApproved created a tx or an invoice
    list ($tnm, $idFnm) = $xid ? ['tx_hdrs', 'xid'] : ['tx_requests', 'nvid'];
    db\update($tnm, ray("$idFnm recursId", $subs[$idFnm], $id), $idFnm);
  }

  unset($DBTX);

  say(tr($msg, $subs), $ok ? '' : 'err');
  return go('dashboard');
}
