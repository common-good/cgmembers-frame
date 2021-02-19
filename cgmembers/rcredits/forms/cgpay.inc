<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/signin.inc';

/**
 * Pay with Common Good, arriving from some other site. The payment form, if any, 
 * is displayed when a member clicks a "CGPay" button on the company's website, 
 * linked to this page with a variety of mostly-optional parameters (see http://cg4.us/paycg for details)
 */
 
function formCGPay($form, &$sta) {
  global $mya;

  $args = u\_GET() ?: $_POST;

  foreach ($args as $k => $v) {
    if (strlen($v) > CGPAY_MAXLEN) return exitErr(tr('field too long', 'field max', $k, CGPAY_MAXLEN));
    if (!in($k, CGPAY_VARS2)) return exitErr(t('"%k" is not a parameter. Check your spelling?', compact('k')));
  }

  if ($err = missingArgs($args, 'code')) return exitErr($err);
  if (!$ray = u\decryRay($args['code'])) return exitErr(t('The "code" parameter is not valid.'));
  if ($dups = just($args, $ray)) return exitErr(t('Parameter "%k" attempts to override that parameter as encoded in "code".', 'k', $dups[0]));

  extract(just(CGPAY_VARS, $ray, NULL)); // use this structure just to set the unused ones to NULL
  extract(just(CGPAY_VARS2, $args, NULL), EXTR_PREFIX_ALL, 's'); // mostly suggested parameters (amount, id, item, name) except for request -- don't overwrite
  
  $co = r\acct($account);
  u\EXPECT($co, 'missing account in code!');
  if (isset($s_amount) and $err = u\badAmount($s_amount, '>0')) return exitErr(t('Parameter "amount" (suggested amount): %err', compact('err')));

  $verbs = t('Donate, Donate, Pay');
  $verb = in(ucwords($verb), $verbs) ? ucwords($verb) : t('Pay');
  if ($verb == t('Donate')) u\setDft($s_item, t('donation'));

  $suffix = t(' to ') . ($co->fullName ?: t('co'));
  if ($amount) {
    $amtDpy = item(u\fmtAmt($amount, ''), "$verb:", '', suffix($suffix));
    $amount = hidFld($amount);
  } else {
    $amtDpy = NULL;
    $amount = numFld("$verb:", '', dft($s_amount ? u\ddddcc($s_amount) : '') + suffix($suffix) + clas('middle'));
  }

  u\setDft($s_name, t('%PROJECT Member'));
  $subtext = t('<h4>Hello %s_name!</h4>', compact('s_name'));

  if ($mya and $mya->proSe) {
    $name = $pass = NULL;
  } else {
    list ($name, $pass) = w\signinFlds($s_id); // keep the nn($a)? here in case we change the logic hereabout
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
      $item = textFld(REQ . t('For:'), [t('What to pay for')], dft($s_item));
      $itemDpy = NULL;
    }
  } else u\FAIL(t('The "for" parameter must be credit, gift, donate, or other -- not "%for"', compact('for')));

  $option = $s_name ? '' : t('OR <%a>Sign up!</a>', '_a', atag('/signup'));
  $submit = submit($verb, '', 'md', w\suffix($option));

  $info = compact(ray('subtext amount amtDpy itemDpy for name pass item submit'));
  foreach (ray(CGPAY_VARS) as $k) if (isset($$k) and !is_array($$k)) $info[$k] = hidFld($$k);

  return cgform($info);
}

function formCGPay_validate($form, &$sta) {
  global $mya;

  extract(just('name amount for item', $input = $sta['input'], NULL));

  if (isset($amount) and $err = u\badAmount($sta['input']['amount'], '>0')) return say($err, 'amount'); // fix amount
  
  if (!$mya and !formSignin_validate($form, $sta)) return;

  if ($for and !in($for, [SELF_CREDIT, SELF_CREDIT50])) {
    $for = trim(strhas($for, '@') ? mb_strtolower($for) : mb_strtoupper($for));
    if (!$a = r\acct($for)) return say(t('That %PROJECT account does not exist. You can currently give only to another %PROJECT member.'), 'for');
    $item .= t(' (to %who)', 'who', $a->fullName);
    u\preRay(compact(ray('for item')), $sta['input']);
  }
}

function formCGPay_submit($form, &$sta) {
  global $mya;

  extract(just(CGPAY_VARS . ' uid request', $input = $sta['input'], NULL)); // overwrite item from args if still set
  
  $co = r\acct($account);
  if (test() and !$co) return; // keep tests from crashing after error creating code
  
  $a = $mya ?: r\acct($uid); // uid is from signin_validate()
  $button = 1;
  
  $data = compact(ray('for button'));
  if ($credit) $data += compact('credit');
  extract(just('ok msg', be\transfer('payment', $a, $co, $amount, $item, FOR_GOODS, $data)));
//  if ($api) u\post($api, ray('request ok msg', $request, (int) $ok, $msg));
  if ($return) {
    $res = compact(ray('ok msg request'));
    $return = tr($$k, $res);
    return u\postnGo($return, $res);
  } else { // show standard CG success / failure page
    return $ok ? sayEmpty(tr('success title') . " $msg") : softErr($msg);
  }
}
