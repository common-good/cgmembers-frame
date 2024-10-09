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
  extract(just('codePw op dt', $args, NULL));
  global $mya;

  if (!$mya or !$mya->admin) hack('panel: ' . ($mya ? $mya->fullName : 'no one'));
  
  if ($op == 'close') {
    setV(VAR_CLOSE_BOOKS, strtotime($dt));
    say(t('Books are now closed through %dt.', compact('dt')));
  }
  
  if ($codePw) { // called from scanqr.js
    extract(just('vKeyPw', u\decryRay($codePw), NULL));
    superSignin($vKeyPw);
  }

  $queries = [
              '' => '',
              ];
  
  $title = t('System Administration');

  if ($mya->admSeeSecure) {
    $lnkScanQr = item(lnk('/scan-qr', t('Scan SuperAdmin QR')));

    $toSMT = submi(t('To SMT'));
    $reAuthQBO = submi(t('ReAuthorize QBO'));
    $inviteeStatus = submi(t('Invitee Status'));
    $signupCos = submi(t('Connections to Companies'));
  } else $lnkScanQr = $toSMT = $reAuthQBO = $inviteeStatus = $signupCos = NULL;
  
  $queries = selectFld(t('Query:'), '', '', $queries);
// NO  $account = textFld(t('Account:'), t('Account to do something to'), dft($qid ?: (svar('adminingAcct') ?: $mya->mainQid)));
  
  if ($mya->admStopServer) {
    if (r\up()) $stop = submi(t('STOP')); else $start = submi(t('START'));
  }

  $tables = ($mya->admSeeAccts and $mya->admRegion) // tables to show
  ? 'users txs usd recurs proxies relations invites admins notices boxes do stats usd nonmembers' : '';

  if ($mya->admTen99) $do1099ktest = submi(t('1099-K TEST'));
  if ($mya->admTen99) $do1099k = submi('1099-K');
  if ($mya->admReconcile) $reconcile = submi('Reconcile Ctty Fund');
  $byState = submi('Accts By State');
  if ($mya->admRegion and $mya->admEditTx) $ieReport = submi('IE Report');
  if ($mya->admEditTx) {
    $setCats = submi(t('Set Cats'));
    $sendTxs = submi(t('Send to QBO'));
  }
  if (isDEV) $makeTestAccts = submi(t('Make Test Accounts'));
  if ($mya->admDeleteAcct) $mergeAccts = submi(t('Merge Accounts'));
  if ($mya->admRegion and $mya->admSeeAccts) $ourAccts = submi(t('Sponsored Accounts'));
  
  if ($mya->admCode) {
    $special = submi(t('Special'));
//    $askForAppData = t('Ask for App Data');
//    $seeAppData = t('See App Data');
  }

  if ($mya->admRecheckSsn) $ssnLink = t('SSN Link');
  if ($mya->admSeeSecure) $seeSecure = t('See Secure');
  if ($mya->admV) $clone = t('Clone');
  if ($mya->admRecheckSsn) $resetSSN = t('Recheck SSN');
  if ($mya->admMakeVAdmin) $makeVKeyE = t('Make vKeyE');
  if ($mya->admPrintCards) $cancelCard = t('Cancel CG Card');
  if ($mya->admMakeRules) $fsRule = t('Set up Sponsorship');
  if ($mya->admChangeUid) $chgAcctId = t('Change Account ID by Addr');
  if ($mya->admDeleteAcct) $delAcct = t('DELETE Account');

  $seeData = t('See Data');
  $seeChanges = t('See Changes');
  
  //  $connectBank = submi(t('Connect Bank'));
  //  $seeHidden = submi(t('See Hidden'));

  if ($mya->admStopCtty) {
    $cttyName = ($cttyA = r\acct($mya->community)) ? $cttyA->fullName : NULL;
    if ($mya->cttyUp) $stopCtty = submi(t('STOP ') . $cttyName); else $startCtty = submi(t('START ') . $cttyName);
  }

  $do = []; foreach (ray($tables) as $one) $do[$one] = submi($one);
  $ks = ray('toSMT reAuthQBO setCats ieReport sendTxs inviteeStatus signupCos do1099ktest do1099k reconcile byState special stop start stopCtty startCtty makeTestAccts mergeAccts ourAccts');
  foreach ($ks as $k) if (isset($$k)) $do[$k] = $$k;
  $do = fieldSet('do', $do, BARE);
  
  $ks = ray('cancelCard seeData seeChanges ssnLink seeSecure clone resetSSN makeVKeyE askForAppData seeAppData fsRule chgAcctId delAcct');
  $acctSet = []; foreach ($ks as $k) if (isset($$k)) $acctSet[$k] = btn('', $$k, 'success', 'xs', $k);
  $acctSet = item(join('', $acctSet), t('Account Operations:'));
  
  $closeBooks = $mya->admReconcile ? dateFld(t('Close books on:'), '', dft(getV(VAR_CLOSE_BOOKS)) + suffix(btn('', t('Close')))) : NULL;
  $goTx = $mya->admSeeTxInfo ? intFld(t('See tx#:'), '', suffix(btn('', t('Go')))) : NULL;
  jsx('panel', 'who', "$mya->fullName ($mya->mainQid)");

  return cgform(compact(ray('title lnkScanQr do acctSet closeBooks goTx')));
}

function formAdminPanel_submit($form, &$sta) {
  global $base_url;
  global $mya;

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

/*  if ($account) {
    if (!$a = a($account)) return say('That is not an account.'); // should use whois first
    svar('adminingAcct', $account); // remember what account we're working on
    if (!$mya->admRegion and $a->community != $mya->community) return say('That account is not in your community.', 'ERR');
  } else */
  $a = $mya;
  
  switch ($op) {
    case 'cancelCard':
    if ($a->co and $a->proSe) return say(t('A company cannot have a %PROJECT card (only its agents have %PROJECT cards).'));
    return w\go("print-rcard/1/$a->id" . ($a->co ? "/$a->agentId" : ''));
    
    case 'chgAcctId': return a\changeUid($a);
      
    case 'fsRule': // make fiscal sponsorship surtx rule (defaults to 5%) and create QB account
      if (!$a->co) return err(t('That account is not a company, so it cannot be sponsored.'));
      $info = ray('action payerType payeeType payee from to', ACT_SURTX, REF_ANYBODY, REF_ACCOUNT, $a->id, MATCH_PAYEE, CGID);
      if ($id = db\get('id', 'tx_rules', u\where($info) . " AND amount=0 AND purpose LIKE '%sponsorship fee%' AND IFNULL(end, :NOW+1) > :NOW", $info)) {
        return err(t('You already have a fiscal sponsorship rule for that account. <%a>Click here</a> to edit it.'), '_a', atag("/sadmin/rules/id=$id"));
      }
      $info += ray('start portion purpose', today(), .05, t('fiscal sponsorship fee (5%)'));
      db\insert('tx_rules', $info);
      $a->setBit(B_OK); $a->setBit(B_CONFIRMED);
      $a->setCoBit(CO_SPONSORED);
      $nm = $a->bestName;
      say(t('A sponsorship fee rule has been added for %nm.', compact('nm')));
      
      require_once R_ROOT . '/classes/qb.class';
      $qb = new \QB(QBO_CREDS, FALSE);
      $qb->makeSponsee($nm, $a->mainQid);
      return say(t('Made a QB account for %nm.', compact('nm')));
      
    case 'askForAppData':
      $a->update('getAppData appData', '!report', []);
      return say(t('Request for app data sent to ') . $a->fullName);
      
    case 'seeAppData': say(pr($a->appData));
    case 'seeSecure':
      say(u\rayTable(ray('fullName phone dob physicalAddr', $a->fullName, substr($a->phone, 2), $mya->admRegion ? fmtDt($a->dob) : '', $a->address . ' ' . $a->zip)));
      //      seeHidden($a);
      return seeSecure($a);
      
    case 'clone':
      $newA = a\cloneA($a);
      say(t('You are now in cloned account %qid. Add any desired relations', 'qid', $newA->mainQid));
      return changeWho('', tr('page=%page&acct=%acct', 'page acct', u\urlify('/settings/relations'), $newA->id));
      
    case 'resetSSN':
      $a->setBit(B_MEMBER, FALSE);
      $a->stepDone('verifyid', FALSE);
      return $a->update('ssnData', '');  
      
    case 'makeVKeyE':
      if (!$mya->admMakeVAdmin) return err(t('You must have superAdmin permission to create a superAdmin card.')); // check again
      if ($a->id == $mya->agentId) return err(t('You cannot revoke and recreate your own superAdmin card.'));
      return exitJust($a->makeVKeyE());
      
    case 'seeData': return $a->proSe ? err(t('You cannot view your own administrative data.')) : say(u\rayTable($a->data, FALSE, TRUE));
  /*  case 'connectBank': return r\Web\connectBank($a); */
    case 'seeChanges': return w\go("sadmin/changes/$a->mainQid");
    case 'ssnLink': return w\say($a->fullName. t(' SSN Link:') . tr('<br><br> &nbsp; %BASE_URL/do/doSsn~%code &nbsp; ', 'code', $a->doCode()));

    case 'do1099ktest': $test1099 = TRUE; // fall through
    case 'do1099k':
      include_once R_ROOT . '/cg-yearly.inc';
      return r\Special\do1099s('K', nn($test1099));
      
    case 'delAcct': return goFirst("/change-account/acct=$a->agentId", "/sadmin/delete-account/$a->id", TRUE);
    case 'reconcile': return w\go('sadmin/reconcile-ctty-fund');
    case 'toSMT': return a\toSMT();
    case 'reAuthQBO': return w\go('qbo');
    case 'setCats': return w\go('sadmin/set-cats');
    case 'ieReport': return w\go('/sadmin/iereport');
    case 'sendTxs': return w\go('qbo/op=txs');
    case 'inviteeStatus': return w\go('community/invite/invited-whom');
    case 'makeTestAccts': return a\makeTestAccounts();
    case 'mergeAccts': return w\go('sadmin/merge-accounts');
    case 'ourAccts': return w\go('/sadmin/our-accounts');
    case 'signupCos':
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
      return w\say($signupCos ? w\showRecords($signupCos) : t('No such companies.'));
      
    case 'byState': return byState();
    
    // handle all other buttons with showQuery()
    case 'users': return say(a\showQuery('users', 'name', 'uid, flags, balance, rewards, savingsAdd AS saveAdd, minimum AS min, floor, city, zip AS zip'));
    case 'txs': return say(a\showQuery('txs', 'xid DESC, type', 'xid, t.created, type as ty, amt, for2 AS `for`, uid1, uid2, t.flags, t.goods AS gd, t.actorId AS init', 'uid1 uid2'));

    case 'recurs': return say(a\showQuery('tx_timed tm', 'id DESC', 'id,tm.start AS created,tm.from,tm.to,amount,purpose,period,tm.end', 'tm.from tm.to'));
    case 'invites': return say(a\showQuery('r_invites', 'invited DESC', 'invited, inviter, invitee, email', 'inviter'));
    case 'notices': return say(a\showQuery('r_notices', 'msgid DESC'));
    case 'admins': return say(a\showQuery('(SELECT uid,fullName,can FROM users LEFT JOIN admins USING (uid) WHERE :IS_ADMIN OR can) x', 'fullName', 'uid, fullName, IFNULL(can, 0) AS can'));
    case 'proxies': return say(a\showQuery('r_proxies', 'id DESC', 'person, proxy, priority', 'person'));
    case 'relations': return say(a\showQuery('u_relations', 'main DESC, reid', 'reid,main,other,otherNum,:IS_DRAW AS draw,permission,:IS_EMPLOYEE AS employee,:IS_OWNER AS owner,:IS_CUSTOMER AS customer,:IS_AUTOPAY AS autopay,code', 'main'));
    case 'usd': return say(a\showQuery('txs2', 'created DESC'));
    
    default:
      foreach (ray('boxes do stats') as $one) if ($op == $one) return say(a\showQuery("r_$one", 'created DESC'));
      err(t('Unknown operation.'));
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
