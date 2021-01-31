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

  $stepUrl = $mya->nextStepUrl('', $msg);
  $extras = '<br>' . u\fmtPhone($mya->phone) . "<br><span id=\"acctEmail\">$mya->email</span> "
  . btn('', 'c', 'default', '', clas('copyEmail'));

  $form = adminSummary();
  foreach (ray('legalName shortName community') as $k) if (nni($form, $k)) { // move these up (see return line)
    $$k = $form[$k];
    unset($form[$k]);
  }

  $buttons = btn('/console/stay=1', t('Simple Console'));
  if (!$mya->isCtty and $mya->cAdmin) {
    if ($mya->trialco or !$mya->co) $buttons .= ' ' . a\cardLink($mya->id);
    $buttons .= ' ' . a\photoLink(0) . ' ' . a\photoLink(1);
  }

  if ($stepUrl == 'console') {
    if (nn($msg)) say(t('New member is told: ') . t($msg));
  } elseif ($stepUrl) say(t('New member will be sent <%a>there</a> upon signing in.', '_a', atag("/$stepUrl")));
    
  $gifts = u\fmtAmt($mya->totalGifts());

  $reports = $mya->co ? w\lnk('/history/company-reports', t('Company Reports')) : '';
//  if ($mya->id == CGID) $deposits = w\lnk('/sadmin/show-deposits', t('Deposits'));
  $deposits = $mya->id == CGID ? w\lnk('/sadmin/show-reconciled', t('Dollar Pool Transactions')) : '';

  $title = item(t('Account Summary'), '', suffix($buttons));
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
    $creditLine = $mya->underage ? NULL : item($credit, t('Credit Line:'), t('Based%normally on your account\'s average monthly activity over the past 6 months.', compact('normally')), w\suffix($suffix));
//    $committed = item(u\fmtAmt(max(0, $mya->committed)), t('Committed:'), t('reserved for donation to Common Good Finance'));

  } else $acctType = t('community account');

  if ($mya->cAdmin) {
    $extra = " id#$mya->id";
    if (FALSE and $bankInfo = $mya->bankInfo) {
      extract($bankInfo, EXTR_PREFIX_ALL, 'b');
      $bankInfo = "$b_name (routing #$b_route)\n$b_address, $b_city, $b_state $b_zip";
      $extra .= ' ' . w\popHelp(t('bank info'), $bankInfo, '#'); //strtr(t(' <@a>bank info</a>'), ['@a'=>$bankInfo]); // strtr circumvents Drupal crap
    }
  } else $extra = '';
  $accountId = item(($mya->mainQid ?: 'N/A') . '<span class="white">..</span>' . "($acctType)$extra", t('ID:')); // .. is for double-click select

  $fields0 = 'photo title clickPhoto buttons accountId accountName address legalName shortName community balance newBalance creditLine creditLimit committed';
//  foreach (ray('details') as $k) $$k = @$form[$k]; unset($form[$k]); // put these on the left if admin
//  $fields = @compact(ray($fields));
  $fields = []; foreach (ray($fields0) as $k) if (isset($$k)) $fields[$k] = $$k;
  
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
  
  return cgform($form, 'wide');

}

function formSummary_validate($form, &$sta) {
  global $mya;

  if (in(op($sta), 'openCompany openJoint')) return;
  extract(just('uid legalName creditLimit dob altId altSt active helper moves tickle adminable', $sta['input'], NULL));
  if ($tickle !== '' and $err = u\badDate($tickle)) return say($err, 'tickle');

  if ($dob and !formBasic_validate($form, $sta, FALSE, 'dob')) return;
  if ($altId xor $altSt) return say(t('You must give both an Alternate ID and a state or neither. If it\'s not a driver\'s license, describe the ID type in one or two words at the start of the number; for example "US Passport #12345". If there is no state, choose "AA".'), 'altId');
  
  if (nn($helper)) {
    if (!$h = whois($helper, 'helper', $sta['input'])) return say(t('I don\'t know what helper account you mean.'), 'helper');
    $helper = $h->id;
  } else {unset($helper); unset($sta['input']['helper']);}

  if (nn($active) and !$mya->ok and !$mya->isCtty) { // activating
    if (!nn($helper)) return say(t('You must specify who gets the helper reward.'), 'helper');
//    if (!@$helper or $helper == 1) return say(t('You must specify who gets the helper reward.'), 'helper');
    if (!$mya->member and is_null(nni($adminable, B_MEMBER, NULL))) return say(t('That account is not READY (not a member).'), 'adminable'); // test "is_null" because 0 is the value when set
    if (!$mya->confirmed and $mya->helper != 1) say(t('NOTE: Member %helperName has not yet confirmed this invitation.', 'helperName', $h->fullName));
    if (!$mya->co and !nn($dob) and !$mya->hasDob and !$mya->idVerifiedBy) return say(t('You cannot activate the account without a date of birth (except for solar1 signups).'), 'dob');
  }

  if (!nn($active) and $mya->ok) { // de-activating
    if ($mya->balance + 0) return say(t('You cannot de-activate an account with a non-zero balance.'), 'err');
  }
  if (isset($creditLimit) and $err = u\badAmount($creditLimit)) return sayFieldErr($err, 'creditLimit'); 
  if (isset($moves) and $err = u\badAmount($moves, '>=0', 0)) return sayFieldErr($err, 'moves');
  foreach (ray('tickle helper creditLimit') as $k) if (isset($$k)) $sta['input'][$k] = $$k;
}

function formSummary_submit($form, &$sta) {
  global $mya;
  $op = op($sta);

  if (!$mya->admin2) return; // hack attempt
  if ($op == 'secureMsgs') return w\go('', showSecureMessages());

  extract($args = just('uid who legalName creditLimit tickle note notes dob altId altSt helper moves active risks adminable', $sta['input'], NULL));
  if (!$mya->cAdmin) u\EXPECT(!array_diff(array_keys($args), ray('uid tickle note')), 'too many unpermitted changing fields in ' . pr($args));

  if (!$tickle) {
    $tickleDone = $mya->ok ? ($note and $mya->tickle) : 'activate';
    list ($zot, $tickle) = $mya->nextTickle($tickleDone);
  }

  u\setDft($notes, $mya->notes);
  if ($note) $notes = r\stampNote($note) . $notes;

  $DBTX = \db_transaction();

  if ($dob) $mya->update(compact('dob')); // must precede activate()
  if ($altId) {
    $mya->update(ray('altId', $altSt . $altId)); // must precede activate()
    $mya->stepDone('verifyid');
  }

  if ($mya->cAdmin or $mya->admin) setRisksAndFlags($mya, nn($risks), nn($adminable)); // don't test for isset (allow turning off all bits); must precede activate() because that function sets ssnOff bit
    
  if (!$mya->isCtty) {
    $fullName = $mya->fullName;
    if (FALSE and !$mya->confirmed and nn($helper) and nn($helper) != $mya->helper) { // self-signup
      $helperA = r\acct($helper);
      $_a1 = $helperA->doLink('confirmInvite', $mya->id); // link to confirm invitedness
      $helperA->tell('confirm invite', compact('_a1', 'fullName')); // ask inviter to confirm
    }
  }

  if (isset($active)) {
    $active = (bool) $active; // convert from string
    if ($active xor $mya->ok) { // changing activation status
      $mya->setBit(B_OK, $active);
      if ($active) {
        include_once R_ROOT . '/cg-activate.inc';
        if ($mya->activated) $notes = r\stampNote('reactivated') . $notes;
        r\activate($mya, $helper);
      } else { $notes = r\stampNote('account closed') . $notes; }
    }
  }
  
  $fields = 'notes';
  foreach (ray('legalName tickle moves helper') as $k) if ($$k) $fields .= " $k"; // don't overwrite
  $info = []; foreach (ray($fields) as $k) if (isset($$k)) $info[$k] = $$k;
  $mya->update($info);
  unset($DBTX);
  
  say('info saved');
  if ($mya->superAdmin and strhas($mya->notes, 'ZAP')) return go('/sadmin/followup');
}

function setRisksAndFlags($a, $risks, $adminable) {
  $risks0 = $a->risks;
  $a->setRisks($risks); // set or unset the adminable risk bits
  if ($a->risks != $risks0) { // if anything changed, refigure ALL risks (some affect others)
    include_once __DIR__ . '/../cg-risk.inc';
    k\acctRisk1($a->id, getv('riskThresholds', [0,0,0,0]));
  }
  $a->setBits(nn($adminable));
}

/**
 * Return Summary fields exclusively for community admins (admin, admin2, cAdmin, cAdmin2).
 */
function adminSummary() {
  global $mya, $base_url;
  include_once __DIR__ . '/../admin/admin.inc';  

  $uid = $mya->id;
  $ctty = $mya->community ?: r\serverUid(); // serverUid for admin, if it has no ctty
  if ($mya->proSe and !$mya->superAdmin) return []; // nothing special on admin's own account
  
  $noEnter = TRUE; // prevent submission by pressing Enter
  
  if ($others = db\lst('fullName', 'sessions s JOIN users u ON u.uid=s.uid', ':uid=IFNULL(s.acct, s.uid) AND s.uid<>:agentId', ray('uid agentId', $uid, $mya->agentId))) {
    w\say(t('<b class="loud">Other people</b> signed in to this account: %others', compact('others')));
  }

  if ($mya->isCtty) { // managing a community itself (omit a lot of fields)

  } else { // managing some individual account
    if (!$mya->ok) $mya->nextStep();

    if ($mya->admin and !$mya->proSe) {
      $community = r\cttys($mya->region, $ctty);
      jsx('change-ctty', compact('uid'));
    }

    if ($mya->cAdmin and !$mya->proSe) {
      require_once R_ROOT . '/admin/admin.inc';
/*      $html = $mya->photoHtml(FALSE, TRUE);
      $photo = item("<a href=\"$base_url/settings/photo\">$html</a>");
      */

      $donations = item($mya->giftsDesc() ?: t('none'), t('Donations:'));
      
//      $creditLimit = numFld(t('Credit limit:'), '', dft(u\fmtAmt(-$mya->floor)));
//      if ($mya->stepsDone['prefs']) { // don't even look at these if newbie hasn't chosen yet
//        $minimum = numFld(t('Minimum:'), '', dft(u\fmtAmt($mya->minimum)));
//        $achMin = numFld(t('ACH Min:'), '', dft(u\fmtAmt($mya->achMin ?: R_ACHMIN_DEFAULT, 's')));
//      }
      $minimum = item(u\fmtAmt($mya->minimum), t('Target Bal:'));
      $achMin = item(u\fmtAmt($mya->achMin), t('ACH Min:'));
      if (!$mya->co) $trust = item($mya->trust, t('Trust:'), t('This person\'s trust rating'));
      $invitedBy = $mya->helper ? ($mya->helperA->fullName . "#$mya->iCode (" . r\iCodeDesc($mya->iCode) . ') + ') : '';
      $invitedBy = item($invitedBy . db\lst('fullName', 'r_invites i LEFT JOIN users u ON u.uid=i.inviter', 'invitee=:uid', compact('uid')), t('Invited by:'));
      $proxies = item(db\lst('fullName', 'r_proxies p LEFT JOIN users u ON u.uid=p.proxy', 'p.person=:uid ORDER BY priority', compact('uid')), t('Proxies:'));
      $proxyFor = item(db\count('r_proxies', 'proxy=:uid', compact('uid')), t('Proxy for:'));
      $risk = item(round($mya->risk) . ' ' . button(t('Recalc'), "$base_url/rerisk/acct/$uid/1"), t('Risk Score:'));
      if (!$mya->co) $trust = item(number_format($mya->trust, 1), t('Trust:'));

      if (!$mya->ok or $mya->helper == 1) { // don't show helper field if it's moot (too late -- somebody already got credit)
        if ($helper = $mya->helper ?: ($mya->co ? $mya->agentId : db\get('inviter', 'r_invites', 'invitee=:uid', compact('uid')))) {
          $hA = r\acct($helper);
          $helper = "$hA->mainQid: $hA->fullName";
        }
        $helper = textFld(t('Helped by:'), t('Who gets the "inviter/helper" reward? Must be exactly one account.'), dft($helper)); //  + autocomplete()
      } else $helper = item($mya->helper ? $mya->helperA->fullName: t('N/A'), t('Helped by:'));
    }
  
    $activity = item(a\showActivity($mya), t('Activity:'));
    $tasks = item(a\showTasks($mya), t('Tasks:'));
    $steps = item(a\showSteps($mya), t('Steps Done:'), t('|fund=connect bank account | work=identify employer/calling | company=company info | discount=offer a discount | backing=support your community\'s %PROJECT economy | stepup=support local nonprofits'));
    if ($rels = a\showRelations($mya)) $relations = item($rels, t('Relations:'));  
  }
  
  if ($mya->cAdmin and !$mya->proSe) { // either a community or regular account (but not the cAdmin's own account)
    include __DIR__ . '/../rweb/risk-descs.inc';
    include __DIR__ . '/../rweb/bit-descs.inc';

    $legalName = textFld(t('Legal Name:'), '', required($mya->legalName));    
    $shortName = textFld(t('Short Name:'), t('Shortest usable version of full name.'), dft($mya->shortName));
    
//    if ($mya->superAdmin or ($mya->cAdmin and $mya->activated)) $active = ($mya->isCtty)
    $active = ($mya->superAdmin or ($mya->cAdmin and $mya->ok)) // allowing all admins to activate causes a mess
    ? boolFld(t('Active?'), t('Is account approved to use %PROJECT?'), $mya->ok)
    : item($mya->ok ? t('Yes') : t('No'), t('Active:'));

    if ($mya->member and !$mya->co and $mya->superAdmin) $ssnShow = item(a\ssnShow($mya));
/*    if ($mya->admin) {
      $dba = ($mya->co and $owner = r\relation('other', 'main=:uid', compact('uid')) and r\acct($owner)->federalId == $mya->federalId);
      $ssnDft = $mya->federalId ? ($dba ? t('SSN') : R_ON_FILE) : '';
      $federalId = $mya->superAdmin ? textFld(t('EIN/SSN:'), '', $ssnDft) : item($ssnDft);
    } */
  
    $acctRisks = ray(K_ACCT_RISKS);
    $riskBits = array_keys($acctRisks);
    $risksEditable = array_flip(ray(K_ACCT_RISKS_EDITABLE));
    foreach ($riskBits as $k) {
      $class = $acctRisks[$k] < 0 ? 'goodRisk' : 'badRisk';
      if (!isset($risksEditable[$k])) $class .= ' noEdit';
      $dpy = w\popHelp($k, nni($riskHelp, $k), '#');
      $aliases[$k] = "<div class=\"$class\">$dpy</div>";
    }
    $risks = bitsFld(t('Risks:'), $mya->risks, '', K_ACCT_RISKS, $aliases);
    if ($mya->isCtty) $flagHelp = just(B_CTTY_BITS, $flagHelp);

    $adminable = $mya->adminables();

    foreach (ray($mya->isCtty ? B_CTTY_BITS : B_LIST) as $k) if (!preg_match('/u[0-9]+/', $k)) { // ignore undefined bits
      $blist[] = $k0 = $k;
      $class = in($k, $adminable) ? '' : 'noEdit';
      if ($mya->isCtty) $k = strtr($k, ray(B_ADMIN_BITS));
      $dpy = w\popHelp($k, $flagHelp[$k0], '#');
      $aliases[$k0] = "<div class=\"$class\">$dpy</div>";
    }
    $adminable = bitsFld(t('Bits:'), $mya->flags, 'b', join(' ', $blist), $aliases);
    $calling = item($mya->calling, t('Calling:'));
    $tenure = item($mya->tenure . t(' month(s)'), t('Tenure:'), t('How long had this person or company been at this address before opening a %PROJECT Account?'));

    if (!$mya->co) {
      $moves = item($mya->moves === '' ? '?' : $mya->moves, t('Moves:'), t('(How many times has this person changed counties in the past decade?)'));
    }      

    if ($mya->id == CGID) {
      $lines = '<tr><th>Community</th><th>Donations</th><th>Paid</th><th>Bounce Fees</th><th>Due</th></tr>';
      foreach (r\cttyGifts(now()) as $row) {
        extract((array) $row);
        $bounces = R_DEPOSIT_RETURN_FEE * $bounces;
        $due = round($in / 2 - $out, 2);
        foreach (ray('in out bounces due') as $k) $$k = number_format($$k, 2);
        $ctty = r\acct($ctty)->fullName;
        $lines .= "<tr><td>$ctty</td><td>$in</td><td>$out</td><td>$bounces</td><td>$due</td></tr>\n";
      }
      $cgfDonations = item("<table id=\"cgf-donations\">$lines</table>");
    }
//    $mediaConx = textFld(t('Media Conx:'), t('(How many 1st or 2nd-degree connections do you have with this person on FaceBook or LinkedIn?)'), dft($mya->mediaConx ?: 0));
  }

  if (!$mya->proSe) {
    list ($tickleType, $zot) = $mya->nextTickle();
    $details = item(callDetails($tickleType, $mya->tickle));
//    $preNote = item($tip);

    if ($mya->superAdmin) {
      $notes = areaFld(t('Notes:'), '', dft(unplain($mya->notes))); // only needed for default in areaFld
    } else $notes = item(str_replace("\n", '<br>', $mya->notes), t('Notes:'));

    $note = textFld(t('New Note:'), [t('Type a Note Here'), t('Abbreviations: PM=phone msg EM=email msg. Be sure to say what you left a message <i>about</i> (for example "PM setup")')]);
    $dob = $altSt = $altId = NULL;

    if (!$mya->co) {
      if ($mya->superAdmin or !$mya->hasId) {
        $dob = $mya->hasDob ? item(u\fmtDate($mya->dob, TRUE), BARE) : dateFld(BARE, '', autofill('off') + clas('noCalendar'));
        $dob = rendA('dob', $dob);
      }
      if ($mya->hasFid) {
        if ($dob) $altId = item($dob, t('DOB:'), '', clas('loneDob')); // dob is not shown directly, so show it here if no license is shown
      } elseif ($mya->superAdmin) { // altId is V-encrypted
        $altSt = rendA('altSt', stFld(substr($mya->altId, 0, 2)));
        $altId = textFld(t('Alt ID / DOB:'), [t('License# etc.')], dft(substr($mya->altId, 2)) + suffix($altSt . $dob));
      }
    }
    
    $suffix = submitTickle(1, t('Wait a day')) . submitTickle(2, t('2 days')) . submitTickle(7, '7') . submitTickle(30, '30') . submitTickle('', t('Auto')) . ' &nbsp; ' . submitTickle($mya->tickle ? 'NONE' : '', t('Save'), 'sm');
    $tickle = dateFld(t('Tickle:'), [t('Followup Date'), t('When to follow up ("Auto" sets an automatic Welcome or Annual date)')], dft($mya->tickle ?: '') + w\suffix($suffix) + clas('noCalendar'));
    $source = item($mya->source, t('Source:'));
  }

//  $back = item('<a href="' . $base_url . '/sadmin/member-list">Go to Member List</a>', ' ');

  $uid = hidFld($uid); // must postcede js (which uses uid)
  w\jsx('tickle');
  
  $bigFields = $mya->ok ? 'adminable risks ssnShow' : 'ssnShow adminable risks';
  $info = []; foreach (ray("noEnter details notes note altId tickle donations active activity relations source helper $bigFields community balance cgfDonations legalName shortName calling signupCo tenure tasks steps dwInfo invited invitedBy proxies proxyFor moves floor minimum achMin risk trust mediaConx count back uid submit") as $k) if (isset($$k)) $info[$k] = $$k;
  return $info;
}

/**
 * Return a button for setting the tickle field and submitting the form.
 * @param mixed $v: what to set tickle to -- empty or number of days hence or "NONE" (meaning don't set the tickle)
 * @param string $text: what the button says
 * @param string $size: size of button (defaults to sm)
 */
function submitTickle($v, $text, $size = 'xs') {
  if ($v == 'NONE') return w\rent(w\submi($text, 'primary', $size)); // not change to tickler
  if ($v) $v = date('Y-m-d', strtotime("+$v days"));
  return w\rent(w\submi($text, 'primary', $size, ray('class tickle', 'tickle', $v) + notab()));
}

/**
 * Return a detailed script for calls to account-holders.
 * @param string $type: annual, welcome, setup, pre, 
 * @return a short description and details if suggested date is within a month +/- of standard welcome or annual call
 */
function callDetails($type = '', $when = '') {
  global $mya;
  
  $now = now();
  $keys = ray('nocall pre setup activate welcome annual');
  $heads = ray(t('No call,Just beginning,Setup,Activate,Welcome,Annual'));
  $heads = array_combine($keys, $heads);
  $head = $heads[$type ?: 'nocall'];
  $setup = in($type, 'pre setup');
  $late = round(($now - $when) / DAY_SECS);
  $subtext = $now < $when ? t('in %days days', 'days', -$late) : t('%late days overdue', compact('late'));
  $subtext = !$when ? '' : "<br><span>$subtext</span>";
  
  if (!$mya->member) $head .= $setup ? '<p>Do you need help?</p>' : '';

  $subs = ray('id', $mya->id);
  $gifts = ($mya->giftsDesc() ?: $mya->totalGifts())
	?: db\get('amount', 'tx_templates t', 'action=:ACT_PAY AND t.from=:id AND t.to=:CGID AND end IS NULL', $subs); // a promised gift counts here, if none yet
        
  $welcome = t('<li><b>Welcome</b> to %PROJECT. Thanks for participating!</li>');
  if (!$mya->co and $myCo = $mya->firstCo()) {
    list ($signupCo, $myCo) = ['', t(' (personally rather than for company "%myCo")', compact('myCo'))];
  } else $signupCo = $mya->signupCo ? t('<li><b>Company</b>. How about signing up your company: %coName? (%co)</li>', 'co coName', w\signupCoDisplay($mya->signupCo, $coName), $coName) : '';
  $notyet = t("<li><b>Want</b>. What's a place you would like to use your %PROJECT card but can't yet?</li>");
  $how = t("<li><b>How's it going?</b> %myCo %lastTx Do you have any questions, suggestions, or concerns? Are you getting our newsletter? What do you find most valuable about the %PROJECT system? If you could change or improve one thing about the %PROJECT system, what would it be?</li>", '_lastTx myCo', $mya->lastTxDesc(), nn($myCo)) . $notyet;
  if ($mya->paper) $how .= t('<li><b>Paper statements</b>. Do you really want paper statements? ($%R_STATEMENT_COST/month)</li>');
  if ($mya->balance < 0) $how .= t('<li><b>Negative</b>. You have a negative balance of <%b>%bal</b>. Do you need help getting money into your account? That amount is, in effect, borrowed from your local community of %PROJECT members. So it is much better of course, for the community\'s financial health, if you bring your balance back up above zero when you can.</li>', '_b bal', 'b class="outgoing"', u\fmtAmt($mya->balance));
  
  $reason = $setup ? t('see if I might help you finish opening your %PROJECT account')
    : ($type == 'welcome' ? t('welcome you to %PROJECT, check in, make sure you got your card, and ask you some questions')
       : ($type == 'annual' ? t('thank you for being a member, make sure we have your address right, and ask some questions')
          : t('ERROR')));
  $intro = t('Hi, this is %name with %PROJECT. I\'m calling to %reason.', 'name reason', $mya->agentA->fullName, $reason);
  $live = $intro . t(' Is this a good time?');
  $callback = t('Please give me a call back when you can at %phone', ['phone' => u\fmtPhone($mya->agentA->phone, '-')]);
  $msg1 = "$intro $callback" . t(' &mdash; or I\'ll try again in a day or two.');
  $msg2 = "$intro $callback" . t(' &mdash; I\'ll also send you an email.');
  $email = t('<p>Dear %name,</p><p>Following up on my phone message, I\'m writing to %reason. %callback &mdash; or reply to this email.</p>', 'name email reason callback', $mya->fullName, $mya->email, $reason, $callback);
  if ($setup) {
    if ($info = db\get('reid,code', 'r_relations', 'other=:myid AND :IS_CUSTOMER', ['myid' => $mya->id])) {
      $_how = t('using <%a>this link</a>', '_a', w\atag(tr('%BASE_URL/signup/reid=%reid&customer=%code', $info)));
    } else $_how = t('by signing in at %CG_DOMAIN with account ID "%acct" (then click "Forgot password?" if you don\'t remember your password)', 'acct', $mya->mainQid);
    $email .= t('<p>Or you can complete the account on your own, %how. But please feel free to ask for help if you run into any problems at all.</p>', compact('_how'));
  } else $email .= t('<p>Here are the questions and information we have for you:</p>');
  
  if ($mya->co) { // company
    $volunteer = '';
    $coAsk = ($mya->crumbs or TRUE) ? '' : t('<li><b>Crumbs</b>. We particularly encourage businesses to contribute what we call "crumbs" to support the system and the common good. Do you know about this option? It means contributing a small percentage of each payment received, instead of the system charging you credit card fees. What percentage would you like to contribute? [one percent is a common choice, a fraction of a percent is also wonderful]</li>'); // "or TRUE" is temporary 201801
    $coAsk .= t('<li><b>Coupons</b>. In addition to providing <i>free</i> advertising, %PROJECT can bring new customers to you with coupons. We give each new member a coupon to a %PROJECT business. Would you like to be in that rotation? (yes: we\'ll call you back about that)</li>');
    $coAsk .= t('<li><b>CC Fees</b>. Since credit card companies typically charge two or three percent plus a per-transaction fee, our board has proposed charging business members a small percentage on each payment received, to make the system sustainable sooner. Do you think this is a good idea and if so what percentage would be reasonable? What rate do you currently pay for credit card transactions? Would you want to <i>donate</i> a small percentage instead? If so, how much? [one percent is a common choice, a fraction of a percent is also wonderful]</li>'); // temporary 201801
    $options = db\exists('r_usd', 'payee=:myid AND amount<0 AND created>:now-:DAY_SECS*30', ray('myid now', $mya->id, $now)) ? t('Do you know about our automatic cash-out option? If you want your company account to transfer your surplus funds to your bank account automatically every week, we can set that up. ') : '';
    $options .= t('If you have questions about our "Pay With %PROJECT" button for your website, our invoicing system, or accounting statements, we can help you with any of those.');
  } else { // individual
    $_volunteer = join("</li>\n<li>", explode('~', t(
      'Hand out promo cards and schedule appointments with local businesses.'
      . '~Help new members open their %PROJECT account.'
      . '~Call new members to welcome them and see if they have questions.'
      . '~Assemble promotional materials (at home or with other volunteers).'
      . '~Deliver things.'
      . '~Print and mail things.'
      . '~Take photographs.'
      . '~Help set up for meetings.'
      . '~Help add new features to the software.'
      . '~Serve on the board of directors.'
    )));
    $_volunteerLink = t('<%a>volunteer</a>', '_a', 'a href="#volunteer" data-toggle="collapse"');
    $volunteer = t('<li><b>Volunteer</b>. (if supportive) Would you like to %volunteerLink with %PROJECT?<ul id="volunteer" class="collapse"><li>%volunteer</li></ul></li>', compact(ray('_volunteerLink _volunteer')));
    $coAsk = '';
    $options = t('Do you know about our joint account option? Sign in at %CG_DOMAIN and visit the Relations page on the Settings menu, to join an existing account with yours. Or to open a new account joined to yours, click "Make this a Joint Account" on your Summary page.');
    if (!$mya->roundup) $options .= t(' And do you know about the roundups option? Roundups let you always pay a whole dollar amount and donate the change to the community fund. Choose it on the Preferences page on the Settings menu. Or just ask and we\'ll set it for you.');
  }
  $giftDetails = t('(Wait for answer, then ask:) What amount would you like to give and do you prefer yearly, quarterly, or monthly? Suggested amounts are $50 for individuals and $250 for companies, but any amount is wonderful.');
  $ask = t('<li><b>Offer</b>. The idea of %PROJECT is to empower us to take responsibility for our local economy, so most of our members contribute some small amount periodically and/or choose roundups or crumbs, to support the system. Would you like to contribute at this time? %details</li>', 'details', $giftDetails) . $coAsk;
  $thx = t('<li><b>Donations</b>. Thank you very much %gifts.</li>', '_gifts', giftsToDate());
  $reask = nn($often) ? $thx : $thx . t('<li><b>Gift</b>. Would you like to %contribute at this time? %details</li>', 'contribute details', $type == 'annual' ? t('make an annual contribution') : t('contribute'), $giftDetails);
  if ($options) $options = t('<li><b>Features</b>. %options</li>', compact('options'));
  $options .= t('<li><b>Tell people!</b> Do you need more handout cards or brochures, to tell %who about %PROJECT?', 'who', $mya->co ? t('your customers and suppliers') : t('your friends or local businesses'));
  $addressCheck = t('<li><b>Address</b>. Are you still at %addr? (if not, ask about postal address too)</li>', 'addr', $mya->location(TRUE));

  if ($mya->co) $ask = $reask = $coAsk; // temporary 201801
  
  $yearsIn = max(1, round(($now - $mya->activated) / DAY_SECS / 365.25)); // how many years after activation are we
  $annualAsk = '<ol><li>' . t("<b>Thanks</b>. Thank you for being a pioneer of economic democracy! (member for %yearsIn year/s) Thanks to your participation, along with other members, we were able to issue $10,000 in grants and loans this year. (Were you in on the discussions of what to fund?)", compact('yearsIn')) . "</li>\n$how\n$ask\n$addressCheck\n$signupCo\n$volunteer\n$options\n</ol>";
  
  $res = '';
  if ($type == 'annual') $res = nn($gifts) ? str_replace($ask, $reask, $annualAsk) : $annualAsk;
  if ($type == 'welcome') $res = nn($gifts) ? "<ol>$welcome\n$thx\n$how\n$signupCo\n$volunteer\n</ol>" : "<ol>$welcome\n$how\n$ask\n$volunteer\n</ol>";
  $email .= $res . t('<p>I look forward to hearing from you!</p><p>Sincerely,</p><p>%name<br>for the %PROJECT Team</p>', 'name', $mya->agentA->shortName);
  $live .= $res . t('<p>Thank you very much for your time and for being a %PROJECT member. Have a great day.</p>');
  if (nn($_volunteerLink)) $email = str_replace($_volunteerLink, t('volunteer'), $email);
  $email = str_replace('id="volunteer"', '', $email);  // make sure the div isn't duplicated (causing expansion to fail)
  $email = preg_replace('/\([^)]+\)/', '', $email);
  
  $links = $details = '';
  foreach (ray('msg1 msg2 email live') as $k) {
    $details .= tr('<div id="%k-details" class="collapse">%detail</div>', 'k _detail', $k, $$k);
    $links .= tr(' <a id="%k-link" href="#%k-details" data-toggle="collapse" class="btn btn-xs btn-warning">%k</a>', 'k', $k);
  }

  $email = str_replace('href="#email-details" data-toggle="collapse"', '', $email); // href=\"javascript:followupEmail();\"
  $email .= '<' . u\emailTag($mya->email, t('your %PROJECT account'), '', $mya->fullName, 'id="email-do" class="collapse"') . '></a>';
  
  w\jsx('followup-email');

  return "<div class=\"well scripts\"><h3 class=\"above-details\">$head$subtext</h3><div id=\"intros\"><b class=\"loud\">$mya->phoneDash</b>$links</div>$details</div>";
}

/**
 * Return an eloquent statement of the member's gifts to date, to follow "Thank you ".
 */
function giftsToDate() {
  global $mya;

  if (!$mya) return '';
  $mya->giftsDesc($often, $all);
  if (!$all) return '';
  list ($total, $x1, $x2) = explode('+', "$all++");
  if ($x = u\unite(trim($x1), $x2)) $x = t('for choosing %x', '_x', $x);
  if ($total) $total = t('for your contributions, totaling ') . $total;
  return u\unite($total, $x, ', ');
}

function stFld($dft) {return selectFld(BARE, t(''), dft($dft), ['' => t('state')] + db\keyed('abbreviation,abbreviation', 'r_states', 'country_id IN (:US_COUNTRY_ID, :CA_COUNTRY_ID) ORDER BY abbreviation'));}
