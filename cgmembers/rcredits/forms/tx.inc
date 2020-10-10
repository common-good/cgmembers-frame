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
function formTx($form, &$sta, $type, $args = '') {
  extract(just('who amount goods purpose', $args));
  global $mya;

  $pay = in($type, 'pay both');
  $charge = in($type, 'charge both');
  $opDesc = $pay ? t('Pay') : t('Charge');
  
  $selling = explode("\n", $mya->selling);
  $purpose = nn($purpose) ?: ($charge ? nni($selling, 0) : '');
  
  focusOn(nn($who) ? 'amount' : 'who');

  $title = $opDesc;
  if ($mya->co) {
    $buttons = $pay ? btn('/tx/pay/payments', t('Upload Payments'), 'default', '', clas('w-pay')) : '';
    if ($pay) $buttons .= btn('/tx/pay/payroll', t('Upload Payroll'), 'default', '', clas('w-pay'));
    if ($charge) $buttons .= btn('/tx/charge/invoices', t('Upload Invoices'), 'default', '', clas('w-charge'));
  } else $buttons = NULL;
  $who = textFld(t('Who:'), [t('name, id, email, or phone')], required(nn($who)));

  $amount = numFld(REQ . t('Amount:'), [t('transaction amount')], dft(nn($amount)) + vmin(.01));
  $purpose = textFld(t('For:'), [t('purpose or description of goods and services')], required(nn($purpose)));
  $start = dateFld(t('Date:'));
  $period = rendA('period', selectFld(BARE, '', dft(''), r\recurRay(FALSE, PERIODS_PLURAL)));
  $periods = intFld(t('Every:'), '', dft(0) + suffix($period));
  $end = dateFld(t('Ending:'), t('Leave blank for "never". You can end it at any time at <%a>History / Recurring Transactions</a>.', '_a', atag('/history/recurring')));
  
  $btns = btn('', t('delay'), 'default', '', 'btn-delay') . btn('', t('repeat'), 'default', '', 'btn-repeat') . btn('/history/recurring', t('manage'), 'default', '', 'btn-manage');
  $submit = submit(t('Submit'), '', '', suffix($btns));

  w\whoFldSubmit('who', $opDesc . t(' %amount to %name?'), ray('allowNonmember', $charge));
  $paying = hidFld($pay);
  
  jsx('tx');
  $form = cgform(compact(ray("title buttons who amount purpose start periods end submit paying")));

  return $form; // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
  global $mya;
  $info = just('who amount purpose start end paying', $sta['input'], NULL);
  extract($info);
  if (mb_strpos($who, '@') and !u\validEmail($email = $who)) return say('bad email', compact('email'), 'who');
  if (!$a = w\whois($who, 'who', $info, 'no self-trading', $paying)) return FALSE;
  $uid = $a->id;
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
  foreach (ray('start end') as $k) if ($$k and $err = u\badDate($$k, $mya->superAdmin ? today() - 180 * DAY_SECS : today())) return say($err, $k);
  u\preray(compact(ray('amount uid start end')), $sta['input']);
}

function formTx_submit($form, &$sta) {
  global $mya;
  $myid = $mya->id;
  extract($info = just('amount purpose period periods start end uid goods paying', $sta['input'], NULL));
//  if (mb_strpos($who, '@')) return w\go('/charge/nonmember/' . u\urlify(serialize($info)));

  u\setDft($goods, FOR_GOODS);
  $DBTX = \db_transaction();
  $delay = ($start > today());
  $repeats = ($periods > 0);
  
  if (!$delay) {
    $data = $period ? ['recurs' => 1] : [];
    if ($start) $data['created'] = $start; // superAdmin chose a date in the past
    list ($ok, $msg, $args) = $paying
    ? be\transfer('payment', $mya, r\acct($uid), $amount, $purpose, $goods, $data)
    : be\invoice($mya, r\acct($uid), $amount, $purpose, $goods, $data);
  } else list($ok, $msg, $args) = [TRUE, '', []];

  if ($paying and $mya->snap) r\tellAdmin(t('SNAP member %who %did something', 'who did', $mya->fullName, $ok ? t('bought') : t('tried to buy')), ['topic' => t('SNAP purchase') . ($ok ? '' : t(' failed'))]);

  if ($delay or ($repeats and $ok)) { // need a rule template
    if (!$end) $end = NULL;
    list ($action, $thing, $tnm, $idFnm) = $paying
    ? [ACT_PAY, t('payment'), 'tx_hdrs', 'xid'] 
    : [ACT_CHARGE, t('invoice'), 'r_invoices', 'nvid'];
    $subs = ray($paying ? 'from to' : 'to from', $mya->id, $uid);
   
    if ($repeats) {
      $where = "tp.from=:from AND tp.to=:to AND action=:ACT_PAY AND period<>'once' AND IFNULL(end, :NOW + 1) > :NOW";
      if (!$delay and db\exists('tx_templates tp', $where, $subs)) {
        $otherName = db\get('fullName', 'users', compact('uid'));
        if ($otherName) say(t('Note: You also have another active recurring transaction with %otherName. Go to <%a>History / Recurring Payments</a> to manage your recurring transactions.', 'otherName _a', $otherName, atag('/history/recurring')));
      }
      $often = r\recurDesc($period, $periods);
      $args += compact('often');
      $msg .= '|repeats';
    } else { list ($periods, $period) = [1, 'once']; }
    
    if (!$start) $start = today(); // handle ''
    $info = $subs + compact(ray('action amount purpose period periods start end'));
    $id = db\insert('tx_templates', $info, 'id');
    if ($delay) {
      $msg .= '|thing scheduled';
      $args += compact('thing');
    } else { db\update($tnm, ray("$idFnm recursId", $args[$idFnm], $id), $idFnm); }
  }
  unset($DBTX);

  if ($msg[0] == '|') $msg = substr($msg, 1);
  say(tr($msg, $args), $ok ? '' : 'err');
  return go('console');
}
