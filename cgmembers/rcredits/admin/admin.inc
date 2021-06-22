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

/**
 * Set up the basic user accounts and relations on the server.
 * called from installation and test setup
 * To prepare for export for developers, f('t.clear');
 */
function setupBasicAccounts() {
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
      r\acct($uid)->update(compact(ray('zips federalId flags')));
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
      $info += compact(ray('postalAddr address city state federalId activated'));
      $serverA = new r\Acct($info);
      $serverA->update(compact('zips'));
    }
  }
  //$region = db\get("CONCAT(iso_code, 'A')", 'r_countries', 'id=:country', compact('country'));
  if (!isDEV) return; // don't mess with existing members on stage or production server
  
  $values = array(1, 0, 'admin', t('System Administrator'), r\regionField('email'), 0, 0, u\bit([B_ADMIN, B_CO]));
  $info = array_combine($keys, $values);
  $a1 = new r\Acct($info); // set up system admin record (uid=1)
  if (isDEV) $a1->update('pw2 vKeyPw', r\passHash(DEV_PW2), u\b64decode(DEV_VKEYPW));

  foreach (ray(t('2:PlaceHolder One,3:PlaceHolder Two')) as $k => $v) {
    $name = str_replace(' ', '', mb_strtolower($v));
    $values = array($k, 0, $name, $v, 'z@o.t', 0, 0, 0);
    $info = array_combine($keys, $values);
    new r\Acct($info);
  }  

  $flagsCo = u\bit([B_MEMBER, B_CONFIRMED, B_OK, B_CO]); // flags for an active company account
  
  $x1=new r\Acct(array_combine($keys, array(CG_ROUNDUPS_UID, 0, t('roundup'), t('%PROJECT Region'), 'cg@commongood.earth', 0, 0, $flagsCo)));
  new r\Acct(array_combine($keys, array(CG_CRUMBS_UID, 0, t('crumbs'), t('%PROJECT Region'), 'cg@commongood.earth', 0, 0, $flagsCo))); 
  new r\Acct(array_combine($keys, array(CG_SERVICE_CHARGES_UID, 0, t('bank fees'), t('Bank service charges'), 'cg@commongood.earth', 0, 0, $flagsCo))); 
  new r\Acct(array_combine($keys, array(BANK_IN_ID, 0, t('bank-in'), t('--'), 'cg@commongood.earth', 0, 0, $flagsCo))); 
  new r\Acct(array_combine($keys, array(BANK_OUT_ID, 0, t('bank-out'), t('--'), 'cg@commongood.earth', 0, 0, $flagsCo)));
  
  $serverA->update('created', R_LAUNCH); // a date in the past makes tests work better
  if ($cgId = getV('cgf_uid') and db\exists('users', 'uid=' . $cgId)) return; // already done (don't use CGID here)

  $values = array(r\qo('NEWAAB')->id, $serverA->id, u\shortName(PROJECT), PROJECT, CGF_EMAIL, US_COUNTRY_ID, 0, $flagsCo);
  $info = array_combine($keys, $values);
  $info += ray('phone address city state postalAddr zip helper',
    u\fmtPhone(CGF_PHONE, '+n'), CGF_ADDRESS, CGF_CITY, R_STATE_ID, CGF_ZIP, CGF_POSTALADDR, 1);
  $cgf = new r\Acct($info);
  setV('cgf_uid', $cgf->id);
  $cgf->update('emailCode website activated created', '1495kJHm0h145PHh2345h', 'cg4.us', R_LAUNCH, R_LAUNCH);
  $cgf->reread(); // keeps tests from dying when using CGF as a funder for doBalance immediately after recreating the db
//    if (!$cgId) w\say(t('CGID had not yet been set up. It is now.'), 'err');
}

/**
 * Re-setup the test cards on the STAGING server, after importing data from the PRODUCTION server.
 * Set bogus email addresses and phones so we DEFINITELY don't accidentally email someone (happened once)
 */

function makeTestAccounts() {
  global $mya;
  if (isPRODUCTION or !$mya->superAdmin) return w\say(t('Make test accounts only on demo or dev'), 'err'); // protects even if constant not set

  if (!db\exists('users', 'uid=:DEMO_CTTY')) new r\Acct(ray('uid community fullName email flags', DEMO_CTTY, DEMO_CTTY, t('Common Good Example'), R_ADMIN_EMAIL, u\bits('ok co cgc up'))); // create a bogus community

  $testAccts = [
  'aaa' => ['Maria Manager', '', 'WeHlioM5JZv1O9G'],
  'aab' => ['Corner Store', TRUE, 'CvRM3AwXJppPmf'], // include cardCode so we can test the company proSe (or with agent)
  'aad' => ['Cathy Cashier', '', 'ME04nW44DHzxVDg'],
  'aak' => ['Curt Customer', '', 'NyCBBlUF1qWNZ2k'],
  'aaq' => ['Helga\'s Hardware', TRUE, 'Y9d8fuwiejKfd'],
  'abb' => ['Susan Shopper', '', 'ZzhWMCq0zcBowqw'],
  ];

  $info0 = ray('dob helper floor pin community activated', strtotime('11/1/2002'), 1, -9999, '1234', DEMO_CTTY, now());
  
  foreach ($testAccts as $qid => $vs) {
    extract(array_combine(ray('fullName co cardCode'), $vs));
    $uid = t\uid(strtoupper(".$qid"));
    $name = u\shortName($fullName);
    $emailCode = $cardCode;
    $flags = u\bits('ok member confirmed carded ided debt');
    if ($co) $flags |= u\bit('co');
    $email = "$qid@example.com";
    $phone = u\fmtPhone('413' . u\randomString(7, 'digits'));
    $federalId = u\randomString(9, 'digits');
    $postalAddr = u\randomString(3, 'digits') . ' Demo St., Demoville, MA 01330';
    list ($address, $city, $state, $zip) = u\parseAddr($postalAddr);
    $state = r\stateNum($state);
    $info = $info0 + compact(ray('uid fullName name flags cardCode emailCode email phone federalId postalAddr address city state zip'));

    if ($a = a($qid)) $a->update($info); else $a = new r\Acct($info);
    a\photoFromFile($a);
  }
  
  foreach (ray('aaa aad aak') as $qid) {
    $coA = a($qid == 'aak' ? 'aaq' : 'aab');
    $coA->setDftAppCans();
    list ($a, $perm) = [a($qid), $qid == 'aad' ? B_SCAN : B_MANAGE];
    if (!r\relation(1, $coA->id, $a->id)) $coA->newRelation(ray('other permission employee otherNum', $a->id, r\perm($perm), TRUE, $qid == 'aad' ? 2 : 1));
    $a->update('cardCode2', $a->cardCode); // this is new for Maria (aaa) and Curt (aak)
  }
  
  a('aaa')->setBit(B_ADMIN);
  a('aab')->update('selling', "groceries\ngifts\nsundries\ndeli\nbaked goods");
  a('aaq')->update('selling', 'hardware');
  
  w\say(t('Done!'));
}

/**
 * Assign the given account a new id (or delete it).
 * @param int $oldUid: record ID of the account to change or delete
 * @param int $newUid: the new record ID to give the account (defaults to next available)
 * @param bool $unusable: whether to mark the old account record ID as unusuable
 * @param bool $delete: whether to delete the account rather than giving it a new ID (called by a\deleteAccount)
 */
function deleteUid($oldUid) {a\changeUid($oldUid, 0, FALSE, TRUE);}
function changeUid($oldUid, $newUid = 0, $unusable = FALSE, $delete = FALSE) {
  global $mya;
  if (is_object($oldUid)) $oldUid = $oldUid->id;
  if (!$a = r\acct($oldUid)) return w\say(t('That account does not exist.'), 'err');
  if ($mya and $a->id == $mya->id) return w\say(t('You can\'t delete or change the uid of an acct while it\'s the current acct.'), 'err');
  $nm = $a->fullName;
  $oldQid = r\qid($oldUid);

  $subs = array( // all cg tables are listed, to be sure we don't leave any out (D=deletable)
    // messages
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
    'r_changes:changedBy',
    'u_company:uid:D',
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
    'r_photos:uid:D',
    'r_proposals:ctty',
    'r_proxies:person:D',
    'r_proxies:proxy:D',
    // r_questions
    'r_ratings:uid',
    // r_regions
    'r_relations:main:D',
    'r_relations:other:D',
    // r_states
    // r_shares',
    'r_stakes:uid',
    'r_stakes:clubid',
    'r_stats:ctty:D',
    'r_tous:uid:D',
    // r_transit
    'r_usd:payee',
    // r_usd2
    'r_user_industries:uid:D',
    // r_votes
    'sessions:uid:D',
    'tx_disputes:uid',
    'tx_disputes:agentUid',
    'tx_entries:uid',
    'tx_entries:agentUid',
    'tx_hdrs:actorId',
    'tx_hdrs:actorAgentId',
    'tx_requests:payer:D',
    'tx_requests:payee:D',
    'tx_rules:to:D',
    'tx_rules:payer:D',
    'tx_rules:payee:D',
    'tx_rules:from:D',
    'tx_templates:to:D',
    'tx_templates:payer:D',
    'tx_templates:payee:D',
    'tx_templates:from:D',
    // u_groups
    'u_groupies:uid:D',
    'u_shouters:uid:D',
    'users:jid', // users should be last, because it's the main table for what we're deleting/changing
    'users:uid:D',
    'users:helper',
    'users:community',
    // variable
  );
  
  $DBTX = \db_transaction();

  if ($delCtty = ($delete and $a->isCtty)) { // deleting a community (merge into Seedpack)
    db\del('users', 'uid', $oldUid);
    $unusable = $delete = FALSE; // don't actually delete anything except the account record
    $newUid = r\serverUid(); // Seedpack uid
  }

  if (!$delete and !$newUid) {
    $region = $a->zip ? r\region($a->country, $a->state, $a->zip) : r\communityId();
    $newUid = r\Acct::nextId($region);
  }

  foreach ($subs as $one) {
    list ($table, $field, $deletable) = explode(':', $one . '::'); // if deleting and not deletable, set to zero
    if ($delCtty and $table == 'users') continue; // for ctty deletion, skip users (already done)
    
    if ($delete) {
      if (!$deletable) {
        if (db\exists($table, "`$field`=:oldUid", compact('oldUid'))) {
          $DBTX->rollback(); 
          return w\say(t('Account "%nm" has record(s) in table %table (field %field), so it cannot be deleted.', compact(ray('nm table field'))), 'err');
        }
      } else {
        db\del($table, $field, $oldUid);
      }
    } else db\q("UPDATE $table SET `$field`=:newUid WHERE `$field`=:oldUid", compact('newUid', 'oldUid'));
  }

  if ($unusable and !$delete) {
    list ($uid, $name, $email) = array($oldUid, "unusable-$oldQid", "unusable$oldQid@" . CG_DOMAIN);
    db\insert('users', compact(ray('uid name email')), 'uid'); // mark old uid unusable
  }
  
  unset($DBTX); // commit

  if (!$delete) {
    $newQid = r\qid($newUid);
    foreach ([R_PICTURE_DIR, PHOTOID_DIR . '/'] as $dir) { // original card photo(s) if any AND photo ID file(s) if any
      foreach (glob("$dir/$oldQid.*") as $flnm) rename($flnm, str_replace("/$oldQid.", "/$newQid.", $flnm));
    }
  }

  if ($mya and $mya->superAdmin) w\say(t('Success. Account %nm has been %what.', 'nm what', $nm, $newUid ? t('changed to ') . $newUid : t('deleted')));
}

/**
 * Return a list of signup steps remaining for the given account.
 */
function showSteps($a) {
  if ($a->task) {
    $steps0 = ray(r\taskSteps($a->task));
  } else $steps0 = array_unique(ray(join(' ', $a->co ? [S_CO, S_CO2] : [S_PERSON, S_CARD, S_SELL, S_VOICE])));
  
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
  $help = ray(t('person:minimal individual signup,co:minimal company signup,co2:complete company signup,card:qualifying for a %PROJECT card,sell:qualifying to accept %PROJECT payments as an individual,voice:qualifying (as an individual) to participate in %PROJECT Democracy,partner:minimal signup (individual or company) jointly with a partner organization'));
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
  foreach (ray('created activated signed') as $k) $subs[$k] = $a->$k ? u\fmtDate($a->$k) : t('NO');
  $subs['use'] = u\fmtDate($a->ok ? $a->access : $a->login);
  $ret .= t(' in the past 3 months (created %created, signed %signed, activated %activated, last use/signin %use)', $subs);
  return $ret;
}

/**
 * Return a list of the account's relations.
 */
function showRelations($a) {
  global $base_url;
  $uid = $a->id;
  $jid = $a->jid ?: 0;
  $ret = '';
  $sql = <<<EOF
    SELECT IF(r.main=:uid, other, main) AS them, main, IF(r.main=:uid, u2.fullName, u1.fullName) as who, 
    permission, r.:IS_EMPLOYEE AS employee, r.:IS_OWNER AS owner, r.:IS_DRAW AS draw,
    r.:IS_CUSTOMER AS customer, r.:IS_AUTOPAY AS autopay, u1.:IS_CO AS iCo, u2.:IS_CO AS uCo
    FROM r_relations r LEFT JOIN users u1 ON u1.uid=r.main LEFT JOIN users u2 ON u2.uid=r.other
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
      $extra = '<small> (' . r\qid($them) . ' / ' . a\cardLink("$main/$them") . ')</small>';
      $role = $owner ? t('owned by')
        : ($draw ? t('drawn on by')
           : ($permission >= B_MANAGE ? t('managed by')
              : (($permission >= B_SCAN or $employee) ? t('employs')
                 : ($permission >= B_READ ? t('readable by')
                    : t('probably employs')))));
    } else {
      $role = $owner ? t('owns')
        : ($draw ? t('draws from')
           : ($permission >= B_MANAGE ? t('manages')
              : (($permission >= B_SCAN or $employee) ? t('works for')
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
  $v = $v . ''; // convert to string
  if ($v and in($k, ID_KEYS) and strlen($v) >= strlen(CGID)) {
    if ($a = r\acct($v)) $v = $a->name; else $v .= t(' (missing)'); // eg, when uid is changed
  } // member ID. Don't include agent/payerAgent/etc. here
  if ($v and $k == 'channel') $v = u\rayv(ray(TX_CHANNELS), $v);
  if ($v and in($k, R_DATE_FLDS)) $v = '<div class="date-field">' . strftime('%d%b', $v) . '<span class="year-field">\'' . strftime('%y', $v) . '<br>' . strftime(isDEV ? '%I:%M %p' : '%l:%M %P', $v) . '</span></div>';
  if ($k == 'flags') $v = "<div class=\"format-flags\">" . a\flagsDesc($v, $flagsField) . '</div>';
  return $v;
}

/**
 * Display the given table or query.
 * @param string $table: table name
 * @param string $where: criteria
 * @param string $order: row order
 * @param string $fields: list of fields to display
 * @param array $joinFields: list of fields joining to users table (limit by community if called by Ctty Admin)
 */
function showQuery($table, $where = '1', $order = '', $fields = '*', $joinFields = '') {
  $limit = 500; // max records to show
  global $mya;
  
  $flagsField = ($table == 'tx_hdrs') ? TX_FLAGS : B_LIST;
  if (!$mya->admin and $joinFields) {
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
  return w\showRecords(db\q("SELECT $fields FROM $table WHERE $where LIMIT $limit")->fetchAll(), 'CG\\Admin\\formatField', $flagsField);
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
  $rows[] = array(t('Data Given'), $name, u\fmtDate(is_numeric($a->dob) ? $a->dob: 0), $a->location(TRUE), '', ''); // is_numeric check for admin without vKeyE
  $name = u\ssnName($one = (object) nn($d['ssnnames']['ssnname']));
  $dob = nn($one->dob) ? u\fmtDate(strtotime($one->dob)) : ''; // arrives as an empty array if none
  $rows[] = array(t('Found'), $name, $dob, nni($d, 'deathsearchresults') ? '<h1 color="red">DEAD</h1>' : '', '', '');
  
  $counties = [];
  $addresses = nn($d['addresses']['address']) ?: [];
  if (nni($addresses, 'lastname')) $addresses = [$addresses];
  foreach($addresses as $rec) {
    $rec = (object) $rec;
    $rows[] = array($rec->datereported ? u\fmtDate(strtotime($rec->datereported)) : '', u\ssnName($rec), $rec->dob ? u\fmtDate(strtotime($rec->dob)) : '', u\ssnAddr($rec), ucwords(mb_strtolower(pr($rec->county)))); // county was an array once
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
 * @param string $filename: the photo filename (defaults to account's shortname)
 * @param string $err: (RETURNED) the error message, if any
 * @return <success>
 include_once R_ROOT . '/admin/admin.inc';
 debug (f('a.photoFromFile', a('aaa'), $err));
 debug ($err);
 currently called only from a.makeTestAccounts()
*/
function photoFromFile($a, $filename = '', &$err = '') {
  if ($a->co) return !$err = 'tried to shrink a company photo';
  //  if ($a->hasPhoto('picture2')) return !$err = 'POS photo already exists';
  if ($a->photo) $err = 'POS photo already exists'; // not really an error
  $photoPath = r\pic($filename ?: "$a->name.jpg");
  $img = u\alterImg(imagecreatefromjpeg($photoPath), R_PHOTO_WIDTH, R_PHOTO_HEIGHT);
  ob_start();
  imagejpeg($img, NULL, 50);
  $photo = ob_get_clean();
  //if (!$photo = u\fixPicture($photoPath, FACE_ASPECT, R_PIXEL_FACTOR / 2, FALSE, $err)) return FALSE;
  return $a->update(compact('photo'));
}

/**
 * Move a field out of data or secure and into its own field in users.
 * (call after reinstall)
 include_once R_ROOT . '/admin/admin.inc';
 f('a.moveFieldFrom', 'helper', 'data');
*/
function moveFieldFrom($field, $from = 'data') {
  $uids = db\q('SELECT uid FROM users WHERE uid<0 OR uid>:CANONIC_ACCTS')->fetchCol();
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
  $q = db\q("SELECT uid,$field FROM users WHERE uid<0 OR uid>:CANONIC_ACCTS");
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
  for ($i=0; $i <= B_MAX; $i++) if ($n & u\bit($i)) {$res .= $bits[$i] . ' ';}
  return $res;
}

/**
 * Return a table of deposits.
 * for example a\deposits(0, TRUE); (to help with reconciliation)
 */
function deposits($starting = 0, $showTxIds = FALSE) {
  global $base_url;
  $sql = <<<EOF
    SELECT SUM(IF(amount>0 AND txid>0, amount, 0)) AS amountIn, SUM(IF(amount>0 AND txid>0, 1, 0)) AS countIn,
      SUM(IF(amount<0 AND txid>0, amount, 0)) AS amountOut, SUM(IF(amount<0 AND txid>0, 1, 0)) AS countOut,
      SUM(IF(txid<0, amount, 0)) AS amountBad, SUM(IF(txid<0, 1, 0)) AS countBad,
      deposit, IF(txid<0, 'bad', IF(amount>0, 'in', 'out')) AS type
    FROM r_usd WHERE amount<>0 AND created<=:NOW AND created>=:starting
    GROUP BY deposit ORDER BY deposit<>0, deposit DESC, amount<0
EOF;

  $today = today();
  $ways = ray('In Out Bad', t('IN'), t('OUT'), t('BAD')); // translate, don't use strtoupper
  $q = db\q($sql, compact('starting')); // include today's potential deposit at the top

  while ($row = $q->fetchAssoc()) {
    extract($row);
    $firstline = TRUE;
    foreach ($ways as $way => $wayDesc) {
      list ($total, $count) = [u\fmtAmt(${"amount$way"}, ''), number_format(${"count$way"})];
      if (!$count) continue;
      if ($firstline) {
        $dateDpy = $deposit == 1 ? t('<b>ANCIENT</b>') : ($deposit ? u\fmtDate($deposit, TRUE) : t('<b>TODAY</b>'));
        $date = strftime('%Y%m%d', $deposit ?: $today);
        $details = w\btn("/sadmin/deposit-details/date=$deposit&total=$total&count=$count", t('details'));
        if (!$deposit or $deposit >= $today - 90 * DAY_SECS) {
          $details .= 
//          ' ' . w\btn('', t('flnm'), 'default', 'xs', w\clas('filename') + ray('data-flnm', "CGach-$date.csv")) . 
          ' ' . w\lnk("/sadmin/achs/date=$deposit&mark=0", t('download'));
        }
        if ($count) $details .= ' ' . ($deposit
          ? w\btn("/sadmin/checks/way=In&date=$deposit&mark=0&limit=$count", t('checks'))
          : w\btn('', t('...........'), 'default') // don't show undeposited checks (a recipe for disaster)
        );
      } else $dateDpy = $details = '';

      if ($showTxIds) {
        $where = $type == 'bad' ? 'txid<0' : ('txid>0 AND amount' . ($type == 'in' ? '>0' : '<0'));
        $baids = db\col('bankTxId', 'r_usd', "deposit=:deposit AND $where", compact('deposit'));
        foreach ($baids as $i => $baid) $baids[$i] = btxid($baid);
        $bankTxIds = '<td>' . join(', ', $baids) . '</td>';
      } else {$bankTxIds = '';}

      $deposits[] =<<<EOF
<tr>
  <td>$dateDpy</td>
  <td>$wayDesc</td>
  <td>$count</td>
  <td>$total</td>
  <td>$details</td>
  $bankTxIds
</tr>
EOF;
      $firstline = FALSE;
    }
  }
  $deposits = join("\n", nn($deposits) ?: []);
  $header = t('<th>Transfers</th><th></th><th>Count</th><th>Total</th><th></th>');
  $header .= $showTxIds ? t('<th>Bank Tx IDs</th>') : '';
  return <<<EOF
<table id="deposits">
<tr>$header</tr>
$deposits
</table>
EOF;

}

/*
    foreach (['In'=>t('IN'), 'Out'=>t('OUT'), 'Bad'=>t('BAD')] as $way => $wayDesc) {
      list ($total, $count) = [u\fmtAmt(${"amount$way"}, ''), number_format(${"count$way"})];
      if (!$count) continue;
      $details = w\btn("/sadmin/deposit-details/way=$way&date=$deposit&total=$total&count=$count", t('details'), w\away());
      if ($way == 'Out') {
        list ($WAY, $date) = [t('OUT'), strftime('%Y%m%d', $deposit ?: today())];
        $details = w\btn('', t('flnm'), 'default', 'xs', ray('class data-flnm', 'filename', "CGach$WAY-$date.csv")) . ' ' . $details;
      }

  list ($func, $label) = ($way != 'Out' or ($deposit > 0 and $deposit < strtotime('1/10/2017')))
  ? ['checks', t('checks')] 
  : ['achs', t('ACHes')];
  $deposits[] =<<<EOF
  <tr>
  <td>$dateDpy</td>
  <td>$wayDesc</td>
  <td>$count</td>
  <td>$total</td>
  <td>$details &nbsp; <a href="$base_url/sadmin/$func/way=$way&date=$deposit&mark=0" target="_blank">$label</a></td>
  </tr>
EOF;
  }
  }
  $deposits = @$deposits ? join("\n", $deposits) : '<tr><td colspan=5>No deposits</td></tr>';
  $header = '<tr><th>Deposit</th><th></th><th>Count</th><th>Total</th><th></th></tr>';
  return <<<EOF
  <table id="deposits">
  $header
  $deposits
  </table>
EOF;
  }
*/

/**
 * Print a check
 * @param Pdf $ck: the Check object to place the check in.
 * @param int $pos: what position on the current page (1, 2, or 3)
 * @param assoc $tx: relevant fields from the r_usd record
 * @param bool $mark: whether to set the deposit date for each relevant transfer, in r_usd
 */
function printCheck(&$ck, $pos, $tx, $mark) {
  extract(just('xid txid created payee amount deposit bankAccount', $tx));
  $escrow = 'USkk' . ESCROW_ROUTING . ($amount > 0 ? ESCROW_IN : ESCROW_OUT);

  $a = r\acct($payee);
  $bankAccount = u\decry('V', $bankAccount);
  if ($bankAccount != $a->bankAccount) {
    r\tellAdmin(t('bank account misMatch in deposit (xid=%xid): %b1 (now), %b2 (deposit)', 'xid b1 b2', $xid, $a->bankAccount, $bankAccount));
    if ($a->bankAccount) {
      list ($b2, $bankAccount) = [$bankAccount, $a->bankAccount];
      if (!$b2) db\update('r_usd', compact(ray('txid bankAccount')), 'txid'); // fix it (else save for analysis)
    }
  } elseif (!$bankAccount) return r\tellAdmin(t('Empty bank account in deposit xid=%xid', compact('xid')));
  
  list ($fromName, $addr, $phone, $fromAcct, $toName, $toAcct) = $amount > 0 // which way
    //    ? [$a->legalNameDpy, $a->postalAddr, $a->phone, $bankAccount, PROJECT, $escrow] // from bank to escrow
    ? [$a->legalNameDpy, $a->postalAddr, $a->phone, $bankAccount, BANK_DBA, $escrow] // from bank to escrow
    : [PROJECT . ' (' . CGF_LEGALNAME . ')', CGF_POSTALADDR, CGF_PHONE, $escrow, $a->legalNameDpy, $bankAccount]; //e2b
  $memo = $amount > 0 ? t('for %RCREDITS') : t('for DEPOSIT to account #') . substr($toAcct, 4 + 9);

  list ($signature, $belowLine) = $amount > 0 
    ? [t('Pre-authorized Check'), t('No Signature Required')] 
    : ['', SIGNATORY];
  
  list ($W, $H) = array($ck->pageW, $ck->pageH);
  $lineH = $ck->lineH; // standard line height in inches
  $m = .25; // left/right margins
  $mt = ($pos - 1) * CHECK_HEIGHT + $m; // top margin

  $phone = u\fmtPhone($phone);
  $routing = substr($fromAcct, 4, 9);
  $account = substr($fromAcct, 4 + 9);

  $bankInfo = db\get('*', 'r_banks', 'route=:routing', compact('routing'));
  if (!$bankInfo) return r\tellAdmin(t('Skipped transaction #%xid for %nm (no bank info)', 'xid nm', $xid, $a->fullName));

  extract($bankInfo, EXTR_PREFIX_ALL, 'b');
  if (strlen($b_zip) > 5) $b_zip = substr($b_zip, 0, 5) . '-' . substr($b_zip, 5);
  if (!$prefix = db\get('id', 'r_transit', 'location=:location', ray('location', "$b_city, $b_state"))) {
    $prefix = db\get('id', 'r_transit', 'location=:b_state', compact('b_state'));
    u\EXPECT((bool) $prefix, 'no transit prefix');
  }
  $transit = $prefix . '-' . substr($routing, 4, 4) . '/' . substr($routing, 0, 4);
  
  $amount = abs($amount);
  $dollars = floor($amount);
  $cents = round(100 * ($amount - $dollars)) ?: t('NO');

  $ck->say("<b>$fromName</b><br><small>$addr<br>$phone</small>", $m, $y = $mt, $W - $m);
  $ck->say("$transit<br>$account", $W / 2, '', $W / 4, '', '8', 'C');
  $ck->say("<b>$xid</b>", -$W / 4, '', -$W / 4 - $m, '', '', 'R');

  $ck->say('DATE:', 6.7, $y += .4, '', .25, '8');
  $ck->say(u\fmtDate($deposit ?: now()), 7.1, '', '', .25);

  $ck->say('Pay to the<br>Order of', $m, $y += .4, '', 2 * $lineH, '8;CAPS');
  $ck->say("  $toName", $m + .7, $y + $lineH, 5.9, $lineH, '', '', 'BR');
  $ck->say('$ ' . number_format($amount, 2), 7, '');
  
  $ck->say(u\n2english($dollars) . " and $cents/100", $m, $y += .65, 6.9, $lineH, '', '', 'B');
  $ck->say('DOLLARS', $m + 6.9, '', '', $lineH, '8');
  //  $ck->Image(R_ROOT . '/images/icons/padlock.gif', $W - $m - .5, $y, $lineH, $lineH);
  
  $ck->say("<b>$b_name</b><br><small>$b_address<br>$b_city, $b_state $b_zip</small>", $m + 1, $y += .4, '', '', '9');
  
  $ck->say($signature, $W - $W / 3, $y + .35, $W / 3 - $m, $lineH, 'CAPS', 'C', 'B'); // $y+ not +=
  $ck->say($belowLine, '', $y + .35 + $lineH, $W / 3 - $m, $lineH, '8;CAPS', 'C');
  //  if (!$signature) $ck->Image(R_ROOT . '/images/checksig.png', $W - $W / 3, $y, $W / 3 - $m, 5 * $lineH);
  
  $ck->say('MEMO:', $m, $y += .5, '', $lineH, '8');
  $ck->say($memo, $m + .5, '');

  // clear band must be at least .625" high, at least 0.1875" from bottom (so leave more). Micr should be 12pt
  //  if ($a->id != 26742000000042) {
  //    $ck->say("A{$routing}A {$account}C $xid", $m, $mt - $m + CHECK_HEIGHT - .25 - $lineH * (12 / PDF_FONT_SIZE), '', '', 'GnuMICR 12');
  //  } else 
  $account = str_replace('-', 'D', $account); // show dashes properly
  $ck->say("C000{$xid}C A{$routing}A {$account}C", $m, $mt - $m + CHECK_HEIGHT - .25 - $lineH * (12 / PDF_FONT_SIZE), '', '', 'GnuMICR;12');
  //  $ck->say('', 0, $mt - $m + CHECK_HEIGHT, $W, 1, '', '', 'T'); // comment this out
  
  if (!$deposit and $mark) db\update('r_usd', ray('deposit txid', today(), $txid), 'txid');
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
     if (!r\acct()->superAdmin) return; // don't bother if not admin
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

function cardLink($link) {return spinLink("/print-rcard/$link", t('Card'), '', 'primary', 'xs', w\away());}
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
    FROM tx_templates g LEFT JOIN users u ON u.uid=g.from
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
 * Randomize personal data in all accounts (for staging site).
 */
function anonymize() {
  if (isPRODUCTION) return;
  
  $uids = db\col('uid', 'users', 'uid>:CANONIC_ACCTS AND uid<>:CGID ORDER BY fullName');
  foreach ($uids as $uid) {
    $a = r\acct($uid);
    $fullName = wordName(3) . ' ' . randomName()[0] . '. ' . wordName();
    if ($a->co) $fullName = wordName() . ' Co';
    $legalName = mt_rand(1,100) < 90 ? $fullName : (wordName() . strstr($fullName, ' '));
    $name = u\shortName($fullName);
    $email = mb_strtolower(randomName()) . '@example.com';
    $address = mt_rand(1, 9999) . ' ' . wordName() . ' St.';
    $city = wordName();
    $postalAddr = $a->risk('poBox') ? 'PO Box ' . mt_rand(1, 99999) : $address;
    $postalAddr .= ", $a->st $a->zip";
    $signedBy = $a->signedBy ? $fullName : '';
    $latitude = $longitude = 0;
    $dob = mktime(0, 0, 0, mt_rand(1,12), mt_rand(1,31), date('Y') - mt_rand(18, 100));
    $phone = '+1' . mt_rand(2, 9) . randDigits(9);
    $federalId = $a->federalId == '000000000' ? $a->federalId : ($a->federalId ? randDigits(9) : '');
    if ($a->bankAccount) {
      $bankAccount = 'USkk' . randRouting() . randDigits(10);
      $last4bank = substr($bankAccount, -4, 4);
    } $bankAccount = $last4bank = '';
    $notes = $photo = $appData = $ssnData = '';
    $pin = randDigits(4);
    $snap = $a->snap ? randDigits() : '';
    foreach (ray($randStrings = 'cardCode cardCode2 emailCode question answer') as $k) $$k = u\randomString();
    $a->update(compact(ray($randStrings . ' fullName legalName name email address city postalAddr signedBy latitude longitude dob phone federalId bankAccount last4bank notes photo appData ssnData pin snap')));
  }
  db\q('UPDATE r_usd SET bankAccount=:ba', ['ba' => 'USkk' . randDigits(19)]);
  db\q('DELETE FROM r_changes WHERE created>=:NOW');
}

/**
 * Export solar1 data to SMT
 */
function toSMT() {
  $q = db\q('SELECT * FROM users WHERE uid>:CANONIC_ACCTS');
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
      u\csvStart('solar1-export-' . date('Y-m-d') . '.csv');
      u\csv(array_keys($out), "\t");
      $hdrDone = TRUE;
    }
    u\csv(array_values($out), "\t");
  }
  if (!$hdrDone) return w\go('/sadmin/panel', t('nothing to send to SMT'));
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
  db\q('UPDATE r_relations SET main=:co2id WHERE main=:coid AND other IN (:uids)', $subs);
}
  
/*
  to find orphaned customers

  eachA(function ($a) {
  $name = str_replace("'", "\\'", $a->fullName);
///  if ($reid = f('db.lookup', 'reid', 'r_relations', "other=0 AND data LIKE '%$name%' AND reid NOT IN(26742000000222, 26742000000216)")) debug("$reid $name $a->id");
  });
*/

/**
 * Compensate for checks inadvertently deposited twice (but not recorded): once on 2/26, once on 3/3.
 * Strategy: create r_usd, tx_hdrs, and tx_entries records for the duplicate checks, but with xids 1000 more than 
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
    foreach (ray('tx_hdrs tx_entries r_usd') as $tnm) {
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
