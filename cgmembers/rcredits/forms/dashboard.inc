<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Admin as a;
use CG\Risk as k;

include_once __DIR__ . '/tx.inc'; // include all Pay and Charge functionality

/**
 * Show a dashboard for the current account (with preference for mobile).
 */
function formDashboard($form, &$sta, $args = '') {
  global $base_url;
  global $mya; $myid = $mya->id;
  extract(just('stay', $args, NULL));

  if ($mya->admin and !$mya->proSe and !$stay) return go('/sadmin/summary'); // admins get way more info
  
  $special = $endorse = $continue = NULL;
  
  if ($mya->ok and !$mya->co) {
    $special = item(t('|<%aCovid>COVID-19</a> | <%aBlm>BLM</a> | <%aOnN>Offers & Needs</a>', '_aCovid _aBlm _aOnN', 'a id="covid"', 'a id="blm"', 'a id="onn"'), '', '', clas('m-nolabs'));
    if (FALSE and !$mya->shouter and $mya->hasPhoto) { // (don't ask partial accounts to endorse)
      $endorseText = t('<h2>We need your help to grow.</h2>');
      $endorseBtn = btn('/community/invite', t('Endorse %PROJECT'), 'success', 'large');
      $skipLnk = lnk('', t('Skip for now'), clas('skipLnk'));
      $endorse = item("<div id=\"endorse\"><div><div>$endorseText<br>$endorseBtn<br>$skipLnk</div></div></div>", BARE);
    }
  }
  
  if ($mya->closed) {
    say(t('Your account is closed, but you can still see your account history.'));
  } else {
    if (nn($msg)) say($msg);

    if ($stepUrl = $mya->nextStepUrl('', $msg)) { // steps to be done
      if (!$mya->ok and r\referrer() == 'signin') return w\go($stepUrl); // go right to next step if not approved
      say(t('To continue opening your %PROJECT account, <%a>click here</a>.', '_a', atag("/$stepUrl")));
    } elseif ($mya->co and $mya->id != 1) {
        if (!$mya->ok) { // no steps to do, but not open
          r\tellAdmin('unapproved company!', ['qid' => $mya->mainQid]);
          say(tr('co tentative|pioneer thanks')); // UNUSED except in tests?
        }
//    } elseif ($mya->stepCount('card')) { // not eligible for card yet
//      if (!$mya->ok) say(tr('member tentative|join thanks'));
//      $continue = btn('/scraps/card', t('Get a Card'), 'success', 'lg');
    } elseif ($mya->stepCount('vote')) { // not eligible to vote yet
      if (!$mya->ok) say(tr('member tentative'));
      $continue = btn('/scraps/vote', t('Get a Vote'), 'success', 'lg');
    } elseif (!$mya->ok) say(tr('voter tentative|pioneer thanks'));
  }

  if ($mya->canManage and $mya->member) {
    $invite = lnk('/community/invite', t('Invite a Friend or Business'));
    $hasMultipleAccounts = ($mya->co or db\exists('u_relations r LEFT JOIN users u ON u.uid=r.main', 'other=:id AND permission>0 AND u.:IS_CO', ['id' => $mya->agentId]));
    $another = $hasMultipleAccounts ? t('Another') : t('a');
    $makeCo = $mya->depends ? '' : lnk('/signup-co/relate=1', t('Open %another Company Account', compact('another')));
//    if (!$mya->jid and !$mya->co) $makeJoint = lnk('/signup/joint=1', t('Make this a joint account'));
    $makeJoint = ($mya->member and !$mya->jid and !$mya->co and !$mya->depends) ? lnk('/prejoin', t('Make this a Joint Account')) : '';
    $donate = lnk('/community/donate', t('Donate'));
  } else $invite = $makeCo = $makeJoint = $donate = '';

  $reports = $mya->co ? lnk('/history/company-reports', t('Company Reports')) : '';
  $deposits = $mya->id == CGID ? lnk('/sadmin/show-reconciled', t('Dollar Pool Transactions')) : '';
  $reopen = $mya->closed ? lnk('/scraps/reopen', t('Ask for your account to be reopened')) : '';
  $comment = lnk('/help', t('Comment, question, or suggestion'));
  $card = $mya->ok ? lnk('', t('Print Your Card'), 'printCard') : NULL;
  
  if (r\cook('scanner')) { // only advance app links for now
    $showAdvanced = lnk('', t('Advanced Links'), clas('showAdvanced'));
    $frontCamera = lnk('/card/frontCamera', r\cook('frontCamera') ? t('Use BACK Camera') : t('Use FRONT camera'), clas('frontCamera'));
    $disconnect = lnk('/card/disconnect', t('Disconnect From CGPay App'), clas('disconnect'));
  } else $showAdvanced = $frontCamera = $disconnect = NULL;
  
  if ($mya->member and !$mya->closed) {
    foreach (ray('reopen invite makeJoint makeCo reports donate comment deposits showAdvanced frontCamera disconnect card') as $k) if ($$k) $shortcuts[] = $$k;
    $shortcuts = nn($shortcuts) ? item(u\ul($shortcuts), t('Shortcuts:')) : NULL;
  } else $shortcuts = NULL;

  $title = t('You: ' . $mya->bestName);
  $acct = $mya->mainQid;
  $desc = $mya->jid ? t(' (joined with %nm)', 'nm', $mya->jA->fullName) : '';
  $lnkShowQr = lnk('/show-qr', t('Show QR'));
  $lnkScanQr = lnk('/scan-qr', t('Scan QR')) . qBtn('qr');
  $acct = item($acct, t('Account:'), $desc, suffix(tr(' %DASH ') . "$lnkShowQr | $lnkScanQr"));

  if ($mya->ok) {
    $pay = btn('', t('Pay'), '', 'md', clas('btn-pay'));
    $charge = btn('', t('Charge'), '', 'md', clas('btn-charge'));
  } else { $pay = $charge = ''; }
  $bank = $mya->member ? btn('/get', t('Bank'), '', 'md') : '';
  $buttons = item($pay . $charge . $bank);

  $balance = u\fmtAmt($mya->balance);
  $balance .= t('| %DASH <%aHist>History</a> | <%aCred>Credits & Discounts</a>', '_aHist _aCred', atag('/history'), atag('/community/coupons/list/ALL'));
  $balance = item($balance, t('Balance:'));

  if (!$mya->isCtty) { // unless this is a community or region account
    $credit = $mya->depends ? '$0' : u\fmtAmt(-$mya->floor);
    $creditLine = $mya->underage ? NULL : item($credit, t('Cred Line:'), t('Based on your account\'s average monthly activity.'), creditLineSuffix());
  
//    $choiceList = ($mya->member or $mya->closed) ? choiceListFld() : NULL;
    
  } else {
    $acctType = t('community account');
    $creditLine = $choiceList = NULL;
  }
  
  jsx('dashboard', 'payDesc chargeDesc carded', t('Pay'), t('Charge'), $mya->carded);
  $dashboard = fieldSet('dashboard', compact(ray('endorse title acct buttons balance creditLine continue special shortcuts')), BARE);
  $tx = fieldSet('tx', formTx($form, $sta, 'both'), BARE);

  return cgform(compact('dashboard', 'tx'), '', 'left');
}

function formDashboard_validate($form, &$sta) {return formTx_validate($form, $sta);}
function formDashboard_submit($form, &$sta) {return formTx_submit($form, $sta);}

/**
 * Return a list of important next steps for the account.
 * @todo: add comment as an option?
 *//*
function choiceListFld() {
  global $mya;
  
  // <br><small>Your local Community Fund gets $2 for every invitation you send!</small>
  $cardText = $mya->canCarded
  ? t('Complete some steps you missed out on.') // don't mention card if they already have it
  : t('Get the card. Get to vote.<br><small>Get a %PROJECT payment card and vote on what to fund.</small>');
  
  $choices0 = explode("\n", t(<<< X
    community:invite:Invite:Invite a friend or company to join.
    community:donate:Give:Make a donation to %PROJECT.
    community:comment:Comment:Send a comment, question, or suggestion.
    scraps:card:Get Card:$cardText
    scraps:co:Finish:Finish opening your complete company account.
    scraps:reopen:Reopen:Ask for your account to be reopened.
X
  ));
    
  foreach($choices0 as $one) {
    list ($dir, $task, $title, $text) = explode(':', trim($one));

    if ($dir == 'community' or $task == 'reopen') {
      $kch = "<div>$title</div><div>$text</div>";
    } else {
      $steps = $mya->stepCount($task);
      if ($steps < 5) $text .= t(' (%steps steps)', compact('steps'));
      $kch = $steps ? tr("<div>$title</div><div>$text</div>") : NULL;
    }
    ${$task . 'Ch'} = $kch ? ["$dir/$task" => $kch] : [];
  }

  if ($mya->closed) {
    $choices = $reopenCh + $commentCh;
  } else {
    list ($task, $choices) = $mya->co ? ['co', $coCh] : ['card', $cardCh];
    if (!$mya->stepCount($task)) return NULL;
  
    $choices += $inviteCh + $donateCh + $commentCh;
  }
  
  $choiceList = '';
  foreach ($choices as $url => $desc) if (!empty($desc)) $choiceList .= lnk("/$url", $desc) . "\n";
  $choiceList = "<div id=\"edit-choiceList\">\n%hdr\n<div class=\"table\">$choiceList\n</div>\n</div>\n";
  return item(tr($choiceList, '_hdr', t('<h3>Next Steps</h3>')), BARE);
}  
*/
