<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Admin as a;
use CG\Risk as k;

/**
 * Show a summary of the current account.
 * This function uses jQuery code defined in misc.js
 * UNUSED @param string $qid: (optional) ID or qid of the account to summarize and manage (for cAdmin)
 */
function formSummary($form, &$sta) {
  global $base_url;
  global $mya; $myid = $mya->id;

  if ($mya->closed) say(t('Your account is closed, but you can still see your account history.'));

  $stepUrl = $mya->ok ? 'summary' : $mya->nextStepUrl('', $msg);
/*
  if ($stepUrl == 'signup' and $info = r\relation('reid,code', "other=:myid AND :IS_CUSTOMER AND code<>''", compact('myid'))) {
    extract($info);
    $stepUrl .= "/reid=$reid&customer=$code";
  }
  */
  $extras = '<br>' . u\fmtPhone($mya->phone) . " ($mya->faxetc)<br>$mya->email ";

  if ($mya->admin2 and !$mya->proSe) { // some kind of admin (but not self)
    include_once __DIR__ . '/../admin/admin-web.inc';  
    $form = w\adminSummary();
    foreach (ray('photo legalName shortName community') as $k) if (@$form[$k]) { // move these up (see return line)
      $$k = $form[$k];
      unset($form[$k]);
    }

    if (!$mya->isCtty and $mya->cAdmin) {
      if ($mya->trialco or !$mya->co) $extras .= ' ' . a\cardLink($mya->id);
      $extras .= ' ' . a\recropLink();
    }
    
    if ($stepUrl == 'summary') {
      if (@$msg) say(t('New member is told: ') . t($msg));
    } elseif ($stepUrl) say(t('New member will be sent <a href="%stepUrl">there</a> upon signing in.', 'stepUrl', "$base_url/$stepUrl"));
    
  } else { // normal
    if ($stepUrl and $stepUrl != 'summary') return w\go($stepUrl, $msg);
    if (@$msg) say($msg);
    if ($mya->legalName != $mya->fullName and !$mya->depends) $legalName = item($mya->legalName, t('Legal Name:'));    
  }

//  extract(just('returnMo return cttyBeneMo cttyBene', $mya->j_stats()));
  $gifts = u\fmtAmt($mya->totalGifts());
//  $cttyBene = item("$cttyBeneMo " . t('over the past month') . "<br>$cttyBene " . t('overall (ever), including %gifts in contributions', compact('gifts')), t('Social return:'), t('Financial benefit to your community to date, from your participation together with other members, empowering economic democracy! [<%a>details</a>]', '_a', atag('/help/social-return')));  
  $cttyBene = NULL;
//  $cttyBene = item("$cttyBene " . t('overall, including your %gifts in contributions', compact('gifts')), t('Social return:'), t('Financial benefit to your community to date, from your participation together with other members, empowering economic democracy! [<%a>details</a>]', '_a', atag('/help/social-return')));  
    
  // $showCustomers = item(w\btn('/community/data/qName=6Customers', t('Customer List')));

  if ($mya->canManage and $mya->member) {
    $hasMultipleAccounts = ($mya->co or db\exists('r_relations r LEFT JOIN users u ON u.uid=r.main', 'other=:id AND permission>0 AND u.:IS_CO', ['id' => $mya->agentId]));
    $another = $hasMultipleAccounts ? t('Another') : t('a');
    $makeCo = $mya->depends ? '' : w\btn('/signup-co/relate=1', t('Open %another Company Account', compact('another')), 'primary', 'xs');
//    if (!$mya->jid and !$mya->co) $makeJoint = w\btn('/signup/joint=1', t('Make this a joint account'), 'primary', 'xs');
    $makeJoint = ($mya->member and !$mya->jid and !$mya->co and !$mya->depends) ? w\btn('/prejoin', t('Make this a joint account'), 'primary', 'xs') : '';
//    $goFood = $mya->food > 0 ? lnk('/settings/food', t('Food Fund')) : btn('/settings/food', t('Food Fund'), 'warning');
//    if ($mya->cAdmin) $goFood = '<br><br>' . $goFood; // make sure it doesn't hide behind the photo
    $goFood = '';
    if (!$mya->co and $hasMultipleAccounts) $clickPhoto = item(t('Click your photo%where to manage your other account(s).', 'where', $mya->cAdmin ? t(' in the upper right corner') : ''));
  } else $makeCo = $makeJoint = $goFood = $reports = '';

  $reports = $mya->co ? w\btn('/history/company-reports', t('Reports')) : '';
//  if ($mya->id == CGID) $deposits = w\btn('/sadmin/show-deposits', t('Deposits'));
  $deposits = $mya->id == CGID ? w\btn('/sadmin/show-reconciled', t('Dollar Pool Transactions')) : '';
  $topButtons = item($makeCo . $makeJoint . $goFood . $reports . $deposits);

  $title = t('Account Summary');
  $accountName = item($mya->j_fullName . ($mya->jid ? '<br>' : ' ') . "($mya->j_name)", t('Name:'));
  $address = $mya->cAdmin ? item(($mya->trialco ? $mya->contact : $mya->postalAddr) . $extras, t('Contact:')) : NULL;
  $balance = u\fmtAmt($mya->balance);
//    $totalDesc = u\fmtAmt($mya->r);
//    $balanceDesc = $mya->savings ? t(' plus your savings, for a total of @total', ['@total' => $totalDesc]) : '';
//    $balanceDesc = '';
  $balanceDesc = $mya->activated ? t(' <small>(see <%a>History</a>)</small>', '_a', atag('/history')) : '';
  $balance = item($balance . $balanceDesc, t('Balance:'));

  if (!$mya->isCtty) { // unless this is a community or region account
    $acctType = $mya->co ? t('company account') : ($mya->jid ? t('joint account') : t('personal account'));
//    $savings = item(u\fmtAmt($mya->savings) . t('<small> (your total incentive rewards to date)</small>', [
//    list ($rewardNote, $rewardDesc) = $mya->cttyRewardy ? [t('<small> (your total incentive rewards to date)</small>'), t('This amount will be yours to keep, once your community reaches a certain size.')] : ['', t('Some of this amount may be yours to keep, at your %PROJECT community\'s decision, once membership reaches a certain size.')];
//    $savings = item(u\fmtAmt($mya->rewards) . $rewardNote, t('Credit Line:'), $rewardDesc);
    $suffix = ($mya->debt or $mya->depends or !$mya->refill) ? '' : t(' (<%a>Click here</a> to let your balance go negative.)', '_a', 'a id="activate-credit"');
    list ($credit, $normally) = $mya->depends ? ['$0', t(' (normally)')] : [u\fmtAmt(-$mya->floor), ''];
    $creditLine = $mya->ofAge ? item($credit, t('Credit Line:'), t('Based%normally on your account\'s average monthly activity over the past 6 months.', compact('normally')), w\suffix($suffix)) : NULL;
//    $committed = item(u\fmtAmt(max(0, $mya->committed)), t('Committed:'), t('reserved for donation to Common Good Finance'));

/*    $successes = db\count('users', 'helper=:myid AND activated<>0', compact('myid'));
    $invites = max($successes, db\count('r_invites', 'inviter=:myid', compact('myid')));
    $doInvite = $mya->cAdmin 
    ? w\btn('/community/invite/print', t('front')) . ' ' . w\btn($mya->invitationBack(), t('back'))
    : t('<small>[<%a>invite someone</a>]</small>', '_a', atag('/community/invite'));
    $invites = item(t('You have invited at least %invites people (%successes are now members). ', compact('invites', 'successes')) . $doInvite, t('Invitations:'));
  */
  
    $choiceList = choiceListFld();
    
  } else $acctType = t('community account');

  if ($mya->cAdmin) {
    $extra = " id#$mya->id";
    if (FALSE and $bankInfo = $mya->bankInfo) {
      extract($bankInfo, EXTR_PREFIX_ALL, 'b');
      $bankInfo = "$b_name (routing #$b_route)\n$b_address, $b_city, $b_state $b_zip";
      $extra .= ' ' . w\popHelp(t('bank info'), $bankInfo, '#'); //strtr(t(' <@a>bank info</a>'), ['@a'=>$bankInfo]); // strtr circumvents Drupal crap
    }
  } else $extra = '';
  $accountId = item(($mya->mainQid ?: 'N/A') . ' (' . $acctType . ')' . $extra, t('ID:'));

  $fields = 'photo title clickPhoto showCustomers topButtons accountId accountName address legalName shortName community balance newBalance creditLine creditLimit committed choiceList';
//  foreach (ray('details') as $k) $$k = @$form[$k]; unset($form[$k]); // put these on the left if admin
  $fields = @compact(ray($fields));
  
//  if ($mya->superAdmin and !$mya->ok) $onSubmit = w\whoFldSubmit('helper', t('This person was invited by %name, yes?'));
  if ($mya->admin2 and !$mya->proSe) {
    $class = ['class' => ['admin']];
    $right = w\fieldSet('right2', $form, '', '', $class); // the admin part
    $left = w\fieldSet('left2', $fields, '', '', $class);
    $form = compact('right', 'left');
    $form['none'] = submit('none'); // force _submit despite lack of submit button    
  } else $form = $fields;

  if ($mya->superAdmin and $mya->proSe) $form = just('title accountId accountName', $form);
  
  jsx('summary');
  
  return cgform($form, '', TRUE);
}

function formSummary_validate($form, &$sta) {
  global $mya;

  if (in(op($sta), 'openCompany openJoint')) return;
  extract(just('uid legalName creditLimit dob rTrader helper moves tickle adminable', $sta['input']));
  if (nn($tickle, NULL) !== '' and $err = u\badDate($tickle)) return say($err, 'tickle');

  if (nn($dob) and !formBasic_validate($form, $sta, FALSE, 'dob')) return;
  
/*  if (in(nn($federalId), [R_ON_FILE, t('SSN')])) {
    unset($sta['input']['federalId']);
  } elseif (@$federalId and $err = u\badSsn($federalId, $mya->dob)) return say($err, ['what' => 'federalId'], 'federalId');
*/
  if (@$helper) {
    if (!$h = whois($helper, 'helper', $sta['input'])) return say(t('I don\'t know what helper account you mean.'), 'helper');
    $helper = $h->id;
  } else {unset($helper); unset($sta['input']['helper']);}

  if (@$rTrader and !$mya->ok and !$mya->isCtty) {
    if (!@$helper) return say(t('You must specify who gets the helper reward.'), 'helper');
//    if (!@$helper or $helper == 1) return say(t('You must specify who gets the helper reward.'), 'helper');
    if (!$mya->member and is_null(nn($adminable[B_MEMBER], NULL))) return say(t('That account is not READY (not a member).'), 'adminable'); // test "is_null" because 0 is the value when set
    if (!$mya->confirmed and $mya->helper != 1) say(t('NOTE: Member %helperName has not yet confirmed this invitation.', 'helperName', $h->fullName));
  }

  if (!@$rTrader and $mya->ok) { // de-activating
    if ($mya->balance) return say(t('You cannot de-activate an account with a non-zero balance.'), 'err');
  }

  foreach (ray('creditLimit') as $k) {
    if (isset($$k)) {
      if ($err = u\badAmount($$k)) return sayFieldErr($err, $k); 
      $$k = u\cleanAmount($$k);
    }
  }
  if (isset($moves) and $err = u\badAmount($moves, '>=0', 0)) return sayFieldErr($err, 'moves');
  u\preray(@compact(ray('tickle helper who creditLimit')), $sta['input']);
}

function formSummary_submit($form, &$sta) {
  global $mya;
  $op = op($sta);

  if (!$mya->admin2) return; // hack attempt
  if ($op == 'secureMsgs') return w\go('', showSecureMessages());

  extract($args = just('uid who legalName creditLimit tickle note notes dob helper mediaConx moves rTrader risks adminable', $sta['input']));
  if (!$mya->cAdmin) u\EXPECT(!array_diff(array_keys($args), ray('uid tickle note')), 'too many unpermitted changing fields in ' . pr($args));

  if (!@$tickle) {
    $tickleDone = $mya->ok ? (nn($note) and !$tickle and $mya->tickle) : 'activate';
    list ($zot, $tickle) = $mya->nextTickle($tickleDone);
//    if ($tickleDone) $note = trim(t('(%typeDone) ', 'type', @$note ? '': "$tickleType ") . @$note);
//    if ($tickleDone === TRUE) $note = trim(t('Done ') . @$note);
  }
  if (@$note) $notes = $mya->stampNote($note) . (isset($notes) ? $notes : $mya->notes);

  $aa = $mya->agentA;
  /*
  if (@$who and ($who->community == $aa->community or $aa->id == 1)) {
    svar('myid', $who->id); // make it the new current account
    list ($newAcct, $oldAcct) = array($who->fullName, $mya->fullName);
    return say('changed account|lost old changes', compact('newAcct', 'oldAcct'));
  } 
  */
  
  $DBTX = \db_transaction();

  if ($mya->cAdmin or $mya->admin) setRisksAndFlags($mya, @$risks, @$adminable); // don't test for isset (allow turning off all bits); must precede activate() because that function sets ssnOff bit
    
  if (!$mya->isCtty) {
    $fullName = $mya->fullName;
    if (FALSE and !$mya->confirmed and @$helper and @$helper != $mya->helper) { // helper has not confirmed yet (must be self-signup)
      $_a1 = r\acct($helper)->makeDo('confirmInvite', $mya->id); // link to confirm invitedness
      r\message($helper, 'confirm invite', compact('_a1', 'fullName')); // ask inviter to confirm
    }

    if (@$rTrader and !$mya->ok) {
      activate($helper);
    } elseif (@$federalId and $federalId != $mya->federalId) $mya->update('ssnData', '');
  }

  if (isset($rTrader)) $mya->setBit(B_OK, @$rTrader);
  if (!nn($helper)) unset($helper); // don't overwrite if too late or empty
//  $floor = -$creditLimit; // floor is calculated automatically
  $mya->update(@compact(ray('legalName federalId tickle notes dob helper mediaConx moves')));
 
  unset($DBTX);
  
  say('info saved');
}

function setRisksAndFlags($a, $risks, $adminable) {
  $risks0 = $a->risks;
  $a->setRisks($risks); // set or unset the adminable risk bits
  if ($a->risks != $risks0) { // if anything changed, refigure ALL risks (some affect others)
    include_once __DIR__ . '/../cg-risk.inc';
    k\acctRisk1($a->id, getv('riskThresholds', [0,0,0,0]));
  }
  $a->setBits(@$adminable);
}

/**
 * Activate the account.
 * @param int $helper: account record ID of account that invited this one
 */
function activate($helper) {
  global $mya;
  u\EXPECT($mya and $mya->cAdmin, 'should be ctty admin'); // only ctty admin can approve an account

  $DBTX = \db_transaction();
  u\EXPECT($helper, 'no helper!');
///  debug("helper=$mya->helper ok=$mya->ok");
  $mya->update(compact('helper')); // must precede welcome message
  $mya->setBit(B_OK, TRUE); // must precede welcome message
  $mya->setBit(B_NONUDGE, FALSE);
///  debug("helper=$mya->helper ok=$mya->ok");
  
  if ($firstTime = !$mya->activated) { // first time activation
//      u\EXPECT($mya->superAdmin, 'initial activation is only for superadmin');
    $mya->update('activated', now() - 1); // must precede update of vsecure, so it gets V-encrypted
    $mya->update('vsecure', $mya->vsecure); // hyper-encrypt
    $mya->eachPartner(function($coA, $zot, $custCode) { // tell partner: account is activated
      if ($api = $coA->api()) u\post($api, ray('partnerCode qid op name', $coA->emailCode, $mya->mainQid, 'complete', $mya->fullName));
    });
    $msg = 'approved';
//    if (!$mya->confirmed) $msg .= ($helper > 1 ? '|must confirm' : '|in person');
    if (!$mya->confirmed) $msg .= ($mya->helper == 1 ? '|must confirm uninvited' : '|must confirm');
    if (!$mya->co and !$mya->stepsDone('card')) $msg .= '|suggest card';
    $subs = ray('inviterName youName specifics', @$mya->helperA->fullName, $mya->fullName, getSpecifics($mya));
    
    r\message($mya->id, $msg, $subs);
    
  }

  if ($firstTime and !$mya->co) { // no bonus or helper bonus for a company signing up
    if ($mya->ssnData['CGerror'] and !$mya->risk('ssnOff') and !$mya->risk('dobOff')) $mya->setRisk('ssnOff'); // make SURE this bit gets set (might not if admin signs the person up

    if (!$mya->helperA->co and !$mya->helperA->superAdmin) { // no helper bonus for company referral or help by superAdmin (cAdmin is okay) [don't require $mya->confirmed]
      $fullName = $mya->fullName;
      $helped = db\count('users', ':IS_OK AND NOT :IS_CO AND helper=:hId', ray('hId', $mya->helper)) + 0;
      if ($helped == ORGANIZER_GRANT_MIN) {
//            r\message($mya->helper, 'organizer grant', ['_a' => w\atag('/community/grant')]);
        list ($name, $topic) = [$mya->helperA->fullName, t('eligible')];
        r\tellAdmin('eligible for grant', compact(ray('name topic')));
      }
    }
  }
  
  unset($DBTX); 
}

/**
 * Return the right specifics to include in the welcome email.
 */
function getSpecifics($mya) {
  $forOver1000 = t('This is a trial account. You will need to complete some additional steps to accept more than $1,000 through %PROJECT.');
  $onlineOnly = t('Your account is limited to payments online. ');
  $acceptPayments = $onlineOnly . t('To accept payments through %PROJECT you will need to complete some additional steps.');
  $finishCo = t(' <%a>Click here</a> to get started on that, by opening a personal account. Then sign back in to your company account and click "Finish" under Next Steps.', '_a', w\atag(BASE_URL . '/signup'));
  $cardComing = t('You may now make purchases through the Common Good system, including using your Common Good card at member businesses. Expect your Common Good card to arrive in the mail within a few days.');
  $getCard = t('To request a free %PROJECT card,');
  $getSell = t(' accept payments through %PROJECT,');
  $otherSteps = t(' or participate in the %PROJECT democracy, sign back in to your account and choose from the Next Steps listed there.');
  
  list ($cardWorthy, $canSell) = [$mya->stepsDone('card'), $mya->stepsDone('sell')];
  
  return str_replace(t('To or '), t('To '), $mya->co
  ? ($mya->trialco ? $forOver1000 . $finishCo : ($mya->stepsDone('co2') ? '' : ($acceptPayments . $finishCo)))
  : (($cardWorthy ? $cardComing . t(' To') : ($onlineOnly . $getCard)) . ($canSell ? '' : $getSell) . $otherSteps));
}

/**
 * Return a list of important next steps for the account.
 * @todo: add comment as an option?
 */
function choiceListFld() {
  global $mya;
  if ($mya->cAdmin) return NULL;
  
  $choices0 = explode("\n", t(<<< X
    community:invite:Invite:Invite a friend or company to join.<br><small>Your local Community Fund gets $2 for every invitation you send!</small>
    community:donate:Give:Make a donation to %PROJECT.
    community:comment:Comment:Send a comment, question, or suggestion.
    scraps:card:Card:Get a %PROJECT payment card
    scraps:sell:Sell:Accept payments (including discount rebates) through %PROJECT
    scraps:voice:Voice:Participate in %PROJECT economic democracy
    scraps:co2:Finish:Finish opening your complete company account &mdash; required before you receive a total of $1,000 or within six months, whichever is first
X
  ));
  foreach($choices0 as $one) {
    list ($dir, $task, $title, $text) = explode(':', trim($one));

    if ($dir == 'community') {
      $kch = "<div>$title</div><div>$text</div>";
    } else {
      $steps = $mya->stepCount($task);
      $kch = $steps ? tr("<div>$title</div><div>$text (%steps)</div>", 'steps', $steps . t(' steps')) : NULL;
    }
    ${$task . 'Ch'} = $kch ? ["$dir/$task" => $kch] : [];
  }
    
  $choices = $mya->co ? $co2Ch + $inviteCh + $donateCh : ($cardCh + $inviteCh + $sellCh + $voiceCh + $donateCh);
  $choiceList = t('<h3>Next Steps</h3>') . "\n";
  foreach ($choices as $url => $desc) if (!empty($desc)) $choiceList .= w\lnk("/$url", $desc) . "\n";
  return item($choiceList . "\n");
}  