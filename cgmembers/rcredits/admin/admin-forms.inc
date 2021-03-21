<?php
/**
 * @file
 * Admin forms
 */
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

include_once __DIR__ . '/admin.inc';
include_once __DIR__ . '/admin-web.inc';
define('T_MSG_FILE', 'test-messages.txt');

/**
 * Make various administrative operations available to the admin and cAdmin.
 * 'wrong community' => 'SELECT * FROM `users` WHERE uid>0 AND MID(community, 2, 7)<>MID(uid, 1, 7)',
 */
function formAdminPanel($form, &$sta, $args = '') {
  extract(just('qid', $args, NULL));
  global $mya;
  if (!$mya->cAdmin) hack('panel');
  
  $tables = 'users txs usd recurs proxies relations invites'; // tables to show
  $queries = [
              '' => '',
              ];
  
  $title = t('System Administration');
  
  $toSMT = submi(t('To SMT'));
  $inviteeStatus = submi(t('Invitee Status'));
  $signupCos = submi(t('Connections to Companies'));
  $queries = selectFld(t('Query:'), '', '', $queries);
  $account = textFld(t('Account:'), t('Account to do something to'), dft($qid ?: (svar('adminingAcct') ?: $mya->mainQid)));
  
  if ($mya->superAdmin) {
    if (r\up()) $stop = submi(t('STOP')); else $start = submi(t('START'));

    $do1099ktest = submi(t('1099-K TEST'));
    $do1099k = submi('1099-K');
    $reconcile = submi('Reconcile Ctty Fund');
    $tables .= ' notices boxes do stats usd nonmembers';
    $special = submi(t('Special'));
    $askForAppData = submi(t('Ask for App Data'));
    $seeAppData = submi(t('See App Data'));
    $ssnLink = submi(t('SSN Link'));
    $seeSecure = submi(t('See Secure'));
    $resetSSN = submi(t('Recheck SSN'));
    $make2 = boxFld('make', t('Make:'));
  }

  $printId = submi(t('Print ID'));
  //  $connectBank = submi(t('Connect Bank'));
  //  $seeHidden = submi(t('See Hidden'));
  $seeData = submi(t('See Data'));
  $seeChanges = submi(t('See Changes'));
  if (!$mya->superAdmin and $ctty = $mya->community) { // not super
    $cttyName = ($cttyA = r\acct($ctty)) ? $cttyA->fullName : NULL;
    if ($mya->cttyUp) $stopCtty = submi(t('STOP ') . $cttyName); else $startCtty = submi(t('START ') . $cttyName);
  }

  $do = []; foreach (ray($tables) as $one) $do[$one] = submi($one);
  $ks = ray('toSMT inviteeStatus signupCos do1099ktest do1099k reconcile special stop start stopCtty startCtty');
  foreach ($ks as $k) if (isset($$k)) $do[$k] = $$k;
  $do = fieldSet('do', $do);
  
  $ks = ray('account make2 printId seeData seeChanges ssnLink seeSecure resetSSN askForAppData seeAppData');
  $acctSet = []; foreach ($ks as $k) if (isset($$k)) $acctSet[$k] = $$k;
  $acctSet = fieldSet('acctSet', $acctSet);

  return cgform(compact(ray('title do acctSet')));
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
    if (!$mya->admin and $a->community != $mya->community) return say('That account is not in your community.', 'ERR');
  } else $a = $mya;
  
  if ($a and $op == 'printId') {
    if ($a->co and $a->proSe) return say(t('A company cannot have a %PROJECT card (only its agents have %PROJECT cards).'));
    if ($make) {
      $oldFile = $a->photoFilename(TRUE); // get this before makeCardCode
      $a->makeCardCode(); // this changes photo filename
      if (file_exists($oldFile)) {
        $newFile = $a->photoFilename(TRUE);
        rename($oldFile, $newFile);
      }
    }
    return w\go('print-rcard/' . $a->id . ($a->co ? "/$a->agentId" : ''));
  } elseif ($op == 'askForAppData') {
    $a->update('getAppData appData', '!report', []);
    say(t('Request for app data sent to ') . $a->fullName);
  } elseif ($op == 'seeAppData') {
/**/  debug($a->appData);
  } elseif ($op == 'seeSecure' and $a) {
    say(u\rayTable(ray('fullName phone dob physicalAddr', $a->fullName, substr($a->phone, 2), $mya->admin ? u\fmtDate($a->dob) : '', $a->address . ' ' . $a->zip)));
    //      seeHidden($a);
    seeSecure($a);
  } elseif ($op == 'resetSSN' and $a) {
    $a->setBit(B_MEMBER, FALSE);
    $a->stepDone('verifyid', FALSE);
    $a->update('ssnData', '');    
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
  } elseif ($op == 'inviteeStatus') {
    return w\go('community/invite/invited-whom');
  } elseif ($op == 'signupCos') {
    global $signupCos; $signupCos = [];
    $crit = $mya->admin ? '1' : 'community=' . $mya->community;
    eachA(function ($a) {
        global $signupCos;
        if (is_integer($a) or (is_string($a) and ctype_digit($a))) $a = r\acct($a);
        u\EXPECT([$a], 'Acct');
        if ($signupCo = $a->signupCo) {
          extract(just('company isOwner employee employeeOk contractor', $signupCo, ''));
          $signupCos[mb_strtolower("$a->community$company$a->id")] = ray('acct fullName relation company', $a->mainQid, $a->fullName, $isOwner ? t('owner') : (($employee or $employeeOk) ? t('employee') : ($contractor ? t('contractor') : '')), $company);
        }
      }, '(uid<0 OR uid>:CANONIC_ACCTS) AND ' . $crit . ' ORDER BY community');
    ksort($signupCos);
    w\say($signupCos ? w\showRecords($signupCos) : t('No such companies.'));
  }
  
  // handle all other buttons with showQuery()
  if ($op == 'users') {
    $fields = 'uid, flags, balance, rewards, savingsAdd AS saveAdd, minimum AS min, floor, city, zip AS zip';
    return say(a\showQuery('users', 1, 'name', $fields));
  }
  
  $logFields = "channel,type,myid,IF(agent=myid,':R_NONE',agent) AS agent, REPLACE(info, ',', ', ') AS info";

  if ($op == 'txs') {
    $fields = 'xid, t.created, type as ty, amt, for2 AS `for`, uid1, uid2, t.flags, t.goods AS gd, t.actorId AS init, tid1, tid2';
    return say(a\showQuery('txs', 1, 'xid DESC, type', $fields, 'uid1 uid2'));
  }

  if ($op == 'recurs') return say(a\showQuery('tx_templates t', 1, 'id DESC', 'id,t.start AS created,t.from,t.to,amount,purpose,period,t.end', 't.from t.to'));
  if ($op == 'invites') return say(a\showQuery('r_invites', 1, 'invited DESC', 'invited, inviter, invitee, email', 'inviter'));
  if ($op == 'notices') return say(a\showQuery('r_notices', 1, 'msgid DESC'));
  if ($op == 'proxies') return say(a\showQuery('r_proxies', 1, 'id DESC', 'person, proxy, priority', 'person'));
  if ($op == 'relations') return say(a\showQuery('r_relations', 1, 'main DESC, reid', 'reid,main,other,otherNum,:IS_DRAW AS draw,permission,:IS_EMPLOYEE AS employee,:IS_OWNER AS owner,:IS_CUSTOMER AS customer,:IS_AUTOPAY AS autopay,code', 'main'));
  if ($op == 'usd') return say(a\showQuery('r_usd', 1, 'created DESC'));
  foreach (ray('boxes do stats') as $one) if ($op == $one) return say(a\showQuery("r_$one", '1 ORDER BY created DESC'));
}

/**
 * Display dated changes made to an account.
 * @param string $qid: the account QID (default to current account)
 */
function formSeeChanges($form, &$sta, $qid = '') {
  $a = r\acct($qid);
  $title = t('Account Changes for ') . $a->fullName;
  $list[] = '<tr><th>Date</th><th>Field</th><th>Old Value</th><th>New Value</th><th>Changed by</th></tr>';
  // $rec = []; // track historical field values

  $sql = 'SELECT h.created, field, oldValue, newValue, changedBy, u.name AS changerName FROM r_changes h LEFT JOIN users u ON h.changedBy=u.uid WHERE h.uid=:id ORDER BY created DESC';
  $q = db\q($sql, ray('id', $a->id));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $date = u\fmtDate($created);
    foreach (ray('oldValue newValue') as $k) {
      $type = u\crypted($$k);
      $$k = a\formatField($field, $type ? decryChange($type, $$k) : $$k);
    }
    $changedBy = $changerName ?: "($changedBy)";
    $list[] = "<tr><td>$date</td><td>$field</td><td>$oldValue</td><td>$newValue</td><td>$changedBy</td></tr>";
  }
	
  if (count($list) > 1) {
    $list = join("\n", $list);
    $list = item("<table>$list</table>");
  } else $list = t('No changes');
  
  return cgform(compact(ray('title list')), 'nolabs');
}

function decryChange($type, $v) {return ($type == 'V' and !r\allKnowing()) ? t('(secret)') : u\decry($type, $v);}

/**
 * Show a list of standard calls to make (to help new members, check-in, ask for donation, etc.)
 * @param bool $one: (optional) <show just my community>
 * @param bool $recent: (optional) <show latest notes first>
 */
function formFollowup($form, &$sta, $args = []) {
  global $base_url;
  global $mya;
  
	if (!$mya or (!$mya->cAdmin2 and !$mya->admin2)) return w\go('dashboard');
  
  extract(just('one recent', $args, null));

  $title = t('Admin Action Page');
  $subtitle = item(t('<p>Start with today and work backward. Click an account name.</p><ul><li><b>Live Person.</b> If you get a live person, click "live" for a script. Then, if no followup is needed, make a note about your conversation and click "Auto".</li><li><b>Message #1.</b> If you get a machine, click "msg1" for a Phone Message script, type "PM setup" or "PM welcome" or whatever as a note, and click the "2 days" button.</li><li><b>Message #2.</b> If you already left one message, try 3 more times to catch them live. If you still don\'t catch them click "msg2" for a Phone Message script, then click "email" to send them an Email Message. Type "EM setup" or whatever and click the "7" button if this is for a Setup or Annual check-in or the "Auto" button if it is for a Welcome.</li><li><b>After Setup Help Email.</b> If the email gets no response for a setup help call, keep phoning or emailing at 1-, 2-, or 7-day intervals until you reach them or we close the account.</li><li><b>After Annual Check-in Email.</b> If the email gets no response for an Annual check-in, send a US Mail fund appeal/check-in.</li></ul><p>(comments on this standard procedure are welcome)</p>') . w\btn('/sadmin/followup/one=1', t('Show just my community')) . ' &nbsp; ' . w\btn('/sadmin/followup', t('Show ALL')) . ' &nbsp; ' . w\btn('/sadmin/followup/recent=1', t('Recent notes first')));
  
  $now = now();
  $signupBit = r\allSteps()['signup'];
  $wantsCardP = 0; foreach(ray(S_CARD) as $step) $wantsCardP |= u\bit(r\allSteps()[$step]);
  $wantsCardCo = 0; foreach(ray(S_CO2) as $step) $wantsCardCo |= u\bit(r\allSteps()[$step]);
  $todayEnd = strtotime('tomorrow') - 1;
  list ($where1, $where2) = (($mya->admin or $mya->isAdmin2) and empty($one)) ? [1, 1] : [0, "community=$mya->community"];
  $cttys = r\cttyList();
  $cttys[key($cttys)] = t('Seedpack');
  foreach ($cttys as $k => $v) $cttys[$k] = trim(str_replace(PROJECT, '', $v));
  
  $by = !empty($recent)
    ? "notes IS NULL OR notes='' OR MID(notes,1,6)='knows ', (MID(notes,6,4)*37+LOCATE(MID(notes,3,3),'JanFebMarAprMayJunJulAugSepOctNovDec'))*32+MID(notes,1,2) DESC"
    : "($todayEnd - IF(tickle>0, tickle, u.created)) * IF(tickle>$todayEnd, -5 * 366 * :DAY_SECS, 1)"; // today's calls first, then yesterday, 2 days ago etc., then tomorrow, the next day, etc.
  $wantsCardQ = "IF(:IS_CO, $wantsCardCo, $wantsCardP)";
  if ($mya->superAdmin) $by = "(:IS_OK AND NOT :IS_CARDED AND (steps & $wantsCardQ)=$wantsCardQ) DESC, (!activated AND u.:IS_MEMBER) DESC, (notes LIKE '%ZAP%' AND BINARY notes LIKE '%ZAP%') DESC, " . $by;
  
  $sql = <<<EOF
    SELECT DISTINCT u.uid, community, tickle, fullName, phone, u.data, u.steps, u.created, activated, notes, :IS_CO AS co, t.created AS lastTxTime, :IS_MEMBER AS member, :IS_OK AS ok, :IS_CARDED AS carded
    FROM users u LEFT JOIN (SELECT created,uid1,uid2 FROM txs ORDER BY created DESC LIMIT 1) t ON u.uid IN (t.uid1,t.uid2)
    WHERE u.uid>:CANONIC_ACCTS AND NOT :IS_NONUDGE AND NOT (activated and NOT :IS_OK) AND $where2 AND (tickle>0 OR NOT :IS_MEMBER
      OR t.created<$now-60*:DAY_SECS OR t.created IS NULL OR :isDEV)
    ORDER BY $by
    LIMIT 50
EOF;
  
  $fields = ray('ctty type call name phone notes');
  if (!empty($one)) array_shift($fields);
  $q = db\q($sql, ['isDEV' => isDEV]);
  $guts = $heads = '';
  foreach ($fields as $k) {
    $heads .= "<th class=\"hd-$k\">" . ucfirst($k) . '</th>';
    $guts .= "<td>$k</td>";
  }
  $list = "<table id=\"memberlist\"><tr>$heads</tr>\n";
  $emails = '';
  $whities = ray('zap fu'); // use white font for these backgrounds
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
    foreach (['tickle', 'activated'] as $k) if ($$k) $$k = strtotime('today', $$k); // round down to nearest day
    $days = round(($tickle - $activated) / DAY_SECS); // tickle: days since activation
    $wantsCard = $co ? $wantsCardCo : $wantsCardP;
    list ($type, $color) = !$activated 
     ? (mb_strpos($notes, 'ZAP') 
       ? [t('DELETE'), 'zap']
       : ($member ? [t('activate'), 'zap'] : (u\getBit($steps, $signupBit) ? [t('setup'), 'setup'] : [t('pre'), 'pre']))
       )
     //    : (round(($activated + TICKLE_WELCOME - $tickle) / DAY_SECS) == 0 ? [t('welcome2'), 'welcome']
     //    : (round(($activated + TICKLE_DELAYED - $tickle) / DAY_SECS) == 0 ? [t('welcome6'), 'welcome']
     : (mb_strpos($notes, 'ZAP') ? [t('DEACTIVATE'), 'zap']
     : ((($steps & $wantsCard) == $wantsCard and !$carded) ? [t('print card'), 'print-card']
     : (!$carded ? [t('no card yet'), 'welcome']
     : (between($days, TICKLE_WELCOME - 1, TICKLE_WELCOME + 14) ? [t('welcome2'), 'welcome']
     : (between($days, TICKLE_DELAYED - 1, TICKLE_DELAYED + 14) ? [t('welcome6'), 'welcome']
     : (($activated < strtotime('9/2/2017') and $tickle < $activated + 90 * DAY_SECS and !$notes) ? [t('welcome'), 'welcome']
     : (($tickle and $days % 365 < 30) ? [t('annual'), 'annual'] // +/-:2/29
     //    in_array(($tickle - $activated) % DAY_SECS, [0, 1, DAY_SECS - 1])) ? [t('annual'), 'annual'] // +/-:2/29
     : (($lastTxTime < $now - 60 * DAY_SECS and $now - $activated > 60 * DAY_SECS) ? [t('dormant'), 'dormant']
     : ($tickle ? [t('followup'), 'fu']
     : ( [t('other'), 'fu'] ))))))))));

    if ($mya->superAdmin and $type == t('DELETE')) $type = w\lnk("$base_url/sadmin/delete-account/$uid", $type);
    if (in($color, $whities)) $color .= ' fu-whitey';
    $type = "<span class=\"fu-$color\">$type</span>";
    $call = u\fmtDate($tickle ?: $created);
    $name = w\lnk("$base_url/change-account/acct=$uid", $fullName ?: $uid);
    if ($co) $name = "<span class=\"co\">$name</span>";
    $phone = u\fmtPhone(u\decry('P', $phone), '-');
    $notes = strtr($notes, ["\n" => '|| ', date('Y') => '']);
    if ($type == t('pre')) $notes = db\get('code', 'r_relations', 'other=:uid AND :IS_CUSTOMER', compact('uid')) . " $notes";
    $notes = substr($notes, 0, 200) . (mb_strlen($notes) > 200 ? '...' : '');
    $ctty = nni($cttys, $community, '?'); // ? is for dev machine after running a test with ZZA in ctty -2
    $list .= strtr("<tr>$guts</tr>\n", compact($fields));
  }
  if (!isset($tickle)) $list .= t('There are no followup calls to be made.');
  $list .= "</table>\n";
  
  return cgform(compact(ray('title subtitle list')), 'wide nolabs');
}

/**
 * Show a list of members in the current account's community
 */
function formMemberList($form, &$sta) {
  global $base_url;
  global $mya;

  $cAdmin = $mya->cAdmin;
  $ctty = r\acct($mya->community);

  $title = (($cAdmin and $ctty) ? $ctty->fullName : '') . t(' Member List');
  $sql = 'SELECT uid, login, access, fullName, notes, :IS_CO AS co FROM users WHERE uid>:CANONIC_ACCTS AND community=:cttyId AND NOT :IS_NONUDGE ORDER BY :IS_OK, IF(:IS_OK, fullName, 0-login)'; // -login fails because it gets treated as a string
  $q = db\q($sql, ray('cttyId', $ctty->id));
  $list = "<table id='memberlist'>\n";
  $emails = '';
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $nameClass = $co ? 'name co' : 'name';
    $notes = str_replace("\n", "<br>\n", $notes);
    $notes = str_replace('do:', '<b class="memberlist-notes">do:</b>', $notes);
    $a = r\acct($uid);
    $phone = str_replace('+1 ', '', u\fmtPhone($phone0 = $a->phone));
    $postalAddr = $a->postalAddr;
    $email = $a->email;
    $fullName = "<a href=\"$base_url/change-account/acct=$a->mainQid\">$fullName</a>";
    $steps = a\showSteps($a);
    $date = u\fmtDate($a->ok ? $access : $login);

    if ($cAdmin) {
      $bits = []; // zap previous value
      foreach (ray('member ok bank') as $one) if ($a->can(u\consta('b', $one))) $bits[] = $one;
      $bits = $bits ? ('<span class="memberlist-bits"> ' . join(' ', $bits) . "</span><br>\n") : '';
    } else $bits = '';
    
    $one = "<tr><td colspan=4 class=\"memberlist-line2\">$date &mdash; $steps$bits$notes</td></tr>\n";
    $list .= <<<EOF
      <tr>
      <td class="$nameClass">$fullName</td>
      <td><a href="mailto:$email" target="_blank">$email</a></td>
      <td>$phone</td>
      <td>$postalAddr</td>
      </tr>
      $one
EOF;
  }
  $list .= "</table>\n";
  
  return cgform(compact(ray('title list')), 'wide nolabs');
}

/**
 * Download the member list for the current community
 */

function formDownloadMemberList($form, &$sta, $args = '') {
  global $base_url;

  extract($params = just(EXPORT_PARAMS, $args, NULL));
  if ($do == 'export') return w\export($params);

  if ($do == 'done') {
    $do = 'export';
    $params = http_build_query(compact(ray(EXPORT_PARAMS)));
///    return w\go("$base_url/sadmin/export-list/$params"); // debug(
    $download = item("<iframe src=\"$base_url/sadmin/export-list/$params\" class=\"hidden\"></iframe>");
  } else {$download = NULL;}
  
  global $mya;
  $cttyA = $mya->cttyA;

  $title = t('Download Member List');
  $expMo = r\rdoMonth();
  $subtext = t(<<<EOF
    <p>Generate a CSV format spreadsheet for accounts in your community (%cttyName). We recommend you use a service like MailChimp or MailerLite and import this list there before each mailing. Company accounts and members are marked, so there is generally no need to export them separately &mdash; you can do that separation within the mailing program (type contains "member" or "company" or "member company").</p>
    <p>Action links for individual accounts take the form %code. For example, a link from MailChimp to the democracy page would be:</p>
    <blockquote>$base_url/%eg1 (MailerLite) OR<br>
    $base_url/%eg2 (MailChimp)</blockquote>
    <p>Other action choices might be listed here someday. :)</p>
EOF
    , 'cttyName code eg1 eg2', $cttyA->fullName, '<site>/do/<op>~<code>', 'do/doEvents~{$code}', 'do/doEvents~*|code|*');
  
$chimp = boolFld(t('Export to email list?'), '', TRUE);
  
$allCttys = $mya->admin ? boxFld('allCttys', t('All communities:'), '', TRUE) : hidFld(FALSE); // temporary for CGF oversight
$status = w\radiosFld(t('Status:'), '', dft(2), ray('active not-yet-active both'));
$types = w\radiosfld(t('Types:'), '', dft(2), ray('individuals companies both'));
$anonymous = boxFld('anonymous', t('Anonymous:'), t('Export only non-identifying information (for data-mapping and analysis)'), FALSE);
$chimpSet = w\fieldSet('fields', compact(ray('allCttys status types anonymous')));
  
$submit = t('Download');
  
w\jsx('chimp');
return cgform(compact(ray('title subtext chimp chimpSet submit download')));
}

function formDownloadMemberList_submit($form, &$sta) {
  extract(just(EXPORT_PARAMS, $sta['input']));
  $do = 'done';
  u\setDft($anonymous, FALSE); // for a checkbox, unchecked results in not set
  return w\go('sadmin/export-list/' . http_build_query(compact(ray(EXPORT_PARAMS))), t('Download initiated.'));
}

/**
 * Generates the execute form.
 */
function formPhp($form, &$sta, $res = '') {
  $res = item($res, BARE);
  $code = areaFld(BARE, '', dft(getV('code')));
  $submit = t('Execute Code');
  return cgform(compact('res', 'code', 'submit'), 'wide nolabs');
}

/**
 * Process PHP execute form submissions.
 * WARNING: ALL ERRORS are suppressed on this page
 */
function formPhp_submit($form, &$sta) {
  extract(just('code', $sta['input']));
  $code = str_replace('<.', '<', $code); // fudge around drupal's sanitizing of all HTML
  \setV('code', $code); // remember for next time

  $u = 'r:, db:\DB, be:\Backend, u:\Util, a:\Admin, w:\Web, cr:\Cron, s:\Smart, v:\Vote, k:\Risk';
  if (!isPRODUCTION) $u .= ', t:\Testing';
  foreach (ray($u) as $k => $v) $uses[] = "use CG$v as $k;";

  $code = join('', $uses) . $code;
  
  ob_start();
  try {
    eval($code);
  } catch (\CompileError $e) {
    say(t('<b>COMPILE:</b> %msg - line %line', 'msg line', $e->getMessage(), $e->getLine()), 'err');
  }
  say(pr(ob_get_clean()));
  //  w\go('sadmin/php/');
}

//define('DEPOSIT_DATE1', 1411517529); // first deposit date

/**
 * Offer to print checks or a deposit slip for Dollar Pool transfers to or from member bank accounts.
 */
function formDeposits($form, &$sta, $deposit = 0) {
  global $mya, $base_url;
  //  if ($deposit) return w\go("sadmin/deposit/$deposit");

  $title = t('Bank Transfers');

  $fields = <<<EOF
    SUM(IF(amount<0, amount, 0)) AS amountOut,
    SUM(IF(amount>0, amount, 0)) AS amountIn,
    SUM(IF(amount<0, 1, 0)) AS countOut,
    SUM(IF(amount>0, 1, 0)) AS countIn
EOF;
  $res = db\rows($fields, 'r_usd', "deposit=$deposit AND amount<>0 AND created<=:NOW"); // fails for db\get
  //  if ($res[0]) {
  extract((array) $res[0]);
  foreach (ray('amountIn amountOut') as $k) $$k = u\fmtAmt($$k);
  $subtext = t('<b>New IN:</b> &nbsp;%countIn (%amountIn)<b class="newout">New OUT:</b> %countOut (%amountOut)', compact(ray('countIn amountIn countOut amountOut')));
  if ($mya->superAdmin) {
    $go = w\btn("/sadmin/checks/way=In&date=$deposit&mark=1", t('Print Checks'))
      . ' ' . w\btn("$base_url/sadmin/achs/date=$deposit&mark=1", t('Download ACH file'));
    $go = item($go, ' ');
  }
  
  $deposits = item(a\deposits(), ' ');
  w\jsx('deposits');

  return cgform(compact(ray('title subtext go deposits')));
}

/**
 * Show a list of reconciled Dollar Pool transfers and other transfers in or out of that (or those) account(s).
 */
function formShowReconciled($form, &$sta) {
  $neg = 'amount<0 XOR txid<0';
  $desc = "IF(txid<0, 'BAD', IF(amount>0, 'IN', 'OUT'))";
  
  $sql = <<< X
    SELECT * FROM

    (SELECT '' as extra, deposit AS dt, $desc AS what, SUM(amount) AS amt, COUNT(*) AS cnt,
     SUM(IF($neg, -amount, '')) AS outt,
     SUM(IF($neg, '', amount)) AS inn
     FROM r_usd WHERE amount<>0 AND bankTxId<>0 GROUP BY deposit,$desc DESC
     UNION ALL SELECT 1 AS extra, completed AS dt, IF(memo='', type, memo) AS what, amount AS amt, '' AS cnt,
     IF(amount<0, -amount, '') AS outt,
     IF(amount<0, '', amount) AS inn
     FROM r_usd2) t
    
    ORDER BY dt DESC    
X;

  $bal = db\sum('amount', 'r_usd', 'bankTxId<>0') + db\sum('amount', 'r_usd2');
  $q = db\q($sql);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $dt = u\fmtDate($dt, TRUE);
    if ($cnt) $what .= ": $cnt";
    if ($what == 'T') $what = t('transfer');
    if ($what == 'S') $what = $amt < 0 ? t('fee') : t('interest');
    $balance = $bal;
    foreach (ray('inn outt balance') as $k) if ($$k) $$k = number_format($$k, 2);
    foreach (ray('dt what outt inn balance') as $i => $k) $line[$i] = "<td class='$k'>" . $$k . '</td>';
    $class = $extra ? ' class="hiRow"' : '';
    $lines[] = "<tr$class>\n" . join("\n", $line) . '</tr>';
    $bal -= $amt;
  }
  $deposits = join("\n", nn($lines) ?: []);
  foreach (ray('Date Memo Payment Deposit Balance') as $i => $k) $line[$i] = "<th class='$k'>$k</th>";
  $header = "<tr>\n" . join("\n", $line) . '</tr>';
  $list = <<< X
    <table id="deposits">
    $header
    $deposits
    </table>
X;
  $title = t('Reconciled Dollar Pool Transactions');
  $txid = db\min('txid', 'r_usd', 'bankTxId=0 AND amount<>0') ?: 999999999;
  $asof = u\fmtDate(db\max('deposit', 'r_usd', 'txid<:txid', compact('txid')) ?: now(), TRUE);
  $subtext = t('These Dollar Pool transactions have been matched one-to-one with the bank\'s records, for all transfers dated on or before %asof.', compact('asof'));
  $list = item($list);
  return cgForm(compact(ray('title subtext list')));
}

/**
 * Show a PDF of a (populated) deposit slip for a particular Dollar Pool transfer group ("deposit") in and/or out.
 */
function depositDetails($args) {
  extract(just('way date total count only', $args, NULL));
  include_once R_ROOT . '/pdf.class';
  $pdf = new r\Pdf();

  if (!$in = (!isset($way) or strtoupper($way) == 'IN')) $total = str_replace('-', '', $total);

  $m = .5; // left/right margin size
  $colW = 1.25; // amount column width
  $pdf->setPageUnit('in'); // measure everything in inches
  $pdf->SetMargins($m, $m, $m);  // left/top/right margin, needed for auto page breaks
  $pdf->SetAutoPageBreak($count > ($in ? 47 : 9), $m+.1); // leave space at bottom (count compensates for TCPDF bug)
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(TRUE);
  //  $pdf->setLanguageArray($l); // (eg "page" in footer)

  //  list ($activity, $where) = $in ? [t('DEPOSIT'), '(NOT (amount<0 XOR txid<0))'] : [t('CASHOUT'), '(amount<0 XOR txid<0)'];
  $activity = t('Bank Transfer');
  $where = 1;
  $today = $date == 1 ? t('(Historical)') : u\fmtDate($date ?: now());
  $escrow = $in ? ESCROW_IN : ESCROW_OUT;
  $pdf->setupFooter('', BANK_DBA . " $activity $today", ['........ A5990D1155A '.$escrow.'C', 'GnuMICR;12'], 'Page @PAGE of @PAGES');
  //  $pdf->setupFooter('', BANK_DBA . " $activity $today", ['ACHes', 'GnuMICR;12'], 'Page @PAGE of @PAGES');
  $pdf->AddPage();

  $left = t(<<<EOF
            <h1><b>%activity Details</b><br>
            %today</h1><br>
            <br>
            <br>
            <b>%CGF_LEGALNAME<br>
            dba %BANK_DBA</b><br>
            %CGF_POSTALADDR<br>
            %CGF_PHONE<br>
            <br>
            <br>
            <b>%BANK_NAME</b><br>
            Account #%escrow<br>
            %BANK_ADDR<br>
            <br>
            <br>
            <b>TOTAL: %total</b><br>
            %count checks<br>
EOF
            , compact(ray('activity today total count escrow')));

  $pdf->say($left, $m, 0);
  //    SELECT ABS(amount) AS amount, txid, uid, u.fullName AS name FROM r_usd t LEFT JOIN users u ON u.uid=t.payee 

  $sql = <<<EOF
    SELECT amount, xid, uid, u.fullName AS name FROM r_usd t LEFT JOIN users u ON u.uid=t.payee 
    WHERE $where AND deposit=$date AND amount<>0 AND t.created<=:NOW ORDER BY xid
EOF;
  $q = db\q($sql);
  $x = $pdf->pageW / 2 - 1;
  $pdf->setY($m);
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
    // (needs htmlcell instead of cell)   if (!$in) $name = r\acct($uid)->bankInfoLink();
    //    if (!$in) $name = r\acct($uid)->legalNameDpy;
    $pdf->setX($x);
    if ($only and ($amount > 0 xor $in)) continue;
    if ($amount < 0) $pdf->SetTextColor(255, 0, 0);
    $pdf->Cell(1, 0, u\fmtAmt($amount, ''), '', 0, 'R');
    $pdf->SetTextColor(0);
    $pdf->setX($x + $colW);
    $pdf->Cell($x - $colW, 0, "(# $xid) $name", '', 1);
    if (FALSE) { // !$in) { // for outgoing checks, show bank address and member's legal name
      $pdf->setX($x + $colW);
      if ($bankInfo = r\acct($uid)->bankInfo) {
        extract($bankInfo, EXTR_PREFIX_ALL, 'b');
        $b_phone = u\fmtPhone($b_phone);
        $pdf->Cell($x, 0, $b_name, '', 1);
        $pdf->setX($x + $colW);
        $pdf->Cell($x, 0, $b_address, '', 1);
        $pdf->setX($x + $colW);
        $pdf->Cell($x, 0, "$b_city, $b_state $b_zip ($b_phone)", '', 1);
      } else $pdf->Cell($x, 0, 'NO BANK ADDRESS', '', 1);
      $pdf->newLine(); // a blank line between
    }
  }
  $pdf->setX($x);
  $pdf->Cell(1, 0, '----------------', '', 1, 'R');
  $pdf->setX($x);
  $pdf->Cell(1, 0, $total, '', 0, 'R');
  $pdf->setX($x + $colW);
  $pdf->Cell(($pdf->pageW - $x) - $colW, 0, t('TOTAL'), '', 1);

  $date = strftime('%Y%m%d', $date);
  if (test()) $pdf->Close(); else 
    //  $pdf->Output(PROJECT . "-deposit-$way-$date.pdf", 'I'); //Close and output
    $pdf->Output(PROJECT . "-deposit-$date.pdf", 'I'); //Close and output
}

/**
 * Create an ACH Batch file for all relevant transfer requests.
 */
function achBatch($args) {
  include DRUPAL_ROOT . '/rcredits/admin/admin-achify.inc';
  extract(just('way date mark', $args));
  //  achify("deposit=$date");
  w\achify("deposit=$date AND amount<=0 AND created<=:NOW"); // temporarily only money going OUT or verifying bank account
  if (test()) return;
}

/**
 * Create a PDF of all relevant checks.
 * @param string $way: IN (from member bank account to CG) or OUT (from CG to member bank account)
 * @param int $date: deposit/check date
 * @param bool $reprint: UNUSED whether to include a reprint of the previous deposit's checks
 * @param int $previous: UNUSED date of previous deposit
 * @param bool $mark: whether to set the deposit date for each relevant transfer, in r_usd
 * @param int $limit: maximum number of checks to print (defaults to 42, the most we can mail for a 3oz stamp)
 */
 define('CHECK_LIMIT', 42); // default maximum numbere of checks to print at a time
 
function printChecks($args) {
  extract(just('way date mark limit', $args));
  include_once R_ROOT . '/pdf.class';
  $ck = new r\Pdf();
  $ck->setPrintHeader(FALSE);
  $ck->setPrintFooter(FALSE);
  $ck->setPageUnit('in'); // measure everything in inches
  //  $ck->SetMargins(0, 0, 0); // left/top/right margin
  $ck->SetAutoPageBreak(FALSE);

  $ways = ['IN'=>'(amount>0 AND txid>0)', 'OUT'=>'(amount<0 AND txid>0)', 'BAD'=>'txid<0', 'VERIFY'=>'amount=0'];
  $where = $ways[strtoupper($way)];
  $sql = "SELECT xid,txid,created,payee,amount,deposit,bankAccount FROM r_usd WHERE deposit=:date AND created<=:NOW AND $where ORDER BY xid";
  $subs = compact('date');
  u\setDft($limit, CHECK_LIMIT);
  if (is_numeric($limit)) $sql .= " LIMIT $limit"; // ' LIMIT :limit' fails
  $q = db\q($sql, $subs);
  $pos = 3; // before first position of 3, on 3-up check paper
  $count = 0;
  while ($tx = $q->fetchAssoc()) {
    if ($pos == 3) {$pos = 1; $ck->AddPage();} else $pos++;
    a\printCheck($ck, $pos, $tx, $mark);
    $count++;
  }
  $date = strftime('%Y-%m-%d', $date);
  if (test()) $ck->Close(); else $ck->Output(PROJECT . "-deposit-checks-$way-$date.pdf", 'I'); //Close and output 
  //  say($count . t(' checks printed'));
}


/**
 * Print an ID card
 * @param mixed $id: qid or uid of account to make a card for
 * @param mixed $agent: qid or uid of agent
 * @param bool $retroactive: create the id regardless of permissions (used only in testing)
 */
function memberId($id, $agent = '', $retroactive = FALSE) {
  global $mya;
  require_once R_ROOT . '/pdf.class';
  
  $a = r\acct($id, $agent);
  $cttyErr = (!$mya or (!$mya->admin and $a->community != $mya->community));
  if (!$a or ($cttyErr and !$retroactive)) u\FAIL('No such account ia: ' . $id . ':' . $agent);
  
  $a->setAgentNum(); // assign an agentCode, if appropriate (and not yet done)
  $qid = $a->qid; // get this before setting $a to agent (below)
  
  $acctName = $a->bestName;
  $nameFont = mb_strlen($acctName) > 38 ? 'font-stretch:condensed;'
    : (mb_strlen($acctName) > 33 ? 'font-stretch:semi-condensed;' : '');
  
  list ($aa, $proSe) = [$a->agentA, $a->proSe];

  list ($role, $ptQid, $yName, $bg) = $proSe 
    ? [t('Member'), 10, 1.70, 'bg'] 
    : [t('Company'), '9.5;font-stretch:semi-condensed', 1.64, 'bgCo'];

  $permitted = ($proSe or B_RELATED + r\relation('permission', $a->id, $aa->id) >= B_BUY);
  $photo = ($aa->hasPhoto and $permitted) ? "@$aa->photo" : R_NOT_VALID; // '@' tells tcpdf it's an image, not a file; $a->canBuy fails because admin has permission

  list ($wCard, $hCard, $m, $xText) = [3.375, 2.125, .24, .06]; // card dimensions, negative page margin, text indent
  //  list ($wPhoto, $mPhoto, $xCardNo) = [1.09, .1, 2.4];
  list ($xProj, $yProj) = [1.54, $m+.04];
  list ($xSite, $ySite, $wPhoto, $mxPhoto, $myPhoto, $xCardNo) = [$xProj+.035, $yProj+.29, 1.1, .085, .09, 2.4];
  list ($cCG, $cSite, $cName) = ['#000065', 'black', '#004000'];
  list ($xTest, $xPhoto, $yPhoto, $hPhoto) = [$m+$wCard-.74, $m+$mxPhoto, $m+$myPhoto, $wPhoto*4/3];

  $style = array( // style for barcode
                 'border' => 0,
                 'vpadding' => '0',
                 'hpadding' => '0',
                 'fgcolor' => array(0,0,0),
                 'bgcolor' => false, //array(255,255,255)
                 'module_width' => 1, // width of a single module in points
                 'module_height' => 1, // height of a single module in points
                  );

  $pdf = new r\Pdf();
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(FALSE);
  $pdf->SetAutoPageBreak(FALSE);

  $pdf->AddPage();
  $pdf->StartTransform();
  $pdf->ScaleXY(100 * 8.5 / $wCard); // widen to page width (for extra resolution)

  $pdf->Image(R_ROOT . "/images/idcard/$bg.png", $m, $m, $wCard, $hCard, '', '', 'L', true); // file, x, y, w, h, type, link, align, resize
  //  $pdf->Image($photo, $m+$mPhoto, $m+$mPhoto, $wPhoto, $wPhoto*4/3, '', '', 'L', true);
  $pdf->Image($photo, $xPhoto, $yPhoto, $wPhoto, $hPhoto, '', '', 'L', true);
  $pdf->Rect($xPhoto, $yPhoto, $wPhoto, $hPhoto, 'D'); // x, y, w, h, style, border, fill
  $pdf->say(PROJECT, $xProj, $yProj, '', '', "18;Arial MT Medium;color:white;", 'L');
  
  $pdf->say('&reg;', $m+3.01, $m+.08, 1, '', "4;color:white", 'L');
  $sp = '<span style="font-size:50%;"> </span>';
  $url = str_replace('.', $sp . '<b style="display:inline; font-stretch:expanded; font-size:120%;">.</b>' . $sp, CG_DOMAIN);
  $pdf->say($url,  $xSite, $ySite, '', '', "7;color:white;font-stretch:normal;", 'L');
  $pdf->write2DBarcode(a\qr($a), 'QRCODE,Q', $m+1.445, $m+0.645, .8, .8, $style, 'N'); // L,M,Q,H are low-high error-correction

  $pdf->say($acctName, $m+$xText, $m+$yName, '', '', "12;B;$nameFont", 'L');
  if ($proSe) $pdf->say($qid, $m+$xText, $m+$hCard-.19, '', '', "6.5;color:#666666;letter-spacing:.7px;", 'L');
  if (!$proSe) $pdf->say($aa->fullName, $m+$xText, $m+$yName+0.22, '', '', 8, 'L');

  if (NOT_PRODUCTION) $pdf->say(t('TEST'), $xTest, $m+$hCard-.19, .5, '', '6.5;color:darkred', 'C');

  $pdf->StopTransform();
/*  if (now() < strtotime('6/20/2017')) {
    $pdf->Output(strtolower("$qid.cgcard.pd"), 'D'); // Close and output PDF document ('I')
  } else */
  if ($mya and $mya->superAdmin) $a->setBit(B_CARDED);
  if (test()) $pdf->Close(); else $pdf->Output(strtolower("$qid.cgcard.pd"), 'I'); // Close and output PDF document ('I')
//  if (now() < strtotime('6/20/2017') and $next = nextMember($id)) w\go("print-rcard/$next");
}

function formSMS($form, &$sta) {
  $title = t('Test SMS');
  $myNumber = svar('lastSMS');
  $number = phoneFld(t('From (number): '), '', required($myNumber));
  $message = textFld(t('Message: '));
  $submit = t('Send it!');

  focusOn('message');
  $form = compact(ray('title number message submit'));
  return cgform($form);  
}

function formSMS_submit($form, $sta) {
  global $sms_devel; $sms_devel = TRUE;
  extract(just('number message', $sta['input']));
  $number = u\fmtPhone($number, '+n');
  svar('lastSMS', $number);
  \rsms_sms_incoming('process', $number, $message);
}

/**
 * Log back in to admin automatically (called after testing, in case session got destroyed).
 */
function formTestResults($form, &$sta, $args = '') {
  if (!isDEV) return;
  extract(just('top', $args, NULL));
  
  r\setAcct(1, $sta);
  test(FALSE);

  $q = db\q('SELECT * FROM test WHERE type IN (:types) ORDER BY id', ['types' => ray('status error warning')]);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $list[] = "<div class=\"$type messages\">$value</div>\n";
  }
  $list = item(join('', nn($list, [])), BARE);
  \drupal_get_messages(); // throw out any existing messages
//  $title = $top ? t('Tests') : NULL;

  return cgForm(compact(ray('list')), 'wide');
}

/**
 * Delete an account (superAdmin only)
 * @param mixed $uid: account to delete (uid, qid, acct or 'this')
 */
function deleteAccount($uid) {
  global $mya;

  if (is_object($a = $uid)) $uid = $a->id;
  if ($uid == 'this') {
    $uid = $mya->id;
  } elseif (!is_numeric($uid)) { $uid = r\acct(strtoupper($uid))->id; } // qid
  
  if (!$uid) return w\softErr(t('You must not delete account zero.'));
  if ($uid == $mya->id) r\setAcct(1); // don't leave us on a deleted account (switch to superAdmin)
  a\deleteUid($uid);
  return w\go('/sadmin/followup');
}

/**
 * Check database sanity.
 */
function checkSanity() {
  be\checkSanity();
  say('Sanity check complete');
  return w\go('sadmin');
}

function util($arg) {
  switch ($arg) {
  case 'reset-menu':
    w\uncacheSubmenus();
    db\q("DELETE FROM menu_router WHERE page_callback LIKE 'CG%'"); // otherwise we get leftover junk
    db\q("DELETE FROM menu_links WHERE module='rweb'");
    \menu_rebuild();
    break;
  case 'remove-test-data':
    if (isDEV) {
      f('t.clear'); 
      say(t('All test data has been deleted.'));
    } else { a\setupBasicAccounts(); }
    break;
  case 'clear':
    foreach (ray('cache cache_bootstrap cache_form cache_menu sessions') as $table) {
      db\q("TRUNCATE TABLE $table");
    }
    break;
  case 'php':
    w\go('devel/php');
    break;
  default:
    break;
  }
  say("DONE with \"$arg\".");
  return w\go('sadmin');
}

function formHandy($form, &$sta) {
  global $base_path, $base_url, $base_root;

  $links = [
    'sadmin/check-sanity|Check database integrity|Sanity Check|ok',
    'rcredits/util/reset-menu|Rebuild menu based on hook_menu() and revert any custom changes. All menu items return to their default settings.|Redo MENUS|menu-hamburger',
    'rcredits/util/remove-test-data|Remove test data|REMOVE test data|fast-backward',
    'rcredits/util/clear|Empty the cache and sessions tables|Clear Cache & Sessions|unchecked',
    'sadmin/variables|Edit and delete site variables.|Variables|copy',
  ];

  foreach ($links as $link) {
    list ($url, $detail, $label, $icon) = u\padRay(explode('|', $link), 4);
    if (!$label) $label = $detail;
    $handy[$url] = ['', $label, '', ANY, '', $detail, $icon];
  }
  return w\subMenu(t('<h2>Handy Links</h2>'), $handy, 'handy');
}

/**
 * Display variables
 */
function formVariables() {
  $header = array(
    'name' => array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc'),
    'value' => array('data' => t('Value'), 'field' => 'value'),
    'length' => array('data' => t('Length'), 'field' => 'length'),
    'edit' => array('data' => t('Operations')),
  );

  $rows = db\rows('name,LENGTH(value) AS length', 'variable', '1 ORDER BY name');

  foreach ($rows as $row) {
    // $variables[$row->name] = '';
    $options[$row->name]['name'] = check_plain($row->name);
    $value = getv($row->name);
    $complex = (is_array($value) or is_object($value));
    if ($complex) {
      $value = str_replace("\n", "<br>\n", pr($value));
    } else {$value = check_plain($value);}

    $options[$row->name]['value'] = $value;
    $options[$row->name]['length'] = $row->length;
    $options[$row->name]['edit'] = $complex ? '' : l(t('Edit'), "sadmin/variables/edit/$row->name");
  }
  $form['variables'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No variables.'),
  );

  return $form;
}

function formVariables_submit($form, &$form_state) {
  $deletes = array_filter($form_state['values']['variables']);
  array_walk($deletes, 'variable_del');
  if (count($deletes)) {
    drupal_set_message(format_plural(count($deletes), 'One variable deleted.', '@count variables deleted.'));
  }
}

function formVariableEdit($form, &$form_state, $name) {
  
  $title = t('Edit Variable');
  $subtext = $name;
  
  $value = getv($name, t('not found'));
  
  if (is_array($value) or is_object($value)) {
    $value = item(t('Sorry, complex variable types may not be edited yet.'), t('New value'));
    $name = $new = $submit = NULL;
  } else {
    $value = pr($value);
    $name = hidFld($name);
    $new = areaFld(t('New Value:'), '', dft($value . ''));
    $value = item($value . '', t('Old value:'));
    $submit = submit();
  }
  return cgform(compact(ray('title subtext name value new submit')));
}

function formVariableEdit_submit($form, &$sta) {
  extract(just('name new', $sta['input']));
  setv($name, $new);
  say(t('Saved new value for %name.', compact('name')));
}
