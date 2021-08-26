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

  if ($mya->admin2 and !$mya->proSe and !$stay) return go('/sadmin/summary'); // admins get way more info
  
  $special = $endorse = NULL;
  
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
    } elseif (!$mya->ok) { // no steps to do, but not open
      if ($mya->co) {
        say(tr('tentative complete|co complete'));
      } elseif ($mya->stepCount('card')) { // not eligible for card yet
        say(tr('tentative partial|want a card|get a card below'));
      } else { say(tr('tentative complete|card soon')); }
    }
  }

  if ($mya->canManage and $mya->member) {
    $invite = w\lnk('/community/invite', t('Invite a Friend or Business'), 'primary', 'xs');
    $hasMultipleAccounts = ($mya->co or db\exists('r_relations r LEFT JOIN users u ON u.uid=r.main', 'other=:id AND permission>0 AND u.:IS_CO', ['id' => $mya->agentId]));
    $another = $hasMultipleAccounts ? t('Another') : t('a');
    $makeCo = $mya->depends ? '' : w\lnk('/signup-co/relate=1', t('Open %another Company Account', compact('another')), 'primary', 'xs');
//    if (!$mya->jid and !$mya->co) $makeJoint = w\lnk('/signup/joint=1', t('Make this a joint account'), 'primary', 'xs');
    $makeJoint = ($mya->member and !$mya->jid and !$mya->co and !$mya->depends) ? w\lnk('/prejoin', t('Make this a Joint Account'), 'primary', 'xs') : '';
  } else $invite = $makeCo = $makeJoint = '';

  $reports = $mya->co ? w\lnk('/history/company-reports', t('Company Reports')) : '';
  $deposits = $mya->id == CGID ? w\lnk('/sadmin/show-reconciled', t('Dollar Pool Transactions')) : '';
  
  if ($mya->member and !$mya->closed) {
    foreach (ray('invite makeJoint makeCo reports deposits') as $k) if ($$k) $shortcuts[] = $$k;
    $shortcuts = nn($shortcuts) ? item(u\ul($shortcuts), t('Shortcuts:')) : NULL;
  } else $shortcuts = NULL;

  $title = t('You: ' . $mya->fullName);
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
    $suffix = ($mya->debt or $mya->depends or !$mya->refill) ? '' : t(' (<%a>Click here</a> to let your balance go negative.)', '_a', 'a id="activate-credit"');
    $credit = $mya->depends ? '$0' : u\fmtAmt(-$mya->floor);
    $creditLine = $mya->underage ? NULL : item($credit, t('Cred Line:'), t('Based normally on your account\'s average monthly activity over the past 6 months.'), w\suffix($suffix));
  
    $choiceList = ($mya->member or $mya->closed) ? choiceListFld() : NULL;
    
  } else {
    $acctType = t('community account');
    $creditLine = $choiceList = NULL;
  }
  
  jsx('dashboard', 'payDesc chargeDesc', t('Pay'), t('Charge'));
  $dashboard = fieldSet('dashboard', compact(ray('endorse title acct buttons balance creditLine special shortcuts choiceList')), BARE);
  $tx = fieldSet('tx', formTx($form, $sta, 'both'), BARE);

  return cgform(compact('dashboard', 'tx'), '', 'left');
}

function formDashboard_validate($form, &$sta) {return formTx_validate($form, $sta);}
function formDashboard_submit($form, &$sta) {return formTx_submit($form, $sta);}

/**
 * Return a list of important next steps for the account.
 * @todo: add comment as an option?
 */
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
  foreach ($choices as $url => $desc) if (!empty($desc)) $choiceList .= w\lnk("/$url", $desc) . "\n";
  $choiceList = "<div id=\"edit-choiceList\">\n%hdr\n<div class=\"table\">$choiceList\n</div>\n</div>\n";
  return item(tr($choiceList, '_hdr', t('<h3>Next Steps</h3>')), BARE);
}  
