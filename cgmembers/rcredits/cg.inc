<?php
/**
 * @file
 * All of the business logic common to the various rCredits interfaces
 * (SMS, web, smartphone, etc.)
 * EXCEPT:
 *   - the Drupalish callbacks, which are in rcredits.module
 *   - application interfaces to the user interfaces, which are in cg-backend.inc (i)
 *   - utilities, which are in cg-util.inc (u)
 */
 
namespace CG; // typically abbreviated as "r"
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Web as w; // use sparingly
use CG\Testing as t;

require_once __DIR__ . '/cg-strings.inc';
require_once __DIR__ . '/cg-db.inc';
require_once __DIR__ . '/acct.class';
require_once __DIR__ . '/qo.class';

define('RECENT_SECS', 60 * DAY_SECS); // back how long is "recent" (don't change this without recreating r_stats data)

/**
 * Save a record in the given table, in the appropriate regional section of the id field's number space.
 * @param string $table: table to save in (users or r_relations)
 * @param string $idField: name of record ID field (uid or reid)
 * @param assoc $record: the record to save (possibly with record ID already chosen)
 * @param int $region: uid of the region in which to create a new record (defaults to server region, negated)
 *   if the region is negated (>0), an individual or company account will be inserted -- otherwise a community
 * @return record ID of the inserted record
 */
function regionalInsert($table, $idField, $record, $region = NULL) {
  if (array_key_exists($idField, $record)) {return db\insert($table, $record, $idField);} // predestined id (probably for testing)
  list ($start, $end, $region) = r\regionRange($region);

  for ($try = 0; $try < 20; $try++) { // limit the retries
    if (!$id = $record[$idField] = u\firstUnusedId($idField, $table, $start, $end)) break;
    try {
      if (db\insert($table, $record, $idField)) return $id;
    } catch (Exception $e) { // BIG MYSTERY: why does this happen sometimes with no collision?
      r\tellAdmin(t('cannot save regional: ') . $e->getMessage(), $record + ['topic' => t('cannot save')]);
    }
  }
  u\EXPECT(FALSE, "failed to save $table record in region $region after $try tries.");
}

/**
 * Return an array of the region's start and end in the users table, inclusive (and region's record ID).
 */
function regionRange($region = NULL) {
  u\setDft($region, -r\serverUid()); // get positive of region UID
  $end = $region + ($region < 0 ? -(R_REGION_MAX - 1) : (R_REGION_MAX - 1));
  $start = $region < 0 ? $region - 1 : $region; // -1: do not use the uid of the region itself
  return [$start, $end, -$region];
}

/**
 * Save a record in the given table *that has no AUTO_INCREMENT* returning the record ID of the inserted record.
 * @param string $table: table to save in (anything but users or r_relations)
 * @param string $idField: name of record ID field (uid or reid)
 * @param assoc  $record: the record to save (possibly with record ID already chosen)
 * @return record ID of the inserted record
 */
function insert($table, $idField, $record) {
  if (@$record[$idField]) return db\insert($table, $record, $idField); // predestined id (probably for testing)
  for ($try = 0; $try < 20; $try++) { // limit the retries
    if (!$id = $record[$idField] = u\firstUnusedId($idField, $table, 1)) break;
    try {
      if (db\insert($table, $record, $idField)) return $id;
    } catch (Exception $e) { // BIG MYSTERY: can this happen sometimes with no collision?
      r\tellAdmin(t('cannot save: ') . $e->getMessage(), $record + ['topic' => t('cannot save')]);
    }
  }
  u\EXPECT(FALSE, "failed to save $table record after $try tries.");
}

/**
 * Say whether a given payment is giving back rewards to the community.
 * @param int $payer: record id of who is maybe giving back
 * @param int $payee: record id of payee
 * @param assoc $data: data record from transaction
 * @return <yes, giving back rewards>
 *//*
     function isGiveback($payer, $payee, $data) {
     return ($payee == r\acct($payer)->community and @$data['isGift']); // may be wrong if member changes community!
     }
   */

/**
 * Return an error message if the given invoice is already paid or doesn't exist.
 * @param int $nvid: record ID of invoice
 * @param assoc $inv: (RETURNED) the invoice record
 * @return the error message (FALSE if no error)
 */
function badUnpaidInv($nvid, &$inv = '') {
  $fields = 'nvid,status,amount,payer,payee,goods,purpose,flags,data,recursId,created';
  if (is_array($nvid)) $nvid = $nvid['nvid'];  // really shouldn't happen -- but apparently did at some point
  if (!$inv = db\get($fields, 'r_invoices', 'nvid=:nvid', compact('nvid'))) return 'no matching invoice';
  if ($inv['status'] >= TX_PAID) return 'already paid';
  return FALSE;
}

/**
 * Say whether an account's proposed transaction duplicates the last recent one.
 * @param int $aId: the account id
 * @param int $otherId: the other accountId
 * @param float $amount: the amount proposed to transfer from the account to the other account (can be negative)
 * @return TRUE if the proposed transaction duplicates the last one.
 * @see also X::cacheTotals(), where the transaction data is recorded
 */
function dupTx($aId, $otherId, $amount) {
  global $channel;
  if ($channel == TX_WEB) return FALSE; // allow dups on web interface
  $lastTx = r\acct($aId)->lastTx;
  if (!$lastTx) { return FALSE; }
  return (($lastTx->actorId == $aId and $lastTx->otherId == $otherId
           and ($lastTx->payerAmount == $amount or $lastTx->payerAmount == $amount))
          and ((now() - $lastTx->created) < R_DUPTX_MINS * MIN_SECS));
}
 
/**
 * Say whether the named employee works for the account owner.
 * @param string $name: the employee's name
 * @param int $us: the employer's account record ID (defaults to current account record ID)
 * @param bool $loose: <the employee need not be formally an employee>
 * @return int: the person's account record ID if s/he does work for us, otherwise FALSE
 */
function worksForUs($name, $us = NULL, $loose = TRUE) {
  u\setDft($us, r\acct()->id);
  $words = explode(' ', u\roughName($name)); // abbreviate middle names
  if (mb_strlen($words[0]) > 1) { // unless first name is already abbreviated
    for ($i = 1; $i < count($words) - 1; $i++) $words[$i] = mb_substr($words[$i], 0, 1) . '%';
  }
  $pattern = join(' ', $words);
  $where = 'r.main=:us' . ($loose ? '' : ' AND r.:IS_EMPLOYEE');
  
  $sql = <<< X
    SELECT u.uid, u.fullName FROM r_relations r
    INNER JOIN users u ON u.uid=r.other
    WHERE $where
    ORDER BY (u.fullName LIKE :pattern) DESC
X;
  $q = db\q($sql, compact('us', 'pattern'));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    //    $data = unserialize($data);
    $legalWords = explode(' ', u\roughName($fullName)); // not $data['legalName']
    if (count($words) != count($legalWords)) continue;
    for ($i = 0, $match = TRUE; $i < count($words) and $match; $i++) {
      if (!u\abbreviates(str_replace('%', '', $words[$i]), $legalWords[$i])) $match = FALSE;
    }
    if ($match) return $uid;
  }
  return FALSE;
}

/**
 * Join or unjoin this account to another.
 * @param mixed $reid: relations record ID or [main, other]
 *   where main and other are the main and related account record IDs
 * @param bool $join: whether to join the accounts (otherwise unjoin)
 * @return a message saying the join was successful
 */
function setJoin($reid, $join) {
  if (is_array($reid)) {
    list($main, $other) = $reid; 
  } else {extract(r\relation('main,other', $reid));}
  
  list ($a1, $a2) = [r\acct($main), r\acct($other)];

  if ($join) {
    $a1->unjoinAllBut($other); // zap any join or other join requests EXCEPT with $other
    r\acct($main, $other)->update('permission', r\perm(B_JOINT));
    if (r\relation('permission', $other, $main) == r\perm(B_JOINT)) { // confirming join
      $a1->join($a2);
      $msg = t('join success');
    } else { // requesting join
      $_atag = $a2->makeDo('join', $main); // seek agreement
      r\message($other, 'join accounts', ray('_atag name', $_atag, $a1->fullName));
      $msg = t('join request success');
    }
  } else {
    $a1->unjoinAllBut();
    $msg = t('Your accounts have been successfully separated.');
  }
  return $msg;
}

/**
 * Return the number of months since the first rdo of this type (for conciseness).
 */
function rdoMonth($time = NULL) {
  u\setDft($time, now());
  return round(($time - strtotime('12/1/2014')) / (MONTH_SECS));
}

/**
 * Return an array or dropdown of communities (just in the given region?).
 * @param int $region: current region uid
 * @param int $ctty: current community (if any, return a dropdown input field)
 * @return array of communities if $ctty is FALSE, otherwise a dropdown
 */
function cttys($region, $ctty = FALSE) {
  $where = 'uid<0'; // uid BETWEEN :region-(:R_REGION_MAX-1) AND :region-1
  $sql = "SELECT uid,fullName FROM users WHERE $where ORDER BY fullName";
  $ray = db\q($sql, compact('region'))->fetchAllKeyed();
  if ($ctty) {
    //    if (!@$cttys[$ctty]) u\preray([$ctty => $a->cttyA->fullName . t(' (outside the region)')], $cttys);
    $ray[r\serverUid()] = t('Seedpack (no community yet)');
    return w\selectFld(t('Community:'), '', w\dft($ctty), $ray);
  } else return $ray;
}

/**
 * Record the current IP as approved (for whitelisting in WHM)
 * @param int $uid: account record ID
 * @param string $device: one device code for this account (not very important, but interesting, especially for POS)
 */
function ipok($uid, $device) {
  $ip = $_SERVER['REMOTE_ADDR'];
  if (!db\exists('r_ips', 'ip=:ip', compact('ip'))) db\insert('r_ips', compact(ray('ip uid device')), 'ip'); 
}

/**
 * Wrap all email, so it doesn't go out when testing.
 * @param string $index: index to email template and subject (empty if supplying explicit body and subject)
 * @param mixed $to: where to send the message -- uid, account, email address, or [$email => $name]
 * @param assoc $subs: substitutions in template. may also include:
 *   noFrame:  don't show salutation and closing and footer
 *   subject:  specific subject
 *   body:     specific message
 *   cc:       email or [email => name] to CC
 *   pseudo:   who to send the mail from (name only -- source email is still us)
 * ... the following special substitions are always available:
 *   site, qid, zip3, shortName, and every cached field for the account
 * @param mixed $reply: uid, account, or email address to reply to OR 'ctty' (of $to) or NULL (CGF_EMAIL) OR [email => name]
 * @param mixed $attachment: path(s) to file(s) to attach, if any (string if just one, else array)
 * @return TRUE if the message got sent (or on dev machine pretending it got sent) else FALSE
 * NOTE: from is always SYS_EMAIL
 */
function rMail($index, $to0, $subs, $reply0 = NULL, $attachment = NULL) {
  global $testEmails, $mya;

  foreach (ray('secure vsecure') as $k) unset($subs[$k]); // show not even in error messages
  $subs['site'] = BASE_URL;
  $subs['orgPhone'] = CGF_PHONE;
  extract(just('subject body noFrame sms cc pseudo', $subs));
  
  if (!is_array($to0) and $toA = is_object($to0) ? $to0 : r\acct($to0)) {
    $toEmail = $toA->email;
    $to = [$toEmail => $toA->fullName];
    $subs += (array) $toA->account(); // this omits any special fields or special formatting
    $subs['qid'] = $toA->mainQid;
    $subs['shortName'] = $toA->name;
    $subs['zip3'] = substr($toA->zip, 0, 2); // temporarily show a wider region than zip3
    if ($toA->closed) return r\tellAdmin(t('attempt to email closed account'), compact(ray('index subs reply0')));
    if ($jid = $toA->jid) $jA = r\acct($jid);
    if ($toA->depends and !$toA->co) $jA = r\acct($toA->helper); // sponsored accounts are never joint
    if (nn($jA) and test()) $testEmails[] = ray('index toEmail subs', $index, $jA->email, $subs);
  } else $to = $toEmail = $to0;
  
  $testEmails[] = compact(ray('index toEmail subs'));

  if (!is_array($reply = $reply0) and !is_object($reply0)) {
    $reply = (!@$reply0 or $reply0 == 'region') ? [CGF_EMAIL => PROJECT] // standard
      : ($reply0 == 'ctty' ? $toA->cttyA : r\acct($reply0)); // get account object
  }
  
  if (is_object($replyA = $reply)) $reply = [$replyA->email => $replyA->fullName];

  foreach ($subs as $k0 => $v) if (!is_object($v) and !is_array($v)) {
    $k = "%$k0";
    $resubs[$k] = $v;
  }

  $subject = u\SUBS(strtr(@$subject ?: $GLOBALS['emailSubjects'][$index], $resubs), '%');
  $body = u\SUBS(strtr(@$body ?: (@$noFrame ? r\emailTemplate($index) : r\emailBody($index)), $resubs), '%');

  // log
  $logSubs = u\noSecrets($resubs) + ["\r\n" => '', ' style=' => ' stylish=']; // easier reading for log and tests & evade CSP
  $message = strtr($body, $logSubs);
  $erInfo = compact(ray('index toEmail subject message'));
  u\loga('email', $erInfo + ['toUid' => @$toA->id]);
  f('t.output', 'Actual (in rMail) ' . pr($erInfo), 'email');

  if (!isPRODUCTION and (isDEV or db\count('users') > 200)) { // make sure we never send rogue emails from test server with real data
/**/  if (!test() and (isDEV or ($mya and $mya->cAdmin))) debug(compact(ray('subject to body')));
    return; // no emails unless real!!
  }
  
  require_once DRUPAL_ROOT . '/vendor/swiftmailer/lib/swift_required.php';
  $transport = \Swift_SendmailTransport::newInstance('/usr/sbin/sendmail -bs');
/*
  $transport = \Swift_SmtpTransport::newInstance()
    ->setHost(CG_SMTP_SERVER)
    ->setPort(CG_SMTP_PORT)
    ->setEncryption(CG_SMTP_TLS)
    ->setUsername(CG_SMTP_USER)
    ->setPassword(CG_SMTP_PASS);
*/
  $mailer = \Swift_Mailer::newInstance($transport);

  $privateKey = "-----BEGIN RSA PRIVATE KEY-----\n" . str_replace(' ', "\n", DKIM_PRIVATE) . "\n-----END RSA PRIVATE KEY-----";
  $signer = new \Swift_Signers_DKIMSigner($privateKey, EMAIL_DOMAIN, 'system');
  
  $m = \Swift_SignedMessage::newInstance()
    ->attachSigner($signer)
    ->setSubject($subject)
    ->setFrom([SYS_EMAIL => nn($pseudo) ?: t('%PROJECT system')])
    ->setTo($to)
    ->setReturnPath(BOUNCE_EMAIL)
    ->setBody($body, 'text/html')
    ->addPart(strip_tags($body), 'text/plain'); // don't bother making the plain text pretty

  if (nn($cc)) $m->setCc($cc);
  
  if ($attachment) {
    if (!is_array($attachment)) $attachment = [$attachment];
    foreach (is_array($attachment) ? $attachment : [$attachment] as $flnm) {
      $attach = \Swift_Attachment::fromPath($flnm);
      $attach->setFilename(basename($flnm));
      $m->attach($attach);
    }
  }
  
  if ($reply) $m->setReplyTo($reply);
  if (@$sms) $m->setSubject('')->setTo($toEmail); // send as text message
  try {
    $mailer->send($m);
    if (nn($jA) and $index != 'tell-staff') { // also send to partner in joint account unless it's an admin notice
      $m->setTo([$jA->email => $jA->fullName]);
      $m->setSubject($subject . t(' (for %acct)', 'acct', $jA->fullName));
      $mailer->send($m);
    }
  } catch (Exception $e) {
    u\EXPECT($toEmail != R_ADMIN_EMAIL, 'cannot send admin email');
    r\tellAdmin(t('cannot send email'), $erInfo);
  }
  return TRUE;
}

/**
 * Add a deactivated or phony (and used) card code to the "bad" list.
 * @param string $qid: the account ID
 * @param string $code: the bad card code
 * @param string $msg: type of error
 */
function badCard($qid, $code, $msg) {
  $created = now();
  $info = compact('qid', 'code');
  if (!db\exists('r_bad', $info)) {
    $info += compact('created');
    db\insert('r_bad', $info, 'created');
    $index = 'card marked bad';
  } else $index = 'attempted card already marked bad';
  r\tellCO($index, $info + compact('msg'));
}

/**
 * Email an arbitrary message to the member, NOW (otherwise use notify).
 * @param int $uid: account record ID
 * @param string $index: index to arbitrary HTML message body (between salutation & closing) or the HTML itself
 * @param assoc $subs: message parameters
 * @param string $topic: optional specific topic (otherwise look in $GLOBALS['topics'])
 */
function message($uid, $index, $subs = [], $topic = '') {
  global $testMessages;

  $subs['otherName'] = @$subs['myName']; // messaging someone else about US, if anyone
  $reply = @$subs['reply'];
  $message = t($index, $subs);
  $topic = $topic ?: (t(@$GLOBALS['topics'][$index], $subs) ?: $index); // index should never be used literally (wrong language) but more importantly never fail
  $testMessages[] = compact(ray('uid index subs topic'));
  r\notify($uid, 'message', compact(ray('message topic reply')), TRUE);
}

/**
 * Notify the specified user, according to their preferences
 * @param integer $uid: account record id
 * @param string $index: message index
 * @param assoc $subs: substitutions/parameters for the message
 * @param bool $force: send by email NOW
 */
function notify($uid, $index, $subs = [], $force = FALSE) {
  global $channel;
  $a = r\acct($uid);
  u\EXPECT($a, 'no account');
  $subs['uid'] = $uid;
  $subs['qid'] = $a->qid;
  $subs['otherName'] = nn($subs['myName'], NULL); // notifying someone else about US, if anyone
  $fullName = $a->fullName;
  $warnIndexes = ray('new invoice,new offer,charge disputed');
  
  if ($force) {
    $mailed = r\rMail($index, $a, $subs, @$subs['reply'] ?: 'ctty');
  } else {
    list ($message, $created) = array(t($index, $subs), now());
    f('t.output', "Actual Notice to $fullName: $message");
    db\insert('r_notices', compact(ray('uid message created')), 'msgid');
  }
  
  if ($a->can(B_BYSMS)) {
///    $ok = SMS\send($a->phone, t($index, $subs));
///    if (!$ok) r\tellAdmin(t('failed sending SMS to %nm @ %phone', 'nm phone', $a->fullName, $a->phone));
  }

}

/**
 * Notify the specified user, according to their preferences
 * @param integer $uid: account record id
 * @param string $index: message index
 * @param array $entries: TxEntries about which we should notify
 * @param bool $force: send by email NOW
 */
function notifyEntries($a, $index, $entries, $force = FALSE) {
  global $channel;
  u\EXPECT($a, 'no account');
  $subs= [];
  $subs['uid'] = $a->uid;
  $subs['qid'] = $a->qid;
  /* $subs['otherName'] = @$subs['myName']; // notifying someone else about US, if anyone */
  $fullName = $a->fullName;
  /* $warnIndexes = ray('new invoice,new offer,charge disputed'); */
  
  if ($force) {
    $mailed = r\rMail($index, $a, $subs, @$subs['reply'] ?: 'ctty');
  } else { //if ($a->can(B_NOTIFY_EMAIL)) {
    list ($message, $created) = array(t($index, $subs), r\rTime());
    f('t.output', "Actual Notice to $fullName: $message");
    db\insert('r_notices', compact(ray('uid message created')), 'msgid');
  }
  
  if ($a->can(B_BYSMS)) {
    if ($num = $a->smsNumber) $smsed = SMS\send($num, t($index, $subs));
  }
  if (FALSE and !@$mailed and !@$smsed and array_search($index, $warnIndexes) !== FALSE) {
    $notification = str_replace('-', ' ', $index);
    $warning = t('cannot contact', compact('fullName', 'notification'));
  }
  if (@$warning) {
    if ($channel == TX_WEB) Web\say($warning);
    if ($channel == TX_SMS) SMS\send($a->smsNumber, $warning);
    if ($channel == TX_POS) { } // dunno how to handle this
  }
}

/**
 * Notify rCredits staff (Community Admin, if uid is specified and in their area)
 * @param string $index: the message or message index
 * @param assoc $subs: optional substitutions for the message and/or data to show (show both)
 * @param bool/int $uid: related account record id (TRUE means current account), defaults to none (NULL), 
 *    0 = suppress report of current account (to avoid confusion)
 * @param string $attachment: path to file to attach, if any
 * @return FALSE;
 */
function tellAdmin($index, $subs = [], $uid = NULL, $attachment = NULL) {
  global $mya, $channel;
/**/  if (isDEV and !test()) return debug(['tellingStaff' => $index] + $subs + compact('uid'));

  $message = t($index, $subs);
  foreach (ray(R_NOLOG) as $k) unset($subs[$k]); // secret stuff
  ksort($subs); // put the keys in a predictable order, for testing and easy reading

  $tellCOA = $uid == 1 ? $mya : ($uid == 0 ? NULL : r\acct($uid));
  $keyUid = nn($subs['uid']);
  $keyA = $tellCOA ?: ($keyUid ? r\acct($keyUid) : NULL);
  
  $currentAcct = $mya ? $mya->nameAndQid : NULL;
  $keyAcct = $keyA ? $keyA->nameAndQid : NULL;
  $email = $tellCOA ? $tellCOA->cttyA->email : R_ADMIN_EMAIL; // tell cAdmin if appropriate, else superAdmin
  $replyTo = ($keyA or $mya) ? ($keyA ?: $mya)->nameAndEmail : NULL;
  
  $topic = mb_strlen($index) < 50 ? strip_tags($index) : '';
  $message .= u\seeAssoc($subs);
  $noFrame = TRUE;
  $chan = ray(TX_CHANNELS)[$channel];
  $info = @compact(ray('cAdmin email topic message keyAcct currentAcct chan noFrame')) + $subs;

  r\rMail('tell-staff', $email, $info, $replyTo, $attachment);
  $info['topic'] .= t(' (to admin)');
  if ($email != R_ADMIN_EMAIL) r\rMail('tell-staff', SYS_EMAIL, $info, $replyTo); // always tell one or more staff (for now)
  return FALSE;
}

/**
 * Report something of interest to a community admin.
 * @param string $index: message index or text
 * @param assoc $subs: substitutions/parameter, if any, to include in the report
 * @param int $uid: record ID of an account in the community to report about.
 *    If none, choose something reasonable, reporting to superAdmin as a last resort.
 */
function tellCO($index, $subs = [], $uid = NULL) {
  if (!$uid) $uid = ($mya = r\acct()) ? $mya->id : @$subs['loserId'];
  return tellAdmin($index, $subs, $uid);
}

/**
 * Return the rough (physical) location of the entity with the given account (city, state, non-US country).
 * @param acct $a: the account
 * @param bool $complete: if TRUE, include the street address
 */
function location($a, $complete = FALSE) {
  $country = realCountry($a->country);
  $result = [];
  if ($complete) $result[] = (string) $a->address; // (string) in case address is NULL
  $result[] = $a->city;
  $result[] = $a->st . (($complete and $a->zip) ? ' ' . $a->zip : '');
  if ($a->country != R_COUNTRY_ID) $result[] = mb_strtoupper($country);
  foreach ($result as $key => $value) if ($value === '') unset($result[$key]);
  return join(', ', $result);
}

/**
 * Return the short qid and full name of the specified account.
 * For example, return ".ZZA Abe One".
 */
function definiteName($uid) {
  if ($uid <= 0) return '';
  $acct = r\acct($uid);
  return r\qid($uid) . ' ' . $acct->fullName;
}

/**
 * Return an rCard security code for the specified qid.
 */
function cardCode($qid) { 
  $codeLen = R_CARDCODE_LEN - (strlen($qid) - 6); // shorter code when qid len > 6
  return u\randomString($codeLen, 'word'); // create an rCard security code
}

/**
 * Return the account for the given gift card code.
 * @param string $code: the code to interpret
 * @param int $iCode: (RETURNED) the code sequence number
 * @return: the account or FALSE if error
 * @see acct::iCardCode()
 */
function iCardAcct($code0, &$iCode = NULL) {
  $code = str_replace(' ', '', $code0); // ignore spaces
  if (!preg_match('/[A-Z0-9]{' . ICARD_CODELEN_MIN . ',}/i', $code)) return FALSE;
  $iCode = u\ai2n(substr($code, ICARD_CODELEN_MIN)) + 0;
  $code = substr($code, 0, ICARD_CODELEN_MIN); // chop off sequence number
  $b = u\lpad(decbin(u\ai2n(substr($code, 0, 5))), ILEN_DIV). u\lpad(decbin(u\ai2n(substr($code, 5))), ILEN - ILEN_DIV);
  $b = u\xorBits($b, substr(R_INVITE_KEY, -strlen($b)), TRUE);
  $b = u\rotateBits($b, -($iCode + 1) * (bindec(substr(R_INVITE_KEY, 0, 31)) % strlen($b)));
  $region = u\n2a(bindec(substr($b, 0, ILEN_REGION)), 3);
  $tail = u\n2a(bindec(substr($b, ILEN_REGION, ILEN_TAIL)), -3);
  $security = u\n2ai(bindec(substr($b, -ILEN_SECURITY)), 3);

  return ($a = r\acct("$region$tail") and $security == $a->lowSecurity) ? $a : FALSE;
}

/**
 * Set a cookie
 * @param string $key: name of cookie
 * @param string $value: value of cookie
 * @param int $expires: time when cookie expires (defaults to never -- about 1000 years from now) 0=end of session
 * @param bool $raw: <suppress URL-encoding>
 * @return: the value
 */
function setCook($key, $value, $expires = NULL, $raw = FALSE) {
	u\EXPECT(($len = mb_strlen(rawurlencode("$key=$value"))) <= 4096, "cookie too long len=$len"); // 4094 not 4096 (browser bug?)
  u\setDft($expires, now() + 365250 * DAY_SECS);
	$func = $raw ? 'setcookie' : 'setrawcookie';
  $res = $func($key, $value, (int) $expires, '/'); // make it easy (fails without '/')
	u\EXPECT($res, 'cookie failed');
  $_COOKIE[$key] = $value; // make it available also before going to a new page
  return $value;
}

function setCryptCook($k, $v) {return setCook($k, substr(u\cry('C', $v), 3), 0, TRUE);}
function cryptCook($k) {return ($v = @$_COOKIE[$k]) ? u\decry('C', CRYPT_FLAG . 'C' . $v) : '';}

/**
 * Return the unformatted private key for highly sensitive data (use u\fmtKey() to get the formatted key).
 */
function vKey() {
	global $mya, $channel;
  u\EXPECT($channel == TX_WEB, 'wrong channel for vKey: ' . $channel);
  if (!@$mya) w\setAcct(); // make sure we got mya
	return ($vKeyE = r\cryptCook('vKeyE') and $pw = $mya->agentA->vKeyPw) ? u\decry('R', $vKeyE, $pw) : '';
}

/**
 * Return the rCredits region id for the given account record ID
 */
function uidRegion($uid) {
  $uid = abs($uid) - 1; // -1 because uids are all bumped up by 1
  return -1 - ($uid - ($uid % R_REGION_MAX));
}

/**
 * Return the rCredits region id for the given geographical location
 * @param int $country: country ID
 * @param int $state: state or province ID (defaults to MA)
 * @param int $zip: postal code
 * @param string $raw: (RETURNED) the alphabetic identifier for the region (usually 3 characters)
 * @return int: the region uid
 */
function region($country = '', $state = '', $zip = '', &$raw = '') {
  if (!$state) $state =  R_STATE_ID;
  $where = "IF(zip='', country_id=:country AND s.id=:state, :zip RLIKE zip) ORDER BY (zip='') LIMIT 1";
  $table = 'r_regions r LEFT JOIN r_states s ON s.abbreviation=r.st';
  if (!$region = db\get('region', $table, $where, compact(ray('state zip country')))) {
    $region = db\get("CONCAT(iso_code, 'A')", 'r_countries', 'id=:country', compact('country'));
  }
  $raw = $region ?: 'ZZZ';
  return r\qo("!{$raw}AAA")->id;
}

/**
 * Say whether one of the accounts owns the other (or can draw from it).
 */
function owning($uid1, $uid2) {
  $where = '(:IS_OWNER or :IS_DRAW) AND ((main=:uid1 AND other=:uid2) OR (main=:uid2 AND other=:uid1))';
  return $uid1 == $uid2 ?: relation(1, $where, compact('uid1', 'uid2'));
}

/**
 * Say whether the first account is an employee of the second (or of the current account)
 * @param int $company: defaults to current account ID
 */
function isEmployee($uid, $company = '') {
  return relation(1, $company ?: @r\acct()->id, $uid, ':IS_EMPLOYEE');
}

/**
 * Return the community record ID for the given postal code.
 * @param string $zip: postal code to identify community (if none, assume something in server community)
 * @param bool $defaultZero: return 0 if zip does not match any community? (else return server uid)
 */
function communityUid($zip = '', $defaultZero = FALSE) {
  if (!$zip) return serverUid();
  $where = "country=:R_COUNTRY_ID AND :zip RLIKE zip AND uid<1";
  return db\get('uid', 'users', $where, compact('zip')) ?: ($defaultZero ? 0 : serverUid());
}

/**
 * Return the name of the given community (optionally without the project name)
 */
function cttyName($uid, $omit = FALSE) {
  if (!$cttyA = r\acct($uid)) return '';
  $name = $cttyA->isRegion ? t('%PROJECT Seedpack') : $cttyA->fullName;
  return $omit ? trim(str_replace(PROJECT, '', $name)) : $name;
}

/**
 * Given a full name, make a username and add digits until it is unique.
 */
function uniqueName($fullName, $info = []) {
  if (!$fullName) {
    extract(just('phone email', $info));
    $fullName = R_TEMP_NAME_PREFIX .
      (@$phone ?  u\fmtPhone($phone, 'n')
       : (@$email ? R_TEMP_NAME_PREFIX . strtr($email, ['@'=>'AT', '.'=>'DOT'])
          : substr(md5('zot' . strval(now())), 0, 10))); // pseudo random
  }
  $name = $name0 = u\shortName($fullName);

  for ($i = 2; ; $i++) {
    if (!db\exists('users', 'name=:name', compact('name'))) return $name;
    $name = $name0 . $i;
  }
}

/**
 * Return the uid for the given qid or (short) name or email.
 * If $id is an email and more than one account has that same email address, the first non-company account is assumed.
 * @todo later: when regional prefix is required for signin, this function will ignore it in matching name field
 * @return the uid (FALSE if unsuccessful)
 */
function loginString($id) {
  if (u\isQid($id = trim(mb_strtoupper($id))) and $acct = r\acct($id)) return $acct->id; // try to interpret as a qid
  $email = u\cry('P', mb_strtolower($id));
  return db\get('uid', 'users', 'name=:id OR email=:email ORDER BY :IS_CO', compact('id', 'email'));
}
  
/**
 * Return a list of employee accts for the current account.
 */
function employees() {
  $myid = r\acct()->id;
  return db\q('SELECT other FROM r_relations WHERE main=:myid AND :IS_EMPLOYEE', compact('myid'))->fetchCol();
}

/**
 * Return the field or fields from the relations table, for the given main and other.
 * Call by:
 *   relation(field, main, other, where, subs) OR
 *   relation(field, where, subs)
 *   relation(field, reid)
 *   relation(field, qid)
 * @param string $field: what field value(s) to return (see varieties at db\get())
 * @param int $main: uid of main account
 * @param int $other: uid of other (agent) account (changed to zero if same as main)
 * @param string $where: SQL criteria
 * @param assoc $subs: criteria parameters
 * @param int $reid: relations record id
 * @param string $qid: QID for a compound account
 */
function relation($field, $main, $other = [], $where = 'TRUE', $subs = []) {
  if (is_numeric($main)) {
    if (empty($other)) { // reid
      list ($where, $subs) = [ray('reid', $main), []];
      /*      if (is_numeric($reid = $main)) {
              $where = "reid=:reid";
              $subs += compact('reid');
              } else { // 
              list ($main, $otherNum) = explode('-', $main);
              list ($main, $otherNum) = [r\qo($main)->id, u\a2n($otherNum)];
              list ($where, $subs) = ['main=:main AND otherNum=:otherNum', compact('main', 'otherNum')];
              } */
    } else { // field, main, other, where, subs
      if ($main == $other) $other = 0; // handle tentative proSe relation with future customer
      $where = "main=:main AND other=:other AND ($where)";
      $subs += compact('main', 'other');
    }
  } elseif ($other) { // where, subs
    list ($where, $subs) = [$main, $other];
  } else { // qid
    $a = r\acct($qid = $main);
    list ($where, $subs) = [ray('main other', $a->id, $a->agentId), []];
  }
  return db\get($field, 'r_relations', $where, $subs);
}

/**
 * Return our standard encryption password and a salt for the given account.
 */
function passSalt($acct = '') {
  $acct = $acct ?: r\acct();
  $pass = R_SALTY_PASSWORD;
  $salt = $acct->created;
  return compact('pass', 'salt');
}

/**
 * Return the first unused uid for the given base.
 * @param int $base: lowest uid to use (first slot in a region)
 * @return: the uid
 */
function newUid($base) {return u\firstUnusedId('uid', 'users', $base, $base + R_REGION_MAX - 1);}

/*  $where = 'u.uid>=:base and u.uid <:base+:R_REGION_MAX AND u2.uid IS NULL ORDER BY u.uid';
    $previous = db\get('u.uid', 'users u LEFT JOIN users u2 ON u2.uid=(u.uid+1)', $where, compact('base'));
    return $previous ? ($previous + 1) : $base;
    }*/

/**
 * Return a link to change the current account 
 * (presumably to one the current account is allowed to change to)
 * @param int $uid: record ID of account to change to
 * @param string $v: link text to display
 * @param string $class: CSS class, if any
 * @return the link
 */
function changeAcctLink($uid, $v, $class = '') {        
  global $base_url;
  $page = str_replace('/', ',', $_SERVER['REQUEST_URI']); // not urlencode because Drupal sees slashes anyway
  if ($class) $class = " class=\"$class\"";
  return "<a href=\"$base_url/change-account/acct=$uid&page=$page\"$class>$v</a>";
}

/**
 * Record the relation of a device with an account.
 * @param int $uid: record ID of related account
 * @param string $code: phone # or device code
 * @param int $ch: channel (SMS, Web, or App)
 * @return int: the device record ID
 */
function makeDevice($uid, $code = '', $ch = NULL) {
  global $channel;
  global $mya;
  
  u\setDft($ch, $channel);

  //  if ($ch == TX_WEB and $mya and $mya->superAdmin) return 0; // don't create spurious devices as admin
  /*  if ($boxName === '0') {
      $where = "t1.boxName REGEXP '^[0-9]+$' AND t1.uid=:uid";
      $boxName = u\firstUnusedId('boxName', 'r_boxes', 1, PHP_INT_MAX-1, $where, compact('uid'));
      } else */
  //if (!$boxName) $boxName = $code; // code serves as boxName for dumb cell phones
  $created = $access = now();
  $boxnum = db\get('MAX(boxnum)', 'r_boxes', 'uid=:uid', compact('uid')) + 1;
  $info = ray('channel', $ch) + compact(ray('uid code boxnum access created'));
  return @db\insert('r_boxes', $info);  // WHY ARE WE IGNORING ERRORS HERE???????
}

/**
 * Assign the given account a new id (or delete it).
 * @param int $a: the account to delete (qid or uid are also acceptable)
 * @param int $newUid: the new record ID to give the account (defaults to next available)
 * @param bool $unusable: whether to mark the old account record ID as unusuable
 * @param bool $delete: whether to delete the account rather than giving it a new ID (called by deleteAccount)
 */
function deleteAccount($a) {r\changeUid($a, 0, FALSE, TRUE);}
function changeUid($a, $newUid = 0, $unusable = FALSE, $delete = FALSE) {
  if (!$a or !$a = is_object($a) ? $a : r\acct($a)) return w\say(t('nope'), 'err');
  $oldUid = $a->id;
  $nm = $a->fullName;
  $photoFlnm = $a->tempPhotoFilename;
  $oldQid = r\qid($oldUid);

  if ($delCtty = ($delete and $a->isCtty)) { // deleting a community (merge into Seedpack)
    db\del('users', 'uid', $oldUid);
    $unusable = $delete = FALSE; // don't actually delete anything except the account record
    $newUid = r\serverUid(); // Seedpack uid
  }
  
  if ($delete) {
    if (db\exists($table = 'tx_entries', 'uid=:oldUid', compact('oldUid'))
        or db\exists($table = 'r_ballots', ':oldUid IN (voter, proxy)', compact('oldUid'))
        or db\exists($table = 'r_usd', 'payee=:oldUid', compact('oldUid'))) {
      return w\say(t('Account "%name" has records in table %table, so it cannot be deleted.', 'name table', $a->fullName, $table), 'err');
    }
  } elseif (!$newUid) {
    $region = @$a->zip ? r\region(@$a->country, @$a->state, $a->zip) : r\communityUid();
    $newUid = r\Acct::nextId($region);
  }
  $subs = array( // all cg tables are listed, to be sure we don't leave any out (D=deletable)
    // r_areas
    // r_bad
    'r_ballots:voter',
    'r_ballots:proxy',
    // r_banks
    'r_boxes:uid:D',
    'r_changes:uid:D',
    'r_changes:changedBy',
    'r_company:uid:D',
    // r_countries
    'r_coupated:uid',
    'r_coupons:fromId',
    'r_coupons:sponsor',
    'r_criteria:ctty:D',
    'r_do:uid:D',
    'r_events:ctty',
    'r_honors:uid:D', // honors are deletable if donation never completed
    // r_industries
    'r_investments:coid',
    'r_investments:clubid',
    'r_invites:inviter',
    'r_invites:invitee:D',
    'r_invoices:payer:D',
    'r_invoices:payee:D',
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
    'r_recurs:payer:D',
    'r_recurs:payee:D',
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
    // signup
    'tx_disputes:uid',
    'tx_disputes:agentUid',
    'tx_entries:uid',
    'tx_entries:agentUid',
    'tx_hdrs:actorId',
    'tx_hdrs:actorAgentId',
    'users:jid',
    'users:uid:D',
    'users:helper',
    'users:community',
    // variable
  );

  $DBTX = \db_transaction();
  foreach ($subs as $one) {
    list ($table, $field, $deletable) = explode(':', $one . '::'); // if deleting and not deletable, set to zero
    if ($delCtty and $table == 'users') continue; // for ctty deletion, skip users (already done)
    
    if ($delete) {
      if (!$deletable) {
        if (db\exists($table, "$field=:oldUid", compact('oldUid'))) {
          $DBTX->rollback(); 
          return w\say(t('Record in table %table (field %field) cannot be deleted.', compact('table', 'field')), 'err');
        }
      } else {
        db\del($table, $field, $oldUid);
      }
    } else db\q("UPDATE $table SET $field=:newUid WHERE $field=:oldUid", compact('newUid', 'oldUid'));
  }

  if ($unusable and !$delete) {
    list ($uid, $name, $email) = array($oldUid, "unusable-$oldQid", "unusable$oldQid@" . CG_DOMAIN);
    db\insert('users', compact(ray('uid name email')), 'uid'); // mark old uid unusable
  }
  
  unset($DBTX); // commit
  if (!$delete) rename($photoFlnm, str_replace($oldQid, r\qid($newUid), $photoFlnm));
  
  w\say(t('Success. Account %nm has been %what.', 'nm what', $nm, $newUid ? t('changed to ') . $newUid : t('deleted')));
}

/**
 * Package the return arguments and alert staff on certain errors.
 * @param mixed $message: the error message or an assoc of parameters
 * @param assoc $info: (optional) message substitutions and other info about transaction
 */
function txErr($message, $info = []) {
  r\tellCO('failed transaction', $info + compact('message'));
  return is_array($message) ? $message : [FALSE, $message, $info];
}

/**
 * Record an invitation
 * @param string $email: email address to which the invite was sent
 * @param int $inviter: the inviter's account uid
 * @param string $code: proof of the invitation (that invitee must have, in order to sign up) (DEFAULT random)
 * @param string $subject, $message: temporary params (can be eliminated from the table at some point)
 * @param string $zip: alleged zipcode of the recipient
 *
 * @return string: the code
 * @see also r\Web\invitation()
 */
function invite($email, $inviter, $code = '', $subject = '', $message = '', $zip = NULL) {
  $code = $code ?: u\code();
  $invited = now();
  $email = u\cry('P', $email);
  $info = compact(ray('code invited email inviter subject message zip'));
  db\insert('r_invites', $info);
  return $code;
}

/**
 * Return parameters for notices for the account.
 * @param int $uid: the account record ID
 * @param string $where: criteria to filter notices for this account
 * @param bool $weekly: digest notices for the week
 * @param bool $monthly: digest notices for the month
 * @param int $at: time of reporting
 * @return assoc of all parameters in the notices template (FALSE if none)
 */
function noticeArgs($uid, $where = '0', $weekly = FALSE, $monthly = FALSE, $at = NULL) {
  u\setDft($at, now());
  $a = r\acct($uid);
  
  if ($where) {
    $q = db\q("SELECT message, created FROM r_notices WHERE uid=:uid AND ($where)", compact('uid'));

    while ($row = $q->fetch()) {
      $date = ($weekly or $monthly) ? (strftime('%b&nbsp;%d', $row->created)) : R_BULLET;
      $messages[] = "<tr><td class=\"notice-date\" stule=\"vertical-align:top; margin-right:5px;\">$date</td><td>$row->message</td></tr>"; // "stule" because CSP forbids inline online (rcron reverts it before emailing)
    }
    if (@$messages) $messages = join("\n", $messages); else return FALSE;
  }
  
  $unit = $weekly ? 'week' : ($monthly ? 'month' : 'day');
  $yestertime = u\fmtDate($start = $unit == 'month' ? u\plusMonths(-1, $at) : strtotime("-1 $unit", $at));
  $range = $weekly ? "the week of $yestertime" : ($monthly ? strftime('%B %Y', $start) : $yestertime);
  extract(just('return', $a->j_stats()));
  $balance = u\fmtAmt($a->balance);
  //  $savings = u\fmtAmt($a->rewards);
  $creditLine = u\fmtAmt(-$a->floor);
  list ($rewards, $ourEmail) = array(u\fmtAmt($a->rewards), $a->cttyA->email);
  $companies = getV('companies');
  $companies = $companies[test() ? r\serverUid() : (int) $a->community];
  return compact(ray('messages unit range yestertime balance creditLine return ourEmail companies'));
}

/**
 * Return a list of participating companies for the given community.
 * @param mixed $ctty: uid of community OR zipcode OR zipcode pattern or array of business uids
 * @param bool $email: <format for use in emails and show just new companies> (else format for promo site or Find Co page)
 */
function companies($ctty, $email = FALSE) {
  $model = $email
    ? '<tr><td width="200">%name</td><td style="margin-left:10px;">%desc</td></tr>'
    : '<div class="row"><div class="cmpy-name">%name</div><div class="cmpy-does">%desc</div></div>';
  $where = (is_numeric($ctty) and strlen($ctty) > 9) ? 'community=:ctty' : 'zip REGEXP :ctty';
  if ($email) $where .= ' AND activated>:REQUEST_TIME-9*:WEEK_SECS'; // show only new companies in emails
  
  $uids = is_array($ctty)
    ? $ctty
    : db\q("SELECT DISTINCT uid FROM users WHERE :IS_CO AND :IS_OK AND uid>:CANONIC_ACCTS AND $where ORDER BY fullName", compact('ctty'))->fetchCol();
  if (!$uids) return $email ? t('no new companies this month') : t('No companies there yet. Invite them!');
  
  $res = '';
  $_newClass = $email ? 'style="color:red; font-weight:bold;"' : 'class="company-listing"';
  $NEW = t(' <small %newClass> NEW!</small>', compact('_newClass'));
  
  foreach ($uids as $uid) {
    $a = r\acct($uid);
    if (!$a->coCan(CO_PRIVATE)) {
      $name = $a->website ? w\lnk("http://$a->website", $a->bestName) : $a->bestName;
      $new = ($a->activated > REQUEST_TIME - 9 * WEEK_SECS) ? $NEW : '';
      $res .= tr($model, '_name desc', "$name$new", $a->shortDesc) . "\n";
    }
  }
  return $res;
}

/**
 * Return the amount the community has ever paid out
 */
function cttyPaidEver($ctty = 0) {
  u\EXPECT(u\myInt($ctty), 'bad ctty');
  return max(0, -db\sum('balance', 'users', $ctty ? "uid=$ctty" : 'uid<0'));
  $dt = db\max('created', 'r_stats', 'ctty=:ctty', compact('ctty')) ?: 0;
  return db\get(R_CTTY_BENE, 'r_stats', 'ctty=:ctty AND created=:dt', compact('ctty', 'dt'));
}

/**
 * Return system-wide or community-wide statistics.
 * @param int $ctty: uid of community or region -- 0 means ALL, 1 means the server's region (for example, qids starting "NEW")
 * @param int $when: latest statistics date to return (or the exact date if $redo is not NULL)
 *   if NULL, calculate statics for today
 * @param ray $redo: list of names of calculations to redo (ALL for all, NULL for none)
 * @return assoc of named statistics for one community on one date (see r_stats table def)
 */
function stats($ctty = 0, $when = NULL, $redo = '') {
  u\setDft($when, today());

  if ($ctty == 1) { // for the server's region overall (omitting distant communities hosted here)
    list ($start, $end) = r\regionRange();
    $cttyCrit = "uid BETWEEN $start AND $end";
    $cttyStart = db\min('created', 'users', "$cttyCrit AND uid>:CANONIC_ACCTS");
  } else { // for one community or all
    $cttyCrit = $ctty ? "community=$ctty" : 'community<>0';
    $cttyStart = db\min('created', 'users', "$cttyCrit AND uid<0");
  }

  if (!$cttyStart or $when < max($cttyStart, R_LAUNCH)) {
    $info = db\cols('r_stats', 'id');
    $info = ['created' => $when] + array_combine($info, array_fill(0, count($info), 0));
  } else {
    $sql = "SELECT * FROM r_stats WHERE created=:when AND ctty=:ctty ORDER BY created LIMIT 1";
    $info = db\q($sql, compact('ctty', 'when'))->fetchAssoc();
    $do = 'accts bals usd txs aAccts conx conxLocal payees basket patronage invites topN botN'; // normal
    if ($redo) $do = $redo == 'ALL' ? str_replace('bals', 'bals bals2', $do) : $redo;
    if (!$info or $redo) $info = r\getStats($ctty, $when, $cttyCrit, $do); // create stats if none for this date OR redoing
    if ($redo) return $info;
  }
  // add composite calculated fields to info  
  extract($info);
  $info['bals'] = $bals = $balsPos + $balsNeg;
  
  foreach (ray('cg usd') as $z) {
    $info[$z] = ${$z . 'In'} + ${$z . 'Out'};
//    $info["{$z}"] = ${$z . 'In'} - ${$z . 'Out'};
    $info["{$z}Count"] = ${$z . 'InCount'} - ${$z . 'OutCount'};
  }
  $info['usdVol'] = $info['usdIn'] + -$info['usdOut'];
  foreach (['', 'In', 'Out'] as $z) {
    if ($ctty) $info["usd$z"] += $info["cg$z"]; // settle trade imbalance with USD (so count it as community USD activity)
  }
  foreach (['', 'Count'] as $z) {
    $info["txs$z"] = ${"p2p$z"} + ${"b2b$z"} + ${"p2b$z"} + ${"b2p$z"} + ${"cashs$z"};
    $info["local$z"] = $info["txs$z"] - $info[$ctty ? "cg$z" : "cgOut$z"];
  }
  
  return $info;
}

/**
 * Calculate and record statistics for the given community. The following fields omit the community itself:
 *   balsPos, balsNeg (and corresponding counts), floors, topN, botN
 * @param int $ctty: uid of community or region -- 0 means ALL, 1 means the server's region (for example, qids starting "NEW")
 * @param int $created: date for which to create (or update) and return statistics
 * @param string $cttyCrit: MySQO criteria to select records for the given community
 * @param string $do: space-delimited list of names of calculations to do
 * @return the created record
 * See the "// get" lines to see what statistics are collected and returned.
 * Called ONLY from stats()
 */
function getStats($ctty, $created, $cttyCrit, $do = NULL) {
  u\EXPECT($created >= R_LAUNCH, 'getStats old date');
  $created0 = $created;
  $creCrit = 'created<=:created';
  $subs = compact('created');
  $cttysCrit = $ctty ? "(u.$cttyCrit OR u2.$cttyCrit)" : '1';
  $nonCtty = $ctty < 0 ? 'uid>:CANONIC_ACCTS' : 'uid=0'; // nonCtty does not apply to overall, nor to the region
  $accounts = "(SELECT uid,jid,community,flags FROM users)";
  $isMaster = '(jid=0 OR uid<jid)';
  
  // get account counts (can't be combined with financial data query because of joint accounts
  $sqls['accts'] = <<< X
    SELECT SUM(IF(:IS_OK AND activated<=:created AND activated<>0 AND NOT :IS_CO, 1, 0)) AS pAccts, 
      SUM(IF(:IS_OK AND activated<=:created AND activated<>0 AND :IS_CO, 1, 0)) AS bAccts,
      SUM(IF(NOT activated OR activated>:created, 1, 0)) AS newbs
    FROM users u
    WHERE $cttyCrit AND $creCrit	
X;

  // get users table financial data (must happen before topN/botN)
  $sqls['bals'] = <<< X
    SELECT 
    SUM(IF(balance>0, balance, 0)) AS balsPos,
    SUM(IF(balance<0, balance, 0)) AS balsNeg,
    SUM(IF(balance>0, 1, 0)) AS balsPosCount,
    SUM(IF(balance<0, 1, 0)) AS balsNegCount,
    SUM(IF(:IS_OK, floor, 0)) AS floors
    FROM users u
    WHERE $nonCtty AND $isMaster AND $cttyCrit AND $creCrit
X;

  // get bank transfer data
  $in = 'amount>0 AND xid<=0';
  
  $thisIn = "t.amount > 0";
  $sqls['usd'] = <<< X
    SELECT
    SUM(IF(amount>0, amount, 0)) AS usdIn,
    SUM(IF(amount<0, amount, 0)) AS usdOut,
    SUM(IF(amount>0, 1, 0)) AS usdInCount,
    SUM(IF(amount<0, 1, 0)) AS usdOutCount
    FROM r_usd t LEFT JOIN users u ON (u.uid=t.payee AND uid NOT BETWEEN 256 AND 511)
    WHERE $cttyCrit AND t.completed IS NOT NULL AND t.completed<=:created AND t.amount<>0
X;

// get transaction table data
  $sql = '';
  list ($cgIn, $cgOut) = $ctty == 0 // make cgIn and cgOut selectors
  ? ['SUM(IF(u.community=u2.community, amt, 0))',
     'SUM(IF(u.community=u2.community, 0, amt))']
  : ["SUM(IF(NOT (u.$cttyCrit) AND u2.$cttyCrit, amt, 0))\n",
     "SUM(IF(u.$cttyCrit AND NOT (u2.$cttyCrit), -amt, 0))\n"];
  foreach (ray('cgIn cgOut') as $k) $sql .= "${$k} AS $k,\n";
  
  foreach (ray('p2b:01,b2b:11,b2p:10,p2p:00') as $k => $v) { // make x2x selectors
    foreach ([0, 1] as $i) ${'not' . $i} = $v[$i] ? '' : 'NOT';
    $sql .= "SUM(IF(goods<>:FOR_USD AND $not0 u.:IS_CO AND $not1 u2.:IS_CO, ABS(amt), 0)) AS $k,\n";
  }
  $sql .= "SUM(IF(goods=:FOR_USD, ABS(amt), 0)) AS cashs,\n";
  $sql .= strtr($sql, ['-amt' => '1', 'amt' => '1', ",\n" => "Count,\n"]); // add counts
  $sql = str_replace(",\n!", '', $sql . '!'); // remove final comma in field list

  $sqls['txs'] = <<< X
    SELECT $sql
    FROM txs_prime t 
    LEFT JOIN $accounts u ON u.uid=uid1 
    LEFT JOIN $accounts u2 ON u2.uid=uid2
    WHERE $cttysCrit AND $creCrit
X;

  // get aAccts
  $recent = 'created BETWEEN :created-' . (RECENT_SECS-1) . ' AND :created'; // not n months (keep length of time consistent)

  $sqls['aAccts'] = <<< X
    SELECT COUNT(DISTINCT uid1) as aAccts FROM txs_prime  t
    JOIN $accounts u ON u.uid=uid1
    WHERE $cttyCrit AND t.$recent
X;

//    WHERE NOT u.:IS_CO AND $cttyCrit AND $creCrit AND $recent

  // get conx, conxLocal
  foreach (ray('conx conxLocal', 1, "u2.$cttyCrit") as $k => $crit) {
    if (!$ctty) $crit = 1; // avoid local query when doing stats for ALL cttys
    $sqls[$k] = <<< X
      SELECT AVG(conx) as $k FROM (
        SELECT me, COUNT(DISTINCT other) AS conx, SUM(isRecentPayer) AS recentPays, u2.community
        FROM (
          SELECT u0.uid AS me, 
            IF(u0.uid=uid1 OR u0.jid=uid1, 1, 0) as isRecentPayer,
            IF(u0.uid=uid1 OR u0.jid=uid1, uid2, uid1) AS other 
          FROM txs_prime t0
          JOIN users u0 ON u0.uid IN (t0.uid1, t0.uid2) OR u0.jid IN (t0.uid1, t0.uid2) AND $isMaster
          WHERE t0.$recent AND NOT u0.:IS_CO AND u0.$cttyCrit
        ) t LEFT JOIN users u2 ON u2.uid=other WHERE $crit
        GROUP BY t.me
      ) u3 WHERE recentPays>0        
X;
  }
  
//    for median: SELECT MEDIAN(payees0) OVER () AS payees,

  // get recent payees (avg payees per "active" account), roundups, and crumbs (total per month)
  $monthFactor = MONTH_SECS / RECENT_SECS;

  $sqls['payees'] = <<< X
    SELECT AVG(payees0) AS payees,
      SUM(roundups0) * $monthFactor AS roundups,
      SUM(crumbs0) * $monthFactor AS crumbs
    FROM (
      SELECT uid, COUNT(DISTINCT uid2) AS payees0,
        SUM(IF(uid2=:CG_ROUNDUPS_UID, amt, 0)) AS roundups0,
        SUM(IF(uid2=:CG_CRUMBS_UID, amt, 0)) AS crumbs0
      FROM txs_prime t JOIN users u ON uid2 IN (u.uid, u.jid)
      WHERE t.$recent AND $cttyCrit AND $isMaster
      GROUP BY uid
    ) x LIMIT 1
X;
    
  // get basket (avg transaction size recently) -- this omits structural transactions (roundups, etc.)
  $sqls['basket']  = <<< X
    SELECT AVG(ABS(amt)) AS basket
    FROM txs_prime t JOIN users u ON u.uid=uid1 JOIN users u2 ON u2.uid=uid2
    WHERE t.$recent AND $cttysCrit
X;

  // get patronage (total recurring gifts expected per month)
  $sqls['patronage'] = <<< X
    SELECT SUM(amount * IF(period='Y', 1/12, IF(period='Q', 1/3, IF(period='W', :MONTH_SECS/:WEEK_SECS, 1)))) AS patronage
    FROM r_recurs r JOIN users u ON u.uid=r.payer
    WHERE payee=:CGID AND r.created<:created AND (ended=0 OR ended>:created-:YEAR_SECS) AND u.$cttyCrit
X;
   
  // get invites ever
  $sqls['invites'] = <<< X
    SELECT COUNT(*) AS invites FROM r_invites i JOIN users u ON u.uid=i.inviter WHERE i.invited<:created AND u.$cttyCrit
X;

/// *********************************    
  
  if (@$created0 and $created < today()) { // redoing stats for the past, so figure bals, floors, and topN the hard way
    $recently = strtotime(-FLOOR_DAYS . ' days', $created); // six months ago
    $isPayer = '(uid1 IN (uid, jid))';
    $isPayee = '(uid2 IN (uid, jid))';
    
    $pastTxs = <<< X
      (SELECT uid,
         SUM(amt) AS amts,
         SUM(IF($isPayee AND created>$recently, amt, 0)) AS amtsIn,
         SUM(IF($isPayer AND created>$recently, amt, 0)) AS amtsOut
       FROM $accounts u LEFT JOIN txs t ON ($isPayer OR $isPayee)
       WHERE amt IS NOT NULL AND $isMaster AND $cttyCrit AND t.$creCrit
       GROUP BY uid)
X;

    // get OLD balsPos, balsNeg, floors
    $sqls['bals2'] = <<< X
      SELECT 
      SUM(IF(amts>0, amts, 0)) AS balsPos,
      SUM(IF(amts<0, amts, 0)) AS balsNeg,
      SUM(IF(amts>0, 1, 0)) AS balsPosCount,
      SUM(IF(amts<0, 1, 0)) AS balsNegCount,
      -SUM(GREATEST(amtsOut,amtsIn))/6 AS floors
      FROM $pastTxs t WHERE $nonCtty
X;
   
    // get OLD topN
    $sqls['topN'] = <<< X
      SELECT SUM(amts) AS topN FROM 
      (SELECT DISTINCT uid, amts FROM $pastTxs u1 
       WHERE $nonCtty AND amts IS NOT NULL ORDER BY amts DESC LIMIT %topnSQL) u
X;

  } else {
    // get regular topN
    $sqls['topN'] = <<< X
      SELECT SUM(bal) AS topN FROM 
      (SELECT balance AS bal FROM users u
       WHERE $nonCtty AND $isMaster AND $cttyCrit AND $creCrit GROUP BY uid
       ORDER BY balance DESC LIMIT %topnSQL) u
X;
  }

  $sqls['botN'] = strtr($sqls['topN'], ray('topN DESC', 'botN', ''));

  $res = [];
  foreach (ray($do) as $k) {
    if (!$sql = nn($sqls[$k])) continue; // don't do bals2 if bal was good enough
    if ($k == 'topN' or $k == 'botN') $sql = tr($sql, '_topnSQL', topN($res));
    $res = (db\q($sql, $subs)->fetchAssoc() ?: []) + $res; // +$res so OLD takes precedence for bals2
///   if ($created == today() and $k == 'accts') debug(compact(ray('res sql subs')));    
  }

  foreach ($res as $k => $v) {if (is_null($v)) $res[$k] = 0;}
  $hdr = compact('created', 'ctty');
  $info = $hdr + $res;
  $id = db\get('id', 'r_stats', $hdr);
  if ($id) db\update('r_stats', compact('id') + $info, 'id'); else db\insert('r_stats', $info);
  return $info;
}

/**
 * Return N or N% the number of active (or business) accounts, whichever is greater.
 * Do not change the default N without recreating all statistics (with a\fixStats)
 */
function topN($res, $n = 3) {return max($n, round($n * array_sum(just('bAccts aAccts', $res)) / 100));}


/**
 * Return the average balance, average credit reserve, and minimum balance for the given account (possibly joint) and period.
 * @param int $uid: account record ID (can be a community or region too)
 * @param int $start: starting date (should be 0 hours) -- first full day to include in the period
 * @param int $end: ending date (should be 0 hours) -- last full day to include in the period
 * @param bool $noj: ignore any joined account
 * @return array: [balance, min, rewards, days]
 *    balance: the average end-of-day balance for that period (possibly with fractional cents)
 *    min:     minimum end-of-day balance in the period
 *    rewards: the average end-of-day credit reserve amount for that period (possibly with fractional cents)
 *    days:    (unused) number of days from start to end, inclusive
 */
function averageBalance($uid, $start, $end = NULL, $noj = FALSE) {
  $uids = is_array($uid) ? $uid : [$uid];
  if (is_null($end)) { $end = r\rTime(); }
  $days = round(($end + 1 - $start) / DAY_SECS);
  if ($days == 0) return [0, 0, 0, 0]; // rounding is for daylight savings time changes
  $info = be\creditInfo($uids, $start);
  $min = $bal = $info['balance'];
  /* $jid = $noj ? 0 : (r\acct($uid)->jid ?: 0); */

  $day = 0;
  $avg = $ravg = 0;

  $sql = <<< EOF
    SELECT SUM(e.amount) AS dailyTotal, FLOOR((t.created-$start)/:DAY_SECS) AS day 
    FROM tx_hdrs t JOIN tx_entries e USING (xid)
    WHERE e.uid IN (:uids) AND t.created >= $start AND t.created < $end
    GROUP BY day
EOF;
  $q = db\q($sql, ['uids' => $uids]);

  while($row = $q->fetchAssoc()) {
    $prevDay = $day;
    extract($row); // dailyTotal, day
    $avg += $bal * ($day - $prevDay) / $days;
    $bal += $dailyTotal;
    $min = min($min, $bal);
  }
  $prevDay = $day;
  $days = round(($end - $start) / DAY_SECS); // end of period
  $avg += $bal * ($days - $prevDay) / $days;
  $min = round($min, 2);
  return [$avg, $min, $days];
}


/**
 * Interpret an integer bit array of risks.
 * @param string $descs: a space-delimited string array of risk descriptions and weights
 * @return formatted list of risk names, with HTML titles
 */
function riskList($risks, $descs) {  
  global $riskHelp, $base_url;

  $bitNames = array_keys($descs = ray($descs));
  foreach ($bitNames AS $bit => $bitName) {
    if (u\getBit($risks, $bit)) {
      $class = $descs[$bitName] < 0 ? 'goodRisk' : 'badRisk';
      $helpIndex = @$riskHelp ? array_search($bitName, array_keys($riskHelp)) : -1;
      $why[] = w\popHelp($bitName, $riskHelp[$bitName], "$base_url/help/risks@$bitName", $class);
      //      $why[] = "<a href=\"$base_url/help/flags@$bitName\" class=\"$class riskHelp$helpIndex\">$bitName</a>";
    }
  }
  return @$why ? join(', ', $why) : '';
}

/**
 * Check account permissions for a transaction or invoice. Note that invoicing a not-yet-complete account is permitted.
 * @param acct $a1: the account paying or requesting payment (the actor)
 * @param acct $a2: the other account
 * @param bool $taking: the first account is taking money from the second (charging or invoicing)
 * @param bool $neg: the amount is negative (refunding or undoing)
 * @return [FALSE, message, subs] on error (FALSE meaning not ok), else FALSE
 */
function txPermErr($a1, $a2, $taking = TRUE, $neg = FALSE) {
  global $scanned, $channel;
  
  $invoicing = ($taking and $channel == TX_WEB);
  if ($invoicing and !$a2->confirmed) $a2->setBit(B_CONFIRMED);

  // Check a1
  u\EXPECT($a1->ok, 'main account inactive tx');
  
  if (!$a1->confirmed and !$a1->isCanonic and !$a2->isCanonic and $a1->id != CGID and $a2->id != CGID) { // communities and CG can transact with newbs
    if ($a1->community != $a2->community or $channel != TX_POS) { // first tx must be in-person at home
      if ($a1->helper != $a2->id) {
        return [FALSE, 'first at home', ray('whose', $a1 == $a1 ? t('Your') : "$a1->fullName's")];
      }
    }
  }
  list ($myAccess, $otherAccess) = u\order($taking xor $neg, B_SELL, B_BUY);
  list ($myWhat, $otherWhat) = u\order($taking xor $neg, t('sales'), t('purchases'));
  if ($scanned and $taking) list ($myAccess, $myWhat) = $neg ? array(B_REFUND, t('refunds')) : array(B_SCAN, t('sales'));
  if (!$a1->can($myAccess)) return [FALSE, "no perm", ray('what', $myWhat)];
  if ((!$a2->can($otherAccess)) and (!($invoicing and $a2->proSe))) {
    return [FALSE, 'other no perm', ray('otherName what', $a2->agentA->fullName, $otherWhat)];
  }

  if (!$a1->cttyUp and !$a1->isCanonic) return [FALSE, 'your community down', ray('contact', $a1->cAdminContact())];
  if (!$a2->cttyUp and !$a2->isCanonic) return [FALSE, 'other community down', []];
  if ($a1->id == $a2->id) return [FALSE, 'no self-trading', []];
  return FALSE;
}

function passHash($pass) {
  u\EXPECT(is_string($pass) and $pass !== '', 'bad pass to hash');
  require_once DRUPAL_ROOT . '/includes/password.inc';
  return \user_hash_password($pass);
}

/**
 * Return the median (untested)
 */
function median($field, $table = 'users', $where = 1, $subs = []) {
  $middle = db\count($table, $where, $subs) / 2;
  $limit = floor($middle) . (is_int($middle) ? ', 1' : ', 2');
  return db\get('AVG($field)', 'users', "$where ORDER BY $field LIMIT $limit", $subs);
}

/**
 * Return the next sequential bank transfer number for an account.
 * @param int $payee: the account uid
 * @return int: the next unused positive integer (1 if there are no bank transfers yet)
 * UNUSED
 *//*
     function getUsdTid($payee) {
     return db\get('MAX(tid)+1', 'r_usd', 'payee=:payee', compact('payee')) ?: 1;
     }*/

/**
 * Send the account a notice for the suggested action, with a direct link to do it.
 * The link does not require the user to sign in.
 * FOR NOW: just suggest it, with no direct link. (never a link if channel is SMS)
 */
function suggest($a, $index, $subs, $page = 'Preferences') {
  // $subs['url'] = 
  // $index .= '|click option';
  r\notify($a->id, $index, $subs + compact('page'));
}

/**
 * Return the url for a page on the promo site, with a way to come back still signed in.
 * @param string $page: what page on the promo site to visit
 * @return the full url
 */
function promoLink($page = '') {
  if (r\signedIn()) $page .= '?region=' . R_SERVER_ID;
  return PROMO_URL . "/$page";
}

/**
 * Start/Stop the system OR return the "up" status.
 * @param bool $up: whether to start (TRUE), stop (FALSE), or query (NULL, the default)
 * @return TRUE if the system is up
 */
function up($up = NULL) {
  if (is_null($up)) return getV('up', TRUE);
  u\log('System is ' . ($up ? 'UP' : 'DOWN'));
  return setV('up', $up);
}

/**
 * Return the reward on the given amount for the given account.
 * @param int $from: payer account record ID
 * @param int $to: payee account record ID
 * @param numeric $amount: transaction amount
 * @param bool $isPayer: <return reward for payer> (else payee)
 * @todo? or manager of company with no (other) employees (set bit?)
 * (not used): Figure the rebate based on R = the payer's total rewards ever and A = the transaction amount,
 *   such that the rebate percentage p tapers linearly from Rmax to 0
 *   The percentage rebate is based on the total rewards AFTER the transaction. That is:
 *   The new percentage rate p = (R + pA)(Pmax-Pmin)/Rmax + Pmin
 *   So p = (RPmax + (Rmax - R)Pmin) / (Rmax^2 - A(Pmax - Pmin))
 * Used here: If the member's balance after the transaction is negative, their reward is negative the maximum reward (-10%).
 *   Otherwise the reward percentage starts at the maximum (10%), then decreases by 1% for every chunk ($500) of rewards
 *   they have received, down to the minimum (1%).
 *   
 */
function reward($from, $to, $amount = 1, $isPayer = TRUE) {
/**/ die('reward function is archaic');
  global $testRewardStep;
  if (relation(1, $to, $from, ':IS_OWNER OR :IS_DRAW')) return 0;
  if (relation(1, $from, $to, ':IS_OWNER OR :IS_DRAW')) return 0;
  $a = r\acct($isPayer ? $from : $to);
  //  $pct = test() ? $a->rebate : max(R_MIN_REWARD_PCT, R_REBATE - floor($a->rewards / R_REWARD_STEP));
  $step = @$testRewardStep ?: R_REWARD_STEP;

  $over = (($isPayer xor $amount < 0) and $a->balance + $a->rewards - abs($amount) < max(0, $a->floor)); // ignore floor<0
  if ($over) { // member is spending in (or into) debt
    //    $pct = $a->cttyNoreduce ? 0 : -max($a->rebate, R_REBATE); // reduce rewards
    $pct = -max($a->rebate, R_REBATE); // reduce rewards
  } else $pct = max(R_MIN_REWARD_PCT, $a->rebate * (1 - floor($a->o_rewards / $step) / R_REBATE)); // increase rewards
  if (!$isPayer) $pct *= R_BONUS_FACTOR;
///   debug(compact(ray('from to amount pct step over res')) + ['new bal' => $a->balance + $a->rewards + ($isPayer ? -$amount : $amount)]);
  return round(min(R_MAX_REBATE, $pct * $amount / 100), 2);
}

/**
 * Handle the investment request for one account in one club.
 * @param int $stakeid: record ID of the account's stake
 * @param string $msg: (RETURNED) a message, if any, resulting from the attempt
 * @return <successful>
 */
function handleInvestmentRequest($stakeid, &$msg) {
  if (!$res = db\get('stake,request,uid,clubid', 'r_stakes', compact('stakeid'))) return !$msg = t('bad stakeid');
  extract($res);
  
  if ($request <= 0) return (bool) $msg = t('No additional investment is requested.');
  $a = r\acct($uid);
  $clubA = r\acct($clubid);
  $stake += $request;
  
  $DBTX = \db_transaction();
  list ($ok, $index, $subs) = be\transfer('payment', $a, $clubA, $request, t('investment'), FOR_USD, ray('stake', 1));
  $msg = t($index, $subs);
  if ($ok) {
    db\update('r_stakes', ray('stakeid stake request', $stakeid, $stake, 0), 'stakeid');
    $msg .= ' ' . t('investment increase', 'amount', u\fmtAmt($request));
  }
  unset($DBTX);
  return $ok;
}

/**
 * Calculate community gifts ever paid and ever due as of a certain date/time.
 * @param int $end: ending date/time
 * @return the rows (an array of objects)
 */
function cttyGifts($end) {
  $sql = <<< o
    SELECT :end AS `end`, u.community AS ctty, 
      SUM(IF(uid2=:CGID, amt, 0)) AS `in`, 
      SUM(IF(uid1=:CGID AND uid2=u.community, amt, 0)) AS `out`,
     (SELECT COUNT(1)
       FROM r_usd d LEFT JOIN users du ON du.uid=d.payee 
       WHERE d.txid<0 AND d.amount<0 AND du.community=u.community AND d.created<=:end 
     ) AS bounces
    FROM txs_noreverse t LEFT JOIN users u ON u.uid=IF(uid1=:CGID,uid2,uid1)
    WHERE (
      (uid2=:CGID AND t.:IS_GIFT) OR (uid1=:CGID AND uid2=u.community)
    ) AND u.community<>:server AND t.created<=:end
    GROUP BY community
o;
  return db\rows('*', "($sql) x", '1', ray('end server', $end, r\serverUid()));
}

/**
 * Return the appropriate exchange fee for the given transaction description and amount.
 */
function xfee($for, $amount) {
  if (is_array($for)) $for = $for[1];
  if (!r\usdin($for)) return '';
  if (mb_strpos($for, BY_CASH)) return 0;
  return '' . round(mb_strpos($for, BY_CARD) ? -R_XFEE_CARD * $amount : R_XFEE_CHECK, 2);
}

function disputeTx($xid, $reason = '') {
  global $mya;
  
  if (empty($mya)) return FALSE;
  if (db\exists('tx_disputes', compact('xid'))) u\FAIL("transaction $xid is already disputed");

  $result = db\insert('tx_disputes', ['xid'=>$xid, 'reason'=>$reason, 'status'=>DS_OPEN, 'uid'=>$mya->uid], 'id');
  u\EXPECT($result, "error inserting dispute record");

  return TRUE;
}
  
function makeDoCode($ray) {
  $code = mb_substr(u\cry('P', serialize($ray)), strlen(CRYPT_FLAG));
  if (test()) t\doCode($code); // must be on separate line (PHP bug)
  return $code; // must be on separate line (PHP bug)
}

// Single line functions that need no explanation
function serverUid($regionId = R_SERVER_ID) {return -1 - R_REGION_MAX * u\a2n($regionId);} // -u\a2n($regionId) - 1;}
function emailTemplate($key) {return file_get_contents(__DIR__ . R_TEMPLATE_DIR . "/$key.html");}
function emailBody($key) {return str_replace('%body', emailTemplate($key), emailTemplate(R_EMAIL_FRAME));}
function realCountry($country) {return db\get('name', 'r_countries', 'id=:country', compact('country'));}
function realState($state, $country = R_COUNTRY_ID) {return db\get($country == US_COUNTRY_ID ? 'abbreviation' : 'name', 'r_states', 'id=:state and country_id=:country', compact('state', 'country'));}
function stateNum($st) {return $st ? db\get('id', 'r_states', 'abbreviation=:st and country_id=:US_COUNTRY_ID', compact('st')) : $st;}
function perm($bit) {return $bit - B_RELATED;}
function foreignServer($uid) {return (r\uidRegion($uid) != serverUid());}
function doCode($code) {return @unserialize(@u\decry('P', CRYPT_FLAG . ($code == 'TESTDOCODE' ? t\doCode() : $code)));} // (see forms/rdo.inc)
function cgA() {return r\acct(CGID);}
function denyInv($nvid) {return db\setData('r_invoices', $nvid, ['denied' => TRUE]);}
//function usdin($for) {return u\abbreviates('usd in (', strtolower(is_array($for) ? $for[1] : $for));}
function usdin($for) {return u\abbreviates(S_USD_IN, is_array($for) ? $for[1] : $for, TRUE);}
function regionField($fld) {return ($a = r\acct(serverUid())) ? $a->$fld : NULL;}
function signedIn() {return r\acct() ? TRUE : FALSE;}
function iCodeDesc($iCode) {$iCodeDescs = ray(IBYS); return $iCodeDescs[min($iCode, IBY_ICARD) - IBY0];}
function cttyList() {return db\q("SELECT uid,fullname FROM users WHERE uid<0 ORDER BY created,fullName")->fetchAllKeyed();}
function allKnowing() {global $mya; return ($mya->superAdmin and $mya->agentA->vKeyPw);}
function clubid() {return r\serverUid();} // temporarily use the region as the club
function pic($flnm) {if (!is_dir(R_PICTURE_DIR)) mkdir(R_PICTURE_DIR); return R_PICTURE_DIR . $flnm;}
function periodDesc($period) {return $period == 'X' ? '' : ray(OFTENS)[$period];}
function isBank($uid) {return ($uid == BANK_IN_ID or $uid == BANK_OUT_ID);}
