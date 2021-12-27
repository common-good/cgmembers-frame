<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/signin.inc';
include_once __DIR__ . '/donate.inc'; // for getCGPayArgs() function

/**
 * Pay with Common Good, arriving from some other site. The payment form, if any, 
 * is displayed when a member clicks a "CGPay" button on the company's website, 
 * linked to this page with a variety of mostly-optional parameters (see http://cg4.us/paycg for details)
 */
 
function formCGPay($form, &$sta, $args = '') {
  global $mya;

  if (!$args) $args = u\_GET(); // can't use u\setDft here
  $args = $args ? urlq2ray($args) : $_POST;
  if (!$res = getCGPayArgs($args)) return; // if there were errors, don't continue
  extract($res); // coA, CGPAY_VARS, and CGPAY_VARS2 (prefixed by s_)
  if (!nn($coA)) return; // softErr
  
  $verbs = t('Donate, Donate, Pay');
  $verb = in(ucwords($verb), $verbs) ? ucwords($verb) : t('Pay');
  if ($verb == t('Donate')) u\setDft($s_item, t('donation'));

  $suffix = t(' to ') . $coA->fullName;
  if ($amount) {
    $amtDpy = item(u\fmtAmt($amount, ''), "$verb:", '', suffix($suffix));
    $amount = hidFld($amount);
  } else {
    $amtDpy = NULL;
    $amount = numFld("$verb:", '', dft($s_amount ? u\ddddcc($s_amount) : '') + suffix($suffix) + clas('middle'));
  }

  if ($mya and $mya->proSe) {
    $qid = $pass = NULL;
  } else {
    list ($qid, $pass) = w\signinFlds($s_id);
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

  if (nn($s_name)) {
    $subtext = t('Hello %s_name!', compact('s_name'));
  } else {
    $subtext = t('Pay with %PROJECT');
    if (!$mya) $option = t('OR <big><%a>Sign up!</a></big>', '_a', atag('/signup')) . nn($option2);
  }
  
  $subtext = "<h4>$subtext</h4>";
  $submit = submit($verb, '', 'md', w\suffix(nn($option)));

  $info = compact(ray('subtext amount amtDpy itemDpy item for qid pass submit'));
  foreach (ray(CGPAY_VARS) as $k) if (isset($$k) and !is_array($$k)) $info[$k] = hidFld($$k);

  return cgform($info);
}

function formCGPay_validate($form, &$sta) {
  global $mya;

  extract(just('qid amount for item', $input = $sta['input'], NULL));

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
