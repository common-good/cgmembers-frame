<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Do whatever.
 */
function formAgent($form, &$sta, $args = '') {
  global $mya;

  $title = t('Connect a Manager Account');
  $subtext = t('<p>Until now your company account has been a "trial" account and you have been able to sign in to it directly, anonymously. Normally, you need to sign in to a personal account, then select the company account to manage it. A personal account can manage several company accounts and a company account can be managed by several personal accounts. For now, you just need one of each.</p><p>If you haven\'t opened a personal account yet, sign out and click "Sign up" to open one. Otherwise type your personal account ID below (you can find it on your personal %PROJECT card, under your name).</p><p>Once you complete this step, you will no longer be able to sign in to this account directly.</p>');
  
  $agent = textFld(REQ . t('Personal Account:'), [t('Account ID')]);
  $phone = textFld(REQ . t('Phone:'), [t('Phone number'), t('Type the phone number on your personal account, for verification.')]);

  return cgform(compact(ray('title subtext agent phone')) + setupFoot());
}

function formAgent_validate($form, &$sta) {
  global $mya;
  extract(just('agent phone', $sta['input']));

  if (!$a = r\acct($agent) or $a->co or !$a->member) return say(t('That is not an account ID of a completed personal account.'), 'agent');
  if ($err = u\badPhone($phone)) return say($err, 'phone');
  if (u\fmtPhone($phone, '-') != $a->phone) return say(t('The phone number on that account does not match.'), 'phone');
}

function formAgent_submit($form, &$sta) {
  global $mya;
  extract(just('agent', $sta['input']));

  $a = r\acct($agent);
  $mya->newRelation(ray('other permission', $a->id, r\perm(B_MANAGE)));
  $a->setBit('depends', FALSE);

  return w\goNextStep('agent');
}