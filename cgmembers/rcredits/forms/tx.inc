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
  extract(just('who amount goods purpose', $args, ''));
  global $mya;

  $fbo = $mya->sponsored;
  $hasCats = r\hasCats($mya->uid) ?: 0;

  $pay = in($type, 'pay both');
  $charge = (in($type, 'charge both') and (!$hasCats or $mya->admNonmemberTx)); // don't show charge button for insider org, except to authorized admin
  list ($payDesc, $chargeDesc) = [t('Pay'), t('Charge')];
  $opDesc = $pay ? $payDesc : $chargeDesc;
  
  $selling = explode("\n", $mya->selling ?: '');
  $purpose = $purpose ?: ($charge ? nni($selling, 0) : '');
  
  focusOn($who ? 'amount' : 'who');

  $mem = $mya->admNonmemberTx ? rentRadios('mem', 0, ray(t('Member Non-member'))) : '';
  $title = $mem ? item($opDesc, NBSP, '', suffix($mem)) : $opDesc;

  if ($mya->co) {
    $buttons = $pay ? btnOpt('/tx/pay/payments', t('Upload Payments'), 'w-pay') : '';
    if ($pay) $buttons .= btnOpt('/tx/pay/payroll', t('Upload Payroll'), 'w-pay');
    if ($charge) $buttons .= btnOpt('/tx/charge/invoices', t('Upload Invoices'), 'w-charge');
  } else $buttons = NULL;
  
  $who = textFld(t('Who:'), [t('Name, id, email, or phone')], dft(nn($who)));

  $fullName = textFld(t('Full Name:'), [t('Full name properly capitalized')]);
  $address = textFld(t('Postal Addr:'), [t('Number and street')]);
  $city = textFld(t('City:'), [t('Postal city')]);
  $state = stateFld(NULL, ''); // should be always possible
  $zip = textFld(t('Zip:'), [t('Postal code')]);
  $phone = phoneFld(t('Phone:'), [t('Phone number')]);
  $email = emailFld(t('Email:'), [t('Email address')]);
  $addr = fieldSet('addr', compact(ray('fullName address city state zip phone email')), BARE);

  $amount = numFld(REQ . t('Amount:'), [t('Amount')], dft($amount) + vmin(.01) + suffix(rent(boxFld('isGift', '', t('this is a donation'), NULL, notab()))));
  $purpose = textFld(REQ . t('For:'), [t('Purpose or description of goods and services')], dft(nn($purpose)));
  $method = radiosFld(t('Method:'), '', dft(0), ray(t('%B_CHECK:Check, %B_DIRECTACH:Direct ACH, %B_WIRE:Wire, %B_CC:Credit Card')));
  $ckNumber = textFld(t('Ck Number:'));
  $ckDate = dateFld(t('Ck Date:'));
  
//  $cat = ($mya->sponsored and !in($mya->id, r\cgsOwnUids()))? fboCatFld('', 'OUT') : ($hasCats ? catFld() : NULL); // note that fbo income gets categorized automatically, so jsx hides this
  $cat = $mya->sponsored? fboCatFld('', 'OUT') : ($hasCats ? catFld() : NULL); // note that fbo income gets categorized automatically, so jsx hides this
  $start = dateFld(t('Date:'));
  $period = rendA('period', selectFld(BARE, '', dft(''), r\recurRay(FALSE, PERIODS_PLURAL)));
  $periods = intFld(t('Every:'), '', dft(0) + suffix($period));
  $end = dateFld(t('Ending:'), t('Leave blank for "never". You can end it at any time at <%a>History / Automated Payments</a>.', '_a', atag('/history/recurring')));
  
  $advanced = item(btnOpt('', t('schedule'), 'btn-delay') . btnOpt('', t('repeat'), 'btn-repeat'));
  $submit = t('Submit');

  list ($field, $question, $selfErr) = ['who', t(' %amount to %name?'), 'self-tx'];
  // duplicate scraps.js's question, restrict, and allowNonmember in whoFldSubmit() for testing
  w\whoFldSubmit($field, $opDesc . $question, $selfErr, $pay ? ':IS_OK' : '', ray('allowNonmember custom', $charge, 1));
  
  $admin = $mya->admNonmemberTx;
  list ($methodDft, $byCheck) = [B_DIRECTACH, B_CHECK];
  jsx('tx', compact(ray('field question selfErr payDesc chargeDesc fbo admin hasCats methodDft byCheck')));
  
  $paying = hidFld($pay);
  $hasCats = hidFld($hasCats);
  $form = cgform(compact(ray("title buttons who addr amount purpose method ckNumber ckDate cat start periods end advanced submit paying hasCats")));

  return $form;
}

function formTx_validate($form, &$sta) {
  global $mya;
  $info = just('amount purpose start end paying who fullName address city state zip phone email ckDate', $sta['input'], NULL);
  extract($info);
/*  if (mb_strpos($who, '@') and !u\validEmail($email = $who)) return say('bad email', compact('email'), 'who');
  if (!$a = w\whois($who, 'who', $info, 'self-tx', $paying)) return FALSE;
  $uid = $a->id; */
  if (!$who) {
    if (!$paying and !$mya->admNonmemberTx) return say(t('Only authorized %PROJECT admins can charge non-members.'), 'err');
    if ($err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');
    if ($zip and $err = u\badZip($zip)) return say($err, 'zip');
    if ($phone and $err = u\badPhone($phone)) return say($err, 'phone');
    if ($email and !u\validEmail($email)) return say('bad email', 'email');
    if ($ckDate and $err = u\badDate($ckDate)) return say($err, 'ckDate');
  }
  
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
  foreach (ray('start end') as $k) if ($$k and $err = u\badDate($$k, $mya->admEditTx ? today() - 3 * MONTH_SECS : today())) return say($err, $k);
  u\preray(compact(ray('fullName zip amount start end phone ckDate')), $sta['input']);
}

function formTx_submit($form, &$sta) {
  global $mya;

  extract($input = just('amount purpose period periods start end goods paying hasCats who fullName address city state zip phone email cat isGift method ckNumber ckDate', $sta['input'], NULL));
//  if (mb_strpos($who, '@')) return w\go('/charge/nonmember/' . u\urlify(serialize($input)));
  if (r\hasCats($mya->uid) and $cat) $input['cat'] = $cat = ($paying and $who) ? [$cat, NULL] : [NULL, $cat]; // if $paying and !$who, cat is cat2 with a negative amount

  if (!$who) return fboTx($input); // non-member transaction by check or direct ACH
/*
  if (!$who) {
    if ($mya->sponsored) return fboTx($input); // non-member transaction by check or direct ACH
    $payerName = $mya->fullName;
    de bug(compact(ray('fullName payerName email amount purpose')));
  }
*/
  $uid = whoId($sta);
  $a = r\acct($uid);
  u\setDft($goods, FOR_GOODS);
  $delay = ($start > today());
  $repeats = ($periods > 0);

  $DBTX = \db_transaction();
  
  if ($delay or $repeats) { // delaying or repeating, so create a rule template
    if ($end) $end = tomorrow($end) - 1; else $end = NULL; // end at end of day, not start of day
    list ($action, $thing, $way) = $paying ? [ACT_PAY, t('payment'), 'from to'] : [ACT_CHARGE, t('invoice'), 'to from'];
    $info = ray($way, $mya->id, $uid);
 
    if ($repeats) {
      // warn user if they already have a repeating transaction like this with this other account
      $where = "tm.from=:from AND tm.to=:to AND action=:ACT_PAY AND period<>:PER_1 AND IFNULL(end, :NOW + 1) > :NOW";
      if (!$delay and db\exists('tx_timed tm', $where, $info)) say(t('Note: You also have another active recurring transaction with %otherName. Go to <%a>History / Recurring Payments</a> to manage your recurring transactions.', 'otherName _a', $a->fullName, atag('/history/recurring')));
      $repeatSubs = ray('often', r\recurDesc($period, $periods));
      $repeatMsg = 'repeats';
    } else list ($periods, $period) = [1, PER_1];
    
    if (!$start) $start = today(); // handle ''
    $info += compact(ray('action amount purpose period periods start end'));
    if ($isGift) $info['flags'] = u\bit(B_GIFT);
    $recursId = db\insert('tx_timed', $info, 'id');
  }
  
  if ($delay) { // delaying, so do nothing now but create a template for the thing (repeating or not)
    $msg = 'thing scheduled';
    $subs = ray('thing who start', $thing, $a->fullName, fmtDt($start));
    $a->tell($msg, $subs, '', $paying ? SEND_OUT : SEND_IN);
    $ok = TRUE;
  } else { // no delay, transact now
    $data = [];
    if ($start and $start < today()) $data['created'] = $start; // admin chose a date in the past
    if ($cat) $data += compact('cat');
    if ($isGift) $data['gift'] = TRUE;

    $z = $paying
    ? $mya->payApproved($uid, $amount, $purpose, $goods, $data, $repeats ? $recursId : NULL)
    : be\invoice($mya, $a, $amount, $purpose, $goods, $data, $repeats ? $recursId : NULL);
    extract(just('ok msg subs', $z));
  }

  unset($DBTX);

  if ($repeats) {$msg .= "|$repeatMsg"; $subs += $repeatSubs;}
  say(tr($msg, $subs), $ok ? '' : 'err');
  return go('dashboard');
}

/**
 * Handle transfer for fiscally sponsored account
 * @param assoc $input: all relevant parameters
 */
function fboTx($input) {
  global $mya;

  extract(just('amount purpose paying hasCats fullName address city state zip phone email cat isGift method ckNumber ckDate', $input, NULL));
  $state = (int) $state;
  u\EXPECT($paying or $mya->admNonmemberTx, 'hasCats income by non-admin');
  $uid = UID_OUTER;
  
  $DBTX = \db_transaction();
  if (!$pid = db\get('pid', 'people', $email ? compact('email') : compact(ray('fullName address zip')))) {
    $pid = db\insert('people', compact(ray('fullName address city state zip phone email')), 'pid');
  }
  if ($method == B_CHECK) $purpose .= t(' (check #%num, %dt)', 'num dt', $ckNumber, fmtDt($ckDate));

  $flags = $method ? u\bit($method) : 0;
  if ($isGift) $flags |= u\bit(B_GIFT);
  $info = ray('purpose pid completed deposit cat flags', $purpose, $pid, now(), $mya->admNonmemberTx ? now() : 0, $cat, $flags);
  $tx = be\createUsdTransfer($mya, $paying ? -$amount : $amount, $info);
  u\EXPECT($tx, 'outer transfer failed');
  say('info saved');
  
  if ($paying) {
    $res = go('dashboard');
  } else {
    $type = $isGift ? ($mya->sponsored ? 'fbo' : 'cggift') : 'purchase';
    $res = be\thanks($tx->xid, $type, FALSE, $pid, $purpose, $amount, PER_1, 0, $mya->id, TRUE);
  }
  
  unset($DBTX);
  
  return $res; // no need to continue (outer transactions never delay or repeat)
}
