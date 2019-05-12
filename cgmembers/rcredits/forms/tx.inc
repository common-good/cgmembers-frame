<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Get a transaction request (usually pay or charge) and process it
 * @param string $args: all or some parameters for the form (when called from directory or draft form)
 * NOTE: currently handles only pay and charge
 */
function formTx($form, &$sta, $args = '') {
  extract(just('who amount goods purpose', $args));
  global $mya;

  if (!$mya->can(B_SELL)) return w\go('/summary');
  $type = strtolower($_SERVER['REQUEST_URI']);
  $paying = ($type == 'pay');
  
  list ($opDesc, $role) = $paying
  ? [t('Pay'), 'payer']
  : [t('Charge'), 'payee'];
  
  $selling = explode("\n", $mya->selling);
  $purpose = @$purpose ?: ($paying ? '' : @$selling[0]);
  
  focusOn(@$who ? 'amount' : 'who');

  $title = $opDesc;
  $buttons = $paying ? w\btn('/pay/payments', t('Upload Payments'), 'default') : '';
  if ($mya->co) $buttons .= ' ' . ($paying 
    ? w\btn('/pay/payroll', t('Upload Payroll'), 'default')
    : w\btn('/charge/invoices', t('Upload Invoices'), 'default')
  );
  if (!$buttons) unset($buttons);
  $who = textFld($opDesc . t(' whom:'), [t('name, id, email, or phone')], required(@$who));

  $amount = textFld(t('Amount:'), [t('amount to ') . strtolower($opDesc)], required(@$amount));
/*  $goodsOpts = ray(R_WHYS);
  unset($goodsOpts[FOR_SHARE]); // this gets set only automatically
  */
  
  $purpose = textFld(t('For:'), [t('purpose or description of goods and services')], required(@$purpose));
  $oftens = ['' => t('No repeat')] + ray(OFTENS);
  unset($oftens['X']);  
  if ($paying) $repeat = selectFld(t('&nbsp;'), '', dft(''), $oftens);
  
  $scanClass = 'not-scanned';
  $$type = submit($opDesc);

  w\whoFldSubmit('who', $opDesc . t(' %amount to %name?'), ray('allowNonmember', !$paying));
  
  $form = cgform(@compact(ray("title buttons who amount purpose repeat $type")));
  $form['#attributes']['class'][] = $scanClass;

  return $form; // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
  $info = just('who amount purpose repeat', $sta['input']);
  extract($info);
  if (strpos($who, '@') and !u\validEmail($email = $who)) return say('bad email', compact('email'), 'who');
  if (!$a = w\whois($who, 'who', $info, 'no self-trading', op($sta) == 'pay')) return FALSE;
  $uid = $a->id;
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
  u\preray(compact(ray('amount uid')), $sta['input']);
}

function formTx_submit($form, &$sta) {
  global $mya;
  $myid = $mya->id;
  extract($info = just('who amount purpose repeat uid goods', $sta['input']));
//  if (strpos($who, '@')) return w\go('/charge/nonmember/' . u\urlify(serialize($info)));

  u\setDft($goods, FOR_GOODS);
  $DBTX = \db_transaction();
  $func = op($sta);
  if ($func == 'pay') {
    list ($msg, $args) = be\transfer('payment', $mya, r\acct($uid), $amount, $purpose, $goods, @$repeat ? ['recurs' => 1] : []);
  } else {
    list ($msg, $args) = be\invoice($mya, r\acct($uid), $amount, $purpose, $goods, @$repeat ? ['recurs' => 1] : []);
  }
  $ok = @$args['success'];
  if ($func == 'pay' and $mya->snap) r\tellAdmin(t('SNAP member %who %did something', 'who did', $mya->fullName, $ok ? t('bought') : t('tried to buy')), ['topic' => t('SNAP purchase') . ($ok ? '' : t(' failed'))]);

  if ($ok and $period = @$repeat) {
    $created = today();
    $subs = ray('payer payee created ended', $mya->id, $uid, $created, 0);
    $info = $subs + compact(ray('amount period'));
    if ($id = db\get('id', 'r_recurs', 'payer=:payer AND payee=:payee AND created=:created AND ended=:ended', $subs)) $info += compact('id');
    db\updateOrInsert('r_recurs', $info, 'id');
    $repeats = ray(OFTENS);
//      $often = strstr($repeats[$repeat], ' ', TRUE);
    $often = $repeats[$repeat];
    $msg .= '|repeats';
  }
  unset($DBTX);
  
  return say($msg, $args + @compact('often'), $ok ? '' : 'amount');
}
