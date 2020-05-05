<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/signin.inc';

const AUTHCODE_TIMEOUT = 10 * 60; // number of seconds before authcode expires
const CGPAY_FLDS = 'company type code item amount suggestedItem suggestedAmount name account zip verb request api for return cancel err confirm';

// There are hooks here for a call from rDo(), with doCode being the encrypted code from rDo

/**
 * Pay with Common Good, arriving from some other site. The payment form, if any, 
 * is displayed when a member clicks a "Pay [or Donate] With Common Good" button on the company's website, 
 * linked to this page with one or more of the following parameters (company, code, and either amount or type is required):
 *   item: name of item being paid for
 *   amount: amount to be paid
 *   return: URL to return to after payment is made
 *   type: "Donate"
 * Example: pay-with-cg?company=NEWZZC&item=ear+plugs&amount=23&return=http://whatever&cancel=http://whatever&type=Donate
 * @param string $doCode: the encrypted code from rDo, if any, or 'done'
 */
function formBuy($form, &$sta, $doCode = '') {
  global $mya;
  if ($doCode == 'done') return []; // shows result message and nothing else
  
  $required = 'company';

  $args = strpos($doCode, '=') ? $doCode 
  : (($doCode . ' ')[0] == 'P' ? r\doCode($doCode)
  : (@$sta['input'] ?: ($_POST ?: $_GET))
  );

  extract($args = just(CGPAY_FLDS, $args, NULL));

  foreach (ray($required) as $k) if (!@$$k) return w\softErr(t('missing field', 'field', $k)); // handle hackers gracefully
  if (!u\isQid(@$company) or !$co = r\acct($company) or !$co->co) return w\softErr('bad account id');
  
  if (!@$code) { // no company code -- this type of link requires confirmation by email or SMS
    $confirm = TRUE;
  } elseif (mb_strtolower($code) != t('demo')) { // not looking at the demo
    if (strpos($code, ' ') or !$cocodes = substr(u\decry('P', CRYPT_FLAG . $code), 1) or !$cocodes = unserialize($cocodes) or @$cocodes['cocode'] != $co->emailCode) return w\softErr('bad co code'); // space in code is probably "bad company ID or cocode" message
    $t = now() - @$cocodes['now'];
    if ($t < 0 or $t > AUTHCODE_TIMEOUT) return w\softErr('co code timeout');
  } else {$tst = t('<span class="loud">DEMO</span>');}
  
  if ($amount0 = @$amount) {
    if ($err = u\badAmount($amount, '>0')) return w\softErr($err);
    $amountDpy = u\fmtAmt($amount);
  } else $amountDpy = '$' . w\rendA('amount', numFld(REQ, '', dft($suggestedAmount)));

//  if ($code != @$co->data['cardCode']) return w\go('', 'bad account ID', 'zot');
  if ($a = $mya) {
    $confirm = FALSE;
  } else {
    if ($account and $id = fmtAcctId($account) and $a = r\acct($id)) {
      if (($name and !u\similar($a->fullName, $name)) or ($zip and !u\starts($zip, $a->zip))) $a = NULL;
    }
    $confirm = !isset($a);
  }
  
  $subtext = $a 
  ? t('<h4>Hello %name!</h4>', 'name acct', $a->bestName, $a->mainQid) . @$tst
  : t('<h4>Hello %PROJECT Member</h4>');

  if ($confirm) {
    $name = textFld(t('Account ID:'), [t('Your account ID or Email'), ''], required(nn($id)));
    $pass = NULL;
  } elseif ($mya) {
    $name = $pass = NULL;
  } else {
    list ($name, $pass) = w\signinFlds(@$a ? $a->mainQid : ''); // keep the $a? here in case we change the logic hereabout
  }

  if ($for == 'gift') {
    $for = textFld(t('As a Gift to:'), [t('Email or account ID')]);
    $item = t('gift of store credit');
    $itemDpy = item(t('store credit'), t('For:'));
  } elseif ($for == 'credit') {
    $for = hidFld(SELF_CREDIT);
    $item = t('store credit');
    $itemDpy = item($item, t('For:'));
  } elseif ($for == 'donate') {
    $for = NULL;
    u\setDft($verb, t('Donate'));
    u\setDft($item, t('donation'));
    $itemDpy = NULL;
  } elseif (!$for or $for == 'other') { // for is other or empty or nonsense
    if ($item) {
      $itemDpy = item($item, t('For:'));
    } else {
      $item = textFld(REQ . t('For:'), [t('What to pay for')], dft($suggestedItem));
      $itemDpy = NULL;
    }
  } else { return w\softErr(t('The "for" parameter must be credit, gift, donate, or other.')); }

  $verbs = t('Donate, Donate, Pay');
  $verb = in(ucwords(@$verb), $verbs) ? ucwords($verb) : t('Pay');
  $payment = item(t('%amount to %who', '_amount who', $amountDpy, @$co->fullName ?: t('co')), "$verb:");
  
  $option = $a ? '' : t('OR <%a>signup</a>', '_a', atag('/signup'));
  $submit = submit($verb, '', 'md', w\suffix($option));
//  focusOn(($amount0 and !$mya) ? 'name' : 'amount');

  $form = compact(ray('subtext payment itemDpy for name pass item submit'));
  foreach (ray(CGPAY_FLDS) as $k) if (isset($$k) and !is_array($$k)) $form[$k] = hidFld($$k);
  
  return cgform($form);
}

function formBuy_validate($form, &$sta) {
  global $mya;
  extract(just('name amount for item confirm', $input = $sta['input'], NULL));

  if (isset($amount) and $err = u\badAmount($sta['input']['amount'], '>0')) return say($err, 'amount'); // fix amount
  
  if (!$mya) {
    if ($confirm) {
      if (!r\acct(fmtAcctId($name))) return say(t('That account does not exist so it cannot be charged.'), 'name');
    } else {
      if (!formSignin_validate($form, $sta)) return buySay('', $input + ['account' => $name]);
    }
  }
  if ($for and $for != SELF_CREDIT) {
    $for = fmtAcctId($for);
    if (!$a = r\acct($for)) return say(t('That %PROJECT account does not exist. You can currently give only to another %PROJECT member.'), 'for');
    $item .= t(' (to %who)', 'who', $a->fullName);
    u\preRay(compact(ray('for item')), $sta['input']);
  }
}

function formBuy_submit($form, &$sta) {
  global $mya;
  extract(just(CGPAY_FLDS, $sta['input'], NULL)); // overwrite item from args if still set
  
  $co = r\acct($company);
  $a = $mya ?: ($confirm ? r\acct(fmtAcctId($name)) : r\acct($sta['uid']));
  
  if ($confirm) {
    list ($ok, $index, $subs) = be\invoice($co, $a, $amount, $item, FOR_GOODS, compact(ray('api request for')));
    $msg = tr($index, $subs);
    if ($ok) $msg = t('pay button success');
  } else {
    list ($ok, $index, $subs) = be\transfer('payment', $a, $co, $amount, $item, FOR_GOODS, compact('for'));
    $msg = tr($index, $subs);  
    if ($ok) $msg = t('<h3>Success!</h3> ') . $msg;
    if ($api) u\post($api, ray('request ok msg', $request, (int) $ok, $msg));
    foreach (ray('return cancel') as $k) if (@$$k) $$k = tr($$k, $args + compact(ray('msg ok')));
  }
  
  if ($ok) {
    if (@$return) return w\goFar($return, "request=$request");
    return buySay($msg);
  } else {
    return @$cancel ? w\goFar($cancel, "request=$request") : softErr($msg);
  }
}

/**
 * Show an error or success message on the Pay-with-CG page.
 * @param string $msg: message to show
 * @param mixed $info: '' or 'err' (in which case show just the message) or assoc of parameters (allows retry)
 */
function buySay($msg, $info = '') {
  list ($code, $info) = is_array($info) ? [r\makeDoCode($info), 'err'] : ['done', $info];
	say($msg, $info);
  return w\go("pay-with-cg/$code");
}

function fmtAcctId($s) {return trim(strhas($s, '@') ? mb_strtolower($s) : mb_strtoupper($s));}
