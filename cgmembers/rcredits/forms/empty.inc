<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Present an empty slate for status and error messages.
 * @param string $args: 
 *   say=1 (if called from doSay) or err=1&msg=<msg> (or empty)
 *   confirm: a confirmation question. If the user answers yes, call rdo with the code parameter
 *   code: encrypted instructions to pass to rdo
 */
function formEmpty($form, &$sta = '', $args = '') {
// NO  if ($form and !is_array($form)) say(plain($form));
  extract(just('say err msg confirm code', $args, NULL));
  
  $title = NBSP;
  $subtext = $say ? t('<p>To protect your account, <%span>no sign-in was required</span> for this action. To sign in safely, always go to %CG_DOMAIN. You may now close this browser page.</p>', '_span', 'span class="loud"') : '';
  
  if ($msg) $msg = u\dery('P', $msg);
  
  if ($confirm) {
    err('No action taken.');
    $confirm = hidfld($msg);
    jsx('confirm', compact('code'));
  } else say($msg, $err);
  
  return cgform(compact(ray('title subtext confirm')));
}
