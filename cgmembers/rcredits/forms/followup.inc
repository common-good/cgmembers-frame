<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

include_once R_ROOT . '/admin/admin.inc';

/**
 * Show a list of standard calls to make (to help new members, check-in, ask for donation, etc.)
 * @param bool $one: (optional) <show just my community>
 * @param bool $recent: (optional) <show latest notes first>
 */
function formFollowup($form, &$sta, $args = []) {
  global $base_url;
  global $mya;
  
  if (!$mya or !$mya->admin) return w\go('dashboard');
  
  extract(just('one recent', $args, null));

  $title = t('Admin Action Page');
  $subtext = t('<p>Start with today and work backward. Click an account name.</p><ul><li><b>Live Person.</b> If you get a live person, click "live" for a script. Then, if no followup is needed, make a note about your conversation and click "Auto".</li><li><b>Message #1.</b> If you get a machine, click "msg1" for a Phone Message script, type "PM setup" or "PM welcome" or whatever as a note, and click the "2 days" button.</li><li><b>Message #2.</b> If you already left one message, try 3 more times to catch them live. If you still don\'t catch them click "msg2" for a Phone Message script, then click "email" to send them an Email Message. Type "EM setup" or whatever and click the "7" button if this is for a Setup or Annual check-in or the "Auto" button if it is for a Welcome.</li><li><b>After Setup Help Email.</b> If the email gets no response for a setup help call, keep phoning or emailing at 1-, 2-, or 7-day intervals until you reach them or we close the account.</li><li><b>After Annual Check-in Email.</b> If the email gets no response for an Annual check-in, send a US Mail fund appeal/check-in.</li></ul><p>(comments on this standard procedure are welcome)</p>') . w\btn('/sadmin/followup/one=1', t('Show just my community')) . ' &nbsp; ' . w\btn('/sadmin/followup', t('Show ALL')) . ' &nbsp; ' . w\btn('/sadmin/followup/recent=1', t('Recent notes first'));
  
  $now = now();
  $signupBit = r\allSteps()['signup'];
  $todayEnd = strtotime('tomorrow') - 1;
  list ($where1, $where2) = ($mya->admRegion and empty($one)) ? [1, 1] : [0, "community=$mya->community"];
  $cttys = r\cttyList(); $cttys[r\serverUid()] = t('Seedpack'); // shorten Seedpack name
  
  $by = !empty($recent)
    ? "notes IS NULL OR notes='' OR MID(notes,1,6)='knows ', (MID(notes,6,4)*37+LOCATE(MID(notes,3,3),'JanFebMarAprMayJunJulAugSepOctNovDec'))*32+MID(notes,1,2) DESC"
    : "($todayEnd - IF(tickle>0, tickle, u.created)) * IF(tickle>$todayEnd, -5 * 366 * :DAY_SECS, 1)"; // today's calls first, then yesterday, 2 days ago etc., then tomorrow, the next day, etc.
  if ($mya->admPrintCards) {
    $wantsCardP = 0; foreach(ray(S_CARD) as $step) $wantsCardP |= u\bit(r\allSteps()[$step]);
    $wantsCardCo = 0; foreach(ray(S_CO) as $step) $wantsCardCo |= u\bit(r\allSteps()[$step]);
    $wantsCardQ = "IF(:IS_CO, $wantsCardCo, $wantsCardP)";
    $sql = <<<X
      SELECT DISTINCT u.uid
      FROM users u
      WHERE u.uid>:UID_CANON9
      AND :IS_OK AND NOT :IS_CARDED AND (steps & $wantsCardQ)=$wantsCardQ
      AND NOT :IS_NONUDGE AND NOT (activated and NOT :IS_OK)
      AND NOT IFNULL(notes LIKE '%ZAP%' AND BINARY notes LIKE '%ZAP%', FALSE)
      ORDER BY :IS_CO, fullName
X;
    if ($ids = db\q($sql)->fetchCol()) {
      $cnt = count($ids);
      $ids = join(',', $ids);
      $subtext .= ' &nbsp; ' . btn('/sadmin/print-cards/ids=' . $ids, t('Print Cards (%cnt)', compact('cnt')));
      $where2 .= " AND uid NOT IN ($ids)";
    }
    $by = "(!activated AND u.:IS_MEMBER) DESC, (notes LIKE '%ZAP%' AND BINARY notes LIKE '%ZAP%') DESC, " . $by;
  }
  
  $sql = <<<EOF
    SELECT DISTINCT u.uid, community, tickle, fullName, phone, u.steps, u.created, activated, notes, :IS_CO AS co, t.created AS lastTxTime, :IS_MEMBER AS member, :IS_OK AS ok, :IS_CARDED AS carded
    FROM users u LEFT JOIN (SELECT created,uid1,uid2 FROM txs ORDER BY created DESC LIMIT 1) t ON u.uid IN (t.uid1,t.uid2)
    WHERE u.uid>:UID_CANON9 AND NOT :IS_NONUDGE AND NOT (activated and NOT :IS_OK) AND $where2 AND (tickle>0 OR NOT :IS_MEMBER
      OR t.created<$now-60*:DAY_SECS OR t.created IS NULL OR :isDEV)
    ORDER BY $by
    LIMIT 20
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
    u\setDft($notes, '');
    foreach (['tickle', 'activated'] as $k) if ($$k) $$k = strtotime('today', $$k); // round down to nearest day
    $days = round(($tickle - $activated) / DAY_SECS); // tickle: days since activation
    list ($type, $color) = !$activated 
     ? (mb_strpos($notes, 'ZAP') 
       ? [t('DELETE'), 'zap']
       : ($member ? [t('activate'), 'zap'] : (u\getBit($steps, $signupBit) ? [t('setup'), 'setup'] : [t('pre'), 'pre']))
       )
     //    : (round(($activated + TICKLE_WELCOME - $tickle) / DAY_SECS) == 0 ? [t('welcome2'), 'welcome']
     //    : (round(($activated + TICKLE_DELAYED - $tickle) / DAY_SECS) == 0 ? [t('welcome6'), 'welcome']
     : (mb_strpos($notes, 'ZAP') ? [t('DEACTIVATE'), 'zap']
     : (!$carded ? [t('no card yet'), 'welcome']
     : (between($days, TICKLE_WELCOME - 1, TICKLE_WELCOME + 14) ? [t('welcome2'), 'welcome']
     : (between($days, TICKLE_DELAYED - 1, TICKLE_DELAYED + 14) ? [t('welcome6'), 'welcome']
     : (($activated < strtotime('9/2/2017') and $tickle < $activated + 90 * DAY_SECS and !$notes) ? [t('welcome'), 'welcome']
     : (($tickle and $days % 365 < 30) ? [t('annual'), 'annual'] // +/-:2/29
     //    in_array(($tickle - $activated) % DAY_SECS, [0, 1, DAY_SECS - 1])) ? [t('annual'), 'annual'] // +/-:2/29
     : (($lastTxTime < $now - 60 * DAY_SECS and $now - $activated > 60 * DAY_SECS) ? [t('dormant'), 'dormant']
     : ($tickle ? [t('followup'), 'fu']
     : ( [t('other'), 'fu'] )))))))));

    if ($mya->admDeleteAcct and $type == t('DELETE')) $type = w\lnk("$base_url/sadmin/delete-account/$uid", $type);
    if (in($color, $whities)) $color .= ' fu-whitey';
    $type = "<span class=\"fu-$color\">$type</span>";
    $call = fmtDt($tickle ?: $created);
    $name = w\lnk("$base_url/change-account/acct=$uid", $fullName ?: $uid);
    if ($co) $name = "<span class=\"co\">$name</span>";
    $phone = u\fmtPhone(u\decry('P', $phone), '-');
    $notes = strtr($notes, ["\n" => '|| ', date('Y') => '']);
    if ($type == t('pre')) $notes = db\get('code', 'u_relations', 'other=:uid AND :IS_CUSTOMER', compact('uid')) . " $notes";
    $notes = substr($notes, 0, 200) . (mb_strlen($notes) > 200 ? '...' : '');
    $ctty = nni($cttys, $community, '?'); // ? is for dev machine after running a test with ZZA in ctty -2
    $list .= strtr("<tr>$guts</tr>\n", compact($fields));
  }
  if (!isset($tickle)) $list .= t('There are no followup calls to be made.');
  $list .= "</table>\n";
  
  return cgform(compact(ray('title subtext list')), 'wide nolabs');
}
