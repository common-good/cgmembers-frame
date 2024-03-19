<?php
/**
 * @file
 * Subroutines for testing steps, for all interfaces.
 */

namespace CG\Testing;
use CG\Testing as t;
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Admin as a;
use CG\Cron as cr;
use CG\Web as w;

const T_BALANCE_FLDS = 'uid balance floor savingsAdd saveWeekly minimum';
const T_ID_FLDS = 'uid jid inviter invitee person proxy helper payer payee fromId from to partner myid reid main agent other ctty voter account clubid actorId actorAgentId agentUid payerId payeeId coid proposedBy company ctty sponsor community qid agentQid cgId payerId payeeId customer coId uid1 uid2 agt1 agt2 fromUid toUid identifier';
const SPECIAL_IDS = 'ctty regulars crumb round stepups cgf bank';
const RLESS_TNMS = 'posts people post_cats messages txs2'; // test tables with no "r_" prefix
const NULL_FLDS = 'payer payee useMax amtMax template end code rule';
const DOJS_TIMEOUT = 2; // number of seconds for simulated ajax; 2 seems to suffice (if tests fail, use a bigger number)
// see R_DATE_FLDS in defs.inc


global $base_path;
define('BASE_PATH', $base_path);
const DOCODE_FLNM = LOG_DIR . 'docode.txt';
const SAME_TIME = 60 * 20; // number of seconds that make no difference in "same time"

/**
 * Output the message with the given background color.
 * Return FALSE (meaning error in returns from callers) unless a color is specified.
 */
function output($msg, $color = 'error') {
  global $wholeModule, $channel;

  $debugFunc = $channel == TX_AJAX ? 'flog' : '\\drupal_set_message';
  if (!isPRODUCTION and !nn($wholeModule)) $debugFunc(color($msg, $color)); // make sure we don't do this on production
  return $color == 'error' ? FALSE : TRUE;
}
function err($msg) { return t\output($msg); }

/**
 * Return the message in a given color box.
 * @param mixed $msg: what to give a background color to (can be any data type)
 * @param string $color: any of highlight|normal|expect|email|input|output|screen|error|doublesize|success|fail|report-ok|report-no|next|pass|divlink|hdr
 * Not to be confused with the color() function in test.php
 */
function color($msg, $color = 'normal') {
  $msg = pr($msg);
  return "<pre class=\"test-$color\">$msg</pre>";
}

/**
 * Simplify HTML output to provide a summary for test results.
 */
function strip($form) {
  $form = str_replace('&nbsp;', ' ', $form);
  $form = preg_replace('~<fieldset id="help".*?</fieldset>~sm', '', $form); // remove help div
  $form = preg_replace('~<div[^>]*class="gone.*?</div>~sm', '', $form); // remove hidden divs (not nested)
  $form = preg_replace('~<div[^>]*class="g([A-E])".*?</div>~sm', '$1 ', $form); // change multiple choice options to letters
  
  $form = preg_replace('~<option value="[^"]*">.*?</option>~sm', '', $form); // remove unselected options (fails for dropdowns created on the fly, eg state and country)
  $form = preg_replace('~<input type="hidden".*?/>~sm', '', $form); // remove hidden inputs
  $form = preg_replace('/<input[^>]* value=" *(.*?) *".*?>/sm', '$1', $form); // show button labels
  $form = preg_replace('/<input[^>]* checked[^>]* data-on="([^"]*)"[^>]*>/sm', '$1', $form); // bootstrap toggle on
  $form = preg_replace('/<input[^>]* data-on="([^"]*)"[^>]* checked[^>]*>/sm', '$1', $form); // ditto (either order)
  $form = preg_replace('/<input[^>]* data-off="([^"]*)"[^>]*>/sm', '$1', $form); // bootstrap toggle off
  $form = preg_replace_callback('~<img src="([^"]+)"[^>]*>~sm', function ($m) {
      $s = strtr(basename($m[1]), ray('.gif .png .jpg', '', '', ''));
      $s = strtr($s, ray('-red -blue -green', '', '', ''));
      return '[' . strtr($s, ray('arrow-left arrow-right', 'L', 'R')) . ']';
    }, $form);
  $form = preg_replace('/> *</sm', '> <', $form); // make sure there's exactly one space between things

  $form = preg_replace('/\s*$\s*/sm', PHP_EOL, strip_tags($form)); // zap every tag that's left

  return $form;
}
 
/**
 * Allow mnemonic and abbreviated data in test scenario columns.
 * @param assoc $info: field names and values
 * @param bool $direct: directly to or from db, rather than user input (default FALSE)
 */
function fixData(&$info, $direct = FALSE) {
  global $testOnly, $rUrl, $testVars, $T, $testCode;
  
  foreach ($info as $k => $v) if (!is_array($v) and $v . '' == NUL) $info[$k] = NULL;
  $fields = 'ok email phone dob federalId idProof flags adminable notices statements who rewards risks toEmail why error country state boxId cat cat1 cat2 method companyOptions coType coFlags code purpose pid vKeyE vKeyPw can reason data'; // first key is probably an ID field, so keep it at the front
  if (!is_numeric($key = key($info))) $fields = "$key $fields";

  if (nn($testVars)) foreach ($info as $k => $v) if (!is_array($v) and mb_strpos(nn($v), '@') !== FALSE) {
      foreach ($testVars as $varName => $var) $info[$k] = str_replace("@$varName", nn($var), $v);
    }
  foreach (ray(NULL_FLDS) as $k) if (isset($info[$k]) and empty($info[$k])) $info[$k] = NULL;
  
  extract(just($fields, $info));
  
  if (nn($code) == 'TESTCODE') $code = $testCode;
  if (isset($ok)) $ok = (int) $ok; // use integers not strings in JSON
  if (nni($info, 'item')) {
    if (isset($cat)) $cat = db\get('id', 'post_cats', compact('cat')); // fix cat for post_cats
  } else foreach (ray('cat cat1 cat2') as $k) if (isset($$k)) $$k = r\nick2cat($$k); // fix cat for tx_cats
  if ($T and $T->feature == 'posts' and isset($method) and !$testOnly) $method = array_search($method, ray('email phone text'));
  if (isset($companyOptions)) $companyOptions = ray($companyOptions);
  if (isset($coType) and !is_numeric($coType)) {
    if ($coType) $coType = array_search($coType, ray(CO_TYPES)); else { unset($coType); unset($info['coType']); }
  }
  if (isset($coFlags)) {
    $res = 0;
    foreach (ray($coFlags) as $one) {
      $flag = u\bit(u\consta('co', $one));
      if (in($one, APP_CAN_BIT_NAMES)) $flag <<= APP_CANS; // this probably fails (see rsmart.steps instead)
      $res |= $flag;
    }
    $coFlags = $res;
  }
  if (isset($reason) and rayhas($info, 'nvid') and !rayhas($info, 'op')) {
    $data = $info['data'] = serialize(compact('reason'));
    unset($info['reason']);
  }

  if (isset($purpose)) $purpose = tr($purpose); // allow keyed tx descriptions
  if (isset($pid) and !$pid) $pid = NULL;

  if (nn($vKeyE) == 'DEV_VKEYE') $vKeyE = getDevVKeyE();
  if (nn($vKeyPw) == 'DEV_VKEYPW') $vKeyPw = getDevVKeyPw();
  
  if (isset($can) and !is_array($can)) {
    $newcan = 0; 
    foreach (ray($can) as $bitName) $newcan |= r\adminCanBit($bitName);
    $can = $newcan;
  }

  if (nn($who)) $who = \fullQid($who);
  foreach (ray('flags adminable') as $k) if (isset($$k)) $$k = t\fixFlags($$k);
  if (isset($adminable)) {
    for ($i = 0; $i < B_MAX; $i++) if (u\getBit($adminable, $i)) $adminable2[$i] = TRUE;
    $adminable = nn($adminable2) ?: [];
  }
   extract(fixState(nn($state), nn($country) ?: US_COUNTRY_ID));

  foreach (ray('why error') as $k) if (isset($$k)) $$k = tr($$k); // convert embedded message references to actual text
  foreach (['rewards'] as $k) if (isset($$k)) $$k += 0; // make sure numeric fields are numeric.
  if (nn($notices) and !strhas($notices, ':')) $notices = array_search($notices, ray('daily weekly monthly'), TRUE);
  if (nn($statements)) $statements = array_search($statements, ray('electronic paper'), TRUE);
  if (nn($email)) $info['email'] = $email = t\fixEmail($email);

  if (!empty($phone)) {
    if (strlen(nn($phone)) == 1) u\prefix('+1413772000', $phone); else $phone = u\fmtPhone($phone, 'internal');
  }
  if (nn($idProof)) u\prefix("$rUrl/images/", $idProof);
  if ($direct) {
    if (nn($federalId)) u\digits($federalId);
    if (nn($dob) and !is_numeric($dob)) $dob = strtotime(str_replace('/', '-', $dob));
  }
  if (!empty($boxId) and !is_numeric($code = $boxId)) $boxId = db\get('id', 'r_boxes', compact('code'));

  foreach (just(T_ID_FLDS, $info) as $k => $v) if (!($k == 'to' and strhas($v, '@'))) { // exception for the "to" parameter in r\rMail() subs
    $$k = fixIdFld($v);
    if ($k == 'qid') $$k = r\qid($$k); // convert back to qid for qid field
    $info[$k] = $$k;
  }

//  foreach (ray('state country') as $k) if (!isset($info[$k])) unset($$k);
  foreach (ray($fields) as $k) if (array_key_exists($k, $info)) $info[$k] = nn($$k, NULL);
  return $info;
}

/**
 * Set vKeyE and vKeyPw for admin and optionally for the specified account. Return vKeyPw.
 * @return vKeyPw
 */
function setDevVKey($a = NULL) {
  if (isPRODUCTION) return;
  $key = file_get_contents(DEV_VPRIVK_FLNM);
  $pw = getDevVKeyPw();
  a(UID_SUPER)->makeVKeyE($key, $pw, FALSE);
  if ($a and $a->id != UID_SUPER) $a->makeVKeyE($key, $pw, FALSE);
  return $pw;
}

/**
 * Return an account record ID for the given shorthand id field value.
 * @param string $v: shorthand field value
 * @return: interpreted field value
 */
function fixIdFld($v) {
  if (strhas($v, '@')) return r\acct(t\fixEmail($v))->id; // sometimes an email identifies an account ($$k not $v)
  if (preg_match('/^!?\.[A-Z]{3}$/', nn($v)) or in($v, SPECIAL_IDS)) return t\uid($v);
  return $v;
}

/**
 * Say whether the given member has the given admin permissions. Or set them.
 */
function adminPerms($id, $perms) {
  global $testOnly;
  
  if (!$a = r\acct(t\uid($id))) return t\output(t('Member %id does not exist and so does not have permissions %perms.', compact(ray('id perms'))));
  if (!$testOnly) $a->setAdminBits(ray($perms));
  return $a->adminCan($perms);
}

function cookieIs($nm, $v, $crypt = FALSE) {
  global $testOnly, $box;

  $info = [$nm => $v]; fixData($info); [$nm => $v] = $info;
  if (!$testOnly) {
    if ($crypt) r\setCryptCook($nm, u\b64decode($v)); else r\setCook($nm, $v);
  }
  if ($nm == 'box') $box = $v; // global $boxId gets set in t\logIn()
  $cookieV = $crypt ? u\b64encode(r\cryptCook($nm)) : r\cook($nm);
  return t\eq("cookie $nm", $v, $cookieV);
}

function pdfHas($list) {
  global $testOnly, $pdfSays;
  $in = strip_tags(join(' ', nn($pdfSays) ?: []));
  $px = 0;
  foreach ($list as $k => $v) {
    foreach ($v as $one) {
      if ($one === '') continue;
      $p = mb_strpos($in, $one, $px);
      if ($p === FALSE) {
        t\output("Can't find \"$one\" in \"$in\" starting at $px. Actual:");
        t\output($pdfSays);
        return FALSE;
      }
      $px = $p + mb_strlen($one);
    }
  }
  return TRUE;
}

/**
 * See whether a file has been downloaded, as required.
 * @param string $flnm: downloaded filename wanted
 * @param array[array] $wantRecs: records the file should containing (either an associative or flat array)
 * @return <downloaded file has the right name and contents>
 */
function download($flnm, $wantRecs) {
  global $testOnly;
  global $testDownload,$testDownloadFlnm;

  $csv = (mb_substr($flnm, -4) == '.csv');
  if (nn($testDownloadFlnm) != $flnm) return t\output("$testDownloadFlnm <> $flnm");
  if (!is_array($testDownload) or empty($testDownload)) return t\output('testDownload is empty');
  if (sizeof($testDownload) != sizeof($wantRecs) + ($csv ? 1 : 0)) return t\output(t('Wrong number of rows in download (wanted %want, got %got)', 'want got', sizeof($wantRecs), sizeof($testDownload)));
  if ($csv) $headers = array_values(array_shift($testDownload));
  
  foreach ($wantRecs as $rowNum => $wantRow) {
    if (!$gotRow = array_shift($testDownload)) return t\output('missing record in download: ' . pr($wantRow));
    if ($csv) $gotRow = array_combine($headers, $gotRow); // add keys

    foreach ($wantRow as $k => $v) {
      $ok = $csv ? t\eq($k, $v, nni($gotRow, $k)) : (empty($v) or strhas($gotRow, $v));
      if (!$ok) return t\output(t('Missing value #%k: %v, in row #%rowNum: %gotRow', 'k v rowNum gotRow', $k, $v, $rowNum, pr($gotRow)));
    }
  }
  return TRUE;
}

function isSignedIn($id) {
  global $testOnly, $mya;
  return $testOnly ? ($mya and $mya = r\acct() and t\uid($id) == $mya->id) : t\login($id);
}

/**
 * Say whether the given variable is set correctly OR set it.
 */
function varIs($k, $v) {
  global $testOnly;
  return ($v == ($testOnly ? getv($k) : setv($k, $v)));
}

/**
 * Interpret flag names
 */
function fixFlags($flags) {
  global $testOnly;

  if (empty($flags)) return 0; elseif (is_numeric($flags)) return $flags + 0;  
  $res = 0;

  foreach (ray($flags) as $one) if ($one) $res |= u\bit(u\consta('b', $one));
  // NO! fails for r_txs and u_relations flags  if ($res & u\bit(B_OK)) $res |= u\bit(B_MEMBER);
  return $res;
}

/**
 * Allow abbreviated email address like "a@" meaning a@example.com. Also avoid weird characters.
 */
function fixEmail($email) {
  if (substr(nn($email), -1, 1) == '@') return $email . EMAILX;
  return $email = plain($email);
}

function memberScansAdminCard($id, $vKeyPw) {
  global $testOnly; if ($testOnly) return FALSE;
  $info = compact('vKeyPw');
  fixData($info);
  return t\memberVisitsPage($id, 'sadmin/panel/codePw=' . u\cryRay($info));
}

function doRecords($list, $thing) {
  global $testOnly;

  if (is_object($list)) $list = (array) $list;
  u\EXPECT(is_array(nn($list)) and !is_numeric(key(nn($list[0]))), 'doThing object should be an assoc: ' . pr($list));
  $func0 = 'CG\\Testing\\do';
  $func = $func0 . ucfirst(substr($thing, (strpos($thing, '_') ?: -1) + 1));
  if (!function_exists($func) or u\starts($thing, 'x_')) $func = $func0 . 'Thing';

  foreach ($list as $one) {
    fixData($one, TRUE);
    if (!$func($one, $thing)) return FALSE;
  }
  return TRUE;
}

/*
 * Generic record-maker/record-tester.
 */
function doThing($info, $thing, $key = 'id') {
  global $testOnly;

  $table = $thing = strtolower($thing);
//  if (!preg_match('/^(tx|u|x)_.+/', $thing) and !in($thing, RLESS_TNMS)) $table = 'r_' . $table;
//  if (!in($thing, 'people txs2') and !strhas($thing, '_') and substr($thing, -1, 1) != 's') $table .= 's';
  foreach (ray('u_photo:uid, u_company:uid, u_shout:uid, tx_hdrs:xid') as $tnm => $k) if ($table == $tnm) $key = $k;

  // NO!  if (@$info['id']) {$info['uid'] = $info['id']; unset($info['id']);}
  if ($testOnly) {
    return verifyRecord($table, $info);
  } else {return db\insert($table, $info, $key);}

}

function these($thing, $list) {return doRecords($list, $thing);}
function members($list) {return doRecords($list, 'accounts');}
function balances($list) {return doRecords($list, 'balances');}
function membersHave($list) {return doRecords($list, 'membersHave');}
function selling($list) {return doRecords($list, 'Selling');}
function companyFlags($list) {return doRecords($list, 'CompanyFlags');}
function coupons($list) {return doRecords($list, 'Coupons');}

/**
 * Create device (box) records.
 */
function doBoxes($info = []) {
  global $testOnly, $channel; // be careful not to overwrite channel with a local variable
  //$boxName = $info['boxName'];
  $info['channel'] = strtr(nn($info['channel']), ray('sms pos web', TX_SMS, TX_APP, TX_WEB)) ?: $channel;
  //if ($testOnly) $boxName = htmlspecialchars(@$boxName);
  return doThing($info, 'r_boxes');
}

/**
 * Set some defaults for users record before calling fixData.
 */
//function preAccount(&$info) {}

/**
 * Fabricate a bogus account.
 * Minimum parameters: none
 * @return uid
 */
function doAccounts($info = []) {
  global $testOnly; if ($testOnly) return doMembersHave($info);

  if (is_object($info)) $info = (array) $info;
  /**/  if (!is_array($info)) {die('not array: ' . pr($info) . trace());} // keep for now
  //  unset($info['rebate']);
  extract($info); // any user table fields
  $uid0 = nn($uid);
  u\setDft($uid, r\Acct::nextId()); // default to first available number
  $uid = t\uid($uid); // change .AAB to 1, etc.

  if (nn($jid)) $jid = t\uid($jid);

  $nmMap = ray('ZZA:Abe One,ZZB:Bea Two,ZZC:Cat Pub,ZZD:Dee Four,ZZE:Eve Five,ZZF:Fin Corp,ZZG:Glo Seven,ZZH:Hal Co,ZZI:Ivy Nine,ZZJ:Joe Ten');
  u\setDft($fullName, substr($uid, 0, 2) == 'ZZ' ? strtr($nmMap) : 'doAccount ' . u\randomString(20, 'word'));
  $name = u\shortName($fullName);
  u\setDft($legalName, $fullName);
  //  $pass = @$my_password ?: '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae'; // default to 123
  if (trim(nn($pass)) === '') $pass = '123';

  $zza = r\qo('.ZZA')->id;
  $zpa = r\qo('.ZPA')->id;
  $mailDft = u\randomString(10, 'word');
  foreach ([$zza, $zpa] as $z0) if($uid >= $z0 and $uid < $z0 + 26) $mailDft = chr(ord('a') + $uid - $z0);
  u\setDft($email, $mailDft . '@' . EMAILX); // a@example.com, etc.

  u\setDft($community, r\serverUid());
  u\setDft($zip, '01301');
  u\setDft($country, 'US');
  extract(fixState(nn($state), nn($country)));

  u\setDft($address, '1 My Street');
  u\setDft($city, 'Mytown');
  u\setDft($state, R_STATE_ID);
  $st = r\realState($state);
  u\setDft($postalAddr, nn($address2) ? "$address2, $city2, $state2 $zip2" : "$address, $city, $st $zip");

  //    u\setDft($photoId, "$state-{$zip}999");
  u\setDft($helper, t\uid('.AAB')); // everyone but .AAB should have a helper
  /**/  if ($helper === '') die(trace()); // set helper to zero if none, never blank

  if (u\getBit(nn($flags), B_OK)) {
    u\setBit($flags, B_MEMBER);
    u\setDft($bankAccount, T_BANK_ACCT);
  }
  u\setDft($last4bank, substr(nn($bankAccount), -4, 4));

  $moneyFields = 'committed floor rewards'; // not minimum because NULL means not set, in Membership page
  foreach (ray($moneyFields) as $one) u\setDft($$one, 0);
  if (nn($cc)) $cardCode = $cc;
  if (nn($cc2)) $cardCode2 = $cc2;
  if (isset($risks)) $risks = getRisks($risks, K_ACCT_RISKS);
  
  foreach (ray('backingNext') as $k) if (nn($$k, NULL) === '') $info[$k] = NULL; // empty for these means NULL

  $fixFields = $moneyFields . ' fullName legalName name pass email secure community zip country postalAddr cardCode cardCode2 uid jid helper flags risks bankAccount last4bank';

  foreach (ray($fixFields) as $k) if (isset($$k)) $info[$k] = $$k; // add in changed fields
  foreach (ray('acctType cc cc2') as $one) unset($info[$one]); // these are not real fields

  if (!$a = new r\Acct($info)) return t\output('Cannot create account: ' . pr($info));

  if (nn($picture) and !makePicture($picture, $a)) return FALSE;
  $a->stepDone('signup');
  if ($uid0 == $uid) $a->setBit(B_IDED); // assume we have a photoId for givens unless a step says otherwise

  return $uid;
}

/**
 * Fabricate transactions OR test them.
 * @param assoc $info: transaction data to create or test
 *   Minimum parameters: amount, payer, payee
 * @return:
 *   if $testOnly return TRUE if the data is as expected
 *   otherwise return the xid of the created transaction(s) (treated as TRUE in test steps)
 * Note: for $testOnly TRUE, if there are multiple entry pairs for one transaction, put the eid field first so verifyRecord() can distinguish them.
 */
function doTxs($info) {
  global $testOnly;

  $map = 'amount:amt, payer:uid1, payee:uid2, purpose:for';
  foreach (ray($map) as $k => $k2) if (rayhas($info, $k)) {$$k2 = $info[$k2] = $info[$k];}
  extract(just('uid1 uid2 taking type', $info, NULL));

  if ($type and !is_numeric($type)) $info['type'] = u\consta('E', $type) ?: -1;
  if (isset($for)) {
    unset($info['for']);
    if (mb_strpos($for, ',')) $for = explode(',', $for); // specifying both
    if ($for) list ($for1, $for2) = is_array($for) ? $for : [$for, $for];
    $info += compact(ray('for1 for2'));
    if (u\starts($for2, 'undoes:')) $info['reversesXid'] = substr($for2, mb_strpos($for2, ':') + 1);
  }

  foreach (ray('amount payer payee purpose for taking') as $k) unset($info[$k]);
  foreach ($info as $k => $v) if (!$v) $info[$k] = (in($k, 'flags risks goods channel amt id') ? 0 : NULL); // interpret empty fields

  if ($testOnly) {
    return verifyRecord('txs', $info);
 
  } else { // set up everything needed for record creation (array_filter then should have no effect or new r\Tx dies)
    global $channel;

    $info['amount'] = $amt = nni($info, 'amt');
    $actorId = $taking ? $uid2 : $uid1;
    $info += ray('created flags for1 for2 channel actorId actorAgentId recursId', // defaults
                now(), 0, t('for1'), t('for2'), $channel ?: TX_SYS, $actorId, $actorId, NULL);

    extract(just('type risks flags goods cat1 cat2 eid agt1 agt2 rel for1 for2', $info, NULL));
    if ($rel) $info['relatedId'] = $rel;
    if (!$goods) $info['goods'] = strhas($for2, 'cash') ? FOR_USD : FOR_GOODS;
    foreach (ray('agt1 agt2', $uid1, $uid2) as $k => $v) if (!$$k) $$k = $v;

    $info['entryType'] = $type ?: ((r\isBank($uid1) or r\isBank($uid2)) ? E_BANK : E_PRIME);
    $info['risks'] = $risks ? getRisks($risks, K_TX_RISKS) : 0;
    if ($flags and !is_numeric($flags)) $info['flags'] = u\bit($flags); // works for '' and arrays too

    list ($hFlds, $eFlds) = [r\Tx::TX_FIELDS, r\TxEntry::E_FIELDS];
    foreach (ray('for1 for2 eid type amt uid1 uid2 rel cat1 cat2 agt1 agt2') as $k) unset($info[$k]);
    if ($extra = justNOT("$hFlds $eFlds", $info)) u\FAIL('Bad fields in transaction: ' . join(',', array_keys($extra)));
   
    $sharedFlds = 'xid entryType rule relType relatedId';
    $otherFlds = 'amount uid agentUid description cat';
    u\EXPECT(strlen("id $sharedFlds $otherFlds") == strlen($eFlds), 'txEntry fields mismatch');
    
    $e1 = $e2 = just($sharedFlds, $info); // shared in an entry pair
    $e1 += ray($otherFlds, -$amt, $uid1, $agt1, $for1, $cat1);
    $e2 += ray($otherFlds,  $amt, $uid2, $agt2, $for2, $cat2);
    if ($eid) {
      u\preray(ray('id', -$eid), $e1);
      u\preray(ray('id',  $eid), $e2);
    }

    $hdr = just($hFlds, $info);
    $hdr['entries'] = [new r\TxEntry($e2), new r\TxEntry($e1)];
    $tx = new r\Tx($hdr);
    return $tx->insert(TRUE);
  }
}

/**
 * Either mark all other steps done or say whether they are.
 */
function stepsLeft($id, $left0) {
  global $testOnly;
  
  if (!$a = t\acct($id)) return FALSE;
  $left = ray($left0);
  
  if (!$testOnly) {
    return $a->update('stepsLeft', $left);
    $a->update(ray('steps stepsLeft', u\bit(count(ray(S_ALL))) - 1, '')); // set all steps done
    foreach ($left as $k) $a->stepDone($k, FALSE); // unset the ones that are supposed to be left
  }

  foreach ($left as $k) if ($a->stepIsDone($k)) return t\output("step $k is marked done (shouldn't be)"); // check the bits
  $actual = join(' ', $a->stepsLeft ?: []);
  if ($actual != $left0) return t\output("actual steps left ($actual) are not what's wanted ($left0)"); // proper order
  return TRUE;
}

/**
 * Return the given (wanted) array JSON encoded.
 * @param string $want: the wanted array
 * @param bool $sub: is this a recursive call? (default FALSE)
 */
function json($want, $sub = FALSE) {
  foreach ($want as $k => $v) {
    if (is_array($v)) {
      $want[$k] = json($v, TRUE);
    } elseif ($v and substr($v, 0, 1) == '[' and substr($v, -1) == ']') {
      $want[$k] = [substr($v, 1, strlen($v) - 2)];
    } elseif ($v == 'true' or $v == 'false') {
      $want[$k] = ($v == 'true');
    } elseif (is_numeric($v)) {
      $want[$k] = strhas($v, '.') ? (float) $v : (int) $v;
    }
  }

  return $sub ? $want : str_replace('"' . NUL . '"', 'null', preg_replace('/"(%[a-z]+[a-z0-9]*)"/', '$1', json_encode($want)));
}

/**
 * Fabricate a bogus invoice or pre-approved payment.
 * Minimum parameters: amount, payer, payee
 * @return:
 *   if $testOnly return TRUE if the data is as expected
 *   otherwise return the record ID of the created invoices (treated as TRUE in test steps)
 */
function doRequests($info) {
  global $testOnly, $channel;
///   debug("doInvoice(" . pr($info, true) . "), testOnly=$testOnly");
  u\setDft($info['purpose'], nn($info['for'], 'this or that'));
  unset($info['for']);

  extract($info);

  if ($testOnly) return verifyRecord('tx_requests', $info);

  $missing = array_diff_key(ray('amount:1, payer:1, payee:1, purpose:1'), $info);
  u\EXPECT($missing == [], "missing fields " . pr($missing));

  $invInfo = just('status goods created', $info, [TX_PENDING, u\forCash($purpose) ? FOR_USD : FOR_GOODS, now()]);
  switch ($invInfo['status']) {
  case 'denied': $invInfo['status'] = TX_DENIED; break;
  case 'pending': $invInfo['status'] = TX_PENDING; break;
  case 'approved': $invInfo['status'] = TX_APPROVED; break;
  }
  $invInfo = array_merge($invInfo, just('nvid amount flags data payer payee purpose recursId reversesXid', $info));  // extract directly, and might not be present
  $nvid = db\updateOrInsert('tx_requests', $invInfo, 'nvid');
  u\EXPECT(!empty($nvid));
  $xid = $invInfo['status'];
  if ($xid > 0) {  // it was paid, and the status refers to the transaction
    // No -- don't do this, because there is insufficient information
    /* $txInfo = just('created amount payer payee purpose for', $invInfo); */
    /* $txInfo['xid'] = $xid; */
    /* doTx($txInfo, 'invoice-payment'); */
  }
  return $nvid;
}

/**
 * Check or set steps done for a given task.
 */
function hasStepsDone($id, $task, $steps) {
  global $testOnly;

  $steps0 = $steps;
  if ($steps == 'all') $steps = u\consta('s', $task);
  if ($task == 'card') $steps = trim(join(' ', [S_PERSON, $steps]));
  if ($task == 'vote') $steps = trim(join(' ', [S_PERSON, S_CARD, $steps]));

  if (!$a = r\acct(t\uid($id))) return t\output(t('Account %id does not exist.', compact('id')));

  if (!$testOnly) {
    $a->update('steps', 0);
    $a->setTask($task);
    foreach (ray($steps) as $k) $a->stepDone($k);
  } else {
    if ($steps0 != 'all' and $a->task != $task) return t\output("Wrong task. Wanted $task, got $a->task.");
  }
  foreach (ray($steps) as $k) if (!$a->stepIsDone($k)) return FALSE;
  return TRUE;  
}

/**
 * Interpret an id in a transaction or invoice.
 * @param string $id: qid or partial qid (eg NEWAAA or .AAA or C:A or NEWZZC-A)
 * @param int $main: (RETURNED) the payer or payee
 * @param int $other: (RETURNED) the payerAgent or payeeAgent
 */
function txUid($id, &$main, &$other) {
  list ($main, $other) = [t\uid($id), t\agent($id)];
}

/**
 * Analyze downloaded fix-length 1099 records.
 */
function got1099($flnm, $rows) {
  global $testRecs;
  
  foreach ($rows as $rowNum => $row) {
    $gotRow = $testRecs[$rowNum];
    $gotSeq = substr($gotRow, 499, 8) + 0;
    if ($gotSeq != $rowNum + 1) return t\output(t('Wrong sequence number in record #') . $gotSeq);

    extract(just('type who cnt amounts seq', $row));
    if ($gotRow[0] != $type) return t\output(t('Wrong type in record #') . $gotSeq);
    if ($type == 'B' or $type == 'K') {
    if ($type == 'B') {
      $qid = a(t\uid($who))->mainQid;
      if (rtrim(substr($gotRow, 20, 20)) != $qid) return t\output(t('Wrong qid in record #') . $gotSeq);
      $got = substr($gotRow, 548, 13);
      if ($got + 0 != $cnt) return t\output(t('Wrong cnt in record #%gotSeq: got %got, want %cnt', 'gotSeq got cnt', $gotSeq, $got, $cnt));
    }
    list ($amtStart, $amtLen) = $type == 'B' ? [54, 12] : [15, 18];
    if (strpos('BCK', $type) !== FALSE) {
      $amounts = explode('/', $amounts);
      foreach ($amounts as $i => $amt) {
        $gotAmt = substr($gotRow, $amtStart + $i * $amtLen, $amtLen) / 100;
        if ($gotAmt != $amt) return t\output(t('Wrong amount in record #%gotSeq: amount #%i is %gotAmt, want %amt', 'gotSeq i gotAmt amt', $gotSeq, $i + 1, $gotAmt, $amt));
      }
    }
    }
  }
  return TRUE;
}

/**
 * Analyze downloaded ACH records.
 */
function gotACH($flnm, $rows) {
  global $testRecs;
  $b = r\bankData();
  $map = [
    'DATETIME' => fmtDt(now(), 'yyMMddHHmm'),
    'BANK' => $b->name,
    'BANKROUTE' => $b->routing,
    'BANKROUT' => substr($b->routing, 0, 8),
    'WHEN' => fmtDt(now(), 'MMMdd'),
    'EFFECTIVE' => fmtDt(tomorrow(), 'Ymd'), // entryDate
  ];
  foreach (ray('A B C') as $k) $map["BA-$k"] = substr(a("ZZ$k")->bankAccount, 4);
    
  foreach ($rows as $i => $row) {
    $line = strtr($row[0], $map);

    foreach (explode(',', $line) as $v) if ($v !== '') {
      if (!strhas($testRecs[$i], $v)) return t\output(t('Missing record value %v, line %i: %line %testRecs', compact(ray('v i line testRecs'))));
    }
  }
  return TRUE;
}

/**
 * Fabricate bogus relations.
 * Minimum parameters: main, agent (both are qids -- at least one must be local)
 */
function doRelations($info) {
  global $testOnly, $memberIdCalled;
  include_once __DIR__ . '/forms/printcard.inc';

  //  t\fixData($info);
  extract($info);
  if (nn($agent)) {
    $info['other'] = $other = $agent;
    unset($info['agent']);
  }

  list ($mainA, $agtA) = [r\acct($main), r\acct(nn($other))];
  foreach ([$main => $mainA, $other => $agtA] as $i => $a) if (!$a or !$a->proSe) return t\output("bad rel acct: $i");

  if (isset($id) and !nn($reid)) $reid = $id;
  u\setDft($otherNum, nn($num) ?: 0);
  $info['permission'] = nn($permission) ? u\consta('b', $permission) - B_RELATED : 0;

  if (nn($draw)) $agtA->setBit(B_DRAWS);
  foreach (ray(REL_FLAGS) as $k) if (nn($$k)) u\setBit($flags, $k);

  foreach (ray('id agent num rCard ' . REL_FLAGS) as $k) unset($info[$k]);
//  $info += @compact(ray('reid main other otherNum flags'));
  foreach (ray('reid main other otherNum flags') as $k) if (isset($$k)) $info[$k] = $$k;

  if ($testOnly) {
    return verifyRecord('u_relations', $info);
  } else {
    $mainA->newRelation($info);
    if (nn($rCard) == 'yes') {
      if (!$agtA->cardCode2) r\acct($main, $other)->makeCardCode();
      if (!nn($memberIdCalled)) r\Web\printCard(0, $main, $other, TRUE); // make a Company Agent rCard
      $memberIdCalled = TRUE;
    }
    return TRUE;
  }
}

/**
 * List what a company sells
 */
function doSelling($info) {
  global $testOnly;
  extract(just('uid selling', $info));
  $selling = str_replace(',', "\n", $selling);
  if (!$a = r\acct($uid)) return t\output("Not an account: $uid");
  return $testOnly ? ($a->selling == $selling) : $a->update(compact('selling'));
}

/**
 * List what flags the company has set
 */
function doCompanyFlags(&$info) {
  global $testOnly;

  extract(just('uid coFlags', $info));
  if (!$a = r\acct(t\uid($uid))) return t\output("Not an account: $uid");

  if ($testOnly) {
    return eq('coFlags', $coFlags, $a->coFlags);
  } else return $a->update('coFlags', $coFlags);
}

/*
 * Temporary (we hope) record-maker for coupons.  It generates rules instead.
 */
function doCoupons($info, $thing, $key = 'coupid') {
  global $testOnly;

  $coupon = just('coupid fromId start   end   amount on  minimum useMax flags sponsor code  amtMax', $info,
                 [ NULL, NULL,  now(),  NULL, 0,     '', 0,      NULL,  0,    NULL,   NULL, NULL ]);
  $amount = $coupon['amount'];
  list ($amount, $portion) = ($amount < 0) ? [0, -0.01 * $amount] : [$amount, 0];
  $rule = array_filter([
                        'id' => $coupon['coupid'],
                        'payer' => NULL,
                        'payerType' => 'anybody',
                        'payee' => $coupon['fromId'],
                        'payeeType' => 'account',
                        'from' => (is_NULL($coupon['sponsor'])) ? $coupon['fromId'] : $coupon['sponsor'],
                        'to' => MATCH_PAYER,
                        'action' => ACT_SURTX,
                        'start' => $coupon['start'],
                        'end' => $coupon['end'],
                        'amount' => $amount,
                        'portion' => $portion,
                        'purpose' => $coupon['on'],
                        'minimum' => $coupon['minimum'],
                        'useMax' => $coupon['useMax'] == 0 ? NULL : $coupon['useMax'],
                        'amtMax' => $coupon['amtMax'],
                        'code' => $coupon['code'] 
                        ]);

  if ($testOnly) {
    return verifyRecord('tx_rules', $rule);
  } else {
    return db\insert('tx_rules', $rule, 'id');
  }
}

/**
 * Return TRUE if the notice has been queued to member $id
 */
function notice($notice, $id, $subs = []) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $base_url, $thatThing;

  if (is_array($subs0 = nn($subs[0]))) $subs = $subs0;
  fixData($subs);
  if (mb_strpos($nm = nn($subs['otherName']), 'href="do/')) {
    $subs['otherName'] = str_replace('href="do/', "href=\"$base_url/do/", $nm);
  }

  $uid = uid($id);
  if (is_array($subs)) foreach ($subs as $key => $value) {
      if (preg_match('/^\$(.*)(r|us)?$/U', nn($value), $matches)) {
      list ($num, $currency) = array($matches[1], nn($matches[2]));
        if ($currency == 'us') $currency = 'u';
        $subs[$key] = u\fmtAmt($num, 's$' . $currency);
      }
    }

  $map = ['&nbsp;'=>'', R_BULLET=>''];
  $message = strtr(strip_tags(tr($notice, nn($subs))), $map);
  $sql = 'SELECT message FROM r_notices WHERE uid=:uid AND sent=:sent ORDER BY msgid DESC';
  $info = ray('uid sent', $uid, nn($subs['sent']) ?: 0);
  $msgs = db\q($sql, $info)->fetchCol();
  $cnt = 0;

  foreach ($msgs as $msg) {
    t\output("GOT notice to $id: $msg");
    if (strtr(strip_tags($msg), $map) == $message) {
      $thatThing['notice'] = $msg; // remember for "And that <thing> blah blah blah"
      $cnt++;
    }
  }

  if ($cnt < 1) return t\output('EXPECTED (but did not get) notice ' . pr($info + compact('message')));
  if ($cnt > 1) return t\output(t('GOT message to %id %cnt times: %msg', compact(ray('id cnt msg'))));
  return TRUE;
}

/**
 * Make sure non-super doesn't try to deCry(V) something.
 */
function deCryVok() {
  global $mya, $sceneTest;
  return (!isDEV or !test() or $sceneTest->state != 'When' or ($mya and $mya->admV)); // or in($sceneTest->func, 'members membersHave these memberScansAdminCard'));
}

/**
 * Say whether the record count in a table is correct.
 */
function count($what, $count) {
  global $testOnly;
  if (!$testOnly) return FALSE;
  $desc = $what . t(' count');

  switch ($what) {
    case 'members':
    case 'users': return eq($desc, $count, db\count('users', 'uid NOT :CANONIC'));
    case 'tx_hdrs': return eq($desc, $count, db\count('tx_hdrs', 'xid > 0'));
    case 'txs': return eq($desc, $count, db\count('tx_entries', 'xid > 0') / 2);
    case 'tx_entries': return eq($desc, $count, db\count('tx_entries', 'xid > 0'));
    default: return eq($desc, $count, db\count((in($what, 'txs2 people') or strhas($what, '_')) ? $what : "r_$what"));
  }
}

/**
 * Pretend the account has completed the given steps or see if it has.
 */
function doneStep($id, $step) {
  global $testOnly;
  if (strpos($step, ' ')) {
    foreach (ray($step) as $one) if (!doneStep($id, $one)) return FALSE;
    return TRUE;
  }

  if (!$a = t\acct($id)) return t\output("Not an account: $id");
  if (!$a->stepsLeft and !$a->steps) $a->setTask($a->co ? 'co' : 'person');

  if (!$testOnly) $a->stepDone($step);
  return $a->stepIsDone($step);
}

/**
 * Make the member fields be thus and such OR test to see if they ARE thus and such.
 * "?" in a field means ignore its value when testing
 */
function doMembersHave($info) {
  global $testOnly, $sceneTest;

//  $balFields = just(T_BALANCE_FLDS, $info);
//  if (!empty($balFields) and !doBalances($balFields, TRUE)) return FALSE;

  if (isset($info['coFlags']) and !doCompanyFlags($info)) return FALSE;
  if (!$a = r\acct($id = t\uid($info['uid']))) return t\output('account does not exist: ' . $id);

//  $info = justNOT(T_BALANCE_FLDS, $info); // don't rehandle balance fields, and already got id
  if (isset($info['risks'])) $info['risks'] = getRisks($info['risks'], K_ACCT_RISKS);

  if ($testOnly) {
    foreach ($info as $k => $v) if ($v != '?') {
      if (in($k, R_DATE_FLDS) and sameTime($k, $a->o($k), $v)) continue;
      if (t\badEq("db $k", $v, $a->o($k), $id, flagList($k))) return FALSE;
    }
    return TRUE;
  } else return ($info ? $a->update1($info) : TRUE);
}

/**
 * Return the flag list for the given table and field name.
 */
function flagList($fnm = 'flags', $tnm = 'users') {
  if ($tnm == 'users') {
    if ($fnm == 'flags') return B_LIST;
  } elseif (in($tnm, 'txs tx_hdrs tx_requests')) {
    if ($fnm == 'flags') return TX_FLAGS;
  }
  return '';
}

/**
 * Check or make member field assignments.
 */
function memberFieldValues($list) {
  global $testOnly;
  foreach ($list as $one) {
    extract(just('uid field value', $one));
    $a = r\acct(t\uid($uid));
    if ($field == 'helper' or $field == 'community') $value = t\uid($value);
    if ($testOnly) {
      if (t\badEq('fieldValue wanted/actual:', $value, $a->o($field), $uid, flagList($field))) return FALSE;
    } else $a->update1($field, $value);
  }
  return TRUE;
}

/**
 * Make the balance and other numeric fields be thus and such.
 * Balance is set by creating dummy transactions (with negative xids)
 * "?" in a field means ignore its value when testing
 */
function doBalances($info) {
/**/  global $conf; if (!$conf) die('no conf in doBal');
  global $testOnly;
  static $testXid = 0;

  if ($info != just(T_BALANCE_FLDS, $info)) return t\output("spurious field in balances: " . pr($info));
  $uid = $uid0 = $info['uid'];

  if (!$a = r\acct($uid)) return t\output("account $uid0 does not exist");
//  $a->reread(TRUE); // otherwise db updates are invisible

  if ($testOnly) {
    foreach ($info as $k => $v) if ($v != '?') {
      if (t\badEq("db $k", $v, $a->o($k), $uid, flagList($k))) return FALSE;
    }
    return TRUE;
  }

  if (isset($info['balance'])) {
    $testXid -= 1;
    if (abs($info['balance'] - $a->balance) >= 0.005) {
      $cgA = r\acct(CGID);
      u\EXPECT((bool) $cgA, 'no cgA!');
      u\EXPECT($a != $cgA, "CG balance ($cgA->balance) cannot be set directly to " . $info['balance']); // (if setting it to what it already is, no problem)
      $z = be\transfer('charge', $cgA, $a, -($info['balance']-$a->balance), 'set balance for testing', FOR_NONGOODS,
                         ['force'=>1, 'xid'=>$testXid]);
      u\EXPECT($z->ok, "trouble creating transfer to set balance");
    }
  }

  foreach ($info as $k => $v) {
    if ($k == 'balance' or $k == 'uid') continue;
    $a->update($k, $v);
  }

  // verify
  foreach ($info as $k => $v) {
    if ($v != '?' and $k != 'committed' and $uid > 0) if (t\badEq("db $k", $v, $a->o($k), $uid, flagList($k))) return FALSE;
  }
  return TRUE;
}

/**
 * Check the member's risks.
 */
function memberHasRisks($id, $risks) {
  global $testOnly; if (!$testOnly) return FALSE;
  return verifyRecord('users', ray('uid risks', t\uid($id), getRisks($risks, K_ACCT_RISKS)), K_ACCT_RISKS);
}

/**
 * Return a field for the account with the given cell number.
 */
function phoneField($field, $number) {
  return ($mya = r\acct(r\SMS\uid($number))) ? $mya->$field : NULL;
}

function usdTransferCount($where = '1') {
  /**/  if (!isDEV or isPRODUCTION) die('usdTransferCount ' . DEV_ONLY_MSG);
  global $testOnly;

  if($testOnly) {
    return db\count('txs2', $where);
  } else {
    db\q("DELETE FROM txs2 WHERE $where");
    return 0; // 0 is the only thing we can make it!
  }
}

function doTxs2($tx) {
  global $testOnly, $channel; $channel0 = $channel;

  extract($tx);
  
  if (isset($payee)) {
    $tx['payee'] = t\uid($payee);
    if (!$a = r\acct($payee)) return t\output('Bad payee: ' . nn($payee));
  } else if (!$testOnly) return t\output('Missing payee: ' . pr($tx));
  if (isset($risks)) $tx['risks'] = getRisks($risks, K_TX_RISKS);

  if ($testOnly) return verifyRecord('txs2', $tx);

  if (!rayhas($tx, 'bankAccount')) $tx['bankAccount'] = nni($a->vsecure, 'bankAccount'); // sometimes we specify %NUL; don't try to deCry
  u\setDft($tx['channel'], $channel0);
  $options = just('pid created completed txid explicit bankAccount isSavings', $tx);

  if ($err = u\badAmount($amount)) return t\output($err); // tested amount should not have commas or $ sign
// No! allow this for bank verification    if (!$amount) return output('zero USD transfer');

  if (nni($tx, 'xid')) { // specifying an xid means the test will list the txs explicitly
    $txid = db\insert('txs2', $tx, 'txid');
  } else {
    if (!$btx = be\createUsdTransfer($a, $amount, $options)) return t\output('failure creating usd record: ' . pr($tx));
    $xid = $btx->xid;
    if ($info = just('txid deposit', $tx)) db\update('txs2', $info + compact('xid'), 'xid');
  }
  return TRUE;
}

/**
 * Run the op in the background, return TRUE.
 include_once R_ROOT . '/cg-testing.inc';
 f('t.cronRuns', 'op');
*/
function cronRuns($op) {
  global $testOnly; if(nn($testOnly)) return FALSE;
  global $cronOp; $cronOp = $op;
  include_once __DIR__ . '/rcron/rcron.inc';

  $now = now();

  if (u\starts($op, 'every')) {
    $period = strtolower(str_replace('every', '', $op));
    cr\often($period);
  } else cr\doTask(compact('op'));
  //  $periods = 'hour day week month year';
  //  setV('last_cron_end', ray($periods, $now, $now, $now)); // normally don't run periodics

  //  if ($op == 'ALL') { //  or strpos(DAILY_TASKS, " $op ")
  ///  if (fmtDt(now(), 'H') < R_DAILY_HOUR) die('Developer: For testing between midnight and :R_DAILY_HOUR:00, you need to temporarily change R_DAILY_HOUR in -settings.');
  //    db\q('TRUNCATE queue');
  //    setV('last_cron_end', ray('day week month', $now - DAY_SECS, 0, 0));
  //  } elseif ($op)
  //  cr\run();

  return TRUE;
}

/**
 * Return the credit info for the given uid.
 * To get just the balance, use uidCredit(uid)->balance
 * @param int $uid: return credit info for this account ID (FALSE if there is an error)
 * @param bool $ignoreCalc: <ignore errors when comparing with calculated values>
 */
function uidCredit($uid, $ignoreCalc = TRUE) {
  $info = be\creditInfo([$uid]);
  $a = r\acct($uid);
  if ($uid > 0 and !$a->jid) foreach (ray('balance') as $k) { // (community amounts don't get cached)
      $v1 = nni($info, $k, NULL);
      if (!t\eq("calculated $k vs cache", $v1, $a->o($k) + 0)) {
        if (!$ignoreCalc) return FALSE;
      }
    }
  return $info;
}

/**
 * Interpret the given identifier (int, full qid, or local qid) as an account ID.
 */
function uid($id, $field = '') {
  u\EXPECT(!$field, 'nonempty field');
  if (is_numeric($id) or empty($id)) { return $id; }
  $map = ray('ctty cgf bank regulars crumb round stepups',
             r\serverUid(), CGID, UID_BANK, UID_REGULARS, UID_CRUMBS, UID_ROUNDUPS, UID_STEPUPS);
  if ($res = nni($map, $id)) return $res;

  $qo = r\qo(fullQid2($id));
  return $qo->id; //  return r\qo($id)->$field; FAILS
}
function agentId($id) {return t\uid($id, 'agentId');}

/**
 * Return an account for the (probably abbreviated) account ID, doing error checking.
 * @param string $id: the ID
 * @return the account object (NULL if there is no account for that ID)
 */
function acct($id) {
  return r\acct(fullQid2($id)) ?: output(t('"%id" is not an account id', compact('id')));
}

function getErrors() {
  $errors = \drupal_get_messages('error', FALSE);
  $errors = nni($errors, 'error') ?: [];
  foreach ($errors as $key => $value) {
    if (u\abbreviates('Unable to send e-mail', $value) or mb_strpos(strip_tags($value), 'expected to be a reference, value given in drupal_retrieve_form()')) unset($errors[$key]); // ignore these when testing
  }
  return $errors;
}

/**
 * Say whether we sent the given email to admin or ctty admin.
 * @param string $topic: message topic
 * @param assoc $subs: message parameters
 * @param int $coUid: <tell ctty admin or promo instead of overall admin>
 */
function weTellAdmin($topic, $subs = [], $coUid = FALSE) {
  global $testOnly; if (!$testOnly) return t\output('Cannot MAKE tellStaff!');

  $subs = !$subs ? [] : (nni($subs, 0) ?: $subs);
  $email = nni($subs, 'to', !$coUid ? R_ADMIN_EMAIL : (strhas($coUid, '@') ? $coUid : r\acct($coUid)->cttyA->email));
  
//  return t\findEmail('tell-staff', $email, $subs + ray('topic noFrame message', $topic, TRUE, '?'));
  return t\findEmail('tell-staff', $email, ray('topic noFrame message', $topic, TRUE, '?')); // including $subs crashes the test
}

function weTellPromo($topic, $subs = []) { return weTellAdmin($topic, $subs, 'info@' . CG_DOMAIN); }

/**
 * Say whether any such email has been sent to the specified address.
 */
function noSuchEmail($index, $email) {
  global $testOnly; if (!$testOnly) return FALSE;
  return !findEmail($index, $email, []);
  //  $subject = strtr($GLOBALS['emailSubjects'][$index], $esubs);
  //  $pattern = strtr('%"index":"INDEX","email":"EMAIL"%', ray('INDEX EMAIL', $index, $email));
}

/**
 * See whether the specified email has been sent (by searching the log file)
 * @param string $index: email message index
 * @param string $email: the email address to find
 * @param assoc $subs: parameters within the message
 * @return <email was sent>
 */
function findEmail($index, $email, $subs) {
  global $testOnly; if (!$testOnly) return t\output('Cannot MAKE findEmail!');
  global $testEmails; if (!nn($testEmails)) return t\output('There are no testEmails!');

  $subs = is_array(nni($subs, 0)) ? $subs[0] : nn($subs);
  extract(just('subject body noFrame', $subs, NULL));

  if (strhas($email, '@')) {
    $email = t\fixEmail($email);
    $a = r\acct($email);
  } elseif ($uid = t\uid($email) and $a = r\acct($uid)) $email = $a->email;

  t\fixData($subs);
  if (nn($a)) {
    foreach (ray('qid fullName emailCoded') as $k) if (!isset($subs[$k])) $subs[$k] = $a->$k; // no need to specify these
  } else $subs['emailCoded'] = '?';

  $esubs = [];
  foreach ($subs as $k => $v) {
    if ($k == 'nudge') $subs[$k] = $v = tr($v);
    $esubs["%$k"] = $v;
  }
  $message = $index ? ($noFrame ? r\emailTemplate($index) : r\emailBody($index)) : $body;
  $message = str_replace("\r\n", '', $message);
  $messageDpy = strtr(t\showSubs($message, $esubs), ray('href= style= src=', 'hrref=', 'styyle=', 'srrc='));
  $message = strtr($message, $esubs);
  u\setDft($subject, nni($GLOBALS['emailSubjects'], $index));
  $subjectDpy = t\showSubs($subject, $esubs);
  $subject = strtr($subject, $esubs);

  $pattern = '/%[A-Z]+[A-Z0-9]*/i';
  if (preg_match($pattern, tr($message . $subject), $m)) {
    t\output("message or subject contains unhandled subs in email \"$index\": " . pr($m));
  } else {
    $map = ["\n"=>'', R_BULLET=>'*', '&nbsp;'=>' '];

    foreach ($subs as $k => $v) {
      if ($v == '?') {
        $ignore[] = $k;
        unset($subs[$k]);
      } elseif ($v == 'TESTDOCODE') {
        $subs[$k] = t\doCode();
      } elseif ($k == 'subs') {
        $subs[$k] = pr($v); // fails
      } else $subs[$k] = tr(strtr(strip_tags($v), $map));
    }
    ksort($subs);

    foreach ($testEmails as $one) {
      extract($one, EXTR_PREFIX_ALL, 't'); // t_ means actual values
      if (is_array($t_toEmail)) $t_toEmail = key($t_toEmail);
      $t_toEmail = fixEmail($t_toEmail);
///     debug(compact(ray('t_index index t_toEmail email')));
      if ($t_index == $index and $t_toEmail == $email) {
        foreach ($t_subs as $k => $v) if (!is_array($v)) $t_subs[$k] = tr(strtr(strip_tags(nn($v)), $map));
        if (isset($t_subs['site'])) {
          $subs['site'] = BASE_URL; // site parameter is set in boot.inc so compiler.php can't see it.
          ksort($subs); // reorder
        }
        if (nn($ignore)) foreach ($ignore as $k) unset($t_subs[$k]);
        ksort($t_subs);
        $got = just(array_keys($subs), $t_subs);
        $wanted = just(array_keys($t_subs), $subs);
        if ($got == $wanted) return TRUE; // actual is exactly what was wanted
        t\output(compact('wanted', 'got'), 'email');
        foreach ($subs as $k=>$v) if (isset($t_subs[$k]) and $v != $t_subs[$k]) t\output("k=$k v=$v tv=" . nni($t_subs, $k), 'email');
        foreach ($t_subs as $k=>$v) if (isset($subs[$k]) and $v != $subs[$k]) t\output("k=$k tv=$v v=" . nn($subs[$k]), 'email');
      }
    }
  }

  output(['EXPECTED (but did not get)'=>''] + compact(ray('index email subjectDpy subs messageDpy testEmails')), 'email');
  return FALSE;
}

/**
 * Say whether any email was sent to the given address.
 * @return FALSE if any email was sent, else TRUE.
 */
function noEmailTo($email) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $testEmails;

  if (nn($testEmails)) foreach ($testEmails as $one) if ($one['toEmail'] == $email) return FALSE;
  return TRUE;
}

/**
 * Set a given phrase, encrypted, as the next code to be used in rSmart tests.
 */
function cryNextCode($phrase, $type = 'private') {
  global $testOnly; if ($testOnly) return FALSE;
  global $nextCode, $testVars; $testVars['nextCode'] = $nextCode = u\b64encode(u\cryPP($phrase, $type));
  return TRUE;
}

/**
 * Highlight the substitutions in a given text.
 * @param string $s: the text
 * @param assoc $subs: the substitutions
 * @return the text with substitutions highlighted
 */
function showSubs($s, $subs) {
  foreach ($subs as $k=>$v) $subs[$k] = "<span class=\"test-highlight\">$v</span>";
  return strtr($s, $subs);
}

/**
 * Simulate posting to a page from some other site (or app).
 */
function postToPage($page, $args) {
  global $testOnly; if ($testOnly) return FALSE;
  $params = t\ray2assoc($args);
  t\POST($params);
  return t\memberVisitsPage('?', $page);
}

function memberVisitsPage($id, $page) { // pass options in URL
  global $testOnly; if ($testOnly) return FALSE;
  if (!t\login($id)) return FALSE;
  fixTestVars($page);
  t\pageForm($page);
  return TRUE;
}

function weShowWith($title0, $content = FALSE, $mustBePresent = TRUE) {
  global $testOnly; if (!$testOnly) return FALSE;
  $title = tr($title0, nn($content[0])); // use multivalued field either as subs or as literal values to show
  if ($title and !weShow($title, $mustBePresent)) return FALSE;
  return $title == $title0 ? weShow($content, $mustBePresent) : TRUE;
}

function makePicture($photo, $a) {return $a->update(compact('photo'));}

/**
 * Send an RPC API request to the server, for rPOS.
 * @todo: after everyone is up to version 217, add penultimate param: $proof = 'calc'
 */
function hitServer($op, $info = [], $agent = NULL, $device = '', $version = 230) {
  global $rposResult;
  if ($agent) $agent = fullQid2($agent);
  extract(just('amount member code created proof goods', $info));
  if (is_numeric(nn($amount))) $amount = number_format($amount, 2);
  if (nn($member)) $member = fullQid2($member);
  if ($version >= 217) foreach (['agent', 'member'] as $k) if (nn($$k)) $$k = str_replace('.', '', $$k);
  if (nn($proof) == 'calc' and nn($member)) {
    if ($a = r\acct($member)) u\setDft($code, $a->cardCode());
    $company = ($i = strpos($agent, '-')) ? substr($agent, 0, $i) : $agent;
    $proof = u\hash($hashOf = "$company$amount$member$code$created");
    t\output("proof is hash of $hashOf", 'normal');
    unset($info['code']);
  }
  if (nn($goods) and !is_numeric($goods)) $goods = t\goodsBit($goods);

//  $input = @compact(ray('op agent device version amount member proof goods')) + $extra;
  foreach (ray('op agent device version amount member proof goods') as $k) if (isset($$k)) $info[$k] = $$k;
  t\output('request: ' . pr($info), 'input'); // keep this
  $rposResult = r\Smart\pos($info);
  if ($op != 'photo') $rposResult = u\dejsonize($rposResult);
  t\output('answer: ' . pr($rposResult), 'output');
  return TRUE;
}

function txCountIs($n) {
  global $testOnly; if (!$testOnly) return FALSE;
  return (db\count('tx_hdrs') == $n);
}

/**
 * Allow member or agent code to be abbreviated (without the region) as .YYY or x:y,
 * where ZZx is the company abbreviation and ZZy is the agent
 */
function fullQid2($agent) {
  // FAILS if (strlen($agent) > 7) return $agent;
  if (strpos($agent, ':')) { // company:agent
    list ($cpy, $agt) = explode(':', $agent);
    $cpy = \fullQid($cpy);
    $agt = is_numeric($agt) ? $agt : \fullQid($agt); // numeric agt actually fails anyway (fudge for superAdmin login -- agt=1)
    //    return \fullQid(r\qid($cpy, $agt));
    return r\qid($cpy, $agt);
  } elseif (strpos($agent, '#')) { // company#agentNum (deprecated)
    list ($cpy, $agt) = explode('#', $agent);
    $cpy = \fullQid('.ZZ' . $cpy);
    if (is_numeric($agt)) $agt = u\n2a($agt, 1, 36);
    return "$cpy-$agt";
  } else return \fullQid($agent);
}

function parseQr($qr) {
  list ($member, $code) = explode(',', $qr . ',');
  return [fullQid2($member), $code];
  //  list ($member, $m, $code) = mb_split('/([\.-])/', $qr, 0, PREG_SPLIT_DELIM_CAPTURE);
  //  $member = R_SERVER_ID . ($m == '-' ? ':' : $m) . $member;
}

function snapShot(&$form) { // (additional sta values have no effect at this stage)
  addAttribs($form);
  return $form; // the rest is redundant
  //global $formOut['text']; $zot = $form; $formOut['text'] = strip(\render($zot)); // for testing
  //output('ACTUAL or EXPECTED: ' . $formOut['text'], 'screen');
}

function addAttribs(&$thing) {
  foreach ($thing as $key => $value) { // else Drupal complains when rendering
    if (substr($key, 0, 1) == '#') continue;
    if (!is_array($value)) continue;
    if (!isset($value['#attributes']['id'])) $thing[$key]['#attributes']['id'] = 'edit-' . strtolower($key);
    if (!isset($value['#title_display'])) $thing[$key]['#title_display'] = 'before';
    addAttribs($thing[$key]);
  }
}

/**
 * Log the given id in.
 * @param string $id: full or partial qid or '?' (meaning anonymous user, so don't log in)
 * @return: successful (always successful for anonymous)
 */
function logIn($id) {
  global $box, $boxId, $mya, $user;
  if ($id == '?') {
    w\signout(TRUE);
  } else {
    list ($qid, $agt, $zot) = explode(':', "$id::"); // in case company:agent
    $a = r\acct(\fullQid($qid), $agt ? \fullQid($agt) : '');
    if (!u\isAcct($a)) return t\output("id not account: $id (qid=$qid agt=$agt)");

    if (!$a->eq($mya)) {
      w\signout(TRUE);
      if (!$a->proSe) r\setAcct($a->agentId); // get agent first, if not proSe
      r\setAcct($a);
    }
    $boxId = w\boxId($box, $a);
  }
  return TRUE;
}

/**
 * Compare the data to a corresponding record in the table.
 * Return TRUE if the record matches the data.
 * Ignore fields with value "?".
 * Example call:
 *   $info = doRelation($info, TRUE);
 *   return verifyRecord('u_relations', $info);
 * @param string $riskRay: how to describe risk bits, if necessary (K_TX_RISKS or K_TX_ACCTS)
 */
function verifyRecord($table, $info, $riskRay = K_TX_RISKS) {
  u\EXPECT(is_array($info), 'in verifyRecord');
  unset($info['data']);
  list ($infoK, $infoV) = [key($info), array_shift($info)]; // must be before ksort
  ksort($info); // sort everything else, but leave key out of it
  $info = [$infoK => $infoV] + $info; // keep key first
  ///  echo pr(compact(ray('info0 info1 info idSub infoK infoV')));

  foreach ($info as $k => $v) {
    if ($v == '?') {unset($info[$k]); continue;}
    $crit[] = $v === '' ? "(`$k` IS NULL OR `$k`=:$k)" // let a blank test field match null or empty string
      : (is_null($v) ? "IF(1, `$k` IS NULL, :$k)" // :$k is just to match subs
         : ((in($k, R_DATE_FLDS) or strhas($k, 'date')) ?
            "ABS(`$k`-:$k)<" . SAME_TIME
            : (in($k, 'trust risk') ? "CAST(ROUND(`$k`, 2) AS CHAR)<=>:$k"
               : "`$k`<=>:$k"))); // null-safe =
  }
  $crit = join(' AND ', $crit);

  if (!$result = db\exists($table, $crit, $info)) {
    foreach ($info as $k => $v) $subs[$k] = "'$v'";
    $sql = "SELECT * FROM $table WHERE " . strtr(u\SUBS($crit), u\prefixKeys(':', $subs));
    output("verifyRecord failed. " . pr(compact(ray('table infoK infoV sql'))));

    if ($actualRow = db\get('*', $table, [$infoK => $infoV])) {
      //      ksort($actualRow);
      //      output("Actual $table record at $infoK=$infoV: " . pr($actualRow));
      $difs = '';
      foreach ($info as $k => $v) {
        $v2 = nni($actualRow, $k);
        foreach (ray('v v2') as $vk) if (is_null($$vk)) $$vk = '<NULL>';
        if ($k == 'flags') foreach (ray('v v2') as $vk) $$vk = u\bits($$vk, B_LIST);
        if ($k == 'risks') foreach (ray('v v2') as $vk) $$vk = u\bits($$vk, $riskRay);
        if ($v2 != $v) $difs .= "<tr><td>$k</td><td>$v</td><td>$v2</td></tr>\n";
      }
      $hdrs = t('<tr><th>field</th><th>want</th><th>got</th></tr>');
      $hdrs .= "\n<tr><td>$infoK</td><td>$infoV</td><td>$infoV</td></tr>\n";
      output("Discrepancies ($table): <br><table>$hdrs$difs</table>");
    }
  }
  return $result;
}

/**
 * Simulate completion (and possible confirmation) of a form.
 * @param string $id: uid or qid for the user (or company agent) completing the form
 * @param string $page: path to form from drupal root
 * @param array $values: array of assoc of field names/values (or empty)
 * @param bool $confirmed: whether the user has confirmed the values (default FALSE)
 * @return TRUE on success
 *
 * @todo: Rewrite this whole form testing system to use just menu_execute_active_handler,
 *   drupal_validate_form, etc. (assuring official behavior)
 */
function completeForm($id, $page, $values = '', $confirmed = FALSE) {
  global $testOnly; if ($testOnly) return FALSE;
  global $fieldErr;
  global $testConfirmation; // confirmation message output, if any
  
  fixTestVars($page);
  $values = nni($values, 0, []);
  fixTestVars($values);
  global $testOp; $testOp = nni($values, 'op');

  global $formSta, $lastGo;
  $xlastGo = nn($lastGo); // notice whether a goto happens, so we can skip post-processing
  $lastGo = NULL;

  t\logIn($id);

  $validate = TRUE;
  if (u\starts($page, 'history/') and $confirmed and !strpos($page, '&do=')) { // re-examine this
    $validate = FALSE;
    $formSta['confirm'] = TRUE;
  }
  $values = t\POST($values);
  if (!$form = t\pageForm($page)) return FALSE; // probably access error
  $sta = $formSta; $formSta = NULL; // don't let $formSta influence other forms that get invoked

  if ($page == 'pay' or $page == 'charge') $page = 'Tx';

  $dfts = [];
  foreach ($form as $key => $field) if (substr($key, 0, 1) != '#') {
    if (isset($field['#value'])) $dfts[$key] = $field['#value'];
  }
  $sta['values'] = $sta['input'] = $values + $dfts; // add to whatever status the form left off in
  $fieldErr = FALSE; // no field errors yet
  if (!nn($lastGo) or $lastGo == $xlastGo) \form_execute_handlers('validate', $form, $sta);
  $lastIdentifierTag = '</div identifiers>';
  if ($i = strpos(nn($testConfirmation), $lastIdentifierTag)) $testConfirmation = substr($testConfirmation, $i + strlen($lastIdentifierTag));
  if ($fieldErr) return TRUE; // no submit if field error
  if (!nn($lastGo) or $lastGo == $xlastGo) \form_execute_handlers('submit', $form, $sta);

  return TRUE;
}

/**
 * Simulate a visit to the page, return its form.
 * @param string $page: the Drupal URL to visit
 * @return: the form before rendering
 */
function pageForm($page) {
  global $formArray, $formSta, $lastGo, $formOut, $lastPage;

  $page = t\parseQ($page);

  $page = t\fixPageArgs($page);
  $formArray = $formSta = $formOut = NULL; // start with a clean slate

  if (!$page) $page = 'dashboard';
  t\SERVER('REQUEST_URI', $page);
  $args = basename($page);
  $xlastGo = nn($lastGo); // notice whether a goto happens. ignore all but the deepest (last).

  $lastPage = $page;
  $result = \menu_execute_active_handler($page, 'test');
  if (is_numeric($result)) {
    $result = $result == MENU_NOT_FOUND ? 'not found'
      : ($result == MENU_ACCESS_DENIED ? 'access error' : "error #$result");
    t\output("page \"$page\": $result in t\\pageForm()");
  } elseif (nn($lastGo) == $xlastGo and test()) {
    u\EXPECT(!is_array($result), t('Page %page crashed (might be inadequate admin permissions).', compact('page')));
    if (!$result) list ($result, $formOut) = [$formOut, '']; // for example, for PDFs
    if ($formOut) t\output('TEST WARNING: formOut is not empty in t\pageForm(): ' . pr($formOut));
    setFormOut($result ?: ''); // '' for when $result is an empty array, eg couponsprint.inc
  }

  return $formArray; // this got set in w\showForm
}

function setFormOut($out) {
  global $formOut;

  $formOut = [];
  $formOut['options'] = t\dropdownOptions($out) . PHP_EOL . t\radioOptions($out);
  $formOut['steps'] = t\getSteps($out);
  $formOut['done'] = join('', t\getSteps($out, TRUE));
  $formOut['text'] = t\strip($out);
  $formOut['notags'] = strip_tags($out);
  $formOut['hasDate'] = strhas($out, 'type="date"');
  t\output('ACTUAL: ' . $formOut['text'], 'screen');
}

/**
 * Return an array of steps in the output
 * @param string $out: the rendered form
 * @param bool $set: TRUE = return just the done steps, FALSE = just the not-done steps, DEFAULT = both
 * @return: array of step numbers
 */
function getSteps($out, $set = NULL) {
  if (!nn($set)) $set = isset($set) ? '>' : '';
  if (!preg_match_all("/done-(\d) state$set/", $out, $matches)) return [];
  return $matches[1];
}

function postProcess($form, $formName, $type) {
  $functions = nn($form["#$type"]);
  if (!nn($functions) or $functions == '<') {
    $function = "CG\\Web\\form{$formName}_$type";
    $functions = function_exists($function) ? array($function) : [];
  }
  return $functions;
}

function postLogout($function) {
  return TRUE; // never actually log out
  global $skipToStep;
  $vars = array(
    'msgs' => nn($_SESSION['messages']) ?: [],
                'skipToStep' => $function, // tell gherkin function to skip steps in interrupted scene until this step
                );
  return FALSE;
}

function dropdownOptions($form) {
  if (preg_match_all('~<option value="([^"]*)"[^>]*>(.*?)</option>~sm', $form, $m)) {
    foreach ($m[1] as $i => $k) $res[] = "$k=>" . $m[2][$i];
    return join(PHP_EOL, $res);
  }
  //    return join(PHP_EOL
  if (preg_match_all('~<option [^>]*>.*?</option>~sm', $form, $m)) return strip_tags(join(PHP_EOL, $m[0]));
  return '';
}

/**
 * Return the selected items in <select> elements.
 */
function selected($form) {
  $form = '<select id="edit-cat-need" name="cat" class="filter form-control">

<option value="">Needs (all)</option>
<option value="1">food</option>
<option value="2">housing</option>
<option value="4">health</option>
<option value="6" selected>delivery</option>
<option value="7">childcare</option>
<option value="9" selected>animal care</option>
<option value="11">legal</option>
<option value="13">fellowship</option>
<option value="14">muscle/labor</option>
<option value="16">technology</option>
<option value="17">information/skills</option>
<option value="19">finance/money</option>
<option value="10">cleaning</option>
<option value="5">travel/rides</option>
<option value="21">stuff</option>
<option value="20">other</option>
<option value="98">my posts</option>
<option value="99">Needs (search)</option>
</select>';

  //  $pat = '~ select>([^,]+)</option~';
  $opt = '<option value="[^"]*"( selected)?>([^<]+)</option>\\s*';
///  debug("~<select [^>]*>\\s*($opt)+</select>~");
  if (!preg_match_all("~<select [^>]*>\\s*($opt)+</select>~", $form, $m)) return [];
///  debug($m);
  
}

function setChecked($name) {global $tChecked; $tChecked[] = $name;}
function setRadio($name, $title) {global $tRadios; $tRadios[$name] = $title;}

function radioOptions($form) {
  if (!preg_match_all('~<label class="option" [^>]*>.*? </label>~sm', $form, $matches)) return '';
  return strip_tags(join(PHP_EOL, $matches[0]));
}

/**
 * Compare the wanted values in an array to what we got.
 * For associative arrays, wanted values can be in any order.
 * For numeric-indexed arrays, the indices must match.
 * In either case, got can have additional values.
 */
function objEq($desc, $want, $got) {
  $wantDesc = t(' Wanted: ') . pr($want);

  if (is_scalar($got)) {
    if (is_scalar($want)) return t\eq($desc, $want, $got);
    return t\err($desc . t(' Expected an array/object, but got a string.') . $wantDesc);
  } else $got = (array) $got;
  
  if (is_scalar($want)) fixTestVars($want);
  if (is_scalar($want) and !$want = (array) json_decode($want)) return t\err($desc . t(' Expected a string but got and array/object: ') . $wantDesc);
  $wantDesc = t(' Wanted: ') . pr($want); // redo this with non-scalar vars fixed

  foreach ((array) $want as $k => $v) {
///    t\output(tr('%got=%want?', 'got want', pr($got), pr($want))); // debug(
    if (!rayhas($got, $k)) return t\err($desc . t(' Missing value for property "%k".', compact('k')) . $wantDesc);
    if (is_scalar($v) and is_scalar($got[$k])) {
      if (!eq("$desc [$k]", $v, $got[$k])) return t\err($wantDesc); // eq() outputs the specifics
    } else {
      if (!objEq("$desc [$k]", $v, $got[$k])) return FALSE;
    }
  }
  return TRUE;
}

/**
 * Compare two values and complain if they aren't equal (or close to equal).
 * @param string $desc: description of the things that should be equal
 * @param scalar $want: value wanted
 * @param scalar $got: actual value
 * @param string $id: relevant account ID, if any
 * @param bool $list: if true, treat $want and $got as names of flag bits
 * @param numeric $offBy: how much the values can differ and still be considered equal
 * @return <the values are equal>
 */
function eq($desc, $want, $got, $id = NULL, $list = '', $offBy = 0) {
  global $testVars;
  foreach (ray('want got') as $k) fixTestVars($$k);
  $want = str_replace('\\n', "\n", nn($want));
  if ($list) foreach (ray('want got') as $k) $$k = u\bits($$k);
  if ($want == '?') return TRUE;
  if (is_array($want)) list ($want, $got) = [pr($want), pr($got)];

  $match = (is_numeric($want) and is_numeric($got)) 
    ? (abs($want - $got) <= $offBy)
    : strMatch($want, $got);
    
  if ($match) return TRUE;

  $id = $id ? "uid=$id " : '';
  return t\output(t('%id%desc: wanted "%want" got "%got"', compact(ray('id desc want got'))));
}
function badEq($desc, $want, $got, $id = '', $list = '') {return !eq($desc, $want, $got, $id, $list);}

/**
 * See if we got the string that was wanted, treating '?' as a wildcard.
 */
function strMatch($want, $got) {
  if ($want == $got or (empty($want) and empty($got))) return TRUE;
  if (empty($got) or !strhas($want, '?')) return FALSE;
  
  $escape = '.^$*+()[{\\|]/';
  $chars = '\\' . join('\\', str_split($escape));
  $want = preg_replace("/[$chars]/", '\\\\$0', $want); // 4 backslashes (instead of 2) seems odd, but it's what works
  $want = str_replace('?', '.*', $want);
  return preg_match("/^$want$/", $got);
}

/**
 * Fake $_SERVER params for testing
 */
function SERVER($key, $value = NULL) {
  if (!test()) return $_SERVER[$key];
  global $T_SERVER;
  if (isset($value)) $_SERVER[$key] = $T_SERVER[$key] = $value; else return $T_SERVER[$key];
}

/**
 * Fake $_POST params for testing
 */
function POST($values = NULL) {
  global $T_POST, $T;
  
  if (!isset($T)) test(FALSE); // reset this
  if (isset($values)) {
    t\fixData($values);
    $_POST = $T_POST = $values;
  }
  return $T_POST;
}

/**
 * Clear out all possible data, so as to start the test fresh
 * @param int $ch: the channel (web, sms, etc.)
 */
function clear($ch = 0) {
  /**/  if (!isDEV or isPRODUCTION) die('clear ' . DEV_ONLY_MSG);
  include_once __DIR__ . '/admin/admin.inc';
  global $mya; $mya = NULL;
  global $sms_devel; $sms_devel = TRUE;
  global $newThresholds; unset($newThresholds);
  global $testRewardStep; $testRewardStep = 500;
  global $T_POST; $T_POST = $_POST = []; // setting $_GET=[] makes things go kaflooey

  $vars = 'lastGo formOut formArray formSta testOp testConfirmation testLink tChecked tRadios testEmails rAPIResult testWho nextCode testSeq testFirst testThen testQr testVars testRecs';
  foreach (ray($vars) as $k) { global $$k; $$k = NULL; }

  $formOut['text'] = '';

  global $channel; $channel = $ch;
  global $testMessages; $testMessages = [];
  global $sceneTest;
  global $conf; $conf = [];

  r\deleteAllCooksBut('');
  foreach (ray('error status') as $one) u\deleteFile(t\dosayFilename($one));

  if (test() and !strpos($sceneTest->name, 'Statement')) {
 /**/  echo "<!--scene: $sceneTest->module / $sceneTest->feature / $sceneTest->name-->\n"; // VERY helpful when test dies
  }

  t\log('clearing');
  w\signout(TRUE);
  \drupal_static_reset(); // clear out drupal central storage

  $tables = [             // RELATED TO
    'admins',             // users
    'cache',              // -
    'flood',              // -
    'messages',           // posts
    'people',             // posts
    'posts',              // -
    'queue',              // -
    'r_bad',              // -
    'r_votes',            // r_ballots, r_options
    'r_ballots',          // r_questions, users
    'r_changes',          // users
    'r_ratings',          // r_investments, users
    'r_shares',           // r_investments
    'r_investments',      // users, or is that u_company?, club?
    'r_do',               // users
    'r_pairs',            // r_options
    'r_options',          // r_questions
    'r_questions',        // r_events
    'r_events',           // -
    'r_honors',           // users
    'r_invites',          // users
    'r_ips',              // users
    'r_near',             // users
    'r_notices',          // users
    'r_proposals',        // -
    'r_proxies',          // users
    'r_stakes',           // users, club?
    'r_stats',            // users
    'r_tous',             // users
    'r_usd2',             // tx_hdrs
    'r_user_industries',  // r_industries, users
    'semaphore',          // -
    'sessions',           // users
    'tx_bads',            // -
    'tx_disputes_all',    // tx_hdrs, users
    'tx_entries_all',     // tx_hdrs, users, tx_rules
    'tx_hdrs_all',        // users, r_boxes, self 
    'tx_requests_all',    // users
    'tx_rules',           // users, u_groups, r_industries, tx_timed
    'tx_timed',           // users, u_groups, r_industries
    'txs2',               // tx_hdrs, users
    'r_boxes',            // users
    'u_company',          // users
    'u_groupies',         // u_groups, users
    'u_groups',           // -
    'u_photo',            // users
    'u_relations',        // users
    'u_shout',            // users

    'x_company',          // users
    'x_photo',            // users
    'x_relations',        // users
    'x_shout',            // users
    'x_txs2',             // tx_hdrs, users
    'x_users',            // standaloneish
  ]; // never truncate: r_areas r_banks r_countries r_criteria r_industries r_regions r_states r_transit users variable

  foreach (ray('bootstrap form') as $k) $tables[] = "cache_$k"; // don't empty cache_menu
  foreach ($tables as $table) db\q("TRUNCATE $table");

  db\q('DELETE FROM users WHERE uid<>0');
  db\q('DELETE FROM r_criteria WHERE ctty<>0');
/**/  foreach (ray(TX_CHANNELS) as $chan => $zot) u\deleteFile(flog('', $chan));
  u\deleteFile(SQL_LOG);

  $varKeys = ray('cgf_uid code drupal_private_key iv menu_expanded menu_masks up');
  db\q('DELETE FROM variable WHERE name NOT IN (:varKeys)', compact('varKeys'));
  a\setupBasicAccounts();

  db\q('UPDATE users SET floor=0,balance=0,minimum=NULL,rewards=0,committed=0');

  a('aab')->cttyA->setBit(B_OK);
  a('aab')->cttyA->setBit(B_UP);
}

function fixPageArgs($page) {
  $special = array(
                   //    '(by|id)=([A-Z]{3}?' . R_MARKS . '[A-Z]{3})',
                   '(flow)=(.*?)',
                   );
  foreach ($special as $one) if (preg_match($pat = "`(/|&)$one(&|$)`", $page, $matches)) {
    list ($zot, $start, $word, $what, $end) = $matches;
    if ($word == 'by' or $word == 'id') $what = t\uid($what);
    if ($word == 'flow' and !is_numeric($what)) $what = u\consta('flow', $what);
    $page = preg_replace($pat, "$start$word=$what$end", $page);
  }
  return $page;
}

/**
 * Interpret the risk bits.
 * @param string $risks0: comma-separated list of names of risk bits to set
 * @param string $descs: comma-separated list of all possible risk bit names, with weights
 * @return an integer bit array of the desired risks
 */
function getRisks($risks0, $descs) {
  if (!$risks0) return 0;
  $bits = array_flip(array_keys(ray($descs)));
  $risks = 0;
  foreach (ray($risks0) as $bit) $risks |= u\bit($bits[$bit]);
  return $risks;
}

/**
 * Did we say it or not?
 */
function weSayWithSubs($type, $index, $subs) {
  global $testOnly;

  if (!in($type, $choices = 'status warning error')) return output(t('"%type" is not a proper type (%choices)', compact(ray('type choices'))));

  $subs = nn($subs[0]) ?: array();
  if (isset($subs['email'])) $subs['email'] = t\fixEmail($subs['email']);
  foreach (explode('|', $index) as $one) if (!tr($one)) return t\output(t('Empty %type message in "%index": "%one"', compact(ray('type index one'))));
  $msg0 = tr($index);
  foreach ($subs as $key => $value) {
    if ($value == '?') $wild = $subs[$key] = u\randomString(20, 'word'); // mark the wildcard spot
    if (mb_strpos($msg0, "%$key") === FALSE) return t\output("missing sub '$key' in message '$index'");
  }
  $wantMsg = trim(t($index, $subs));
  $test = \sceneId();
  $msgs = db\col('value', 'test', compact('type', 'test')); // don't look at things added by test.php or by other tests
///  debug(compact(ray('wantMsg msgs subs')));
  $res = FALSE;
  if (isset($wild)) {
    $i = mb_strpos($wantMsg, $wild);
    $tailLen = strlen($wantMsg) - $i - strlen($wild);
    foreach (nn($msgs) ?: [] as $actual) {
      if (mb_substr($actual, 0, $i) == mb_substr($wantMsg, 0, $i) and mb_substr($actual, -$tailLen) == mb_substr($wantMsg, -$tailLen)) {$res = TRUE; break;}
    }
  } else {
    $tmsg = strip_tags($wantMsg);
    foreach (nn($msgs) ?: [] as $actual) {
      $actual = strip_tags($actual);
      $actual = strtr($actual, ["ACTUAL: \r\n"=>'', "Confirm\r\n"=>'', "\r\nOkay\r\n"=>'', "Cancel\r\n\r\n"=>'']);
      if ($actual == $tmsg) {$res = TRUE; break;}
    }
  }
  if (!$res) t\output("EXPECT: $wantMsg", 'output'); // this must come after array_search!
  return $res;
}

/**
 * Say whether the most recent page included the given text or options.
 * @param array $content: array of assocs -- named values to check for
 *   The names (labels) will be sought once also, unless preceded by underscore (_).
 *   The array can go either way, horizontally or vertically
 * @param string $type: description of what to check for (default 'text')
 * @param bool $mustBePresent: whether to complain about missing info (otherwise complain about info that is present)
 * @return TRUE if all the content is included
 */
function shownWith($content, $type = 'text', $mustBePresent = TRUE) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $formOut;

  $model = t\strip(nni($formOut, $type));
  //  if (!$model and $type == 'text') $model = file_get_contents(TEST_PDF_FLNM);

  if (!$content) return FALSE; // probably an accidental single-row array with no labels
  u\EXPECT(is_numeric(key($content[0])), 'shown with should be array, not assoc: ' . pr($content));
  $ray = call_user_func_array('array_merge', $content);

  $xi = $i = 0;
  foreach ($ray as $one) {
    if ($one === '' or mb_substr($one, 0, 1) == '~') continue; // ignore ~labels
    if (strhas($one, '%')) $one = tr($one); // translate constants in member
    if (($one = u\redash($one)) === '') continue;
    $i = $i > mb_strlen($model) ? FALSE : mb_strpos($model, $one, $xi = $i);
///    debug("one=$one i=$i xi=$xi");
    if ($i === FALSE) {
      if (nn($formOut['hasDate']) and $onetm = strtotime($one) and date('m/d/Y', $onetm) == $one) { // date formatted m/d/Y
        $i = mb_strpos($model, date('Y-m-d', $onetm), $xi); // look for Y-M-d instead (quirk of input type="date")
      } else { $i = mb_strpos($model, plain($one), $xi); }
    }
    if ($i === FALSE xor !$mustBePresent) {
      t\output(t('EXPECT page %with %this', 'with this', $mustBePresent ? t('with') : t('without'), u\purify($one, '')));
      t\output(compact('xi','i','one'));
      if (!$mustBePresent) $xi = $i; // show what's found if it's not supposed to be there
      t\output(mb_substr($model, 0, $xi) . ' <b class="test-doublesize">[OK until here]</b> ' . mb_substr($model, $xi));
      return !$mustBePresent;
    } else $i += mb_strlen($one);
  }
  return $mustBePresent;
}

/**
 * Say whether we have shown the given text.
 * @param bool $show: <complain if not shown> (else complain if shown)
 */
function weShow($what, $show = TRUE) {
  global $testOnly; if (!$testOnly) return FALSE;
  if ($what === '') return TRUE;
  return t\shownWith(is_array($what) ? $what : [[t($what)]], 'text', $show);
}

function constants($list) {
  global $testOnly;
  foreach ($list as $one) {
    extract(just('name value', $one));

  }
}

/**
 * Say whether the member has shown a photo ID or (if not just testing) make it so.
 * @param string $id: abbreviated member ID
 */
function noPhotoId($id) {
  global $testOnly;
  if (!$a = r\acct(t\uid($id))) return t\output("Not an account: $id");
  return $testOnly ? !$a->ided : $a->setBit(B_IDED, FALSE);
}

function weMessage($index, $id, $subs) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $testMessages;

  $subs = $subs[0];
  $uid = t\uid($id);

  foreach ($testMessages as $one) {
    extract($one, EXTR_PREFIX_ALL, 't');
///    debug(compact(ray('t_uid uid t_index index'))); // useful for trim errors
    if ($t_uid == $uid and $t_index == $index) {
      $matching = true;
      foreach ($subs as $k => $want) if ($want != '?') {
        $got = nni($t_subs, $k);
        if (in($k, R_DATE_FLDS) ? !sameTime($k, $want, $got) : !eq($k, $want, $got))  {
          output("$k: want $want, got $got");
          $matching = FALSE; break;
        }
      }
      if ($matching and !preg_match('/[@\{]/', nn($t_topic))) return TRUE;
    }
  }
  return t\output('Overall wanted: ' . pr(compact(ray('index id uid subs'))) . '<br>Got: ' . pr($testMessages));;
}

/**
 * Check the linked URL content and any Drupal messages (which don't show up in file_get_contents) against expectations.
 */
function hasLinkResults($info) {
  global $testLink, $formOut;
  global $testOnly; if (!$testOnly) return FALSE;

  if (!preg_match('/ href="([^"]*)"/', $testLink, $matches)) return FALSE;
  $url = $matches[1];

  $arrContextOptions = [ "ssl" => [ 'verify_peer' => FALSE, 'verify_peer_name' => FALSE ] ];
  $url = preg_replace('@' . preg_quote(BASE_URL, '@') . '@', LOCAL_URL, $url);
  $zot = file_get_contents($url, FALSE, stream_context_create($arrContextOptions));
  foreach (ray('error status') as $one) $$one = u\getFile(t\dosayFilename($one));
  $formOut['text'] = "$error$status";

  return t\shownWith($info, 'text', TRUE);
}

function seq($table = 'txs2') {
  global $testSeq;
  $id = $testSeq[$table] = nn($testSeq[$table]) + 1;
  $k = $table == 'txs2' ? 'txid' : '';
  //  t\log("seq table=$table id=$id");
  u\EXPECT(!db\exists('txs2', "$k=:id", compact('id')), "duplicate seq: $id in table $table");
  ///  print_r(compact('table','testSeq','id','k'));
  return $id;
}

/**
 * Return the state and country, defaulted (if any) and fixed.
 */
function fixState($state, $country) {
  if (!nn($state) and !nn($country)) return [];
  u\setDft($country, 'US');
  if ($country == 'US' or $country == US_COUNTRY_ID) u\setDft($state, 'MA');

  if (!$country or !is_numeric($country)) $country = db\get('id', 'r_countries', ':country IN (name, iso_code)', compact('country'));
  if (!$state or !is_numeric($state)) $state = db\get('id', 'r_states', ':state IN (name, abbreviation) AND country_id=:country', compact('state', 'country'));
  return compact(ray('state country'));
}

/**
 * Create a code to pass in a URL to the posts feature.
 * @param assoc $code: record id and creation date, either: [postid, created] or [id, created]
 */
function postCode($code) {
  fixData($code);
  extract($code);

  foreach (ray('posts:postid, messages:id') as $tnm => $idnm) if (nn($code[$idnm])) {
    $dt = db\get('created', $tnm, "$idnm=" . $code[$idnm]);
    if (t\sameTime('created', $dt, $created)) $code['created'] = $dt; // wiggle room
  }
  return u\cryRay($code, 8);
}

function getLocus($q) {
  return strpos($q, 'Greenfield, MA') !== FALSE ? [GFLD_LAT, GFLD_LON, GFLD_ZIP]
    : FALSE;
}

/**
 * Launch the given page in the default browser, appending the given javascript.
 * This is kinda complicated. Search for "jsCode" (occurs in 5 files: this, bootstrap.inc, scraps.js, ajax.inc, html.tpl)
 */
function dojs($page, $id, $js) {
  global $base_url;
  $qid = t\acct($id)->qid;
  $jsCode = u\cryRay(compact('js'));
//  return FALSE; // this suddenly started failing in windows (deep recursion exhausting session memory?)
  t\execWithTimeout("start \"\" \"$base_url/$page?qid=$qid&jsCode=$jsCode\"", DOJS_TIMEOUT); // leave enough time to display page
  return TRUE;
}

/**
 * Execute a command and return its output. Wait until the time limit has expired (even if the command ends).
 *
 * @param string $cmd: command to execute.
 * @param number $timeout: time limit in seconds.
 * @return string: output of the command.
 */
function execWithTimeout($cmd, $timeout) {
  // File descriptors passed to the process.
  $descriptors = array(
    0 => array('pipe', 'r'),  // stdin
    1 => array('pipe', 'w'),  // stdout
    2 => array('pipe', 'w')   // stderr
  );
  $options = []; // ['bypass_shell' => TRUE];

  // Start the process.
  $prefix = strihas(PHP_OS, 'WIN') ? '' : 'exec ';
  $process = proc_open($prefix . $cmd, $descriptors, $pipes, NULL, NULL, $options);
  u\EXPECT(is_resource($process), 'Could not execute process');

  stream_set_blocking($pipes[1], 0); // Set the stdout stream to non-blocking, so stream_get_contents returns immediately.
  stream_set_blocking($pipes[2], 0); // Set the stderr stream to non-blocking, so stream_get_contents returns immediately.

  $timeout *= 1000000; // Turn the timeout into microseconds.

  $buffer = ''; // Output buffer.

  while ($timeout > 0) { // Wait until we have output or the timer expired.
    $start = microtime(true);
    $read  = array($pipes[1]);
    $other = array();
    stream_select($read, $other, $other, 0, $timeout);

    // Get the status of the process. Do this before we read from the stream,
    // this way we can't lose the last bit of output if the process dies between these functions.
    $status = proc_get_status($process);

    // Read the contents from the buffer. This function will always return immediately as the stream is non-blocking.
    $buffer .= stream_get_contents($pipes[1]);

// (end on timeout only)    if (!$status['running']) break; // Break if the process exited before the timeout.

    $timeout -= (microtime(true) - $start) * 1000000; // Subtract the time we waited.
///    if ($timeout % 10000 == 0) echo '.';
  }
  
  $errors = stream_get_contents($pipes[2]);
  u\EXPECT(empty($errors), $errors);

  proc_terminate($process, 9); // Kill the process in case the timeout expired and it's still running.

  for ($i = 0; $i < 3; $i++) fclose($pipes[$i]);// Close all streams.

  proc_close($process);

  return $buffer;
}

function hitAPI($op, $userId, $password, $requests) {
  global $testOnly; if ($testOnly) return FALSE;
  global $rAPIResult;
  include_once __DIR__ . '/forms/api.inc';

  $userId = fullQid2($userId);
  foreach ($requests as $i => $zot) fixData($requests[$i]);
  $request = u\jsonize(compact(ray('op userId password requests')));
  w\api(compact('request'));
  if (!$result_json = getExitMsg()) return FALSE;
  $rAPIResult = u\dejsonize($result_json);
  return TRUE;
}

function jsonResponses($wants, $gots) {
  global $testOnly; if (! $testOnly) return FALSE;
  foreach ($wants as $want) {
    fixData($want);
    if (! oneJsonResponse($want, $gots)) {
      output("Correct response not found " . pr($want));
      return FALSE;
    }
  }
  return TRUE;
}

function oneJsonResponse($want, $gots) {
  foreach ($gots as $got) if (rayEq($want, $got)) return TRUE;
  t\output("correct response '" . pr($want) .
           "' is not present in actual responses: '" . pr($gots));
  return FALSE;
}

function rayEq($wantRay, $gotRay) {
  $gotRay = (array)$gotRay;
  fixData($gotRay);
  foreach ($wantRay as $k => $v) {
    if ($v == '?') continue;
    if (! array_key_exists($k, $gotRay)) return t\output("$k missing from actual: " . pr($gotRay));
    $gotV = $gotRay[$k];
    if (in($k, R_DATE_FLDS) and sameTime($k, $gotV, $v)) continue;
    if (t\badEq("For $k", $v, $gotV, flagList($k))) return FALSE;
  }
  return TRUE;
}

/**
 * Handle URLs containing question mark (put the part after the question mark in $T_GET)
 * @param string $page: the URL
 * @return $page without query parameters
 */
function parseQ($page) {
  global $T_GET;
  if ($i = strpos($page, '?')) {
    parse_str(substr($page, $i + 1), $get);
    u\_GET($get);
    $page = substr($page, 0, $i);
    t\output("Parsed out $page with GET: " . pr($T_GET));
  }
  return $page;
}

/**
 * Create an identifiable photo for the member.
 */
function hasPic($id, $picture) {
  global $testOnly; if ($testOnly) return FALSE;
  if (!$a = r\acct(t\uid($id))) return t\output("Not an account: $id");
  return t\makePicture($picture, $a);
}
  
/**
 * Add a record with the given type and value to the test table, along with the test name.
 */
function logRec($type, $value) {
  $info = ray('type value test', $type, is_array($value) ? serialize($value) : $value, \sceneId());
  return db\insert('test', $info);
}

/**
 * Return the latest exit message.
 */
function getExitMsg(&$status = NULL) {
/**/  $id = db\get('id', 'test', "type='exit' ORDER BY id DESC");
  $res = db\get('value', 'test', compact('id'));
  if (preg_match('/^%([0-9]+): (.*)/', $res, $m)) list($zot, $status, $res) = $m;
//  db\update('test', ray('id type', $id, 'wasExit'), 'id');
  return $res;
}

function fixTestVars(&$parm) {
  global $testVars, $nextCode, $testCode; 

  $testVars['nextCode'] = $nextCode;
  if (is_array($parm)) {
    foreach ($parm as $k => $v) fixTestVars($parm[$k]);
  } else foreach ($testVars as $k => $v) if (!is_array($v)) $parm = str_replace('TESTCODE', nn($testCode), str_replace("%$k", nn($v), nn($parm)));
}

function itsLater() {return timeIs(now() + 1);}
function timeIs($tm) {global $testOnly, $tTime; return $testOnly ? FALSE : ($tTime = $tm);}
function log($msg) {if (test()) u\log($msg, 'debug');}
//function that($thing) {global $thatThing; return nni($thatThing, $thing, FALSE);}
function dosayFilename($type) {return LOG_DIR . "dosay-$type.txt";}
function goodsBit($s) {return $s == 'goods' ? "1" : ($s == 'cash' ? "0" : "ERROR");} // no 3rd why for rSmart
function sameTime($desc, $a, $b) {return t\eq($desc, $a, $b, '', '', SAME_TIME);}
function rewarding() {return a('aab')->cttyA->cttyRewardy;}
function ray2assoc($ray) {foreach ($ray as $one) $res[$one[0]] = $one[1]; return nn($res) ?: [];}
function doCode($code = '') {if ($code) file_put_contents(DOCODE_FLNM, $code); else return file_get_contents(DOCODE_FLNM);}
function getDevVKeyPw() { preg_match("/value='([^']+)'/", file_get_contents(DEV_CONFIG_FLNM), $m); return $m[1]; }
function getDevVKeyE() { setDevVKey(); return db\get('vKeyE', 'admins', 'uid=:UID_SUPER'); }
