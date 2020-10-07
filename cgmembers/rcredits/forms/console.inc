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
 * Show a console for the current account (with preference for mobile).
 */
function formConsole($form, &$sta) {
  global $base_url;
  global $mya; $myid = $mya->id;

  if ($mya->admin2 and !$mya->proSe) return go('/sadmin/summary'); // admins get way more info
  
  $special = $endorse = NULL;
  
  if ($mya->ok and !$mya->co) {
    $special = t('|<%aCovid>Our COVID-19 response</a> | <%aBlm>BLM</a> | <%aOnN>Offers & Needs</a>', '_aCovid _aBlm _aOnN', 'a id="covid"', 'a id="blm"', 'a id="onn"');
    if (!$mya->shouter and $mya->hasPhoto) { // (don't ask partial accounts to endorse)
      $endorseText = t('<h2>We need your help to grow.</h2>');
      $endorseBtn = btn('/community/invite', t('Endorse %PROJECT'), 'success', 'large');
      $skipLnk = lnk('', t('Skip for now'), clas('skipLnk'));
      $endorse = item("<div id=\"endorse\"><div><div>$endorseText<br>$endorseBtn<br>$skipLnk</div></div></div>", BARE);
    }
  }
  
  if ($mya->closed) say(t('Your account is closed, but you can still see your account history.'));

  $stepUrl = $mya->nextStepUrl('', $msg);
  if ($stepUrl and $stepUrl != 'console') {
    if (r\referrer() == 'signin') return w\go($stepUrl, $msg); // go right to next step
    say(t('To continue opening your %PROJECT account, <%a>click here</a>.', '_a', atag("/$stepUrl")));
  }
  if (nn($msg)) say($msg);

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
  foreach (ray('invite makeJoint makeCo reports deposits') as $k) if ($$k) $shortcuts[] = $$k;
  $shortcuts = join('</li><li>', nn($shortcuts, []));;
  $shortcuts = $shortcuts ? item("<ul><li>$shortcuts</li></ul>", t('Shortcuts:')) : NULL;

  $title = t('You: ' . $mya->fullName);
  $acct = $mya->mainQid;
  if ($mya->jid) $acct .= t(' (joined with %nm)', 'nm', $mya->jA->fullName);
  $acct = item($acct, t('Account #:'));

  $pay = btn('/pay', t('Pay'), '', 'md');
  $charge = btn('/charge', t('Charge'), '', 'md');
  $bank = btn('/get', t('Bank'), '', 'md');
  $buttons = item($pay . $charge . $bank);

  $balance = u\fmtAmt($mya->balance);
  $balanceDesc = $mya->activated ? t(' <small>(see <%a>History</a>)</small>', '_a', atag('/history')) : '';
  $balance = item($balance . $balanceDesc, t('Balance:'));

  if (!$mya->isCtty) { // unless this is a community or region account
    $suffix = ($mya->debt or $mya->depends or !$mya->refill) ? '' : t(' (<%a>Click here</a> to let your balance go negative.)', '_a', 'a id="activate-credit"');
    list ($credit, $normally) = $mya->depends ? ['$0', t(' (normally)')] : [u\fmtAmt(-$mya->floor), ''];
    $creditLine = $mya->underage ? NULL : item($credit, t('Credit Line:'), t('Based%normally on your account\'s average monthly activity over the past 6 months.', compact('normally')), w\suffix($suffix));
  
    if ($choiceList = choiceListFld()) $shortcuts = NULL; // don't show both
    
  } else {
    $acctType = t('community account');
    $creditLine = $choiceList = NULL;
  }

  jsx('console');
  return cgform(compact(ray('endorse title acct buttons balance creditLine special shortcuts choiceList')));
}

/**
 * Return a list of important next steps for the account.
 * @todo: add comment as an option?
 */
function choiceListFld() {
  global $mya;
  if ($mya->cAdmin) return NULL;
  
  $warning = t('<b class="loud">NOTE!</b> Once you click this step, your account will be temporarily deactivated and you will need to finish all steps before it can be reactivated.');
  $choices0 = explode("\n", t(<<< X
    community:invite:Invite:Invite a friend or company to join.<br><small>Your local Community Fund gets $2 for every invitation you send!</small>
    community:donate:Give:Make a donation to %PROJECT.
    community:comment:Comment:Send a comment, question, or suggestion.
    scraps:person:Finish:Finish opening your account.<br><small>Get a %PROJECT payment card, accept payments (including discount rebates), and participate in %PROJECT economic democracy.</small>
    scraps:co:Finish:Finish opening your complete company account.
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
    
  list ($task, $finishCh) = $mya->co ? ['co', $coCh] : ['person', $personCh];
  if (!$choices = $mya->stepCount($task) ? $finishCh : FALSE) return NULL;
  $choices += $inviteCh + $donateCh + $commentCh;
  $choiceList = t('<h3>Next Steps</h3>') . "\n";
  foreach ($choices as $url => $desc) if (!empty($desc)) $choiceList .= w\lnk("/$url", $desc) . "\n";
  return item($choiceList . "\n");
}  
