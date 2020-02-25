<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Connect a bank account.
 */
function formConnect($form, &$sta, $args = '') {
  global $rUrl;
  global $mya;
  
  if (!$mya) if ($ret = doPartner()) return r\go($ret);

  $usa = ($mya->country == US_COUNTRY_ID);
  $usa = TRUE; // always true for now

  $refills = boolFld(t('Refills:'), t('Refill your %PROJECT Account automatically when it goes below a chosen "Target Balance"? Say no if you tend to bounce checks or live close to the wire.'), $mya->refill);
  if ($mya->co) $cashout = radiosFld(t('Cashout:'), t('If you choose weekly or monthly automatic transfers to your bank account, the system will leave a balance in your %PROJECT account equal to your average weekly payments received, over the past six months.'), required($mya->cashoutW ? 1 : ($mya->cashoutM ? 2 : 0)), ray('0 1 2', t('no automatic cashout'), t('weekly (automatic)'), t('monthly (automatic)')));
  $autopay = autopays($orig);
  $autoboxes = hidFld(serialize($orig));
  $minDesc = t('How much to keep in your %PROJECT Account, for purchases. When your balance goes below this amount, the system automatically refills it for you from your bank account. Refilling may take 3-5 business days.');
  $minDescNegative = $mya->balance < 0 ? t(' <b>If you choose a negative amount</b>, it will be increased by %bump a week until it is greater than zero.', 'bump', u\fmtAmt(MIN_WEEKLY_BUMP)) : '';
  $minWarning = t('note|ach warning');
  $minDefault = u\fmtAmt($mya->minimum ?: 0, 's$');
  $target = textFld(t('Target Bal:'), [t('Your preferred "normal" balance'), "<p>$minDesc$minDescNegative</p><p>$minWarning</p>"], dft($minDefault));
  $achMin = textFld(t('Min Transfer:'), [t('Minimum bank transfer amount'), t('This much (or more, if necessary) will be transferred from your bank account in exchange for %RCREDITS when your balance goes below the Target Balance you chose, above. The suggested amount works well for most people.')], dft(u\fmtAmt($mya->achMin ?: R_ACHMIN_DEFAULT, 's$')));

//  $savingsAdd = textFld(t('Extra Savings:'), [t('Extra savings amount'), t('Your incentive rewards go automatically into a Savings Reserve, for later or as needed. Your Savings Reserve works like a traditional reserve savings account that kicks in whenever you would otherwise overdraw your primary account -- except there are no fees. This also helps the community by keeping more funds in the system. You get a monthly inflation adjustment on any funds you hold in the %PROJECT system (at @inflate% APR). If you want to hold even more in your Savings Reserve (a great idea!) enter the additional amount here.', ['@inflate' => R_INFLATION_RATE * 100])], required(u\fmtAmt($mya->savingsAdd)));
  $saveWeekly = textFld(t('Save Weekly:'), [t('Additional amount to add weekly to Target Balance'), t('Putting money away as savings can be a challenge. This may help: choose a small additional amount to be added automatically to your %what every week.', 'what', $mya->balance < 0 ? t('Target Balance') : t('Target Balance'))], dft(u\fmtAmt($mya->saveWeekly)));
  
  $targetFields = fieldSet('targetFields2', compact(ray('target achMin saveWeekly')), BARE);
  //$bankAccountType = radiosFld(t('Type:'), '', required($mya->can(B_SAVINGS)), $typeOptions);
  $title = t('Banking Settings');
    
  if ($hasBank = $mya->hasBank) {
//    if ($usa) list ($route, $bankAccount) = u\parseBankAccount(@$bankAccount);
    $remove = submi(t('Remove connection'), '', 'xs', ['parents' => ['remove']]);
    $bankAccount = item("xxxxxx$mya->last4bank &nbsp; &nbsp;", t('Account:'), '', w\suffix(\render($remove)));
//    $submit = submit();
    $fields = 'title bankAccount cashout autopay autoboxes refills targetFields';
    $subtext = $chox = NULL;
  } else { // no connected account yet
    if ($mya->stepIsDone('fund')) {
      $connect = boolFld(t('Connect:'), '', 0);
      $chox = 2;
    } else { // during account setup
      $title = t('Getting Money In or Out');
      $subtext = t('<p>Choose how you want to get money into (or out of) your %PROJECT Account. The first two options are available through %PROJECT only in Western Massachusetts, Ann Arbor Michigan, and Goshen Indiana.</p>');
      $methodChoices = ray(t('My employer will pay me through %PROJECT or I will sell goods and services through %PROJECT.,I live near a store that will put money on my card in exchange for cash.,I want to transfer funds to or from my bank account.'));
      $connect = radiosFld(t('Connect How:'), '', required(-1), $methodChoices);
      $chox = 3; // number of choices
    }
    $chox = hidFld($chox);

    $securityLink = r\promoLink('about-us/privacy-and-security');
    $reassurance = $mya->minimum ? '' : t('<p><b>NOTE:</b> Funds will be taken from your bank account ONLY when you explicitly request a transfer or when you choose automatic refills.</p>');
    $connectText = item(t('<p>Complete this form to authorize funds to be transferred to or from your checking account at your request, by Remotely-Created Check (RCC) or by Automated Clearing House (ACH). See our <%a>Privacy and Security Policy</a> for details on how your information is protected.</p>', '_a', atag($securityLink)) . $reassurance);
    
    if ($usa) $routingNumber = textFld(t('Routing:'), t('Type carefully your bank\'s routing number.'));
    //$bankAccount = textFld(t('Account:'), t('Type carefully your checking account number.'));
    $bankAccount = textFld(t('Account:'), t('Type <b class="loud">carefully</b> your <b>checking</b> account number. <span>Savings accounts will not work, at this time.</span>'));
//    $bankAccount2 = textFld(t('Again:'), t('Type your account number again.<br>(we need to be REALLY SURE we don\'t connect to the wrong account)'));
  
    $connectFields = fieldSet('connectFields2', @compact(ray('connectText routingNumber bankAccount cashout autopay autoboxes refills targetFields')), BARE);
//    $submit = submit(t('Connect'));
    $fields = 'title subtext connect connectFields chox';
  }

  if ($mya->depends) {
    $subtext .= t('<p><b class="loud">Note: </b>This feature is not permitted for trial and sponsored accounts.</p>');
    return cgform(compact(ray('title subtext connect')));
  }
  
  jsx('bank-prefs', $vs = ray('connectLabel saveLabel mindft chox', t('Connect'), t('Save'), R_MIN_DEFAULT, $chox));
  return cgform(@compact(ray($fields)) + w\setupFoot($hasBank ? $vs['saveLabel'] : $vs['connectLabel']));
}

function formConnect_validate($form, &$sta) {
  global $mya;

  extract(just('connect routingNumber bankAccount bankAccountType refills target achMin saveWeekly chox', $sta['input']));

  foreach (ray('target:,achMin:,saveWeekly:>=0') as $k => $v) { // and savingsAdd someday 
//    if (test() and !isset($sta['input'][$k])) $sta['input'][$k] = 0; // empty makes tests fail?
    if (empty($sta['input'][$k])) $sta['input'][$k] = 0; // empty makes tests fail?
    if ($err = amtErr($k, $sta, $v) and @$refills) return sayFieldErr($err, $k); // fix the data even if invisible and irrelevant
    $$k = $sta['input'][$k];
  } // must be before return because params are always present and must have "$" removed by amtErr()

  if (op($sta) == 'remove') return;

  if (!$mya->hasBank) {
    if ($chox == 3 and is_null(@$connect)) return say(t('You must choose a funding method'), 'connect');

    if ($connect < $chox - 1) return; // nothing more to do

    foreach (ray($accts = 'routingNumber bankAccount') as $one) $$one = preg_replace('/[\.\- ]/', '', @$$one);
    u\preray(compact(ray($accts)), $sta['input']);
    if (!u\routingOk($routingNumber)) return say('bad routing number', 'routingNumber');
    if (!u\bankAcctOk($bankAccount)) return say('bad account number', 'bankAccount');
  }

  if (@$refills) {
    $floor = min($mya->balance, max(0, $mya->floor)); // can't go deeper in debt
    if ($target < $floor) return say('min sub floor', ['floor' => u\fmtAmt($floor)], 'target');
    if ($achMin < R_ACHMIN) return say('bad achmin', 'achMin');
  }

}

function formConnect_submit($form, &$sta) {
  global $mya;

  extract(just('connect routingNumber bankAccount cashout autopay autoboxes refills target achMin saveWeekly chox', $sta['input']));
  if (nn($chox) == 3 and $connect < 2) $mya->update('fundBy', $connect == 1 ? 'cash' : 'pay');
  $connect = (op($sta) != 'remove' and nn($connect) !== 0 and nn($connect) != $chox - 2);
  $minimum = $mya->minimum;

  if ($connect) { // wants bank or has bank, not removing
    if (!$mya->hasBank) { // wants bank
      if ($mya->country == US_COUNTRY_ID) $bankAccount = "USkk$routingNumber$bankAccount";
      $last4bank = substr($bankAccount, -4, 4);
    } else $bankAccount = 'no change'; // has bank
    if ($refills) $minimum = $target; // change minimum only explicitly
  } else { // removing or choosing not to connect
    $bankAccount = $last4bank = '';
    $achMin = $mya->achMin ?: R_ACHMIN_DEFAULT;
  }

  list ($hasBank, $refill) = [$connect, $connect and $refills];
  list ($cashoutW, $cashoutM) = [$connect and @$cashout == 1, $connect and @$cashout == 2];
  $fields = 'hasBank refill cashoutW cashoutM minimum achMin saveWeekly';
  if ($bankAccount != 'no change') { // bank account changed. update and reverify.
    $fields .= ' bankAccount last4bank'; // avoid overwriting v-encrypted value
    $mya->setBit(B_BANKOK, FALSE);
    if (!$connect) while ($xid = db\get('xid', 'r_usd', 'deposit=0 AND payee=:id', ray('id', $mya->id))) {
      $DBTX = \db_transaction();
      foreach (ray('tx_entries tx_hdrs r_usd') as $table) {
        db\del($table, '', 'xid = :xid', compact('xid')); // no transfers for a deleted bank connection
      }
      unset($DBTX);
    }
  }
  $mya->update(compact(ray($fields)));
  
  if (@$autoboxes and $orig = unserialize($autoboxes)) foreach ($orig as $uid => $wasSet) {
    $setNow = isset($autopay[$uid]);
    if ($setNow xor $wasSet) r\acct($uid, $mya->id)->setRel('autopay', $setNow);
  }
  
//    if ($saveWeekly < 0) say(t('@amount will be moved every week from your Savings Reserve to your Primary Account, until your Savings Reserve reaches its minimum.', ['@amount' => u\fmtAmt($saveWeekly)]));
  return w\goNextStep('fund');
//  say('info saved' . ($mya->ok ? '' : '|return to membership'));
}

/**
 * Return a set of automatic payment checkboxes, if appropriate.
 * @param array $orig: (RETURNED) list of the autopayable account IDs (if any) and original setting
 * @return the checkbox fields or NULL if none.
 */
function autopays(&$orig = '') {
  global $mya;

  $rows = db\q('SELECT uid,fullName,r.:IS_AUTOPAY AS auto FROM r_relations r LEFT JOIN users u ON u.uid=r.main WHERE other=:myid AND r.:IS_CUSTOMER', ray('myid', $mya->id))->fetchAll();
  if (!$rows) return NULL;
  
  foreach ($rows as $i => $one) {
    extract((array) $one); // uid fullName auto
    $nms[$uid] = t('Autopay ') . $fullName;
    if ($orig[$uid] = $auto) $dfts[] = $uid;
  }
  if (!@$dfts) return NULL; // no autopays, so don't show choices
  $help = t('When you choose automatic invoice payment for a particular company, funds will be drawn from your bank account as needed. Leave the box(es) unchecked if you want to request transfers explicitly.');
  return boxesFld(t('Invoices:'), $help, @$dfts ?: [], $nms);
}
