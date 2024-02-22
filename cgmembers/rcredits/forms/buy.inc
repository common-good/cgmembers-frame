<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/signin.inc';

const AUTHCODE_TIMEOUT = 60 * 60; // number of seconds before authcode expires
const CGPAY_FLDS = 'op company cocode code item amount credit suggestedItem suggestedAmount name account zip verb request api for for0 return cancel err confirm';
const CGPAY_MAXLENGTH = 255; // maximum field length (to prevent bloating our tables) -- return and cancel have no limit

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

  foreach (justNOT('return cancel', $args) as $k => $v) if (strlen($v ?? '') > CGPAY_MAXLENGTH) return apiErr(tr('field too long', 'field max', $k, CGPAY_MAXLENGTH));

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

  if ($err = missingArgs($args, 'company')) apiErr($err);
  extract(just(CGPAY_FLDS, $args, NULL)); // use almost all parameters
  $co = getCo($company);
  
  if ($code) { // make it easy to get paid the easy way (can't include a code in emails or on a web page)
    if (mb_strtolower($code) != t('demo')) { // not looking at the demo
      if ($code != $co->emailCode) {
        if (strpos($code, ' ') or !$coRay = u\decryRay($code) or nni($coRay, 'cocode') != $co->emailCode) return apiErr(t('bad code in request')); // space in code is probably "bad company ID or cocode" message
        $t = now() - nni($coRay, 'now', 0);
        if ($t < 0 or $t > AUTHCODE_TIMEOUT) {
          if (strihas(nni($_SERVER, 'HTTP_USER_AGENT'), 'bot')) exit('link is out of date');
          unset($coRay['r']); // work around guzzleHttp bug (try including chr(181) in an email)
          r\tellAdmin('co code timeout', compact(ray('code t')) + $coRay + $args + $_SERVER);
          return w\softErr('co code timeout'); // error for customer
        }
      }
    } else { $tst = t('<span class="loud">DEMO</span>'); }
  }
  
  $verbs = t('Donate, Donate, Pay');
  $verb = in($verb0 = ucwords($verb ?? ''), $verbs) ? $verb0 : t('Pay');
  if ($verb == t('Donate')) u\setDft($suggestedItem, t('donation'));

  if ($credit) {
    if ($err = u\badAmount($credit, '>0')) return apiErr($err); // fix amount
    if (!in($for, 'credit credit50')) return apiErr(t('The <b>credit</b> parameter applies only when for is "credit" or "credit50".'));
  }

  $suffix = t(' to ') . ($co->fullName ?: t('co'));
  if ($amount) {
    if ($err = u\badAmount($amount, '>0')) return apiErr($err);
    $amtDpy = item(u\fmtAmt($amount, ''), "$verb:", '', suffix($suffix));
    $amount = hidFld($amount);
  } else {
    if ($suggestedAmount and $err = u\badAmount($suggestedAmount, '>0')) return apiErr($err);
    $amtDpy = NULL;
    $amount = numFld("$verb:", '', dft($suggestedAmount ? u\ddddcc($suggestedAmount) : '') + suffix($suffix) + clas('middle'));
  }

  if ($account and $id = fmtAcctId($account) and $a = r\acct($id)) { // company passed customer's email or account ID
    if (($name and !u\similar($a->fullName, $name)) or ($zip and !u\starts($zip, $a->zip))) $a = NULL;
  } elseif ($a = $mya and $mya->proSe) {
    $confirm = FALSE; // currently signed in as self, so no need to confirm
  } else { $a = NULL; }
  
  $confirm = !isset($a);

  $subtext = $a 
  ? t('<h4>Hello %name!</h4>', 'name acct', $a->bestName, $a->mainQid) . nn($tst)
  : t('<h4>Hello %PROJECT Member</h4>');

  $confirm = FALSE; // always ask for sign-in instead (standard)
  if ($confirm) {
    $name = textFld(REQ . t('Account ID:'), [t('Your account ID or Email'), ''], nn($id));
    $pass = NULL;
  } elseif ($mya and $mya->proSe) {
    $name = $pass = NULL;
  } else {
    list ($name, $pass) = w\signinFlds(nn($a) ? $a->mainQid : ''); // keep the nn($a)? here in case we change the logic hereabout
  }

  $for0 = $for; // for errors
  if ($for == 'gift') {
    $for = textFld(t('As a Gift to:'), [t('Email or account ID')]);
    $item = t('gift of store credit');
    $itemDpy = item(t('store credit'), t('For:'));
  } elseif ($for == 'credit') {
    $for = hidFld(SELF_CREDIT);
    $item = ($credit ? u\fmtAmt($credit) . ' ' : '') . t('store credit');
    $itemDpy = item($item, t('For:'));
  } elseif ($for == 'credit50') {
    $for = hidFld(SELF_CREDIT50);
    $item = t('store credit');
    $itemDpy = item($item, t('For:'));
  } elseif ($for == 'donate') {
    $for = NULL;
    u\setDft($verb, t('Donate'));
    u\setDft($item, t('donation'));
    $itemDpy = NULL;
  } elseif (!$for or $for == 'other') { // for is other or empty or nonsense
    $for = NULL;
    if ($item) {
      $itemDpy = item($item, t('For:'));
    } else {
      $item = textFld(REQ . t('For:'), [t('What to pay for')], dft($suggestedItem));
      $itemDpy = NULL;
    }
  } else { return apiErr(t('The "for" parameter must be credit, gift, donate, or other -- not ') . $for); }

//  $payment = item(t('%amount to %who', '_amount who', $amountDpy, $co->fullName ?: t('co')), "$verb:", '', $amtExtra);
  
  $option = $a ? '' : t('OR <%a>Sign up!</a>', '_a', atag('/signup'));
  $submit = submit($verb, '', 'md', w\suffix($option));

  $info = compact(ray('subtext amount amtDpy itemDpy for name pass item submit'));
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
  $status = db\get('status', 'tx_requests', 'payee=:coid AND data LIKE :request', ray('coid request', $co->id, $s));
  exitJust($status === FALSE ? TX_NOSUCH : $status);
}

/**
 * Validate charge op
 */
function formBuy_validate($form, &$sta) {
  global $mya;

  extract(just('name amount for item confirm', $input = $sta['input'], NULL));

  if (isset($amount) and $err = u\badAmount($sta['input']['amount'], '>0')) return say($err, 'amount'); // fix amount
  
  if (!$mya) {
    if ($confirm) {
      if (!r\acct(fmtAcctId($name))) return say(t('That account does not exist so it cannot be charged.'), 'name');
    } else {
      if (!formSignin_validate($form, $sta)) return buyErr('', ray('op account', 'charge', $name) + $input);
    }
  }
  if ($for and !in($for, [SELF_CREDIT, SELF_CREDIT50])) {
    $for = fmtAcctId($for);
    if (!$a = r\acct($for)) return say(t('That %PROJECT account does not exist. You can currently give only to another %PROJECT member.'), 'for');
    $item .= t(' (to %who)', 'who', $a->fullName);
    u\preRay(compact(ray('for item')), $sta['input']);
  }
}

/**
 * Handle charge op submission
 */
function formBuy_submit($form, &$sta) {
  global $mya;

  extract(just(CGPAY_FLDS . ' uid', $input = $sta['input'], NULL)); // overwrite item from args if still set
  
  $co = getCo($company);
  $a = $mya ?: ($confirm ? r\acct(fmtAcctId($name)) : r\acct($uid)); // uid is from signin_validate()
  $button = 1;
  
  if ($confirm) {
    $data = compact(ray('api request for button'));
    if ($credit) $data += compact('credit');
    extract(just('ok msg', be\invoice($co, $a, $amount, $item, FOR_GOODS, $data)));
    if ($ok) $msg = t('pay button success');
  } else {
    $data = compact(ray('for button'));
    if ($credit) $data += compact('credit');
    extract(just('ok msg subs', be\transfer('payment', $a, $co, $amount, $item, FOR_GOODS, $data)));
    if ($api) u\post($api, ray('request ok msg', $request, (int) $ok, $msg));
    foreach (ray('return cancel') as $k) if (nn($$k)) $$k = tr($$k, $input + compact(ray('msg ok subs')));
    if ($ok) $msg = tr('success title') . " $msg";
  }
  
  if ($ok) {
    if ($return == 'json') return exitJust(u\jsonize(ray('ok msg request', $ok ? 1 : 0, strip_tags($msg), $request ?: '')));
    if (nn($return)) return w\goFar($return, "request=$request");
    return sayEmpty($msg); // default - standard success message page
  } else {
    return nn($cancel) ? w\goFar($cancel, "request=$request") : softErr($msg);
  }
}

/**
 * Show an error message on the Pay-with-CG page.
 * @param string $msg: message to show
 * @param assoc $info: an assoc of parameters for retry, but with info[for0] holding the original value of info[for]
 */
function buyErr($msg, $info) {
  if ($for0 = nni($info ?: [], 'for0')) $info['for'] = $for0;
  $code = u\cryRay($info + ray('op', 'charge'));
  say($msg, $info, 'err');
  return w\go("pay-with-cg/$code");
}

function fmtAcctId($s) {return trim(strhas($s, '@') ? mb_strtolower($s) : mb_strtoupper($s));}

/**/ function apiErr($s) {exitJust('CG API: ' . $s);} // give simple error messages for api interface errors

/**
 * Check the company parameter for errors and return its account.
 * @param string $company: the company's qid
 */
function getCo($company) {  
  $company = strtoupper($company); // allow lowercase parameter
  if (!u\isQid($company) or !$co = r\acct($company) or !$co->co) return apiErr('bad company account id');
  return $co;
}
