<?php
/**
 * @file
 * CG admin
 error_reporting(E_ALL);
 ini_set("display_errors", 1);
*/
namespace CG\Admin;

use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Admin as a;
use CG\Web as w;
use CG\Testing as t;
use CG\QR as qr;

/**
 * Set up the basic user accounts and relations on the server.
 * called from installation and test setup
 * To prepare for export for developers, f('t.clear');
 */
function setupBasicAccounts($withEIN = TRUE) {
  $keys = ray('uid community name fullName email country minimum flags');
  $flags = u\bit(B_CO) | u\bit(B_OK);
  
  if (isDEV and !isPRODUCTION) db\q('TRUNCATE users'); // !isPRODUCTION is a safety measure
  if (!db\exists('users', 'uid=0')) db\insert('users', ['uid' => 0], 'uid'); // required for signed-out messages (w\say())
  
  $q = db\q('SELECT region,fullName,st,zips,postalAddr,federalId,hasServer FROM r_regions');
  while ($row = $q->fetchAssoc()) { // create or update all regions
    extract($row);
    if (!$hasServer) continue;
    //    if (isDEV and !in_array($region, [R_SERVER_ID])) continue; // don't slow the tests down
    if ($uid = db\get('uid', 'users', "name=CONCAT(:region, '.')", compact('region'))) {
      r\acct($uid)->update(compact(ray('zips flags' . ($withEIN ? ' federalId' : ''))));
    } else {
      $fullName = $fullName ?: db\get('name', 'r_states', 'abbreviation=:st', compact('st'));
      $email = u\shortName($fullName) . '@' . CG_DOMAIN;
      $fullName = PROJECT . ' ' . $fullName;
      $uid = r\serverUid($region);
      $activated = R_LAUNCH;
      $state = r\stateNum($st);
      $values = array($uid, $uid, $region . '.', $fullName, $email, R_COUNTRY_ID, 0, $flags);
      $info = array_combine($keys, $values);
      list ($address, $city, $zot1, $zot2) = u\parseAddr($postalAddr); // don't overwrite state or zip
      $info += compact(ray('postalAddr address city state activated' . ($withEIN ? ' federalId' : '')));
      $serverA = new r\Acct($info);
      $serverA->update(compact('zips'));
    }
  }
  //$region = db\get("CONCAT(iso_code, 'A')", 'r_countries', 'id=:country', compact('country'));
  if (!isDEV) return; // don't mess with existing members on stage or production server
  
  $values = array(1, 0, 'admin', t('System Administrator'), r\regionField('email'), 0, 0, u\bit([B_ADMIN, B_CO]));
  $info = array_combine($keys, $values);
  $a1 = new r\Acct($info); // set up system admin record (uid=1)

  foreach (ray(t('2:PlaceHolder One,3:PlaceHolder Two')) as $k => $v) {
    $name = str_replace(' ', '', mb_strtolower($v));
    $values = array($k, 0, $name, $v, 'z@o.t', 0, 0, 0);
    $info = array_combine($keys, $values);
    new r\Acct($info);
  }  

  $flagsCo = u\bit([B_MEMBER, B_CONFIRMED, B_OK, B_CO]); // flags for an active company account

  $serverA->update('created', R_LAUNCH); // a date in the past makes tests work better
  if ($cgId = getV('cgf_uid') and db\exists('users', 'uid=' . $cgId)) return; // already done (don't use CGID here)

  $values = array(r\qo('NEWAAB')->id, $serverA->id, u\shortName(PROJECT), PROJECT, CGF_EMAIL, US_COUNTRY_ID, 0, $flagsCo);
  $info = array_combine($keys, $values);
  $logoUrl = preg_replace('~https?://~', '', BASE_URL . '/rcredits/images/logo80.png');
  $info += ray('legalName phone address city state postalAddr zip helper logo',
    CGF_LEGALNAME, u\fmtPhone(CGF_PHONE, '+n'), CGF_ADDRESS, CGF_CITY, R_STATE_ID, CGF_POSTALADDR, CGF_ZIP, 1, $logoUrl);
  $cgf = new r\Acct($info);
  setV('cgf_uid', $cgf->id);
  $cgf->update('emailCode website activated created', '1495kJHm0h145PHh2345h', 'cg4.us', R_LAUNCH, R_LAUNCH);
  if ($withEIN) $cgf->update('federalId', CGF_EIN);
  $cgf->reread(); // keeps tests from dying when using CGF as a funder for doBalance immediately after recreating the db
//    if (!$cgId) w\say(t('CGID had not yet been set up. It is now.'), 'err');

  $canKeys = 'uid name fullName community flags';
  new r\Acct(ray($canKeys, UID_REGULARS, t('regulars'), PROJECT, $cgf->community, $flagsCo));
  new r\Acct(ray($canKeys, UID_ROUNDUPS, t('roundups'), PROJECT, $cgf->community, $flagsCo));
  new r\Acct(ray($canKeys, UID_CRUMBS, t('crumbs'), PROJECT, $cgf->community, $flagsCo));
  new r\Acct(ray($canKeys, UID_STEPUPS, t('stepups'), PROJECT, $cgf->community, $flagsCo));
  new r\Acct(ray($canKeys, UID_BANK, t('cttyfund'), t('Bank'), r\serverUid(), $flagsCo));
  new r\Acct(ray($canKeys, UID_OUTER, t('txs-outer'), t('Txs Outer'), r\serverUid(), $flagsCo));
}

/**
 * Re-setup the test cards on the DEMO server (or other dev server).
 * @param bool $print: if true show corresponding objects for mocking in the app
 */
function makeTestAccounts($print = FALSE) {
  global $mya, $channel;
  if (isPRODUCTION) return w\say(t('Make test accounts only on staging, testing, demo, or dev'), 'err'); // protects even if constant not set
  require_once R_ROOT . '/cg-qr.inc';

  if (!db\exists('users', 'uid=:DEMO_CTTY')) new r\Acct(ray('uid community fullName email flags', DEMO_CTTY, DEMO_CTTY, t('Common Good Example'), R_ADMIN_EMAIL, u\bits('ok co cgc up'))); // create a bogus community

  $testAccts = [
    'a' => [0, 'Abe One'],
    'b' => [0, 'Bea Two'],
    'c' => [1, 'Citre'],
    'd' => [0, 'Dee Four'],
    'e' => [0, 'Eli Five'],
    'f' => [0, 'Flo Six'],
    'g' => [1, 'Gisette'],
    'h' => [0, 'Hal Ate'],
    'i' => [0, 'Ida Nine'],
  ];

  $info0 = ray('dob helper floor pin community activated', strtotime('11/1/2002'), 1, -9999, '1234', DEMO_CTTY, now());
  
  foreach ($testAccts as $id => $vs) {
    $d = (ord($id) - ord("a") + 1);
    $ID = strtoupper($id);
    extract(array_combine(ray('co fullName'), $vs));
    $uid = t\uid(strtoupper(TEST_QID_HD . $id));
    $testUids[] = $uid;
    
    $name = u\shortName($fullName);
    $emailCode = $cardCode = "12345$id";
    $flags = u\bits('ok member confirmed carded ided debt');
    if ($co) $flags |= u\bit('co');
    $email = "$id@example.com";
    $phone = u\fmtPhone("700-000-000$d");
    $federalId = "00000000$d";
    $location = "{$ID}ton, MA";
    $postalAddr = "10$d Demo St., $location 1000$d";
    list ($address, $city, $state, $zip) = u\parseAddr($postalAddr);
    $state = r\stateNum($state);
    $info = $info0 + compact(ray('uid fullName name flags cardCode emailCode email phone federalId postalAddr address city state zip'));
    if (!$co) $info['pass'] = 'k';

    if ($a = a($uid)) $a->update($info); else $a = new r\Acct($info);
    if (!$a->co) a\photoFromFile($a);

    $selling = 'null';
    if ($id == 'c') a(TEST_QID_HD . 'c')->update('selling', $selling = "groceries\ngifts\nsundries");
    if ($id == 'g') a(TEST_QID_HD . 'g')->update('selling', $selling = 'hardware');
    
    if ($print) {
      $isCo = $co ? 'true' : 'false';
      $accountId = qr\url($a, 'main');
      list ($name, $deviceId, $qr) = [$fullName, "dev$ID", "qr$ID"];
      $firstName = ray($name)[0];
      $info = compact(ray('name location isCo accountId cardCode deviceId selling'));
      if ($co) $coInfo[$id] = $info;
/**/  echo testAcctLine($firstName, $info);
    }
  }
  
  foreach (str_split('abdefhi') as $id) { // the individual accounts
    $a = a(TEST_QID_HD . $id);
    if (in($id, 'a b f')) {
      $cid = $id == 'f' ? 'g' : 'c';
      $CID = strtoupper("$cid:$id");
      $coA = a(TEST_QID_HD . $cid);
      $coA->setDftAppCans();
      $perm = $id == 'b' ? B_SELL : B_MANAGE;
      if (!r\relation(1, $coA->id, $a->id)) $coA->newRelation(ray('other permission employee otherNum', $a->id, r\perm($perm), TRUE, $id == 'b' ? 2 : 1));
      $a->update('cardCode2', $cardCode2 = "98765$id");
      foreach (ray(S_CO) as $k) $coA->stepDone($k); // this gets done twice when there are 2 agents for a company, but no harm
      if ($print) {
        $firstName = ray($a->fullName)[0];
        $info = $coInfo[$cid];
        $info['accountId'] = qr\url(a($coA->id, $a->id), 'main');
        $info['cardCode'] = $cardCode2;
/**/    echo testAcctLine("$firstName/$coA->fullName", $info);
      }
    }
    foreach (ray(S_CARD) as $k) $a->stepDone($k);
  }
  
  db\del('txs', 'uid1 IN (:testUids) OR uid2 IN (:testUids)', compact('testUids'));
  db\del('txs2', 'payee IN (:testUids)', compact('testUids'));
  db\del('tx_requests', 'payee IN (:testUids)', compact('testUids'));
  db\q('UPDATE users SET balance=9999 WHERE uid IN (:testUids)', compact('testUids'));
  
  if ($channel == TX_WEB) w\say(t('Done!'));
}

function testAcctLine($key, $info) { // used only by makeTestAccounts() - DO NOT try to put this inside that function (kills tests that call parent function twice)
  $ray = []; foreach ($info as $k => $v) $ray[] = "$k:" . (in($v, 'true false null') ? $v : "'$v'");
/**/  echo "'$key': { " . join(', ', $ray) . " },<br>\n";
}

/**
 * Assign the given account a new id (or delete it).
 * @param int $oldUid: record ID of the account to change or delete
 * @param int $newUid: the new record ID to give the account (defaults to next available)
 * @param bool $unusable: whether to mark the old account record ID as unusuable
 * @param bool $delete: whether to delete the account rather than giving it a new ID (called by a\deleteAccount); -1 means undelete
 * @return TRUE if successful else NULL
 */
function deleteUid($oldUid) {return a\changeUid($oldUid, 0, FALSE, TRUE);}
function unDeleteUid($oldUid) {return a\changeUid($oldUid, 0, FALSE, -1);}

function changeUid($oldUid, $newUid = 0, $unusable = FALSE, $delete = FALSE) {
  $undelete = ($delete === -1);
  $change = !$delete;
  $delete = ($delete === TRUE);
  
  global $mya;
  if (is_object($oldUid)) $oldUid = $oldUid->id;
  
  $subs = array( // all cg tables are listed, to be sure we don't leave any out (D=deletable)
    // messages
    'admins:uid:D',
    'people:uid',
    // posts
    // post_cats
    // r_areas
    // r_bad
    'r_ballots:voter',
    'r_ballots:proxy',
    // r_banks
    'r_boxes:uid:D',
    'r_changes:uid:D',
    'r_changes:changedBy:D',
    'u_company:uid:D', // this gets tracked
    // r_countries
    'r_criteria:ctty:D',
    'r_do:uid:D',
    'r_events:ctty',
    'r_honors:uid:D', // honors are deletable if donation never completed
    // r_industries
    'r_investments:coid',
    'r_investments:clubid',
    'r_invites:inviter',
    'r_invites:invitee:D',
    'r_ips:uid:D',
    'r_near:uid1:D',
    'r_near:uid2:D',
    // r_nonces
    'r_notices:uid:D',
    // r_options
    // r_pairs
    'u_photo:uid:D', // this gets tracked
    'r_proposals:ctty',
    'r_proxies:person:D',
    'r_proxies:proxy:D',
    // r_questions
    'r_ratings:uid',
    // r_regions
    'u_relations:main:D', // this gets tracked
    'u_relations:other:D', // this gets tracked
    // r_states
    // r_shares',
    'r_stakes:uid',
    'r_stakes:clubid',
    'r_stats:ctty:D',
    'r_tous:uid:D',
    // r_transit
    // r_usd2
    'r_user_industries:uid:D',
    // r_votes
    'sessions:uid:D',
    // tx_bads
    'tx_disputes:uid',
    'tx_disputes:agentUid',
    'tx_entries:uid',
    'tx_entries:agentUid',
    'tx_hdrs:actorId',
    'tx_hdrs:actorAgentId',
    'tx_requests:payer:D', // this gets tracked
    'tx_requests:payee:D', // this gets tracked
    'tx_rules:to:D',
    'tx_rules:payer:D',
    'tx_rules:payee:D',
    'tx_rules:from:D',
    'tx_timed:to:D',
    'tx_timed:payer:D',
    'tx_timed:payee:D',
    'tx_timed:from:D',
    'txs2:payee',
    // u_groups
    'u_groupies:uid:D',
    'u_shout:uid:D', // this gets tracked
    'users:jid', // users should be last, because it's the main table for what we're deleting/changing
    'users:uid:D',
    'users:helper',
    'users:community',
    // variable
  );
  
  $DBTX = \db_transaction();

  if ($undelete) {
    $uid = $oldUid;
    if (!db\undel('u_company', compact('uid'))) return dbtxErr($DBTX);
    if (!db\undel('u_photo', compact('uid'))) return dbtxErr($DBTX);
    if (!db\undel('u_shout', compact('uid'))) return dbtxErr($DBTX);
    if (!db\undel('u_relations', ray('main', $uid))) return dbtxErr($DBTX);
    if (!db\undel('u_relations', ray('other', $uid))) return dbtxErr($DBTX);
    if (!db\undel('users', compact('uid'))) return dbtxErr($DBTX);
    return w\say(t('Recovered!'));
  }
   
  if (!$a = r\acct($oldUid)) return w\say(t('That account does not exist.'), 'err');
  if ($mya and $a->id == $mya->id) return w\say(t('You can\'t delete or change the uid of an acct while it\'s the current acct.'), 'err');
  $nm = $a->fullName;
  $oldQid = r\qid($oldUid);
  
  if ($delete) {
    foreach (ray('jid helper') as $fnm) db\q("UPDATE users SET $fnm=0 WHERE $fnm=:oldUid", compact('oldUid')); // don't leave a hanging partner or helper
    db\q('UPDATE people SET uid=NULL WHERE uid=:oldUid', compact('oldUid'));
  }
  
  if ($delCtty = ($delete and $a->isCtty)) { // deleting a community (merge into Seedpack)
    db\del('users', 'uid', $oldUid);
    $unusable = $delete = FALSE; // don't actually delete anything except the account record
    $newUid = r\serverUid(); // Seedpack uid
  }

  if ($change and !$newUid) {
    $region = $a->zip ? r\region($a->country, $a->state, $a->zip) : r\communityId();
    $newUid = r\Acct::nextId($region);
  }

  foreach ($subs as $one) {
    list ($table, $field, $deletable) = explode(':', $one . '::'); // if deleting and not deletable, set to zero
    if ($delCtty and $table == 'users') continue; // for ctty deletion, skip users (already done)
    
    if ($delete) {
      if ($deletable) {
        db\del($table, $field, $oldUid);
      } elseif (db\exists($table, "`$field`=:oldUid", compact('oldUid'))) {
        $DBTX->rollback(); 
        return w\say(t('Account "%nm" has record(s) in table %table (field %field), so it cannot be deleted.', compact(ray('nm table field'))), 'err');
      }
    } else db\q("UPDATE $table SET `$field`=:newUid WHERE `$field`=:oldUid", compact('newUid', 'oldUid'));
  }

  if ($change and $unusable) {
    list ($uid, $name, $email) = array($oldUid, "unusable-$oldQid", "unusable$oldQid@" . CG_DOMAIN);
    db\insert('users', compact(ray('uid name email')), 'uid'); // mark old uid unusable
  }
  
  unset($DBTX); // commit

  if ($change) {
    $newQid = r\qid($newUid);
    foreach ([R_PICTURE_DIR, PHOTOID_DIR . '/'] as $dir) { // original card photo(s) if any AND photo ID file(s) if any
      foreach (glob("$dir/$oldQid.*") as $flnm) rename($flnm, str_replace("/$oldQid.", "/$newQid.", $flnm));
    }
  }

  if ($mya and $mya->admChangeUid) {
    $what = $change ? t('changed to ') . $newUid 
    : ($delete ? t('deleted') : t('recovered'));
    w\say(t('Success. Account %nm has been %what.', compact(ray('nm what'))));
  }
  return TRUE;
}

function dbtxErr($DBTX, $msg = 'FAILED') {$DBTX->rollback(); return w\say($msg);}

/**
 * Return a list of signup steps remaining for the given account.
 */
function showSteps($a) {
  if ($a->task) {
    $steps0 = ray(r\taskSteps($a->task));
  } else $steps0 = array_unique(ray(join(' ', $a->co ? [S_CO] : [S_PERSON, S_CARD, S_SELL, S_VOTE])));
  
  foreach ($steps0 as $step) {
    $class = $a->stepIsDone($step) ? 'step-done' : 'step-not-done';
    $steps[] = "<span class=\"$class\">$step</span>";
  }
  return join(' ', $steps);
}

/**
 * Return a list of possible tasks, highlighting which are active, done, and not done.
 */
function showTasks($a) {
  $help = ray(t('person:minimal individual signup,co:minimal company signup,card:qualifying for a %PROJECT card,sell:qualifying to accept %PROJECT payments as an individual,vote:qualifying (as an individual) to participate in %PROJECT Democracy,partner:minimal signup (individual or company) jointly with a partner organization'));
  foreach (ray($a->co ? S_COTASKS : S_PTASKS) as $task) {
    $class = $task == $a->task ? 'task-active' : ($a->stepsDone($task) ? 'task-done' : 'task-not-done');
    $task = w\popHelp($task, $help[$task]);
    $tasks[] = "<span class=\"$class\">$task</span>";
  }
  return join(' ', $tasks);
}
  

/**
 * Return a description of the account's recent purchase activity (sales activity for companies).
 */
function showActivity($a) {
  $uid = $a->id;
  $when = u\plusMonths(-3);
  $count = $a->co ? db\count('txs_prime','uid2=:uid AND created>:when', compact('uid', 'when'))
  : db\count('txs', 'uid1=:uid AND uid2<>:CGID AND created>:when', compact(ray('uid when')));
  $ret = t('%count %what', 'count what', $count, $a->co ? t('sales') : t('purchases'));
  foreach (ray('created activated signed') as $k) $subs[$k] = $a->$k ? fmtDt($a->$k) : t('NO');
  $subs['use'] = fmtDt($a->ok ? $a->access : $a->login);
  $ret .= t(' in the past 3 months (created %created, signed %signed, activated %activated, last use/signin %use)', $subs);
  return $ret;
}

/**
 * Return a list of the account's relations.
 */
function showRelations($a) {
  global $mya, $base_url;
  $uid = $a->id;
  $jid = $a->jid ?: 0;
  $ret = '';
  $sql = <<<EOF
    SELECT IF(r.main=:uid, other, main) AS them, main, IF(r.main=:uid, u2.fullName, u1.fullName) as who, 
    permission, r.:IS_EMPLOYEE AS employee, r.:IS_OWNER AS owner, r.:IS_DRAW AS draw,
    r.:IS_CUSTOMER AS customer, r.:IS_AUTOPAY AS autopay, u1.:IS_CO AS iCo, u2.:IS_CO AS uCo
    FROM u_relations r LEFT JOIN users u1 ON u1.uid=r.main LEFT JOIN users u2 ON u2.uid=r.other
    WHERE :uid IN (r.main, r.other) AND :uid<>:jid AND (permission OR r.:IS_EMPLOYEE OR r.:IS_OWNER OR r.:IS_DRAW OR NOT r.:IS_CUSTOMER)
EOF;
  $q = db\q($sql, compact('uid', 'jid'));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $permission += B_RELATED; // shift to use bit names
    $extra = '';
    if ($owner and !$iCo and !$uCo) { // family
      $role = t('related to');
      if ($draw) $role .= $uid == $main ? t(' and drawn on by') : t(' and draws on');
    } elseif ($uid == $main) {
      $qo = r\qo($main, $them);
      if ($mya->admManageAccts) $extra = '<small> (' . r\qid($them) . ' / ' . a\cardLink("$main/$them") . ')</small>';
      $role = $owner ? t('owned by')
        : ($draw ? t('drawn on by')
           : ($permission >= B_MANAGE ? t('managed by')
              : (($permission >= B_SELL or $employee) ? t('employs')
                 : ($permission >= B_READ ? t('readable by')
                    : t('probably employs')))));
    } else {
      $role = $owner ? t('owns')
        : ($draw ? t('draws from')
           : ($permission >= B_MANAGE ? t('manages')
              : (($permission >= B_SELL or $employee) ? t('works for')
                 : ($permission >= B_READ ? t('can read')
                    : ($customer ? t('buys from')
                       : t('probably works for'))))));
    }
    $ret .= ($ret ? ', ' : '') . "$role <a href=\"$base_url/change-account/acct=$them\">$who</a>$extra";
  }
  return $ret;
}

define('ID_KEYS', 'id uid uid1 uid2 t.uid myid payer payee main other inviter invitee proxy person owner defaultAgent');

/**
 * Return the given field, formatted for viewing.
 */
function formatField($k, $v, $flagsField = B_LIST) {
  global $base_url;
  $v = $v . ''; // convert to string
  if ($v and in($k, ID_KEYS) and is_numeric($v) and strlen($v) >= strlen(CGID) - 1) { // note that some uids are a little shorter or longer than CGID
    if ($a = r\acct($v)) $v = $k == 'uid' ? w\lnk("$base_url/change-account/acct=$v", $a->name) : ($a->name ?: $v); else $v .= t(' (missing)'); // eg, when uid is changed
  } // member ID. Don't include agent/payerAgent/etc. here
  if ($v and $k == 'channel') $v = u\rayv(ray(TX_CHANNELS), $v);
  if ($v and in($k, R_DATE_FLDS)) {
    $v = '<div class="date-field">' . fmtDt($v, 'ddMMM') . '<span class="year-field">\'' . fmtDt($v, 'yy') . '<br>' . fmtTime($v) . '</span></div>';
  } elseif (in($k, 'flags can')) {
    $flags = a\flagsDesc($v, $k == 'can' ? A_LIST : $flagsField);
    $v = "<div class=\"format-flags\">$flags</div>";
  }
  return $v;
}

/**
 * Display the given table or query.
 * @param string $table: table name
 * @param string $where: criteria
 * @param string $order: row order
 * @param string $fields: list of fields to display
 * @param string $joinFields: list of fields joining to users table (limit by community if called by Ctty Admin)
 * @param string $url: page for editing, followed by /$idFld where $idFld is the name of a clickable id field (for editing)
 * @return: the formatted output
 */
function showQuery($table, $order = '', $fields = '*', $joinFields = '', $url = '') {
  global $mya;
  
  $limit = 500; // max records to show
  $where = 1;
  
  $flagsField = ($table == 'tx_hdrs') ? TX_FLAGS : B_LIST;
  if (!$mya->admRegion and $joinFields) {
    if ($table != 'users') {
      if (!is_array($joinFields)) $joinFields = ray($joinFields);
      $fieldCount = count($joinFields);
      $table .= ' t';
      $crit = null;
      for ($fieldI = 0; $fieldI < $fieldCount; $fieldI++) {
        $table .= " INNER JOIN users u$fieldI ON u$fieldI.uid=t." . $joinFields[$fieldI];
        $crit = (is_null($crit) ? '' : "$crit OR ") . "u$fieldI.community=$mya->community";
      }
      $where .= " AND ($crit)";
    } else { $where .= " AND community=$mya->community"; }
  } else $fields = str_replace('t.', '', $fields);

  if ($order) $where .= " ORDER BY $order";
  return w\showRecords(db\q("SELECT $fields FROM $table WHERE $where LIMIT $limit")->fetchAll(), 'CG\\Admin\\formatField', $flagsField, $url);
}

/**
 * Show results of an SSN search, compared to personal data supplied by the member.
 * Updates the moves field, if it isn't set yet.
 */
function ssnShow($a) {
  if (isDEV) return t('no SSN data');
  
  $ssnData = $a->ssnCheck();
  if (!is_array($ssnData)) return "<p class=\"loud\">$ssnData</p>";
  
  $d = nni($ssnData, 'searchResults') ?: []; // [] for admin without vKeyE
  if ($err = nni($ssnData, SSN_ERR_INDEX)) $rows[] = ["<b class=\"err\">ERROR</b>", $err, '', '', ''];
  $name = $a->fullName . ($a->legalName == $a->fullName ? '' : "<br>$a->legalName");
  $rows[] = array(t('Data Given'), $name, fmtDt(is_numeric($a->dob) ? $a->dob: 0), $a->location(TRUE), '', ''); // is_numeric check for admin without vKeyE
  $name = u\ssnName($one = (object) nn($d['ssnnames']['ssnname']));
  $dob = nn($one->dob) ? fmtDt(strtotime($one->dob)) : ''; // arrives as an empty array if none
  $rows[] = array(t('Found'), $name, $dob, nni($d, 'deathsearchresults') ? '<h1 color="red">DEAD</h1>' : '', '', '');
  
  $counties = [];
  $addresses = nn($d['addresses']['address']) ?: [];
  if (nni($addresses, 'lastname')) $addresses = [$addresses];
  foreach ($addresses as $rec) {
    $rec = (object) $rec;
    $rows[] = array($rec->datereported ? fmtDt(strtotime($rec->datereported)) : '', u\ssnName($rec), $rec->dob ? fmtDt(strtotime($rec->dob)) : '', u\ssnAddr($rec), ucwords(mb_strtolower(pr($rec->county)))); // county was an array once
    if (now() - strtotime(pr($rec->datereported)) < 10 * YEAR_SECS) $counties[pr($rec->county)] = 1;
  }
  if (!$a->moves) $a->update('moves', max(0, count($counties) - 1));
  
  $lines = '';
  foreach ($rows as $rec) {
    list ($head, $name, $dob, $address, $county) = $rec;
    $lines .= "<tr><th>$head</th><td>$name</td><td>$dob</td><td>$address</td><td>$county</td></tr>\n";
  }
  
  return <<< EOF
    <table>
    <tr><th>SSN CHECK</th><th>Name</th><th>BDate</th><th>Address</th><th>County</th></tr>
    $lines
    </table>
EOF;
} 

/**
 * Redo all the stats
 * @param int $keepUpto: date of last valid stats records (redo all subsequent dates)
 */
function fixStats() {
  /*
    db\q('TRUNCATE r_stats');
    f('cr.cttyStats', -1);
  */
}


// MEMBER LIST: SELECT fullName, city FROM users WHERE (zip LIKE '010%' OR zip LIKE '013%') AND NOT (flags&(1<<6)) AND NOT (flags&(1<<9)) ORDER BY city, fullName

/**
 * Read and parse an updated list of financial institution routing numbers into the r_banks table.
 * Download from http://www.fededirectory.frb.org/agreement.cfm
 * branch is o (main office) or B (branch)
 * type is 0=fed bank 1=use routing 2=use newRouting
 include_once R_ROOT . '/admin/admin.inc';
 f('a.getBanks');
*/
function getBanks() {
  return; // seldom used (and move SQL name fixes here? see changes.log)
  $fieldNames = 'route branch fedRoute type modified newRoute name address city state zip phone status view';
  $lens = ray('9 1 9 1 6 9 36 36 20 2 9 10 1 1'); // also 5 chars of filler at end
  $fields = array_combine(ray($fieldNames), $lens);
  db\q('TRUNCATE r_banks');
  
  $s = explode("\n", file_get_contents(R_ROOT . '/misc/FedACHdir.txt'));
  foreach ($s as $line) {
    if (!$line) continue; // handle possible blank line at end
    $i = 0;
    foreach ($fields as $field => $len) {
      $$field = substr($line, $i, $len);
      $i += $len;
    }
    $branch = $branch == 'B' ? 1 : 0;
    foreach (ray('name address city') as $field) $$field = ucwords(mb_strtolower(trim($$field)));
    db\insert('r_banks', compact(ray($fieldNames)), 'route');
  }
}

/**
 * Create a smaller photo to send to POS devices
 * CALLED ONLY FROM a\makeTestAccounts()!
 * @param acct $a: the account whose photo is to be shrunk
 * @param string $err: (RETURNED) the error message, if any
 * @return <success>
 include_once R_ROOT . '/admin/admin.inc';
 debug (f('a.photoFromFile', a('aaa'), $err));
 debug ($err);
 currently called only from a.makeTestAccounts()
*/
function photoFromFile($a, &$err = '') {
  if ($a->co) return !$err = 'tried to shrink a company photo';
  //  if ($a->hasPhoto('picture2')) return !$err = 'POS photo already exists';
  if ($a->photo) $err = 'POS photo already exists'; // not really an error
  $photoFlnm = R_ROOT . "/images/test-faces/$a->name.jpg";
  $img = u\alterImg(imagecreatefromjpeg($photoFlnm), R_PHOTO_WIDTH, R_PHOTO_HEIGHT);
  ob_start();
  imagejpeg($img, NULL, 50);
  $photo = ob_get_clean();
  //if (!$photo = u\fixPicture($photoFlnm, FACE_ASPECT, R_PIXEL_FACTOR / 2, FALSE, $err)) return FALSE;
  return $a->update(compact('photo'));
}

/**
 * Move a field out of data or secure and into its own field in users.
 * (call after reinstall)
 include_once R_ROOT . '/admin/admin.inc';
 f('a.moveFieldFrom', 'helper', 'data');
*/
function moveFieldFrom($field, $from = 'data') {
  $uids = db\q('SELECT uid FROM users WHERE uid<0 OR uid>:UID_CANON9')->fetchCol();
  foreach ($uids as $uid) {
    $a = r\acct($uid);
    $ray = $a->$from;
    $$field = nni($ray, $field); // not $$from[] because PHP attaches the index first
    if ($field = 'helper') {
      $$field = $$field ? r\acct($$field)->id : 0;
    }
    unset($ray[$field]);
    $$from = $ray;
    $a->update(compact($from, $field));
  }
}

/**
 * Move a field into data or secure in users.
 include_once R_ROOT . '/admin/admin.inc';
 f('a.moveFieldTo', 'question', 'secure');
*/
function moveFieldTo($field, $to = 'data') {
  $q = db\q("SELECT uid,$field FROM users WHERE uid<0 OR uid>:UID_CANON9");
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($uid);
    $v = $a->$to;
    $v += compact($field);
    $a->update($to, $v);
  }
}

/**
 * General purpose utility, usually called from eachAcct, as above.
 * For example:
 *    changeIt($a);
 include_once R_ROOT . '/admin/admin.inc';
 foreach (array(3201932, 3244519, 3350610, 3510402) as $one) {
 f('a.changeIt', $one);
 OR changeIt(a('aaa'));
 }
*/
function changeIt($a) {
  if ($a->bankAccount) $a->setRisk('hasBank');
  $a->setBit(4, FALSE);
}

/**
 * Return a readable list of names of bits set in the given integer bit array.
 * @param int $n: the bit array
 * @param string $list: delimited list of bit names
 * @return list of bit names set in $n
 */
function flagsDesc($n, $list) {
  $bits = ray($list);
  if (!is_numeric(key($bits))) $bits = array_keys($bits);
  for ($i = 0; $i < 32; $i++) if (!nni($bits, $i)) $bits[$i] = $i; // show bit number if no name
  $res = '';
  for ($i=0; $i < count($bits); $i++) if ($n & u\bit($i)) {$res .= $bits[$i] . ' ';}
  return $res;
}

/*
  function moveFieldsToHidden() {
  $q = db\q("SELECT uid FROM users WHERE uid<>0");
  $dataFields = [];
  $secureFields = ray('photoId pin bankAccount');
  while ($row = $q->fetchAssoc()) {
  extract($row);
  $a = r\acct($uid);
  if ($a->pin or $a->bankAccount) continue; // don't do this twice (and overwrite with nulls)!
  $data = $a->data;
  $secure = $a->secure;
  foreach ($dataFields as $k) $$k = @$data[$k];
  foreach ($secureFields as $k) $$k = @$secure[$k];
  $data = $secure = [];
  foreach (ray(DATA_FLDS) as $k) if (!is_null($a->$k)) $data[$k] = $a->$k;
  foreach (ray(SECURE_FLDS) as $k) if (!is_null($a->$k)) $secure[$k] = $a->$k;
///    debug($row + compact($dataFields, $secureFields, 'data', 'secure'));
  $a->update(compact($dataFields, $secureFields, 'data', 'secure'));
  }
  }
*/

/**
 * Make sure the very secure fields are encrypted with the admin's extra password.
 *//*
     function secureVery($a = '') {
     if (!r\acct()->admV) return; // don't bother if not super
     if ($a) {
     if ($vflds = just(VSECURE_FLDS, $a->secure)) return $a->update($vflds); // extra encryption
     } else {
     $q = db\q('SELECT uid FROM users WHERE :IS_OK');
     while ($row = $q->fetchAssoc()) secureVery(r\acct($row['uid']));
     }
     }*/

/**
 * Show sluggish accounts
 */
/*
  function showSlugs() {
  $q = f('db.q', 'SELECT u.uid, u.fullName, (SELECT created FROM r_txs WHERE payer=u.uid AND payee>0 AND payee<>26742000000002 ORDER BY created DESC LIMIT 1) AS lastUse, u.activated FROM users u WHERE u.uid>0  AND community=-26742000000001 ORDER BY lastUse, u.activated');
  while ($row = $q->fetchAssoc()) {
  extract($row);
  $a = a($uid);
  if ($a->ok) {
  $ray = [$a->fullName, f('u.fmtPhone', $a->phone), $a->email, @$lastUse ? f('u.fmtDate', $lastUse) : t('never'), '(' . f('u.fmtDate', @$activated) . ')'];
/** /    echo join("\t", $ray) . "\n";
  }
  }
  }
*/

/* // print iCards for all active members in the greater Greenfield ctty */
/* // be sure to chmod ...rcredits/temp to 777 before running this. */
/* function special() { */
/*   return; */
/*   eachX(function($x) { */
/*       foreach (ray('type payer payee goods flags taking amount payeeFor') as $k) $$k = $x->$k; */
/*       if ($payeeFor == 'contribution (percentage of past month receipts)' and ($payee < 0 or $payee == CGID)) { */
/*         foreach ([B_GIFT, B_CRUMBS] as $k) u\setBit($flags, $k); */
/*         $payee = CGID; */
/*       } elseif ($payeeFor == 'contribution of rounded-up payment change' and ($payee < 0 or $payee == CGID)) { */
/*         foreach ([B_GIFT, B_ROUNDUPS] as $k) u\setBit($flags, $k); */
/*         $payee = CGID; */
/*       } elseif ($payee == CGID and $payer > 0) { */
/*         foreach (ray('ontribution onation everses') as $k) if (strpos($payeeFor, $k)) { */
/*           if (strpos($payeeFor, 'everses')) u\setBit($flags, B_UNDOES); */
/*           if (strpos($payeeFor, 'eversed')) u\setBit($flags, B_UNDONE); */
/*           u\setBit($flags, B_GIFT); */
/*           if (strpos($payeeFor, 'onation (')) u\setBit($flags, B_RECURS); */
/*         } */
/*       } */
/*       if ($flags & u\bit(B_GIFT)) $x->update(compact(ray('flags payee'))); */
/*     }); */
/* } */

function showInactive() {
  $ctty = a('aaa')->community;
  $q = f('db.q', 'select uid from users where community=:ctty order by access', compact('ctty'));
  while ($row = $q->fetchAssoc()) {
    $a = a($row['uid']);
    if ($a->slave) continue;
    $dt = f('u.fmtDate', $a->access);
    $ok = $a->ok ? '(member)' : '';
/**/ echo "$a->phone\t$a->fullName\t$dt\t$ok\n";
  }
}

function cardLink($link, $make = FALSE) {return spinLink("/print-rcard/$make/$link", t('Card'), '', 'primary', 'xs', w\away());}
function photoLink($recrop = 0) {return spinLink("/settings/photo/recrop=$recrop", $recrop ? t('Recrop Photo') : t('Photo'), '', 'primary', 'xs');}

function nextMember($id = 0) {
  $members = db\col('uid', 'users', ':IS_OK AND NOT :IS_CO AND activated<1486098000 ORDER BY community, uid');
  $i = $id ? array_search($id, $members) : -1;
  return nni($members, $i + 1);
}

/**
 * Return statistics about member donations for members signed up after a certain date.
 * This function is currently called only from a PHP window.
 */
function donationTrend($co = FALSE, $since = NULL) {
  u\setDft($since, now() - YEAR_SECS);
  $co = $co ? 'u.:IS_CO' : 'NOT u.:IS_CO';
  $where = "$co AND u.:IS_OK AND u.created>:since";
  
  $sql =<<< X
    SELECT AVG(g.amount*perYear(period, periods)) AS regular
    FROM tx_timed g LEFT JOIN users u ON u.uid=g.from
    WHERE g.action=:ACT_PAY AND g.to=:CGID AND $where
X;
  $subs = compact('since');
  $res = db\q($sql, $subs)->fetchAssoc();
  
  $sql = "SELECT AVG(u.:IS_ROUNDUP) AS roundupRatio, AVG(u.crumbs) AS crumbs FROM users u WHERE $where";
  $res += db\q($sql, $subs)->fetchAssoc();
  
  $accts = db\count('users u', $where, $subs);
  
  $sql = "SELECT SUM(amt) AS txTotal, COUNT(*) AS txCount, AVG(amt%1) AS cents FROM txs_prime t JOIN users u ON u.uid=uid1 WHERE $where";
  $res += db\q($sql, $subs)->fetchAssoc();

  return $res + compact(ray('accts txCount txTotal'));
}

/**
 * Return a random name.
 */

define('vowels', 'aeiou');
define('consonants', 'bcdfghjklmnpqrstvwxyz');
define('vowels2', 'ar er ir or ur ay ey oy aw ew ow');

function randomName() {
  $cons = !u\prob(1/8); // decide whether to start with consonant(s) or vowel(s)
  $res = ucfirst(cOrV($cons)) . cOrV(!$cons); // first two characters
  if (u\prob(1/16)) return $res;
  $res .= cOrV($cons);
  if (u\prob(1/4)) return $res;
  $res .= cOrV(!$cons);
  $p = .25;
  while (u\prob($p)) {
    $res .= cOrV($cons);
    $cons = !$cons;
    $p /= 2; // extra letters are increasingly unlikely
  }
  return $res;
}

/**
 * Return a consonant or vowel or combination.
 */
function cOrV($cons) {
  return $cons ? getCons() : getVow();
}

function getCons($p2 = 1/3) {return randC(consonants) . ($p2 ? u\prob($p2, getCons(0)) : '');}
function getVow($p2 = 1/3) {return u\prob(1/8, getVow2(), randC(vowels) . ($p2 ? u\prob($p2, getVow(0)) : ''));}
function randC($s) {return $s[rand(0, strlen($s) - 1)];}
function getVow2() {$ray = ray(vowels2); return $ray[rand(0, count($ray) - 1)];}
function randDigits($digits = 10) {return u\randomString($digits, 'digits');}
function randRouting() {return db\get('route', 'r_banks', 'LENGTH(route)=9');}
function wordName($len = 1) {return ucwords(u\randPass($len));}

/**
 * Randomize personal data in all accounts (for staging site). Here's how:
 * A. Make sure you have a current backup of the database and the database is disabled
 *     (that is, no possible changes by web interface, donation links, or app API -- except by you).
 * B. If you are downloading the anonymized data to give to someone for development or analysis, prepare to re-encrypt.
 *   - Make a backup copy of the cgAdmin.html file that you have on your flash drive.
 *   - Copy the lowercase encryption values from misc/config-models/config.json into the same place in config/config.json.
 *   - If you are anonymizing a deployed database, copy cgmembers/rcredits/misc to the server (so v.privk and vKeyPw are accessible).
 * C. ANONYMIZE.
 *   - Sign in and scan your SuperAdmin QR code.
 *   - Select PHP on the Admin menu. Enter the following in the box and press "Execute Code":

      f('a.anonymize', FALSE); // change FALSE to TRUE if changing the encryption
     
 * D. Followup if changing encryption:
 *   - Sign out. Delete (or back up) the old encryption values and change the new ones to uppercase (c=>C, h=>H, etc.)
 *   - Change the default value of vKeyPw in cgAdmin.html (on your flash drive) to match the one in misc/config-models/cgAdmin-dev.html.
 *   - Sign in as admin again by clicking the button in cgAdmin.html.
 *   - Run encryption tests on the PHP page as follows:

      foreach (ray('C H P R S V') as $k) {
       $v = u\cry($k, 'Success!');
       w\say("k=$k " . u\deCry($k, $v));
      }
  
  * E. Verify that values have been anonymized. Paste this into the PHP box and press "Execute Code":

      $a = a('NEWAAB'); w\say("CG email=$a->email phone=$a->phone address=$a->address"); w\say("CG ein=$a->federalId");
      $a = a('NEWAAA'); w\say("$a->mainQid email=$a->email phone=$a->phone address=$a->address"); w\say("$a->mainQid ein=$a->federalId");
      
  *   The Common Good values should remain unchanged, the ones for NEWAAA should be random.
        
  * F. If you are giving the anonymized data to a developer (internal or external), give them just the database
  *    (the model config.json file in cgmembers/rcredits/misc/config-models/ will work).
 */
function anonymize($encrypting = FALSE) {
  if (isPRODUCTION) return;
  global $channel; $channel = TX_WEB;
  global $cryHows;
  
  if ($encrypting and !array_key_exists('p', $cryHows)) return w\softErr(t('You must include lowercase new encryption types in config.json.'));; 
  if (!t\deCryVok()) return w\softErr(t('You must scan yourself in as a Superadmin before anonymizing.'));
  
  flog('Anonymizing...');
  w\say('Scroll down to make sure the process completed and see next steps.');
  
  $uids = db\col('uid', 'users', 'uid<0 or uid=1 or uid>:UID_CANON9 ORDER BY uid'); // order by uid in case we get only part way through (it doesn't change)
  foreach ($uids as $uid) {
    $a = r\acct($uid);
    flog($a->mainQid); w\say($a->mainQid); // remember where we left off, in case this fails

    foreach (ray('fullName legalName name shortName email phone address city postalAddr yob dob federalId') as $k) $$k = $a->$k; // keep these for a moment in case CG or owned by CG

    if (!in($uid, [CGID, UID_SUPER])) {
    // the fields shared with the people table (in case we can refactor)
      $fullName = $uid == r\serverUid() ? t('Anonymous %PROJECT Region')
        : ($uid < 0 ? PROJECT . ' ' . wordName()
        : ($a->co ? (wordName() . ' Co') : wordName(2)) );
      $email = $a->email ? (mb_strtolower(randomName()) . '@example.com') : NULL;
      $phone = $a->phone ? ('+1' . mt_rand(2, 9) . randDigits(9)) : NULL;
      $address = $a->address ? (mt_rand(1, 9999) . ' ' . wordName() . ' St.') : NULL;
      $city = $a->city ? wordName() : NULL;
      $latitude = 41 + 3 / mt_rand(0, mt_getrandmax()); // in or near Massachusetts
      $longitude = 70 + 3 / mt_rand(0, mt_getrandmax());

      $name = u\shortName($fullName);
      $shortName = $a->shortName ? strstr($fullName, ' ', TRUE) : '';
      $postalAddr = $a->risk('poBox') ? 'PO Box ' . mt_rand(1, 99999) : $address;
      $postalAddr .= ", $a->st $a->zip";
      $yob = date('Y') - mt_rand(18, 100);
      $dob = mktime(0, 0, 0, mt_rand(1,12), mt_rand(1,31), $yob);

      if (!in($uid, r\catUids())) { // unless owned by CG change legal name and EIN
        $federalId =  $a->hasFid ? randDigits(9) : ''; // don't change basic CG-owned EIN
        $legalName = mt_rand(1,100) < 90 ? $fullName : (wordName() . ' ' . randomName()[0] . '. ' . strstr($fullName, ' '));
      }
    }

    $signedBy = $a->signedBy ? $fullName : '';
    $faxetc = '';
    $pin = randDigits(4);
    $snap = $a->snap ? randDigits() : '';
    if ($signupCo = $a->signupCo) { $signupCo['company'] = wordName() . ' Co'; $signupCo['companyPhone'] = '+1' . mt_rand(2, 9) . randDigits(9); }
    if (!$a->co) $pass = '123'; // make it easy to sign into any account
 
    foreach (ray($randStrings = 'cardCode cardCode2 emailCode question answer idProof') as $k) $$k = u\randomString(12, 'word');
    foreach (ray($empties = 'notes photo appData ssnData partnerInfo') as $k) $$k = '';
    $flds = "$randStrings $empties fullName legalName name shortName email address city postalAddr signedBy latitude longitude dob yob phone faxetc federalId pin snap signupCo";

    if (!$a->slave and $a->hasBank) { // master of joint account with bank account
      $flds .= ' bankAccount last4bank';
      $bankAccount = 'USkk' . randRouting() . randDigits(10);
      $last4bank = substr($bankAccount, -4, 4);
    }

    $a->update(compact(ray($flds)));
    
    if ($encrypting) { // if new encryption methods exist
      foreach (ray('secure vsecure') as $k) $$k = $a->$k;
      foreach ($vsecure as $k => $v) if (u\crypted('V', $v)) $vsecure[$k] = CRYPT_FLAG . 'V' . substr(u\cry('v', u\deCry('V', $v)), strlen(CRYPT_FLAG) + 1);
      foreach (ray('email phone') as $k) if (isset($$k)) $$k = CRYPT_FLAG . 'P' . substr(u\cry('p', $$k), strlen(CRYPT_FLAG) + 1);;
      foreach (ray('secure vsecure') as $k) {
        $crypt = u\cry('s', serialize($$k));
        $$k = CRYPT_FLAG . 'S' . mb_substr($crypt, strlen(CRYPT_FLAG) + 1);
      }
      db\update('users', compact(ray('uid email phone secure vsecure')), 'uid');
    }
  }
  
  $flds = 'pid,source,displayName,fullName,email,phone,address,city,latitude,longitude';
  $q = db\q("SELECT $flds FROM people ORDER BY fullName");
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $source = $displayName = '';
    
    $email = $email ? (mb_strtolower(randomName()) . '@example.com') : NULL;
    $phone = $phone ? ('+1' . mt_rand(2, 9) . randDigits(9)) : NULL;
    $address = $address ? (mt_rand(1, 9999) . ' ' . wordName() . ' St.') : NULL;
    $city = $city ? wordName() : NULL;
    $latitude = $longitude = 0;
    $fullName = wordName(2);

    db\update('people', compact(ray($flds)), 'pid');
  }
  
  db\q('UPDATE txs2_bank SET bankAccount=:ba', ['ba' => 'USkk' . randDigits(19)]);
  db\q('UPDATE r_invites SET email=NULL');
  db\q("UPDATE u_company JOIN users USING(uid) SET contact=NULL, website=CONCAT(name, '@example.com')");
  foreach (ray('cache cache_bootstrap cache_form cu_folders cu_lists cu_members cu_spaces cu_tasks cu_times legacy_x_invoices legacy_x_txs messages r_boxes r_changes r_do r_honors r_invites r_near x_company x_photos x_relations x_shout x_txs2 x_users') as $k) db\q("TRUNCATE $k");
  
  if ($encrypting) { 
    $vKeyPw = t\setDevVKey(a(UID_SUPER), TRUE); // this fails, so just delete all admin records and create admin's vKeyE automatically on startup
    db\q('DELETE FROM admins'); // should be db\q('DELETE FROM admins WHERE uid<>:UID_SUPER'); (make the previous line work)
    a(1)->update('pass', '123');
    $pass = db\get('pass', 'users', 'uid=1');
    db\q('UPDATE users SET pass=IF(:IS_CO, NULL, :pass) WHERE uid>:UID_CANON9', compact('pass'));
    w\say(t('<h2>Done anonymizing!</h2>
      <p>Change the default vKeyPw in cgAdmin.html on your flash drive to this: "%vKeyPw" (the same as in misc/config-models/cgAdmin-dev.html).</p>
      <p>Change config.json to use the new encryptions, then SIGN OUT and sign back in.</p>
      <p>Verify that accounts have random address, email, phone, and dob.</p>', compact('vKeyPw')));
  } else w\say(t('Done anonymizing! Same encryption used. SIGN OUT and sign back in.'));
}

/**
 * Export solar1 data to SMT
 */
function toSMT() {
  $q = db\q('SELECT * FROM users WHERE uid>:UID_CANON9');
  $hdrDone = FALSE;

  while ($row = $q->fetchAssoc()) {
    extract(just('uid', $row));
    $a = r\acct($uid);
    if ($a->closed) continue;
    foreach (ray('isSavings fullName postalAddr tenure owns partnerInfo created notes') as $k) $$k = $a->$k;
    if (!$partnerInfo) continue;

    if (!$a->hasBank) continue;
    list ($routing, $bankAccount) = u\parseBankAccount($a->bankAccount);
    if (empty($bankAccount)) continue;
    
    u\setDft($smtid, 0); // for testing
    u\setDft($noContract, 0); // for testing
    
    $notes = strtr($notes, ["\r" => '', "\n" => '']);
    $nameOnBankAcct = $fullName;
    list ($billingAddress, $billingCity, $billingState, $billingZip) = u\parseAddr($postalAddr);
    $project = NULL; // null out previous value
    extract($partnerInfo ?: []);
    u\setDft($project, $source == 'solar1' ? 'ssps' : 'cooppower');
    $cgAccount = $a->member ? $a->mainQid : '';
    $phone = u\fmtPhone($phone, '-');
    $out = compact(ray('smtid pid project fullName orgName m_company m_number cgAccount phone email address city state zip tenure owns routing bankAccount isSavings nameOnBankAcct billingAddress billingCity billingState billingZip noContract created notes'));
///  if ($a->member and $bad = db\get('uid', 'users', "fullName='$a->fullName' AND NOT :IS_MEMBER")) debug("maybe delete $bad ($a->uid is good) -- $a->fullName");
    if (!$hdrDone) {
      u\startDown('solar1-export-' . fmtDt(now(), 'Ymd'));
      u\csv(array_keys($out), "\t");
      $hdrDone = TRUE;
    }
    u\csv(array_values($out), "\t");
  }
  if (!$hdrDone) return w\go('/sadmin/panel', t('nothing to send to SMT'));
  exit();
}

/**
 * Export a list of major donors, prioritized by probable donation size.
 */
function majorDonors() {
  $q = db\q('SELECT uid FROM users WHERE :IS_OK AND :MASTER AND uid>:UID_CANON9');
  $hdrDone = FALSE;

  while ($row = $q->fetchAssoc()) {
    extract(just('uid', $row));
    $a = r\acct($uid);
    $out = $a->majorInfo();
    if (!$hdrDone) {
      u\startDown('major-donors-' . fmtDt(now(), 'Ymd'));
      u\csv(array_keys($out), "\t");
      $hdrDone = TRUE;
    }
    u\csv(array_values($out), "\t");
  }
  if (!$hdrDone) return w\go('', t('no active users'));
  exit();
}

/**
 * Move customer transactions to a subsidiary company. (used 9/2020 for Co-op Power subsidiaries)
 * @param string $coQid: the main company's QID
 * @param string $custQids: space-delimited list of customer's QIDs to move txs and invoices from coQid to co2Qid
 * @param string $co2Qid: the subsidiary company's QID
 */
function moveTxsToSubco($coQid, $custQids, $co2Qid) {
  $coA = r\acct($coQid); $coid = $coA->id;
  $co2A = r\acct($co2Qid); $co2id = $co2A->id;
  foreach (ray($custQids) as $qid) {
    if (!$a = r\acct($qid)) continue;
    $uids[] = $uid = $a->id;
    $xids = db\col('xid', 'txs', ':uid IN (uid1, uid2) AND :coid IN (uid1, uid2)', compact(ray('uid coid')));
    if (!$xids) continue;
    $tableFlds = ray('tx_entries_all:uid agentUid, tx_hdrs_all:actorId actorAgentId, tx_disputes_all:uid agentUid');
    $subs = compact(ray('coid co2id xids'));
    foreach ($tableFlds as $tnm => $flds) {
      list ($f1, $f2) = ray($flds);
      db\q("UPDATE $tnm SET $f1=:co2id, $f2=IF($f2=:coid, :co2id, $f2) WHERE $f1=:coid AND xid IN (:xids)", $subs);
    }
  }

  $subs = compact(ray('coid co2id uids'));
  db\q('UPDATE tx_requests SET payer=IF(payer=:coid, :co2id, payer), payee=IF(payee=:coid, :co2id, payee) WHERE :coid IN (payer, payee) AND (payer IN (:uids) OR payee IN (:uids))', $subs);
  db\q('UPDATE u_relations SET main=:co2id WHERE main=:coid AND other IN (:uids)', $subs);
}
  
/**
 * Calculate how much the CG system has saved local businesses in credit card fees, to date.
 * Call in code window with: d('a.ccFees');
 */
function ccFees() {
  $totals = <<< X
    SELECT SUM(IF(t.type=:E_PRIME, 1, 0)) AS cnt, SUM(t.amt) AS total 
    FROM txs t JOIN users u ON u.uid=t.uid2
    WHERE t.type IN (:E_PRIME, :E_AUX) AND u.:IS_CO
X;
  return u\fmtAmt(db\get('.1 * cnt + .02 * total', "($totals) xx"));
}

/**
 * Clone an account and return the clone.
 */
function cloneA($oldA) {
  $ray = justNOT('uid name pass ' . VSECURE_FLDS . ' ' . SECURE_FLDS . ' ' . PSECURE_FLDS, $oldA->account());
  
  $ray['fullName'] .= '2';
  foreach (ray(PSECURE_FLDS) as $k) $ray[$k] = $oldA->$k;
  $a = new r\Acct($ray);
  foreach (ray('u_photo u_company') as $tnm) {
    if ($ray = db\get('*', $tnm, ray('uid', $oldA->uid))) {
      $ray['uid'] = $a->uid;
      db\insert($tnm, $ray, 'uid');
    }
  }
  return $a;
}

/*
  to find orphaned customers

  eachA(function ($a) {
  $name = str_replace("'", "\\'", $a->fullName);
///  if ($reid = f('db.lookup', 'reid', 'u_relations', "other=0 AND data LIKE '%$name%' AND reid NOT IN(26742000000222, 26742000000216)")) debug("$reid $name $a->id");
  });
*/

/**
 * Compensate for checks inadvertently deposited twice (but not recorded): once on 2/26, once on 3/3.
 * Strategy: create txs2, tx_hdrs, and tx_entries records for the duplicate checks, but with xids 1000 more than 
 * originally. Start with the highest xid so new transactions don't get interspersed. Date should be 2/27. Purpose is
 *  "from bank - duplicate of #xxx (combined human and bank error)".
 * We need to be careful not to combine these with other deposits.
 * Reconnect any disconnected bank account first (and undo it afterwards).
 
   $xids = '133819,133820,133831,133832,133833,133834,133835,133846,133847,133848,133849,133875,133876,133904,133905,133906,133907,133908,133909';
   $q = db\q("SELECT uid2,amt,xid from txs WHERE xid IN($xids)");
   $DBTX = \db_transaction();
   while ($row = $q->fetchAssoc()) {
     extract($row);
     bank20200312($uid2, $amt, $xid);
   }
   unset($DBTX);

  $map = [134573=>133819, 134574=>133820, 134575=>133831, 134576=>133832, 134577=>133833, 134578=>133834, 134579=>133835, 134580=>133846, 134581=>133847, 134582=>133848, 134583=>133849, 134584=>133875, 134585=>133876, 134586=>133904, 134587=>133905, 134588=>133906, 134589=>133907, 134590=>133908, 134591=>133909];
  
  foreach ($map as $newxid => $oldxid) {
    $xid = $oldxid + 1000;
    foreach (ray('tx_hdrs tx_entries txs2') as $tnm) {
/** /      debug("changing xid $newxid to $xid");
//      db\q("UPDATE $tnm SET xid=$xid WHERE xid=$newxid");
    }
  }
  
  $q = db\q('SELECT xid, for1 FROM txs WHERE xid BETWEEN 134819 AND 134909');
  while ($row = $q->fetchAssoc()) {
    extract($row);
/** /    if ($for1 != 'from bank') die($xid);
    $oldxid = $xid - 1000;
    $description = "from bank - duplicate of #$oldxid (combined human and bank error)";
    db\update('tx_entries', compact(ray('description xid')), 'xid');
  }
   
 */

function clearCache() {foreach (ray('cache cache_bootstrap cache_form cache_menu sessions semaphore') as $table) db\q("TRUNCATE TABLE $table");}
