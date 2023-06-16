<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Admin as a;

include_once R_ROOT . '/admin/admin.inc';
include_once R_ROOT . '/admin/admin-web.inc';

/**
 * Make various administrative operations available to the admin and cAdmin.
 * 'wrong community' => 'SELECT * FROM `users` WHERE uid>0 AND MID(community, 2, 7)<>MID(uid, 1, 7)',
 */
function formAdminPanel($form, &$sta, $args = '') {
  extract(just('qid code', $args, NULL));
  global $mya;

  if (!$mya or !$mya->admin) hack('panel: ' . ($mya ? $mya->fullName : 'no one'));
  
  if ($code) { // called from scanqr.js
    extract(just('vKeyPw', u\decryRay($code), NULL));
    if ($vKeyPw) superSignin($vKeyPw);
  }

  $tables = 'users txs usd recurs proxies relations invites'; // tables to show
  $queries = [
              '' => '',
              ];
  
  $title = t('System Administration');
  
  $lnkScanQr = item(lnk('/scan-qr', t('Scan SuperAdmin QR')));

  $toSMT = submi(t('To SMT'));
  $reAuthQBO = submi(t('ReAuthorize QBO'));
  $inviteeStatus = submi(t('Invitee Status'));
  $signupCos = submi(t('Connections to Companies'));
  $queries = selectFld(t('Query:'), '', '', $queries);
  $account = textFld(t('Account:'), t('Account to do something to'), dft($qid ?: (svar('adminingAcct') ?: $mya->mainQid)));
  
  if ($mya->admStopServer) {
    if (r\up()) $stop = submi(t('STOP')); else $start = submi(t('START'));
  }

  if ($mya->admTen99) $do1099ktest = submi(t('1099-K TEST'));
  if ($mya->admTen99) $do1099k = submi('1099-K');
  if ($mya->admReconcile) $reconcile = submi('Reconcile Ctty Fund');
  if ($mya->admRegion) $tables .= ' admins notices boxes do stats usd nonmembers';
  $byState = submi('Accts By State');
  if ($mya->admRegion) $ieReport = submi('IE Report');
  if ($mya->admEditTx) {
    $setCats = submi(t('Set Cats'));
    $sendTxs = submi(t('Send to QBO'));
  }
  
  if ($mya->admCode) {
    $special = submi(t('Special'));
    $askForAppData = submi(t('Ask for App Data'));
    $seeAppData = submi(t('See App Data'));
  }
  if ($mya->admRecheckSsn) $ssnLink = submi(t('SSN Link'));
  if ($mya->admSeeSecure) $seeSecure = submi(t('See Secure'));
  if ($mya->admV) $clone = submi(t('Clone'));
  if ($mya->admRecheckSsn) $resetSSN = submi(t('Recheck SSN'));
  if ($mya->admMakeVAdmin) $makeVKeyE = submi(t('Make vKeyE'));
  if ($mya->admMakeVAdmin or $mya->admPrintCards) $make2 = boxFld('make', t('Make:'));
  if ($mya->admPrintCards) $printId = submi(t('Print ID'));
  if ($mya->admMakeRules) $fsRule = submi(t('Set Sponsorship Fee'));
  if (isDEV) $makeTestAccts = submi(t('Make Test Accounts'));

  //  $connectBank = submi(t('Connect Bank'));
  //  $seeHidden = submi(t('See Hidden'));
  $seeData = submi(t('See Data'));
  $seeChanges = submi(t('See Changes'));
  if ($mya->admStopCtty) {
    $cttyName = ($cttyA = r\acct($mya->community)) ? $cttyA->fullName : NULL;
    if ($mya->cttyUp) $stopCtty = submi(t('STOP ') . $cttyName); else $startCtty = submi(t('START ') . $cttyName);
  }

  $do = []; foreach (ray($tables) as $one) $do[$one] = submi($one);
  $ks = ray('toSMT reAuthQBO setCats ieReport sendTxs inviteeStatus signupCos do1099ktest do1099k reconcile byState special stop start stopCtty startCtty makeTestAccts');
  foreach ($ks as $k) if (isset($$k)) $do[$k] = $$k;
  $do = fieldSet('do', $do, BARE);
  
  $ks = ray('account make2 printId seeData seeChanges ssnLink seeSecure clone resetSSN makeVKeyE askForAppData seeAppData fsRule');
  $acctSet = []; foreach ($ks as $k) if (isset($$k)) $acctSet[$k] = $$k;
  $acctSet = fieldSet('acctSet', $acctSet);

  return cgform(compact(ray('title lnkScanQr do acctSet')));
}

function formAdminPanel_submit($form, &$sta) {
  global $base_url;
  global $mya;

  extract(just('account make', $sta['input'], NULL));
  $op = op($sta);

  if ($op == 'start' or $op == 'stop') {
    r\up($op == 'start');
    return r\up() ? say(t('System is UP.')) : say(t('System is DOWN.'), 'ERR');
  }
  if ($op == 'special') return a\special();
  if ($up = ($op == 'startCtty') or $op == 'stopCtty') {
    $ctty = r\acct($mya->community);
    $ctty->setBit(B_OK, $up);
    return $up ? say(t('Community is UP.')) : say(t('Community is DOWN.'), 'ERR');
  }

  if ($account) {
    if (!$a = a($account)) return say('That is not an account.'); // should use whois first
    svar('adminingAcct', $account); // remember what account we're working on
    if (!$mya->admRegion and $a->community != $mya->community) return say('That account is not in your community.', 'ERR');
  } else $a = $mya;
  
  if ($a and $op == 'printId') {
    if ($a->co and $a->proSe) return say(t('A company cannot have a %PROJECT card (only its agents have %PROJECT cards).'));
    return w\go("print-rcard/$make/$a->id" . ($a->co ? "/$a->agentId" : ''));
  } elseif ($a and $op == 'fsRule') { // make fiscal sponsorship surtx rule (defaults to 5%)
    if (!$a->co) return err(t('That account is not a company, so it cannot be sponsored.'));
    $info = ray('action payerType payeeType payee from to', ACT_SURTX, REF_ANYBODY, REF_ACCOUNT, $a->id, MATCH_PAYEE, CGID);
    if ($id = db\get('id', 'tx_rules', u\where($info) . " AND amount=0 AND purpose LIKE '%sponsorship fee%' AND IFNULL(end, :NOW+1) > :NOW", $info)) {
      return err(t('You already have a fiscal sponsorship rule for that account. <%a>Click here</a> to edit it.'), '_a', atag("/sadmin/rules/id=$id"));
    }
    $info += ray('start portion purpose', today(), .05, t('fiscal sponsorship fee (5%)'));
    db\insert('tx_rules', $info);
    $a->setBit(B_OK); $a->setBit(B_CONFIRMED);
    $a->setCoBit(CO_SPONSORED);
    say(t('A sponsorship fee rule has been added for %nm.', 'nm', $a->bestName));
  } elseif ($op == 'askForAppData') {
    $a->update('getAppData appData', '!report', []);
    say(t('Request for app data sent to ') . $a->fullName);
  } elseif ($op == 'seeAppData') {
/**/  debug($a->appData);
  } elseif ($op == 'seeSecure' and $a) {
    say(u\rayTable(ray('fullName phone dob physicalAddr', $a->fullName, substr($a->phone, 2), $mya->admRegion ? fmtDt($a->dob) : '', $a->address . ' ' . $a->zip)));
    //      seeHidden($a);
    seeSecure($a);
  } elseif ($op == 'clone' and $a) {
    $newA = a\cloneA($a);
    say(t('You are now in cloned account %qid. Add any desired relations', 'qid', $newA->mainQid));
    return changeWho('', tr('page=%page&acct=%acct', 'page acct', u\urlify('/settings/relations'), $newA->id), $sta);
  } elseif ($op == 'resetSSN' and $a) {
    $a->setBit(B_MEMBER, FALSE);
    $a->stepDone('verifyid', FALSE);
    $a->update('ssnData', '');    
  } elseif ($op == 'makeVKeyE' and $a) {
    if (!$make) return err(t('You must check the "Make" box to create a superAdmin card.'));
    if (!$mya->admMakeVAdmin) return err(t('You must have superAdmin permission to create a superAdmin card.'));
    if ($a->id == $mya->agentId) return err(t('You cannot revoke and recreate your own superAdmin card.'));
    exitJust($a->makeVKeyE());
  } elseif ($op == 'seeData') {
    if ($a and $a->proSe) say(u\rayTable($a->data, FALSE, TRUE));
    /*  } elseif ($op == 'connectBank') {
        r\Web\connectBank($a); */
  } elseif ($op == 'seeChanges') {
    w\go("sadmin/changes/$a->mainQid");
  } elseif ($op == 'ssnLink') {
    w\say($a->fullName. t(' SSN Link:') . tr('<br><br> &nbsp; %BASE_URL/do/doSsn~%code &nbsp; ', 'code', $a->doCode()));
  } elseif ($op == 'do1099k' or $test1099 = ($op == 'do1099ktest')) {
    include_once R_ROOT . '/cg-yearly.inc';
    r\Special\do1099s('K', nn($test1099));
  } elseif ($op == 'reconcile') {
    w\go('sadmin/reconcile-ctty-fund');
  } elseif ($op == 'toSMT') {
    a\toSMT();
  } elseif ($op == 'reAuthQBO') {
    return w\go('qbo');
  } elseif ($op == 'setCats') {
    return w\go('sadmin/set-cats');
  } elseif ($op == 'ieReport') {
    return w\go('/sadmin/iereport');
  } elseif ($op == 'sendTxs') {
    return w\go('qbo/op=txs');
  } elseif ($op == 'inviteeStatus') {
    return w\go('community/invite/invited-whom');
  } elseif ($op == 'makeTestAccts') {
    return a\makeTestAccounts();
  } elseif ($op == 'signupCos') {
    global $signupCos; $signupCos = [];
    $crit = $mya->admRegion ? '1' : 'community=' . $mya->community;
    eachA(function ($a) {
        global $signupCos;
        if (is_integer($a) or (is_string($a) and ctype_digit($a))) $a = r\acct($a);
        u\EXPECT([$a], 'Acct');
        if ($signupCo = $a->signupCo) {
          extract(just('company isOwner employee employeeOk contractor', $signupCo, ''));
          $signupCos[mb_strtolower("$a->community$company$a->id")] = ray('acct fullName relation company', $a->mainQid, $a->fullName, $isOwner ? t('owner') : (($employee or $employeeOk) ? t('employee') : ($contractor ? t('contractor') : '')), $company);
        }
      }, '(uid<0 OR uid>:UID_CANON9) AND ' . $crit . ' ORDER BY community');
    ksort($signupCos);
    w\say($signupCos ? w\showRecords($signupCos) : t('No such companies.'));
  } elseif ($op == 'byState') return byState();
  
  // handle all other buttons with showQuery()
  if ($op == 'users') return say(a\showQuery('users', 'name', 'uid, flags, balance, rewards, savingsAdd AS saveAdd, minimum AS min, floor, city, zip AS zip'));
  if ($op == 'txs') return say(a\showQuery('txs', 'xid DESC, type', 'xid, t.created, type as ty, amt, for2 AS `for`, uid1, uid2, t.flags, t.goods AS gd, t.actorId AS init', 'uid1 uid2'));

  if ($op == 'recurs') return say(a\showQuery('tx_timed tm', 'id DESC', 'id,tm.start AS created,tm.from,tm.to,amount,purpose,period,tm.end', 'tm.from tm.to'));
  if ($op == 'invites') return say(a\showQuery('r_invites', 'invited DESC', 'invited, inviter, invitee, email', 'inviter'));
  if ($op == 'notices') return say(a\showQuery('r_notices', 'msgid DESC'));
  if ($op == 'admins') return say(a\showQuery('(SELECT uid,fullName,can FROM users LEFT JOIN admins USING (uid) WHERE :IS_ADMIN OR can) x', 'fullName', 'uid, fullName, IFNULL(can, 0) AS can'));
  if ($op == 'proxies') return say(a\showQuery('r_proxies', 'id DESC', 'person, proxy, priority', 'person'));
  if ($op == 'relations') return say(a\showQuery('u_relations', 'main DESC, reid', 'reid,main,other,otherNum,:IS_DRAW AS draw,permission,:IS_EMPLOYEE AS employee,:IS_OWNER AS owner,:IS_CUSTOMER AS customer,:IS_AUTOPAY AS autopay,code', 'main'));
  if ($op == 'usd') return say(a\showQuery('txs2', 'created DESC'));
  foreach (ray('boxes do stats') as $one) if ($op == $one) return say(a\showQuery("r_$one", 'created DESC'));
}

/**
 * Do periodic stuff that only superAdmin can do.
 */
function superSignin($vKeyPw) {
  global $mya, $cryTest;
  r\setCryptCook('vKeyPw', u\b64decode($vKeyPw), 0);
  if (u\decry('V', u\cry('V', $cryTest = 'test')) != 'test') {
    r\deleteCook('vKeyPw');
    return softErr(t('Your admin password is invalid.'));
  }
  u\EXPECT($mya->admV, 'Non-admin scanned superAdmin card: ' . $mya->fullName);
  
  foreach ($kids = db\col('uid', 'users', ':IS_UNDERAGE') as $uid) { // underage bit got set when activating account
    $a = r\acct($uid);
    if (now() - ($a->dob ?: now()) >= MATURITY * YEAR_SECS) $a->setBit(B_UNDERAGE, FALSE); // unset bit (youngster is now of age)
  }
}

/**
 * Show a list of account counts by state (for our bank partners).
 */
function byState() {
    $start = strtotime('+3 months', u\qtrDt()); // start of next quarter
    for ($q = 0; $q < 7; $q++) {
      $end = $start - 1;
      $start = strtotime('-3 months', $start);
      $qtr = date('Y', $start) . 'Q' . (floor((date('n', $start) - 1) / 3) + 1);
      $flds[] = "SUM(IF(activated <= $end, 1, 0)) AS $qtr";
    }
    $flds = join(",\n", $flds);
    return say(a\showQuery("(SELECT abbreviation AS state, $flds FROM users u JOIN r_states s ON s.id=u.state WHERE :IS_OK GROUP BY s.id) x", 'state'));
}
