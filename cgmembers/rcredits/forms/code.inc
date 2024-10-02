<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Return or exit with a CGPay button code
 * @param string $args: the parameters to encode (if none, use $_GET or $_POST)
 * @return (only when testing): the code (NULL if error)
 * 
 */
function code($args = NULL, $validate = TRUE) {
  global $codeExternal;
  
  if ($codeExternal = empty($args)) $args = $_GET ?: $_POST; // called from outside (so exit after outputting the code)
  $args = urlq2ray($args);

  if (!$validate and !test()) {
    u\EXPECT(!$codeExternal, 'not validating external params');
  } else { // validate
    foreach ($args as $k => $v) {
      if (!in($k, 'return') and strlen($v) > CGPAY_MAXLEN) return codeErr(tr('field too long', 'field max', $k, CGPAY_MAXLEN));
      if (!in($k, CGPAY_VARS)) return codeErr(t('"%k" is not a parameter. Check your spelling?', compact('k')));
    }

    if ($err = missingArgs($args, 'account secret')) return codeErr($err);
    extract($args);

    $account = $args['account'] = strtoupper($account);
    try {
      $emailCode = $codeExternal ? u\dery('P', $secret) : $secret;
    } catch (BadCry $e) {return codeErr($e->message());}
    
    if (!$secret or !$co = r\acct($account) or !$co->co or $co->emailCode != $emailCode) return codeErr(t('bad company account ID or secret'));
    unset($args['secret']);

    foreach (ray('amount credit') as $k) if (isset($$k) and $err = u\badAmount($args[$k], '>0')) return codeErr(t('Parameter "%k": %err', compact(ray('k err'))));

    foreach (ray('return') as $k) if (isset($$k) and !\valid_url($$k, TRUE)) return codeErr(t('The "%k" parameter is not a valid URL.', compact('k')));
    if (isset($for) and !in($for, CGPAY_FORS)) return codeErr(t('That is not a proper value for the "for" parameter.'));
    // item and verb can be anything
    if (isset($credit) and !in(nn($for), 'credit credit50')) {
      if ($codeExternal) return codeErr(t('The "credit" parameter applies only when the "for" parameter is "credit".'));
      unset($args['credit']);
    }
  }
  
  return $codeExternal ? exitRay($args, TRUE) : u\cryRay($args);
}

function codeErr($s) {global $codeExternal; return $codeExternal ? exitErr($s) : $s;}

