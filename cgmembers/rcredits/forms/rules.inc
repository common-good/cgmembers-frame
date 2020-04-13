<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Handle rules.
 */
 
const RULE_FLDS = 'id payerType payer payeeType payee from to action amount portion purpose minimum useMax extraMax template start end code';
const NULL_RULE_FLDS = 'payer payee from to purpose useMax extraMax template end code';
const ZERO_RULE_FLDS = 'amount portion minimum start';
 
function formRules($form, &$sta, $args = '') {
  global $mya;
  
  foreach (ray(RULE_FLDS) as $k) $$k = NULL;
  extract(just('id', $args, NULL));
  if ($id and $res = db\get('*', 'tx_rules', compact('id'))) extract($res);
  foreach (ray('payerType payeeType') as $k) if (isset($$k)) $$k = array_search($$k, ray(REF_LIST));
  foreach (ray('payer payee from to') as $k) if (isset($$k) and !is_numeric($$k)) $$k = db\get('fullName', 'users', ['uid' => $$k]);
  foreach (ray('start end') as $k) if ($$k) $$k = u\fmtDate($$k, TRUE);
  
  $payeDesc = t('Type the full name on the account or a group/industry ID. Leave empty for ANY.');
  $fromToDesc = t('Type a name or %SAME_AS_PAYER = same as payer, %SAME_AS_PAYEE = same as payee');
  $dtDesc = [t('mm/dd/yyyy')];
  $maxDesc = t('Leave empty for no max.');
  
  jsx('rules');
  
  $title = t('Rules');
  
  $btn = $id ? [] : suffix(btn('', t('Edit'), 'default', 'xs', 'edit'));
  $id = textFld(t('Rule ID:'), [t('Rule ID'), t('Leave this blank to add a new record')], dft($id) + $btn);
  
  $payerType = radiosFld(REQ . t('Payer Type:'), '', dft($payerType), ray(REF_LIST));
  $payer = textFld(t('Payer:'), $payeDesc, dft($payer));
  $payeeType = radiosFld(REQ . t('Payee Type:'), '', dft($payeeType), ray(REF_LIST));
  $payee = textFld(t('Payee:'), $payeDesc, dft($payee));
  $from = textFld(REQ . t('From:'), $fromToDesc, dft($from));
  $to = textFld(REQ . t('To:'), $fromToDesc, dft($to));
  $action = radiosFld(REQ . t('Action:'), '', dft($action), ray(ACT_LIST));
  $amount = textFld(t('Amount:'), '', dft($amount));
  $portion = textFld(t('Portion:'), '', dft($portion));
  $purpose = textFld(REQ . t('Purpose:'), '', dft($purpose));
  $minimum = textFld(t('Minimum:'), '', dft($minimum));
  $useMax = textFld(t('Use Max:'), $maxDesc, dft($useMax));
  $extraMax = textFld(t('Extra Max:'), $maxDesc, dft($extraMax));
  $template = textFld(t('Template:'), '', dft($template));
  $start = textFld(t('Start:'), $dtDesc, dft($start));
  $end = textFld(t('End:'), $dtDesc, dft($end));
  $code = textFld(t('Code:'), '', dft($code));
  
  $submit = t('Submit');

  return cgForm(compact(ray('title ' . RULE_FLDS . ' submit')));
}

function formRules_validate($form, &$sta) {
  extract($input = just(RULE_FLDS, $sta['input'], NULL));

  foreach (ray(ZERO_RULE_FLDS) as $k) if (!$$k) $$k = 0;
  
  foreach (ray('from to') as $k) if (!in($$k, [-1, -2])) {
    if (!$$k = db\get('uid', 'users', ['fullName' => $$k])) return say(t('User not found: ') . $$k, $k);
  }
  foreach (ray('payer payee') as $k) if ($$k and !is_numeric($$k)) {
    if (!$$k = db\get('uid', 'users', ['fullName' => $$k])) return say(t('User not found: ') . $$k, $k);
  }
  
  foreach (ray('payerType payeeType') as $k) $$k = ray(REF_LIST)[$$k];
  $action = ray(ACT_LIST)[$action];
  
  foreach (ray('start end') as $k) if ($$k and $err = u\badDate($$k, time())) return say($err, $k);
  if (!$start) $start = time();

  foreach (ray('amount portion minimum useMax extraMax') as $k) if ($$k and $err = u\badAmount($$k, '>=0', $k == 'useMax' ? 0 : 2)) return say("$err ($k)", $k);

  if ($id and !db\exists('tx_rules', compact('id'))) return say(t('To insert a record, leave the id empty.'), 'id');
  
  u\preRay(compact(ray(RULE_FLDS)), $sta['input']);
}

function formRules_submit($form, &$sta) {
  extract($input = just(RULE_FLDS, $sta['input'], NULL));
  foreach (ray(NULL_RULE_FLDS) as $k) if (empty($$k)) $input[$k] = NULL;
  
  if ($id) {
    db\update('tx_rules', $input, 'id');
  } else {
    unset($input['id']);
    db\insert('tx_rules', $input, 'id');
  }

  return go('/sadmin/rules', 'info saved');
}
