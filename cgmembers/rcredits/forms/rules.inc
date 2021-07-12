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
 
const BOTH_FLDS = 'id payer payerType payee payeeType from to action amount portion purpose minimum useMax amtMax start end';
const TIMED_FLDS = BOTH_FLDS . ' period periods duration durations';
const RULE_FLDS = BOTH_FLDS . ' template code';
const EITHER_FLDS = BOTH_FLDS . ' period periods duration durations template code';
const NULL_FLDS = 'payer payee from to purpose useMax amtMax end template code';
const ZERO_FLDS = 'amount portion minimum start';
 
function formRules($form, &$sta, $args = '') {
  global $mya;
  
  foreach (ray(EITHER_FLDS) as $k) $$k = NULL;
  extract(just('id ttype', $args, NULL));
  if ($id and $res = db\get('*', $ttype ? 'tx_rules' : 'tx_timed', compact('id'))) extract($res);
  if (isset($action)) $action = array_search($action, ray(ACT_LIST));
  foreach (ray('payerType payeeType') as $k) if (isset($$k)) $$k = array_search($$k, ray(REF_LIST));
  foreach (ray('payer payee from to') as $k) if (isset($$k) and !is_numeric($$k)) $$k = db\get('fullName', 'users', ['uid' => $$k]);
  
  $payeDesc = t('Type the full name on the account or a group/industry ID. Leave empty for ANY.');
  $fromToDesc = t('Type a name or %MATCH_PAYER = same as payer, %MATCH_PAYEE = same as payee');
  $dtDesc = [t('mm/dd/yyyy')];
  $maxDesc = t('Leave empty for no max.');
  
  jsx('rules');
  
  $ttype = rentRadios('ttype', (int) $ttype, ray(t('Timed Rule')));

  $title = item(t('Rules'), NBSP, '', suffix($ttype));
  
  $btn = $id ? [] : suffix(btn('', t('Edit'), 'default', 'xs', 'edit'));
  $id = intFld(t('Record ID:'), [t('Record ID'), t('Leave this blank to add a new record')], dft($id) + vmin(1) + $btn);
  
  $payer = textFld(t('When Payer:'), $payeDesc, dft($payer));
  $payerType = radiosFld(REQ . t('Payer Type:'), '', dft($payerType), ray(REF_LIST));
  $payee = textFld(t('Pays Payee:'), $payeDesc, dft($payee));
  $payeeType = radiosFld(REQ . t('Payee Type:'), '', dft($payeeType), ray(REF_LIST));
  $from = textFld(REQ . t('Then From:'), $fromToDesc, dft($from));
  $to = textFld(REQ . t('To:'), $fromToDesc, dft($to));
  $action = radiosFld(REQ . t('Action:'), '', dft($action), ray(ACT_LIST));
  $amount = numFld(t('Amount:'), '', dft($amount > 0 ? $amount : '') + vmin(.01));
  $portion = numFld(t('Portion:'), '', dft($portion > 0 ? $portion : '') + vmin(.001) + step('.001'));
  $purpose = textFld(REQ . t('Purpose:'), '', dft($purpose));
  $minimum = numFld(t('Minimum:'), '', dft($minimum));
  $useMax = intFld(t('Use Max:'), $maxDesc, dft($useMax));
  $amtMax = numFld(t('Extra Max:'), $maxDesc, dft($amtMax));
  $start = dateFld(t('Start:'), $dtDesc, dft($start));
  $end = dateFld(t('End:'), $dtDesc, dft($end));
  $periods = intFld(t('Periods:'), '', dft($periods ?: 1));
  $period = selectFld(t('Period:'), '', dft($period), r\recurRay(TRUE, PERIODS));
  $durations = intFld(t('Durations:'), '', dft($durations ?: 1));
  $duration = selectFld(t('Duration:'), '', dft($duration), r\recurRay(TRUE, PERIODS));
  $template = textFld(t('Template:'), '', dft($template));
  $code = textFld(t('Code:'), '', dft($code));
  
  $submit = t('Submit');
  jsx('rules');

  return cgForm(compact(ray('title ' . EITHER_FLDS . ' submit')));
}

function formRules_validate($form, &$sta) {
  list ($tnm, $flds, $input) = getTtype($sta);
  extract($input);

  foreach (ray(ZERO_FLDS) as $k) if (isset($$k) and !$$k) $$k = 0;
  
  foreach (ray('from to') as $k) if (!in($$k, [-1, -2])) {
    if (!$$k = db\get('uid', 'users', [is_numeric($$k) ? 'uid' : 'fullName' => $$k])) return say(t('User not found: ') . $$k, $k);
  }
  foreach (ray('payer payee') as $k) if ($$k and !is_numeric($$k)) {
    if (!$$k = db\get('uid', 'users', [is_numeric($$k) ? 'uid' : 'fullName' => $$k])) return say(t('User not found: ') . $$k, $k);
  }
  
  foreach (ray('payerType payeeType') as $k) $$k = ray(REF_LIST)[$$k];
  $action = ray(ACT_LIST)[$action];
  
  foreach (ray('start end') as $k) if ($$k and $err = u\badDate($$k)) return say($err, $k);
  if (!$start) $start = now();

  foreach (ray('amount portion minimum useMax amtMax') as $k) if ($$k and $err = u\badAmount($$k, '>=0', $k == 'useMax' ? 0 : 2)) return say("$err ($k)", $k);

  if ($id and !db\exists($tnm, compact('id'))) return say(t('To insert a record, leave the id empty.'), 'id');
  
  u\preRay(compact(ray($flds)), $sta['input']);
}

function formRules_submit($form, &$sta) {
  list ($tnm, $flds, $input) = getTtype($sta);

  extract($input);
  foreach (just(NULL_FLDS, $input) as $k => $v) if (empty($v)) $input[$k] = NULL;
  
  if ($id) {
    db\update($tnm, $input, 'id');
  } else {
    unset($input['id']);
    db\insert($tnm, $input, 'id');
  }

  return go('/sadmin/rules', 'info saved');
}

function getTtype($sta) {
  extract(just('ttype', $input = $sta['input'], NULL));
  list ($tnm, $flds) = $ttype ? ['tx_rules', RULE_FLDS] : ['tx_timed', TIMED_FLDS];
  $input = just($flds, $input, NULL);
  return [$tnm, $flds, $input];
}
