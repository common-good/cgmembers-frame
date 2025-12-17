<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * CRUD for tx_entries_all
 */
 
const E_FLDS = 'id amount uid agentUid xid entryType description cat rule relatedId relType';
const E_TNM = 'tx_entries';
 
function formTxEntry($form, &$sta, $args = '') {
  global $mya;
  
  foreach (ray(E_FLDS) as $k) $$k = NULL;
  extract(crud(E_TNM, $args, 'id', E_FLDS)); // get ret or [title, orig, id, and $flds]
  if (nn($ret)) return $ret; // show list of records, if appropriate

  foreach (ray('uid agentUid') as $k) if (isset($$k) and !is_numeric($$k)) $$k = db\get('fullName', 'users', ['uid' => $$k]);
  
  $uid = textFld(REQ . t('Account:'), t('Type the full name on the account.'), dft($uid));
  $agentUid = textFld(t('Agent:'), t('Type the full name of the agent (if different).'), dft($agentUid));
  $xid = intFld(REQ . t('Xid:'), '', dft($xid));
  $cat = selectFld(REQ . t('Category:'), '', dft($cat), ray(0, t('none')) + db\keyed('id,category', 'tx_cats'));
  $rule = intFld(t('Rule:'), '', dft($rule));
  $relatedId = intFld(t('Related ID:'), '', dft($relatedId));
  $relType = textFld(t('Rel Type:'), t('I or nothing'), dft($relType));
  $entryType = selectFld(REQ . t('Type:'), '', dft($entryType), ray(E_TYPES));
  $amount = numFld(REQ . t('Amount:'), '', dft($amount ?: 0) + vmin(PHP_INT_MIN));
  $description = textFld(REQ . t('Description:'), '', dft($description));
  
  $id = hidFld($id);
  $submit = t('Submit');
  
  return cgForm(compact(ray('title ' . E_FLDS . ' submit')));
}

function formTxEntry_validate($form, &$sta) {
  extract(just('amount uid agentUid xid rule relatedId', $sta['input']));

  foreach (ray('uid agentUid') as $k) if ($v0 = $$k and !is_numeric($$k)) {
    if (!$$k = db\get('uid', 'users', ray(is_numeric($$k) ? 'uid' : 'fullName', $$k))) return say(t('User not found: ') . $v0, $k);
  }
  if (!$agentUid) $agentUid = $uid;
  
  if ($amount and $err = u\badAmount($amount, '', 2)) return say("$err (amount)", 'amount');
  foreach (ray('xid rule relatedId') as $k) if ($$k and $err = u\badAmount($$k, '>0', 0)) return say("$err ($k)", $k);

  u\preRay(compact(ray('uid agentUid amount')), $sta['input']);
}

function formTxEntry_submit($form, &$sta) {
  extract($input = just(E_FLDS, $sta['input']));
  foreach (just('rule relatedId relType cat', $input) as $k => $v) if (empty($v)) $input[$k] = NULL;
  
  if ($id) {
    db\update(E_TNM, $input, 'id');
  } else {
    unset($input['id']);
    $id = db\insert(E_TNM, $input, 'id');
    db\insert(E_TNM, $input + ray('id', -$id), 'id');
  }

  return go('/sadmin/tx-entry', 'info saved');
}
