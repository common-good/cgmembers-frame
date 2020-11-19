<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/signin.inc';

const AUTHCODE_TIMEOUT = 60 * 60; // number of seconds before authcode expires
const CGPAY_FLDS = 'op company cocode code item amount suggestedItem suggestedAmount name account zip verb request api for return cancel err confirm';
const CGPAY_MAXLEN = 255; // maximum field length (to prevent bloating our tables) -- return and cancel have no limit

// There are hooks here for a call from rDo(), with doCode being the encrypted code from rDo

/**
 * Pay with Common Good, arriving from some other site. The payment form, if any, 
 * is displayed when a member clicks a "Pay [or Donate] With Common Good" button on the company's website, 
 * linked to this page with a variety of mostly-optional parameters (see http://cg4.us/paycg for details), including:
 *   op: the operation (authcode, charge, or status)
 *   company: the payee's account ID
 *   item: name of item being paid for
 *   amount: amount to be paid
 *   return: URL to return to after payment is made
 * Example: pay-with-cg?company=NEWZZC&item=ear+plugs&amount=23&return=http://whatever&cancel=http://whatever&for=gift
 * @param string $doCode: the encrypted code from rDo, if any, or 'done'
 */
function formBuy($form, &$sta, $doCode = '') {
  if ($doCode == 'done') return []; // shows result message and nothing else
  
  $args = strpos($doCode, '=') ? $doCode 
  : (($doCode . ' ')[0] == 'P' ? u\decryRay($doCode)
  : (nni($sta, 'input') ?: ($_POST ?: $_GET))
  );

  $args = just(CGPAY_FLDS, $args, NULL);
  foreach (justNOT('return cancel', $args) as $k => $v) if (strlen($v) > CGPAY_MAXLEN) return apiErr(tr('field too long', 'field max', $k, CGPAY_MAXLEN));

  $op = nni($args, 'op', 'charge'); // default to "charge"
  if ($op == 'authcode') return w\authcode($args); // eventually move authcode function to this file (check logs)
  if ($op == 'charge') return charge($args);
  if ($op == 'status') return status($args);
  return w\apiErr(t('"%op" is an unknown operation.', compact('op')));
}

/**
 * Display a form for the customer to agree to pay the company.
 */
function charge($args) {
  global $mya;

  if ($err = missingArgs($args, 'company')) return $err;
  extract(just(CGPAY_FLDS, $args, NULL)); // use almost all parameters
  $co = getCo($company);
  
  if ($code) { // make it easy to get paid the easy way (can't include a code in emails or on a web page)
    if (mb_strtolower($code) != t('demo')) { // not looking at the demo
      if ($code != $co->emailCode) {
        if (strpos($code, ' ') or !$coRay = u\decryRay($code) or nni($coRay, 'cocode') != $co->emailCode) return apiErr(t('bad code in request')); // space in code is probably "bad company ID or cocode" message
        $t = now() - nni($coRay, 'now', 0);
        if ($t < 0 or $t > AUTHCODE_TIMEOUT) return w\softErr('co code timeout'); // error for customer
      }
    } else { $tst = t('<span class="loud">DEMO</span>'); }
  }
  
  if ($amount) {
    if ($err = u\badAmount($amount, '>0')) return apiErr($err);
    $amountDpy = u\fmtAmt($amount);
  } else $amountDpy = '$' . w\rendA('amount', numFld(REQ, '', dft($suggestedAmount)));

  if ($account and $id = fmtAcctId($account) and $a = r\acct($id)) { // company passed customer's email or account ID
    if (($name and !u\similar($a->fullName, $name)) or ($zip and !u\starts($zip, $a->zip))) $a = NULL;
  } elseif ($a = $mya and $mya->proSe) {
    $confirm = FALSE; // currently signed in as self, so no need to confirm
  } else { $a = NULL; }
  
  $confirm = !isset($a);
  
  $subtext = $a 
  ? t('<h4>Hello %name!</h4>', 'name acct', $a->bestName, $a->mainQid) . nn($tst)
  : t('<h4>Hello %PROJECT Member</h4>');

  if ($confirm) {
    $name = textFld(REQ . t('Account ID:'), [t('Your account ID or Email'), ''], nn($id));
    $pass = NULL;
  } elseif ($mya and $mya->proSe) {
    $name = $pass = NULL;
  } else {
    list ($name, $pass) = w\signinFlds(nn($a) ? $a->mainQid : ''); // keep the nn($a)? here in case we change the logic hereabout
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
  } else { return apiErr(t('The "for" parameter must be credit, gift, donate, or other.')); }

  $verbs = t('Donate, Donate, Pay');
  $verb = in(ucwords($verb), $verbs) ? ucwords($verb) : t('Pay');
  $payment = item(t('%amount to %who', '_amount who', $amountDpy, $co->fullName ?: t('co')), "$verb:", '', clas('middle'));
  
  $option = $a ? '' : t('OR <%a>signup</a>', '_a', atag('/signup'));
  $submit = submit($verb, '', 'md', w\suffix($option));

  $info = compact(ray('subtext payment itemDpy for name pass item submit'));
  foreach (ray(CGPAY_FLDS) as $k) if (isset($$k) and !is_array($$k)) $info[$k] = hidFld($$k);
  
  return cgform($info);
}

/**
 * Return the status of the indicated invoice.
 */
function status($args) {
  if ($err = missingArgs($args, $fields = 'company cocode request')) return $err;
  extract(just($fields, $args, NULL));
  $co = getCo($company);
  if ($co->emailCode != $cocode) return apiErr(t('bad cocode'));
  $s = mb_substr(serialize(compact('request')), 5); // for example, a:1:{s:7:"request";s:5:"abcde";} (start with s:7)
  $s = '%' . substr($s, 0, strlen($s) - 1) . '%'; // chop off last char
  $status = db\get('status', 'r_invoices', 'payee=:coid AND data LIKE :request', ray('coid request', $co->id, $s));
  exitJust($status === FALSE ? TX_NOSUCH : $status);
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
    extract(just('ok msg', be\invoice($co, $a, $amount, $item, FOR_GOODS, compact(ray('api request for')))));
    if ($ok) $msg = t('pay button success');
  } else {
    extract(just('ok msg subs', be\transfer('payment', $a, $co, $amount, $item, FOR_GOODS, compact('for'))));
    if ($ok) $msg = t('<h3>Success!</h3> ') . $msg;
    if ($api) u\post($api, ray('request ok msg', $request, (int) $ok, $msg));
    foreach (ray('return cancel') as $k) if (nn($$k)) $$k = tr($$k, $args + compact(ray('msg ok subs')));
  }
  
  if ($ok) {
    if (nn($return)) return w\goFar($return, "request=$request");
    return buySay($msg);
  } else {
    return nn($cancel) ? w\goFar($cancel, "request=$request") : softErr($msg);
  }
}

/**
 * Show an error or success message on the Pay-with-CG page.
 * @param string $msg: message to show
 * @param mixed $info: '' or 'err' (in which case show just the message) or assoc of parameters (allows retry)
 */
function buySay($msg, $info = '') {
  list ($code, $info) = is_array($info) ? [u\cryRay($info), 'err'] : ['done', $info];
	say($msg, $info);
  return w\go("pay-with-cg/$code");
}

function fmtAcctId($s) {return trim(strhas($s, '@') ? mb_strtolower($s) : mb_strtoupper($s));}

/**
 * Check for missing required parameters.
 */
function missingArgs($args, $require) {
  foreach (ray($require) as $k) if (empty($args[$k])) return apiErr(t('missing field', 'field', $k)); // handle interface errors gracefully
  return FALSE;
}

/**/ function apiErr($s) {die('CG API: ' . $s);} // give simple error messages for api interface errors

/**
 * Check the company parameter for errors and return its account.
 */
function getCo($company) {  
  if (!u\isQid($company) or !$co = r\acct($company) or !$co->co) return apiErr('bad company account id');
  return $co;
}
