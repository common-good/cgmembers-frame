<?php
/**
 * @file
 * All of the business logic common to the various Common Good interfaces
 * (SMS, web, mobile, etc.)
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

/*use Symfony\Bundle\FrameworkBundle\Controller\AbstractController;
use Symfony\Component\HttpFoundation\Response;
use Symfony\Component\Mailer\MailerInterface;
*/
use Symfony\Component\Mailer\Mailer; 
use Symfony\Component\Mailer\Transport; 
use Symfony\Component\Mailer\Transport\SendmailTransport; 
use Symfony\Component\Mime\Crypto\DkimSigner;
use Symfony\Component\Mime\Email;
use Symfony\Component\Mime\Address;
use Symfony\Component\Mailer\Exception\TransportExceptionInterface;

/**
 * Return TRUE if the current account has all the required permissions.
 * @see also: w\subMenu()
 */
function access($perms = NULL) {
  global $mya;
  
  if ($perms == ANY) return TRUE;
  $pRay = ray($perms);
  if (!$a = $mya and in('linkable', $pRay)) { // allow limited access to the account without signing in
    if ($args = u\_GET() ?: $_POST) $a = w\eLinkAcct($zot, $args, ANY);
  }

  foreach ($pRay as $perm) if ($perm != 'linkable') {
    if ($not = (substr($perm, 0, 1) == '-')) $perm = substr($perm, 1);

    $pass = $perm == 'dev' ? isDEV
    : ($perm == 'okManageIfIn' ? ($a ? ($a->ok and $a->can(B_MANAGE)) : TRUE)
    : ($perm == 'manageIfIn' ? ($a ? $a->can(B_MANAGE) : TRUE)
    : (!$a ? FALSE
    : (in($perm, B_PERMISSIONS) ? $a->can(u\consta('b', $perm)) : $a->$perm) )));

    if ($not ? $pass : !$pass) return FALSE;
  }

  return TRUE;
}
function adminPerm($perm) {return preg_match('/^adm[A-Z]/', $perm);}

/**
 * Save a record in the given table, in the appropriate regional section of the id field's number space.
 * @param string $table: table to save in (users or u_relations)
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
    } catch (\Exception $e) { // BIG MYSTERY: why does this happen sometimes with no collision?
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
 * Return city, state, and coordinates from zip and country
  {"post code": "01002", "country": "United States", "country abbreviation": "US", "places": [{"place name": "Amherst", "longitude": "-72.4646", "state": "Massachusetts", "state abbreviation": "MA", "latitude": "42.3671"}]}
  */
function zipCity($zip, $country = R_COUNTRY_ID) {
  /*
  if ($info = @file_get_contents('http://api.zippopotam.us/us/' . $zip)) {
    if ($info = u\dejsonize($info)) $info = (array) nn($info->places)[0];
    if ($info) return [$info['place name'], r\stateNum($info['state abbreviation']), $info['latitude'], $info['longitude']];
  }
  */

  if ($res = db\get('primary_city AS city,state,latitude,longitude', 'zips', ['zip' => substr(nn($zip), 0, 5)])) extract($res);
  return $res ? [$city, r\stateNum($state), $latitude, $longitude] : [NULL, 0, 0, 0];
}

/**
 * Add a date stamp and agent's initials to the given note (before updating the notes field with it).
 * Agent is deemed to be self if no current account, because it's surely an unsubscribe from an email notice link.
 */
function stampNote($note) {
  global $mya;

  if ($mya) {
    if ($mya->proSe) {
      $rep = t('self');
    } elseif ($mya->id == UID_SUPER) {
      $rep = 'sadmin';
    } else {
      list ($first, $last) = u\parseName($mya->agentA->fullName);
      $rep = mb_strtolower(($first ? $first[0] : '') . $last);
    }
  } else { $rep = 'self'; }
  $date = fmtDt(NOW, 'dmY');
  return "$date  $rep: $note\n";
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
  $fields = 'nvid,status,amount,payer,payee,goods,purpose,cat,flags,data,recursId,reversesXid,created,deleted';
  if (is_array($nvid)) $nvid = $nvid['nvid'];  // really shouldn't happen -- but apparently did at some point
  if (!$inv = db\get($fields, 'tx_requests', 'nvid=:nvid', compact('nvid'))) return 'no matching invoice';
  if ($inv['deleted']) return 'inv is canceled';
  if ($inv['status'] >= TX_PAID) return FALSE; // invoice already paid
  return FALSE;
}

/**
 * Say whether an account's proposed transaction duplicates the last recent one.
 * @param int $a1: the actor account
 * @param int $a2: the other account
 * @param float $amount: the amount proposed to transfer from the other account to the actor account (can be negative)
 * @param bool $force: does this transaction have to go through?
 * @return TRUE if the proposed transaction duplicates the last one unintentionally (that is, unless the actor insists)
 * @see also X::cacheTotals(), where the transaction data is recorded
 */
function dupTx($a1, $a2, $amount, $force = FALSE) {
  global $channel, $allowDupTxs;
  
  $lastDup = $a1->lastDup; $a1->update('lastDup', 0); // clear dup if we return FALSE
  
  if ($force) return FALSE; // tx was taken offline -- too late to warn
  if (isset($allowDupTxs)) return FALSE; // testing (don't prevent reruns)
  if ($channel == TX_WEB) return FALSE; // allow dups on web interface
  if (!$lastTx = $a1->lastTx) return FALSE; // no previous transaction
  if ($lastTx->getAmount($a1) != $amount) return FALSE; // different amount (so not duplicate)
  if (now() - ($lastDup ?: 0) < DUPTX_SECS) return FALSE; // recent duplicate complaint, so allow this one
  if ($lastTx->actorId != $a1->id or $lastTx->otherId != $a2->id) return FALSE; // different payer or payee
  if (now() - $lastTx->created > DUPTX_SECS) return FALSE; // last tx like this was long enough ago

  $a1->update('lastDup', now()); // allow cashier to retry transaction after warning
  return TRUE;
}
 
/**
 * Give credit to someone at some company.
 */
function credit($fromUid, $toUid, $amount, $xid = 0, $purpose = STORE_CREDIT, $created = NOW, $id = NULL) {
  u\EXPECT($amount < 0, t('credit must be negative'));
  $flds = 'fromUid toUid amount xid purpose created' . ($id ? ' id' : '');
  return db\insert('tx_credits', compact(ray($flds)));
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
    SELECT u.uid, u.fullName FROM u_relations r
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
      $_atag = $a2->doLink('join', $main); // seek agreement
      $a2->tell('join accounts', ray('_atag name', $_atag, $a1->fullName));
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
 * Record the current IP as approved (for whitelisting in WHM)
 * @param int $uid: account record ID
 * @param string $device: one device code for this account (not very important, but interesting, especially for POS)
 */
function ipok($uid) {
  global $boxId; $device = $boxId ?: 0; // see Acct::boxId()
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
function rMail($index, $to0, $subs = [], $reply0 = NULL, $attachment = NULL) {
  global $testEmails, $mya, $channel;

  foreach (ray('secure vsecure') as $k) unset($subs[$k]); // show not even in error messages
  $subs['site'] = BASE_URL;
  $subs['orgPhone'] = CGF_PHONE;
  extract(just('subject body noFrame sms cc pseudo', $subs));

  if (!is_array($to0) and $toA = is_object($to0) ? $to0 : r\acct($to0)) {
    $toEmail = $toA->email;
    $to = [$toEmail => $toA->fullName];
    $subs += r\balAndCred($toA) + (array) $toA->account(); // this omits any special fields or special formatting (except balance and creditLine)
    u\setDft($subs['qid'], $toA->mainQid); // might have an agent, so don't overwrite
    $shortName = $toA->shortName;
    $zip = $toA->zip; // must be separate from substr (PHP 8.2 bug)
    $zip3 = substr(nn($zip), 0, 2); // temporarily show a wider region than zip3
    $surveyLink = $toA->co ? CO_SURVEY_LINK : SURVEY_LINK;
    u\preray(compact(ray('shortName zip3 surveyLink')), $subs);
    if ($toA->closed and $index != 'password-reset') return r\tellAdmin(t('attempt to email closed account'), compact(ray('index subs reply0')));
    if ($jid = $toA->jid) $jA = r\acct($jid);
    if ($toA->depends and !$toA->co) $jA = r\acct($toA->helper); // sponsored accounts are never joint
    if (nn($jA) and test()) $testEmails[] = ray('index toEmail subs', $index, $jA->email, $subs);
  } else $to = $toEmail = $to0;

  if (!$toEmailSimple = is_array($toEmail) ? key($toEmail) : $toEmail) return FALSE; // no email address, no way to send
  $subs['emailCoded'] = urlencode(u\cry('P', $toEmailSimple));
  
  $testEmails[] = compact(ray('index toEmail subs'));

  if (!is_array($reply = $reply0) and !is_object($reply0) and !strpos(nn($reply0), '@')) {
    $reply = (!nn($reply0) or $reply0 == 'region') ? [CGF_EMAIL => PROJECT] // standard
      : ($reply0 == 'ctty' ? $toA->cttyA : r\acct($reply0)); // get account object
  }

  if (is_object($replyA = $reply)) $reply = [$replyA->email => $replyA->fullName];

  foreach ($subs as $k0 => $v) if (!is_object($v) and !is_array($v)) {
    $k = "%$k0";
    $resubs[$k] = $v;
  }
  $subject = strip_tags(u\SUBS(strtr(nn($subject) ?: nni($GLOBALS['emailSubjects'], $index), $resubs), '%'));
  $body = u\SUBS(strtr(nn($body) ?: (nn($noFrame) ? r\emailTemplate($index) : r\emailBody($index)), $resubs), '%');
  $body = mb_convert_encoding($body, 'UTF-8', 'UTF-8');
  
  // log
  $whoDpy = nn($toA) ? $toA->mainQid : (is_array($to) ? reset($to) . ' <' . key($to) . '>' : "<$to>");
  u\loga('email', $erInfo = t('%whoDpy %index (%subject)', compact(ray('whoDpy index subject'))));
  
  $testSubs = u\noSecrets($resubs) + ["\r\n" => '', ' style=' => ' stylish=']; // easier reading for log and tests & evade CSP
  $erInfo .= ':<br>' . strtr($body, $testSubs);

  if (!isPRODUCTION and !isDEMO) { // make sure we never send rogue emails from test server with real data
    t\output('Actual (in rMail) ' . pr($erInfo), 'email'); // output whether or not testing
    if (isDEV or $toEmailSimple != R_ADMIN_EMAIL) return TRUE; // allow emails to admin from non-production servers
  }

  try { // catch Mailer errors
    require_once DRUPAL_ROOT . '/../vendor/autoload.php';
        $m = obj();
    $m->To = eAddr($to);
    $m->From = eAddr([SYS_EMAIL => nn($pseudo) ?: t('%PROJECT system')]);
    $m->TrackOpens = true;
    $m->Subject = $subject;
    $m->TextBody = strip_tags($body);
    $m->HtmlBody = $body;
  //  $m->Tag = "New Year's Email Campaign";
  //  $m->Headers = [ "X-CUSTOM-HEADER" => "Header content"];
  //  $m->MessageStream = "outbound" // here you can set your custom Message Stream

    if (nn($cc)) $m->Cc = eAddr($cc);
  //  $m->Bcc = 'bcc@example.com';
    if ($reply) $m->ReplyTo = eAddr($reply);

    if ($attachment) foreach (is_array($attachment) ? $attachment : [$attachment] as $flnm) {
      $m->Attachments[] = \Postmark\Models\PostmarkAttachment::fromFile($flnm, basename($flnm)); //  ($flnm, 'attachment-file-name.jpg', 'image/jpg');
    }

    $client = new \Postmark\PostmarkClient(POSTMARK_KEY);
    $sendResult = $client->sendEmailBatch([(array) $m]);
    
    if (nn($jA) and $index != 'tell-staff' and $jA->email) { // also send to partner in joint account unless it's an admin notice
      $m->To = eAddr([$jA->email => $jA->fullName]);
      $m->Subject = $subject . t(' (for %acct)', 'acct', $jA->fullName);
      $sendResult = $client->sendEmailBatch([(array) $m]);
    }
    
  } catch (\Postmark\Models\PostmarkException $e) { //  | \Exception
    $exErr = $e->message . "<br>\n" . trace();
    if (!isPRODUCTION) {
      if (!isDEV) u\loga('FATAL', $exErr);
/**/  if ($channel == TX_WEB) die($exErr);
    }

    if ($toEmailSimple != R_ADMIN_EMAIL) r\tellAdmin(t('cannot send email'), compact(ray('erInfo exErr')));
    return FALSE;
  }
  return TRUE;
}

/**
 * Report attempted use of a deactivated or phony card code to admin.
 * @param string $acctId: the attempted account ID
 * @param string $code: the bad card code
 * @return FALSE 
 */
function badCard($acctId, $code) {
  r\tellAdmin('bad card code', compact(ray('acctId code')));
  return FALSE;
}

/**
 * Notify the specified user, according to their preferences
 * @param integer $uid: account record id
 * @param string $index: message index
 * @param array $entries: TxEntries about which we should notify
 * @param bool $force: send by email NOW
 *//* UNUSED
function notifyEntries($a, $index, $entries, $force = FALSE) {
  global $channel;
  u\EXPECT($a, 'no account');
  $subs= [];
  $subs['uid'] = $a->uid;
  $subs['qid'] = $a->qid;
  /* $subs['otherName'] = @$subs['myName']; // notifying someone else about US, if anyone
  $fullName = $a->fullName;
//   $warnIndexes = ray('invoiced you,new offer,charge disputed'); 
  
  if ($force) {
    $mailed = r\rMail($index, $a, $subs, nni($subs, 'reply') ?: 'ctty');
  } else { //if ($a->can(B_NOTIFY_EMAIL)) {
    list ($message, $created) = array(t($index, $subs), now());
    f('t.output', "Actual Notice to $fullName: $message");
    db\insert('r_notices', compact(ray('uid message created')), 'msgid');
  }
  
  if ($a->can(B_BYSMS)) {
    if ($num = $a->smsNumber) $smsed = SMS\send($num, t($index, $subs));
  }
  if (FALSE and !nn($mailed) and !nn($smsed) and array_search($index, $warnIndexes) !== FALSE) {
    $notification = str_replace('-', ' ', $index);
    $warning = t('cannot contact', compact('fullName', 'notification'));
  }
  if (nn($warning)) {
    if ($channel == TX_WEB) Web\say($warning);
    if ($channel == TX_SMS) SMS\send($a->smsNumber, $warning);
    if ($channel == TX_APP) { } // dunno how to handle this
  }
}
*/

/**
 * Notify CG staff (Community Admin, if uid is specified and in their area)
 * @param string $index: the message or message index
 * @param assoc $subs: optional substitutions for the message and/or data to show (show both)
 * @param bool/int $uid: related account record id (TRUE means current account), defaults to none (NULL), 
 *    0 = suppress report of current account (to avoid confusion)
 * @param string $attachment: path to file to attach, if any
 * @return FALSE;
 */
function tellAdmin($index, $subs = [], $uid = NULL, $attachment = NULL) {
  global $mya, $channel, $tellingAdmin;
  
  $tellingAdmin = nn($tellingAdmin, 0) + 1; if ($tellingAdmin > 2) return; // don't loop (for example because log file is missing)
/**/  if (isDEV and !test()) return debug(['tellingStaff' => $index] + $subs + compact('uid'));

  $to = nni($subs, 'to', R_ADMIN_EMAIL);
  $message = t($index, $subs);
  foreach (ray(NOLOG_FLDS) as $k) unset($subs[$k]); // secret stuff

  $tellCOA = $uid == 1 ? $mya : ($uid == 0 ? NULL : r\acct($uid));
  $keyUid = nni($subs, 'uid');
  $keyA = $tellCOA ?: ($keyUid ? r\acct($keyUid) : NULL);
  
  $currentAcct = $mya ? $mya->nameAndQid() : NULL;
  $keyAcct = $keyA ? $keyA->nameAndQid() : NULL;
  $email = $tellCOA ? $tellCOA->cttyA->email : $to; // tell cAdmin if appropriate, else superAdmin
  $replyTo = ($keyA or $mya) ? ($keyA ?: $mya)->nameAndEmail() : NULL;
  
  $topic = mb_strlen($index) < 100 ? $index : '';
  $noFrame = TRUE;
  $chan = ray(TX_CHANNELS)[$channel];
  $subs += compact(ray('email message keyAcct currentAcct chan topic noFrame'));
  ksort($subs); // put the keys in a predictable order, for testing and easy reading
  $body = u\seeAssoc($subs);
  $info = $subs + compact('body');
  r\rMail('tell-staff', $email, $info, $replyTo, $attachment);
  $tellingAdmin = 0;
  return FALSE;
}

/**
 * Report something of interest to a community admin.
 * @param string $index: message index or text
 * @param assoc $subs: substitutions/parameter, if any, to include in the report
 * @param int $uid: record ID of an account in the community to report about.
 *    If none, choose something reasonable, reporting to superAdmin as a last resort.
 */
function tellCAdmin($index, $subs = [], $uid = NULL) {
  global $mya;
  if (!$uid) $uid = $mya ? $mya->id : nni($subs, 'loserId');
  $res = tellAdmin($index, $subs, $uid);
  return $res;
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
  $a = r\acct("$region$tail");
  return ($a and ($security == $a->lowSecurity)) ? $a : FALSE;
}

/**
 * Set a cookie
 * @param string $key: name of cookie
 * @param string $value: value of cookie (NULL is converted to '')
 * @param int $expires: time when cookie expires (defaults to never -- about 1000 years from now) 0=end of session
 * @param bool $raw: <suppress URL-encoding>
 * @return: the value
 */
function setCook($key, $value = '', $expires = NULL, $raw = FALSE) {
  u\EXPECT(($len = mb_strlen(rawurlencode("$key=$value"))) <= 4096, "cookie too long len=$len"); // 4094 not 4096 (browser bug?)
  if (isset($value)) $_COOKIE[$key] = $value; else unset($_COOKIE[$key]); // make it available also before going to a new page
  if (test()) return $value;

  u\setDft($expires, COOK_NO_EXPIRE);
  $func = $raw ? 'setrawcookie' : 'setcookie';
  $res = $func($key, nn($value), (int) $expires, '/'); // make it easy (fails without '/')
  u\EXPECT($res, 'cookie failed');
  return $value;
}

function cook($k) {return nni($_COOKIE, $k);}
function setCryptCook($k, $v, $expires = NULL) {return setCook($k, u\ry('C', $v), $expires, TRUE);}
function cryptCook($k) {return ($v = cook($k)) ? u\dery('C', $v) : '';}
function deleteCook($k) {return setCook($k, NULL, time() - 99999);}
function deleteAllCooks($except = '') {foreach ($_COOKIE as $k => $v) if (!in($k, $except)) deleteCook($k); $_COOKIE = [];}
function deleteAllCooksBut($except) {deleteAllCooks($except);} // use this if specifying $except (clearer)

/**
 * Return the unformatted private key for highly sensitive data (use u\fmtKey() to get the formatted key).
 * If no admin is signed in with super permission (or partially signed in -- see w\superSignin()), return FALSE.
 */
function vKey() {
  global $mya, $channel;
  u\EXPECT($channel == TX_WEB, 'wrong channel for vKey: ' . $channel);
  $ermsg = t(' Create a new vKeyE and vKeyPw (see acct::makeVKeyE).');

  if (!$mya or empty($vKeyE = db\get('vKeyE', 'admins', ray('uid', $mya->agentId))) or empty($vKeyPw = r\cryptCook('vKeyPw'))) {
    $msg = t('missing ') . (empty($vKeyE) ? 'vKeyE!' : 'vKeyPw!');
    r\tellAdmin($msg . trace());
    return w\softErr($msg);
  }
  return u\decry('R', $vKeyE, $vKeyPw);
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
  $table = 'r_regions r LEFT JOIN r_states s ON s.abbreviation=r.st';
  $where = "IF(zips IS NULL, country_id=:country AND s.id=:state, :zip RLIKE zips) ORDER BY (zips IS NULL) LIMIT 1";
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
  global $mya;
  return relation(1, $company ?: ($mya ? $mya->id : ''), $uid, ':IS_EMPLOYEE');
}

/**
 * Return the community record ID for the given postal code.
 * @param string $zip: postal code to identify community (if none, assume something in server community)
 * @param bool $defaultZero: return 0 if zip does not match any community? (else return server uid)
 * @param string $field: field to return from users or u_company (normally uid)
 */
function communityId($zip = '', $defaultZero = FALSE, $field = 'uid') {
  if (!$zip) return serverUid();
  $where = 'country=:R_COUNTRY_ID AND :zip RLIKE zips AND uid<1';
  return db\get($field, 'u_company JOIN users USING(uid)', $where, compact('zip')) ?: ($defaultZero ? 0 : serverUid());
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
 * Return a suggested transaction description, based on this account, the other account, and transaction type (paying or not).
 */
function suggestDesc($myid, $otherId, $paying, $minScore = 100000) {
  $channel = TX_WEB;
  list($me, $you) = u\order($paying, 1, 2);
  extract(ray('myid:100, otherId:100, paying:100000, channel:15'), EXTR_PREFIX_ALL, 'match');

  $score = <<< X
    IF(uid2=:myid xor $paying, $match_paying, 0)
  + IF(uid$me=:myid, $match_myid, 0)
  + IF(uid$you=:otherId, $match_otherId, 0)
  + IF(channel=:channel, $match_channel, 0)
X;

  $tnm = "(SELECT for$me, $score AS score, COUNT(*) AS cnt FROM txs WHERE type=:E_PRIME GROUP BY for$me, score HAVING score>=$minScore ORDER BY score DESC, cnt DESC) x";

  return db\get("for$me", $tnm, 1, compact(ray('myid otherId channel'))) ?: '';
}
  
/**
 * Return a suggested category (tx_cats record ID) for the given hash of transaction fields, including:
 *    myid, otherId, recursId, desc, type, amt (to me), channel, flags, mainCat, paying
 */
function bestCat($tx, $minScore = 100000) {
  extract(just('myid type paying mainCat', $tx, NULL));
  u\EXPECT(r\hasCats($myid), 'Do not suggest a category for non-CG transactions.');
  list($me, $you) = u\order($paying, 1, 2);
  foreach (ray('otherId type recursId amt channel flags mainCat') as $k) if (empty($tx[$k])) $tx[$k] = 0;
  extract(ray('paying:100000, myid:100, otherId:100, type:10, recursId:10000, desc:1000, desc7:300, amt:20, channel:10, flags:20'), EXTR_PREFIX_ALL, 'match');

  $score = <<< X
    IF(uid2=:myid xor $paying, $match_paying, 0)
  + IF(uid$me=:myid, $match_myid, 0)
  + IF(uid$you=:otherId, $match_otherId, 0)
  + IF(type=:type, $match_type, 0)
  + IF(recursId IS NOT NULL AND recursId=:recursId, $match_recursId, 0)
  + IF(for$me=:desc, $match_desc, 
      IF(LEFT(for$me, 7)=LEFT(:desc, 7), $match_desc7, 0) )
  + IF(amt=:amt, $match_amt, 0)
  + IF(channel=:channel, $match_channel, 0)
  + IF(flags!=0 AND flags=:flags, $match_flags, 0)
X;

  $tnm0 = 'txs';
  $canN = $paying ? 'cat1' : 'cat2';
  if (FALSE and !in($type, E_PRIMES) and $mainCat) {
    $score .= " + IF(mainCat=$mainCat, $match_mainCat, 0)"; // $mainCat, not %mainCat, avoids having to add it to subs
    $tnm0 = "(SELECT t.*,t2.$canN AS mainCat FROM txs t JOIN txs t2 USING(xid) WHERE t2.type IN :E_PRIMES AND t2.eid!=t.eid AND t2.$canN IS NOT NULL";
  }
  $tnm = "(SELECT $canN, $score AS score, COUNT(*) AS cnt FROM $tnm0 WHERE $canN IS NOT NULL GROUP BY $canN, score HAVING score>=$minScore ORDER BY score DESC, cnt DESC) x";
  
///  global $mya; if ($mya and $mya->admin) debug(compact(ray('tx minScore me you score tnm')));

  return db\get($canN, $tnm, 1, $tx) ?: '';
}

/**
 * Given a full name, make a username and add digits until it is unique.
 */
function uniqueName($fullName, $info = []) {
  if (!$fullName) {
    extract(just('phone email', $info));
    $fullName = R_TEMP_NAME_PREFIX .
      (nn($phone) ?  u\fmtPhone($phone, 'n')
       : (nn($email) ? R_TEMP_NAME_PREFIX . strtr($email, ['@'=>'AT', '.'=>'DOT'])
          : substr(md5('zot' . strval(now())), 0, 10))); // pseudo random
  }
  $name = $name0 = u\shortName($fullName);

  for ($i = 2; ; $i++) {
    if (!db\exists('users', 'name=:name', compact('name'))) return $name;
    $name = $name0 . $i;
  }
}

/**
 * Return the uid for the given qid or email (or name field, for old time's sake).
 * If $qid is an email and more than one account has that same email address, the first non-company account is assumed.
 * @todo later: when regional prefix is required for signin, this function will ignore it in matching name field
 * @return the uid (NULL if unsuccessful)
 */
function loginString($qid) {
  if (!$qid) return NULL;
  if (strhas($qid, '@')) {
    $name = '!!!';
    $email = u\cry('P', mb_strtolower($qid));
  } else {
    if ($qid = trim(mb_strtoupper($qid)) and $a = r\acct($qid)) return $a->id; // try to interpret as a qid
    if (preg_match('/^[A-Z0-9\\.\\-]{3,12}$/', $qid) and $a = f('qr.acct', $qid, TRUE)) return $a->id; // abbreviated account code
    $name = u\shortName($qid);
    $email = '!!!';
  }
  return db\get('uid', 'users', 'name=:name OR email=:email ORDER BY :IS_CO', compact('email', 'name'));
}
  
/**
 * Return a list of employee accts for the current account.
 */
function employees() {
  $myid = r\acct()->id;
  return db\q('SELECT other FROM u_relations WHERE main=:myid AND :IS_EMPLOYEE', compact('myid'))->fetchCol();
}

/**
 * Return the field or fields from the relations table, for the given main and other.
 * Call by:
 *   relation(field, main, other, where, subs) OR
 *   relation(field, where, subs)
 *   relation(field, reid)
 *   relation(field, acct)
 * @param string $field: what field value(s) to return (see varieties at db\get())
 * @param int $main: uid of main account
 * @param int $other: uid of other (agent) account (changed to zero if same as main)
 * @param string $where: SQL criteria
 * @param assoc $subs: criteria parameters
 * @param int $reid: relations record id
 * @param string $acct: the compound account
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
  } elseif (is_object($a = $main)) { // acct
    list ($where, $subs) = [ray('main other', $a->id, $a->agentId), []];
  } else u\FAIL('bad arg#2 in relation');
  return db\get($field, 'u_relations', $where, $subs);
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
 * Return a link to change the current account (presumably to one the current account is allowed to change to)
 * @param int $uid: record ID of account to change to
 * @param string $v: link text to display
 * @return the link
 */
function changeAcctLink($uid, $v, $page = NULL) {return w\lnk("/change-account/acct=$uid", $v);}     

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
 * @param string $having: criteria to filter notices for this account
 * @param bool $weekly: digest notices for the week
 * @param bool $monthly: digest notices for the month
 * @param int $at: time of reporting
 * @return assoc of all parameters in the notices template (FALSE if none)
 */
function noticeArgs($uid, $having = '1', $weekly = FALSE, $monthly = FALSE, $at = NULL) {
  global $mya;
  u\setDft($at, now());
  $a = r\acct($uid);
  foreach (ray('weekly monthly') as $k) $$k = (int) $$k;
  foreach (ray('n:never,m:month,w:week,d:day') as $k => $unit) {
    if ($k != 'n') ${$k . 'Date'} = strtotime("-1 $unit", $at);
    ${$k . 'Type'} = "'$a->notices' LIKE CONCAT('%', type, ':$k%')";
  }
  $mDate = u\monthDay1($mDate); // in case cron fails on first day of the month
  
  $notices = ray($a->notices);
  
  $q = db\q("SELECT msgid, message, created, sent, $nType AS nType, $wType AS wType, $mType AS mType FROM r_notices WHERE uid=:uid HAVING $having", compact('uid'));

  while ($row = $q->fetch()) {
    $date = ($weekly or $monthly) ? str_replace(' ', '&nbsp;', fmtDt($row->created, ' MMM dd')) : R_BULLET;
    $sent = $row->sent ? ' &x2713;' : '';
    $extra = $mya->admin ? "<td>$row->msgid$sent</td>" : '';
    $messages[] = "<tr>$extra<td class=\"notice-date\" stule=\"vertical-align:top; min-width:50px; margin-right:5px;\">$date</td><td>$row->message</td></tr>"; // "stule" because CSP forbids inline online (rcron reverts it before emailing)
  }
  if (nn($messages)) $messages = join("\n", $messages); else return FALSE;
  
  $unit = $monthly ? 'month' : ($weekly ? 'week' : 'day');
  $ranges = ray('m w d',  t('month of ') . fmtDt($mDate, 'MMMM yyyy'), t('week of ') . fmtDt($wDate), fmtDt($dDate));
  foreach (ray(NOTICE_TYPES) as $k => $desc) if (!in($k, POST_NOTICE_TYPE_KEYS)) { // just the non-post freq settings
    $freq = $notices[$k];
    if ($freq == 'm' ? $monthly : ($freq == 'w' ? $weekly : !in($freq, 'i n'))) {
      $descs[] = $desc;
      $range[] = ' (' . $ranges[$freq] . ')';
    }
  }
  if ($range = nn($range)) { // can be empty when called from Notices page
    for ($i = 0; $i < count($range); $i++) $descs[$i] .= $range[$i] == nni($range, $i + 1) ? '' : $range[$i]; // don't repeat
    $range = join(t(' and '), $descs);
  }
  extract(just('return', $a->j_stats()));

  list ($rewards, $ourEmail) = array(u\fmtAmt($a->rewards), $a->cttyA->email);
  return compact(ray('messages unit range return ourEmail')) + r\balAndCred($a);
}

/**
 * Return an account's balance and credit line, formatted.
 */
function balAndCred($a) {
  $balance = u\fmtAmt($a->balance);
  $creditLine = $a->underage ? t('disabled')
  : ($a->debt ? u\fmtAmt(-$a->floor) . t(' (based on your monthly activity)') 
  : t('disabled (sign in and see Settings / Preferences / Advanced)')); // (<%a>click here</a> to enable it)', '_a', 
  return compact(ray('balance creditLine'));
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
    : db\q("SELECT DISTINCT uid FROM users WHERE :IS_CO AND :IS_OK AND uid>:UID_CANON9 AND $where ORDER BY fullName", compact('ctty'))->fetchCol();
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
    $cttyStart = db\min('created', 'users', "$cttyCrit AND uid>:UID_CANON9");
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
  $nonCtty = $ctty < 0 ? 'uid>:UID_CANON9' : 'uid=0'; // nonCtty does not apply to overall, nor to the region
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

  // get bank transfer data (including with non-members)
  $in = 'amount>0 AND xid<=0';
  
  $thisIn = "t.amount > 0";
  $sqls['usd'] = <<< X
    SELECT
    SUM(IF(amount>0, amount, 0)) AS usdIn,
    SUM(IF(amount<0, amount, 0)) AS usdOut,
    SUM(IF(amount>0, 1, 0)) AS usdInCount,
    SUM(IF(amount<0, 1, 0)) AS usdOutCount
    FROM txs2 t LEFT JOIN users u ON (u.uid=t.payee AND uid NOT IN (:UID_BANK, :UID_OUTER))
    WHERE $cttyCrit AND t.completed IS NOT NULL AND t.completed<=:created AND t.amount<>0
X;

// get transaction table data
  $sql = '';
  list ($cgIn, $cgOut) = $ctty == 0 // make cgIn and cgOut selectors
  ? ['SUM(IF(u.community=u2.community, amt, 0))',
     'SUM(IF(u.community=u2.community, 0, amt))']
  : ["SUM(IF(NOT (u.$cttyCrit) AND u2.$cttyCrit, amt, 0))\n",
     "SUM(IF(u.$cttyCrit AND NOT (u2.$cttyCrit), -amt, 0))\n"];
  foreach (ray('cgIn cgOut') as $k) $sql .= "{${$k}} AS $k,\n";
  
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
        SUM(IF(uid2=:UID_ROUNDUPS, amt, 0)) AS roundups0,
        SUM(IF(uid2=:UID_CRUMBS, amt, 0)) AS crumbs0
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
    SELECT SUM(amount * perYear(tm.period, tm.periods) / 12) AS patronage
    FROM tx_timed tm JOIN users u ON u.uid=tm.from
    WHERE tm.action=:ACT_PAY AND tm.to=:CGID AND tm.start<:created AND (tm.end IS NULL OR tm.end>:created-:YEAR_SECS) AND u.$cttyCrit
X;
   
  // get invites ever
  $sqls['invites'] = <<< X
    SELECT COUNT(*) AS invites FROM r_invites i JOIN users u ON u.uid=i.inviter WHERE i.invited<:created AND u.$cttyCrit
X;

/// *********************************    
  
  if (nn($created0) and $created < today()) { // redoing stats for the past, so figure bals, floors, and topN the hard way
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
    if (!$sql = nni($sqls, $k)) continue; // don't do bals2 if bal was good enough
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
 * @param int $end: ending date (should be 0 hours) -- just past end of period
 * @param bool $noj: ignore any joined account
 * @return array: [balance, min, rewards, days]
 *    balance: the average end-of-day balance for that period (possibly with fractional cents)
 *    min:     minimum end-of-day balance in the period
 *    rewards: the average end-of-day credit reserve amount for that period (possibly with fractional cents)
 *    days:    (unused) number of days from start to end, inclusive
 */
function averageBalance($uid, $start = 0, $end = NULL, $noj = FALSE) {
  $uids = is_array($uid) ? $uid : [$uid];
  u\setDft($end, now());
  $days = round(($end - $start) / DAY_SECS);
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
      $helpIndex = nn($riskHelp) ? array_search($bitName, array_keys($riskHelp)) : -1;
      $why[] = w\popHelp($bitName, $riskHelp[$bitName], "$base_url/help/risks@$bitName", $class);
      //      $why[] = "<a href=\"$base_url/help/flags@$bitName\" class=\"$class riskHelp$helpIndex\">$bitName</a>";
    }
  }
  return nn($why) ? join(', ', $why) : '';
}

/**
 * Check account permissions for a transaction or invoice. Note that invoicing a not-yet-complete account is permitted.
 * @param acct $a1: the account paying or requesting payment (the actor)
 * @param acct $a2: the other account
 * @param bool $taking: the first account is (really) taking money from the second (charging or invoicing)
 * @param bool $neg: the amount is negative (refunding or undoing)
 * @return a txRet object on error, else FALSE
 */
function txPermErr($a1, $a2, $taking = TRUE, $neg = FALSE) {
  global $scanned, $channel;
  
  $invoicing = ($taking and $channel != TX_APP);

  // Check a1
  if (!$a1->ok) return be\txRet(FALSE, t('Requesting account is inactive.'));
  
  $confirmed = ($a1->co ? ($a1->agentA->confirmed or $a1->helperA->confirmed) : $a1->confirmed);
  if (!$confirmed and !$a1->isCanonic and !$a2->isCanonic and $a1->id != CGID and $a2->id != CGID) { // actor is not confirmed (but newbs can transact with bank and CG)
    if ($a1->helper != $a2->id) { // unless doing business with who invited you
      if (!$a1->hasId or $a1->risk('ssnOff')) {
        $atHome = in(r\serverUid(), [$a1->community, $a2->community]) ? ($a1->community == $a2->community) : (substr($a1->zip, 0, 3) == substr($a2->zip, 0, 3) or u\earthDistance($a1->latitude, $a1->longitude, $a2->latitude, $a2->longitude) < 20);
        if (!$atHome or $channel != TX_APP) { // first tx must be in-person at home if ID is unverified
          return be\txRet(FALSE, 'first at home');
        }
      }
    }
  }
  list ($myAccess, $otherAccess) = u\order($taking xor $neg, B_SELL, B_BUY);
  list ($myWhat, $otherWhat) = u\order($taking xor $neg, t('sales'), t('purchases'));
  if ($scanned and $taking and $neg) list ($myAccess, $myWhat) = array(B_REFUND, t('refunds'));
  if (!$a1->can($myAccess)) return be\txRet(FALSE, "no perm", ray('what', $myWhat));
  if ((!$a2->can($otherAccess)) and (!$invoicing or $a2->closed)) {
    return be\txRet(FALSE, 'other no perm', ray('otherName what', $a2->agentA->fullName, $otherWhat));
  }

  if (!$a1->cttyUp and !$a1->isCanonic) return be\txRet(FALSE, 'your community down', ray('contact', $a1->cAdminContact()));
  if (!$a2->cttyUp and !$a2->isCanonic) return be\txRet(FALSE, 'other community down');
  if ($a1->id == $a2->id) return be\txRet(FALSE, 'self-trading');
  
  if ($a1->confirmed) $a2->setBit(B_CONFIRMED);

  return FALSE;
}

/**
 * Return a unique short, lowercase version of a name.
 */
function shortName($fullName) {
  global $mya;
  
  $myid = $mya ? $mya->id : 0;
  $name = $name0 = u\shortName($fullName);
  $len = strlen($name);
  $suffix = 1; // first suffix to add is "2"
  while (db\exists('users', 'name=:name AND uid<>:myid', compact('name', 'myid'))) {
    $suffix++;
    $name = $name0 . $suffix;
  }
  return $name;
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
 * Send the account a notice for the suggested action, with a direct link to do it.
 * The link does not require the user to sign in.
 * FOR NOW: just suggest it, with no direct link. (never a link if channel is SMS)
 */
function suggest($a, $index, $subs, $page = 'Preferences') {
  // $subs['url'] = 
  // $index .= '|click option';
  $a->tellMisc($index, $subs + compact('page'));
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
  $step = nn($testRewardStep) ?: R_REWARD_STEP;

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
       FROM txs2 d LEFT JOIN users du ON du.uid=d.payee 
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
 * Return a selection list for recurrence period.
 */
function recurRay($keepOne = FALSE, $text = PERIODLY) {
  $oftens = array_combine(ray(PERIODS), ray($text));
  unset($oftens['forever']);
  if (!$keepOne) unset($oftens[PER_1]);
  if ($text != PERIODLY) unset($oftens['quarter']);
  return $oftens;
}

function recurIndex($period) {return nni(array_flip(ray(PERIODS)), $period);}

/**
 * Say whether the given account is owned by Common Good and its transactions get categorized.
 * Omit the various donation collection accounts (UID_CRUMBS etc) which conceptually stand in for donors
 *  -- those are conceptually part of CG's main account and appear only on ctty's and CG's Balance Sheets.
 * @param int $uid: account record ID
 */
function hasCats($uid) {return in($uid, catUids());}

/**
 * Return an array of record IDs of all accounts owned by CG whose transactions get categorized.
 * That is, CG itself plus all sponsored accounts (internal and external), 
 * omitting the collection buckets (BUCKET_UIDS minus CGID), which get dumped into CG.
 */
function catUids() {
  return array_merge([CGID], db\col('uid', 'u_company', 'coFlags&((1<<:CO_SPONSORED) | (1<<:CO_OUROWN))')); // cache this someday?
}

function cgIncomeUids() {return array_merge(BUCKET_UIDS, catUids());} // uids of all accounts owned by CG (called in queries.inc)

/**
 * Return an array of record IDs of all accounts whose income AND expenses belong to CG (the EIN and legalName should be CG's).
 * CG, NEWAZV, NEWAIN, and NEWCBQ should be marked as "ourOwn"
 */
function cgsOwnUids() {
  return array_merge(db\col('uid', 'u_company', 'coFlags&(1<<:CO_OUROWN)'), BUCKET_UIDS);
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

/**
 * Mark a rule (or template) ended (now) and delete any related unpaid invoices.
 * @param bool $tbl: table abbreviation (ru for tx_rules, tm for tx_timed)
 * @param int $id: rule or template id
 */
function endRule($tbl, $id) {
  u\EXPECT(in($tbl, 'tm ru'), 'bad tbl in endRule');
  $tnm = $tbl == 'tm' ? 'tx_timed' : 'tx_rules';
  db\update($tnm, ray('id end', $id, now()), 'id');
  if ($tbl == 'tm') db\del('tx_requests', 'recursId=:id AND status<:TX_PAID', compact('id'));
}

function recurDesc($period, $periods = 1) {
  $p = array_search($period, ray(PERIODS));
  return $periods > 1 ? t('every %n %units', 'n units', $periods, ray(PERIODS_PLURAL)[$p]) : ray(PERIODLY)[$p];
}

/**
 * Return contact info for the given people record ID.
 */
function pContactInfo($pid) {
  if (!$info = db\get('fullName,address,city,state,zip,phone,email', 'people', compact('pid'))) return u\FAIL(t('Missing non-member information!'));
  extract($info);
  $fullName .= t(' (non-member)');
  $state = r\realState($state);
  $addr = "$address, $city, $state $zip";
  return [$fullName, $addr, $phone, $email];
}

/**
 * Return an assoc of communities indexed by uid, including 0 for overall and "Seedpack" for no-community-yet.
 */
function cttyList($withAll = TRUE) {
  $cttys = db\q("SELECT uid,fullname FROM users WHERE uid<0 ORDER BY created,fullName")->fetchAllKeyed();
  foreach ($cttys as $k => $v) $cttys[$k] = trim(str_replace(PROJECT, '', $v));
  if ($withAll) $cttys[0] = t('ALL communities');
  asort($cttys);
  unset($cttys[$seed = r\serverUid()]);
  $cttys[$seed] = t('Seedpack (no community yet)'); // last
  return $cttys;
}

// Single line functions that need no explanation
function serverUid($regionId = R_SERVER_ID) {return -1 - R_REGION_MAX * u\a2n($regionId);} // -u\a2n($regionId) - 1;}
function emailTemplate($key) {return file_get_contents(__DIR__ . R_TEMPLATE_DIR . "/$key.html");}
function emailBody($key) {return str_replace('%body', emailTemplate($key), emailTemplate(R_EMAIL_FRAME));}
function realCountry($country) {return db\get('name', 'r_countries', 'id=:country', compact('country'));}
function realState($state, $country = R_COUNTRY_ID) {return db\get($country == US_COUNTRY_ID ? 'abbreviation' : 'name', 'r_states', 'id=:state and country_id=:country', compact('state', 'country'));}
function stateNum($st) {return $st ? db\get('id', 'r_states', ':st IN (id, name, abbreviation) and country_id=:US_COUNTRY_ID', compact('st')) : FALSE;}
function fullState($abbrev, $country = R_COUNTRY_ID) {return db\get('name', 'r_states', 'abbreviation=:abbrev and country_id=:country', compact('abbrev', 'country'));}
function perm($bit) {return $bit - B_RELATED;}
function foreignServer($uid) {return (r\uidRegion($uid) != serverUid());}
function cgA() {return r\acct(CGID);}
function denyInv($nvid) {return db\setData('tx_requests', $nvid, ['denied' => TRUE]);}
//function usdin($for) {return u\abbreviates('usd in (', strtolower(is_array($for) ? $for[1] : $for));}
function usdin($for) {return u\abbreviates(S_USD_IN, is_array($for) ? $for[1] : $for, TRUE);}
function regionField($fld) {return ($a = r\acct(serverUid())) ? $a->$fld : NULL;}
function signedIn() {return r\acct() ? TRUE : FALSE;}
function iCodeDesc($iCode) {$iCodeDescs = ray(IBYS); return $iCodeDescs[min($iCode, IBY_ICARD) - IBY0];}
function clubid() {return r\serverUid();} // temporarily use the region as the club
function pic($flnm) {if (!is_dir(R_PICTURE_DIR)) mkdir(R_PICTURE_DIR); return R_PICTURE_DIR . $flnm;}
function isBank($uid) {return ($uid == UID_BANK);}
function foodFundId() {return db\get('uid', 'users', "name='foodfund'") ?: 0;}
function referrer() {global $referrer; return test() ? nn($referrer) : basename(nni($_SERVER, 'HTTP_REFERER'));}
function countryList() {return db\keyed('id,name', 'r_countries', "name in ('United States', 'Canada')");}

const CREDIT_RULE_ROW = [ACT_SURTX, MATCH_PAYEE, MATCH_PAYER, 0.00, 1.00, REF_ACCOUNT, REF_ACCOUNT, 0.00];
function isCreditRule($row) {return (array_values(just('action from to amount portion payerType payeeType minimum', $row)) == CREDIT_RULE_ROW);}
function cardCode() {return u\randomString(R_CARDCODE_LEN, 'word');}
function adminCanBit($k) {$bit = array_search($k, ray(A_LIST)); return $bit === FALSE ? u\FAIL('bad admin permission') : u\bit($bit);}
function annualAmt() {return "tm.amount * IF(tm.period='day', 365, IF(tm.period='week', 52 / tm.periods, IF(tm.period='month', 12 / tm.periods, IF(period='quarter', 4 / tm.periods, 1))))";}
function bankData($bank = BANK) {return (object) array_combine(ray(BANK_KEYS), explode(', ', BANK_DATA[$bank]));} // not ray() because of colons in the data
//function eAddr($x) {return is_array($x) ? new Address(key($x), reset($x)) : $x;}
function eAddr($x) {return is_array($x) ? '"' . reset($x) . '" <' . key($x) . '>' : $x;}
function nick2cat($nick) {return db\get('id', 'tx_cats', compact('nick'));}
function qbCat($id) {return db\get('externalId', 'tx_cats', ray(is_numeric($id) ? 'id' : 'nick', $id));}
function setQbOk($xid, $set = TRUE) {return db\setFlag('tx_hdrs', $xid, B_QBOK, $set);}
function tellPromo($body, $topic = NULL, $reply = NULL) {u\setDft($topic, t('for Promo Team')); return r\rMail('tell-staff', 'info@' . CG_DOMAIN, compact(ray('topic body')), $reply);}
function launchWebsocket() {} // exec(tr('nohup setsid php %APP_SOCKET_URL > /dev/null 2>&1 &'));} // launch inter-mobile communications for testing (it checks if it's already running); "php" must be lowercase. But this has to run as a root cron job instead (edit with crontab -e).
function ccGiftType($coId) {return in($coId, BUCKET_UIDS) ? 'cggift' : (r\acct($coId)->sponsored ? 'fbo' : 'gift');}
function no1099k($tot) { return ($tot < MIN1099); }
