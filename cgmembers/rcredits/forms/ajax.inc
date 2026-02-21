<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Cron as cr;

/**
 * Handle an arbitrary AJAX request.
 * GET parameters:
 * @param string op: what to do
 * @param string sid: session id
 * NOTE: to test for syntax errors, paste "include R_ROOT . '/forms/ajax.inc';" into the PHP window.
 * NOTE: autocomplete and account selection dropdown are below under "suggestWho" (hard to find otherwise)
 * @todo: maybe all zero uid for some ops (by looking up two fields in sessions?)
 */

function ajax($args = NULL) {
  global $channel; $channel = TX_AJAX; // tell log and tellAdmin our environment
  if (!$args) $args = $_POST ?: $_GET; // not u\setDft ($args comes in as FALSE usually)
  extract(just('op sid data', $args, NULL));

  header('Content-Type: application/json');

  $myid = $aid = NULL;
  if (test()) extract(just('aid myid', $args)); else // skip the next block if testing (because session isn't recorded yet)
  if (!in($op, 'qBtn setNotice signinThenPay stripeSetup stripeTx')) { // no sid if not signed in
    if (!nn($sid) or !$vars = w\sessionVars($sid)) return ajaxErr(tr('signed out'));
    if (!$aid = $vars['uid'] or !$myid = w\svar('myid')) return ajaxErr('not signed in');
  }

  if ($data) $data = u\dejsonize($data); // must be passed encoded or jQuery post() passes null
/**/  if (!in($op, 'suggestWho suggestNonmember')) u\loga('debug', compact(ray('op myid aid')) + ($data ?: [])); // pass data as array so its NOLOG args get hidden
  $mya = ($myid and $aid) ? r\acct($myid, $aid) : NULL; // usually relevant
  
  switch ($op) {
    case 'expandTx':
      extract(just('uids xid template', $data, NULL));
      $template = base64_decode($template);
      $uids = plain($uids); // since we're using it in SQL
      $toMe = 'uid2 IN (:uids)';
      $cols = tr("IF($toMe,uid1,uid2) AS uid, IF($toMe,for2,for1) AS purpose, IF($toMe,amt,-amt) AS amt, type", compact('uids'));
      $where = tr("xid=:xid AND (uid1 IN (:uids) OR $toMe) ORDER BY eid", compact('uids'));
      $sql = "SELECT $cols FROM txs WHERE $where";
      $rows = db\rows($cols, 'txs', $where, compact(ray('xid uids')));
      foreach (ray('expand xid date name purpose amount runBalance buttons') as $k) $subs0[$k] = '';
      $amt1 = '';
      $auxTxs = '';
      
      foreach ($rows as $row) {
        extract((array) $row);
        if (in($type, E_PRIMES)) {$amt1 = $amt; continue;} // don't duplicate the prime entry (but fix its amount)
        $name = db\get('fullName', 'users', compact('uid'));
        $amount = number_format($amt, 2);
        $subs = compact(ray('name purpose amount'));
        $auxTxs .= tr($template, $subs + $subs0);
      }
      return ajaxRet(compact(ray('auxTxs, amt1')));
    
    case 'suggestWho': // get selections for a whoFld (see w\whoFldSubmit()
      $res = be\memberRay($myid, $aid, $data ? nni($data, 'restrict') : '');
      return exitJust(u\jsonize($res ? array_values($res) : []));
    
    case 'suggestNonmember':
    /*  foreach (ray('fullName email phone, fullName address city state zip') as $flds) {
          if ($orig = db\row('*', 'people', compact(ray($flds)))) break;
        } */
        
    case 'signinThenPay':
      $url = BASE_URL . '/' . w\first('signin', 'ccpay/code=' . u\cryRay($data));
      return ajaxRet(compact('url'));
      
    case 'stripeSetup': return stripeSetup($data); // get client secret for card payment
    
    case 'stripeTx': return stripeTx($data); // do a Stripe payment (possibly recurring, based on stripeSetup)

    case 'suggestTxDesc': // suggest a description and/or category for a transaction being entered
      extract(just('otherId purpose paying', $data, NULL));
      $purpose = $purpose ?: r\suggestDesc($myid, $otherId, $paying);
      $cat = r\bestCat(ray('myid otherId desc type channel paying', $myid, $otherId, $purpose, E_PRIME, TX_WEB, $paying));
      return ajaxRet(compact(ray('purpose cat')));

    case 'delPair': // delete a transaction and its reversal
      extract(just('xid', $data));
      if (!$xid0 = db\get('reversesXid', 'tx_hdrs', compact('xid'))) return ajaxErr(t('This transaction does not reverse anything.'));
      $tnm = db\exists('txs2', compact('xid')) ? 'txs2' : 'txs';
      $DBTX = \db_transaction();
      db\del($tnm, 'xid IN (:xid, :xid0)', compact(ray('xid xid0')));
      unset($DBTX);
      return ajaxMsg(t('Done!'));
      
    case 'delAux': // delete an auxiliary transaction
      extract(just('id', $data));
      if (!$id0 = db\get('id', 'tx_entries', 'id=:id AND entryType IN (:E_AUX, :E_REBATE, :E_XFEE)', compact('id'))) return ajaxErr(t('That is not an auxiliary transaction.'));
      db\del('tx_entries', 'ABS(id)=:id', compact('id'), TRUE);
      return ajaxMsg(t('Done!'));
      
    case 'qBtn': // question mark help button
      include_once R_ROOT . '/help-scraps.inc';
      return ajaxRet(helpScrap($data['topic']));
    
    case 'vKeyPw': return ajaxRet(ray('codePw', u\cryRay(just('vKeyPw', $data))));
  
    case 'ssn': // verify member's SSN
      if ($mya) $mya->ssnCheck();
      exit();
    
    case 'cgPayCode': // get cgPay code
      include_once R_ROOT . '/forms/code.inc';
      extract(just('fer expires', $data));
      $data['for'] = nni(CGBUTTON_CODES, $fer);
      unset($data['fer']);
      if ($expires and $err = u\badDate($data['expires'], strtotime('tomorrow'))) return ajaxErr(tr($err));
      foreach ($data as $k => $v) if ($v == '') unset($data[$k]);
      $code = w\code($data + ray('account secret', $mya->mainQid, $mya->emailCode));
      return strhas($code, ' ') ? ajaxErr($code) : ajaxRet(compact('code'));
    
    case 'setNotice': // change notice preference for one notice type (code, type, freq)
      extract(just('code type freq', $data, ''));
      if (!$ray = u\decryRay($code)) return ajaxErr(t('Bad ID code.'));
      $idRay = just('pid uid', $ray);
      $tnm = nni($idRay, 'pid') ? 'people' : 'users';
      $notices = ray(db\get('notices', $tnm, $idRay));
      $notices[$type] = $freq;
      db\update($tnm, $idRay + ray('notices', u\deray($notices)), key($idRay));
      return ajaxMsg(t('Done!'));

    case 'relations':
      extract(just('name v', $data, ''));
      list ($fld, $reid) = explode('-', $name);
      $other = r\relation('other', $reid);
      $a = r\acct($myid, $other);
      
      if ($fld == 'delete' or $fld == 'permission') {
        list ($managePerm, $joinPerm) = [r\perm(B_MANAGE), r\perm(B_JOINT)];
        $v0 = r\relation('permission', $reid);
        if ($mya->co and $v < $managePerm and $fld == 'permission' and !db\exists('u_relations', 'main=:main AND reid<>:reid AND permission>=:perm', ray('main reid perm', $myid, $reid, $managePerm))) return relPermErr(tr('need a manager'), $reid, $managePerm);
        if ($v == $joinPerm or $v0 == $joinPerm) { // handle joining or unjoining accounts
          if ($v == $joinPerm and db\exists('u_relations', 'main=:myid AND other<>:other AND permission=:joinPerm', compact(ray('myid other joinPerm')))) return relPermErr(t('too many joins'), $reid, $v0);
          $msg = r\setJoin($reid, $v == $joinPerm);
        } else { $msg = ''; }
        
        if ($fld == 'delete') {
          db\del('u_relations', compact('reid'));
          $subs = ray('otherName', db\get('fullName', 'users', ray('uid', $other)));
          $msg = u\joi('|', [$msg, ajaxMsg(tr('deleted relation', $subs))]);
        } else { $a->setRel($fld, $v); }
        return ajaxMsg($msg); // show messages, if any
      } else {
        $a->setRel($fld, $v); // toggle
        return ajaxMsg(t('Done!'));
      }
    
    case 'changeCtty': // change member's community
      extract(just('newCtty retro', $data));
      if (!$mya or !$mya->changeCtty($newCtty, $retro, $err)) return ajaxErr(t('Community change failed.|') . $err);
      return ajaxMsg(t('Done!'));

    case 'getLocus':
      u\FAIL('ajax getLocus unused');
      extract(just('location', $data));
      return ajaxRet(array_combine(ray('latitude longitude zip'), u\getLocus($location)));
      
    case 'setBit':
      extract(just('bit on', $data));
      if ($mya and $b = u\consta('b', $bit) and $b < B_ADMIN) {
        $mya->setBit($b, $on);
        return ajaxMsg(t('Done!'));
      } else return ajaxErr(t('nope'));
    
    case 'eval':
      if (!isDEV) return;
      extract(just('jsCode', $data, ''));
      return ajaxRet(u\decryRay($jsCode));
    
    case 'set':
      extract(just('k v', $data));
      if ($mya) {
        if (in($k, 'minimum') and $err = u\badAmount($v)) return ajaxErr($err);
        $mya->update($k, $v);
        return ajaxMsg(t('Done!'));
      } else return ajaxErr(t('nope'));
    
    case 'dpOffset':
      if (!$mya or !$mya->admReconcile) return ajaxErr(t('You do not have permission.'));
      extract(just('amount', $data));
      if (!is_numeric($amount) or $amount <= 0) return ajaxErr('bad dollar pool offset transfer amount: ' . $amount);
      // disabled until we remember what this is supposed to do! r\acct(1)->bank(-amount, '', [], $why);
      return ajaxMsg($msg);
    
    case 'who':
      extract(just('who question amount selfErr restrict', $data));
      $amtDpy = u\fmtAmt(nn($amount) ?: 0);
      $who = be\identify($who0 = $who, $mya, $selfErr, $restrict);
      if (is_string($who)) return ajaxErr(tr($who));
      if (is_object($whoA = $who)) ajaxRet(ray('who confirm', $whoA->id, $question ? tr($question, 'name amount', $whoA->fullName, $amtDpy) : ''));
      
      // $who must be an array, so create modal selection form
      $choice = selectFld(BARE, '', ['size' => 12], $who);
      $which = render($choice);
      $title = t($question ?: t('Which account?'), ray('name amount', t('whom'), $amtDpy));
      $message = '';
      return ajaxRet(compact('which', 'title', 'message'));
    
    case 'whoami': // return one or more data fields
      return $mya ? ajaxRet(ray('whoami', "$mya->fullName ($mya->mainQid)")) : ajaxErr('no current account!');
    
    case 'bumpShout':
      db\q('UPDATE u_shout SET rating=IF(rating=3, 0, rating+1) where uid=:uid', ray('uid', r\qo($data['qid'])->id));
      return ajaxMsg(t('Success!'));
    
    default: return ajaxErr(t('Invalid op'));
  }
  
}

/**
 * Create the basis for a (possibly recurring) payment through Stripe
 * @return {id, client_secret} or an error message (id is the setupIntent ID)
 */
function stripeSetup($data) {
  extract(just('for item period honor honored coId fullName email phone zip country notes', $data, NULL));
  require_once R_ROOT . '/stripe.inc';

  if ($period == 'once') $period = '';
  $periodDesc = $period ? ucfirst(r\recurRay()[$period]) : '';
  $description = $item ?: ($for == FOR_DONATE ? t('donation') : ($for ?: t('purchase')));
  
  $email = strtolower($email);
  list ($city, $state) = r\zipCity($zip);
  $coA = r\acct($coId);
  $notices = NOTICE_NONE;
  $honor = $honored ? t('in %honor of %honored', compact(ray('honor honored'))) : NULL;
  $notes = $honor . (($honor and $notes) ? ' - ' : '') . $notes;
  $source = t('paid %nm (by card)', 'nm', $coA->bestName);
//    $reportInfo += compact(ray($inputFlds));
  
  $nmNow = "CONCAT('%', REPLACE(fullName, ' ', '%'), '%')";
  $newNm = "CONCAT('%', REPLACE(:fullName, ' ', '%'), '%')";
  $where = "email=:email AND (fullName LIKE $newNm OR :fullName LIKE $nmNow)";
  $old = db\row('people', $where, compact(ray('fullName email')));

  if (!$old or empty($old->stripeCid)) {
    $cust = stripe('cust', $data);
    if (!is_object($cust)) return ajaxErr($cust); // anything other than an object is an error message
    $stripeCid = $cust->id;
  }

  if ($old) { // if person already in people table
    extract(just('pid city notices source notes stripeCid', $old), EXTR_PREFIX_ALL, 'o');
    $new = compact(ray('phone notes'));
    if (!$o_city) $new += compact(ray('city state zip country'));
    foreach (ray('pid notices source stripeCid') as $k) $new[$k] = $$k = $old->$k ?: $$k;
    $notes = ($o_notes and $notes) ? "$notes\n$o_notes" : "$notes$o_notes";
    db\update('people', $new, 'pid');
  } else {
    $pid = db\insert('people', compact(ray('fullName email phone zip city state country notices notes source stripeCid')), 'pid');
  }
  
  $intent = stripe('setup', compact(ray('stripeCid description')));
  if (!is_object($intent)) return ajaxErr($intent); // anything other than an object is an error message
  
  return ajaxRet(ray('pid purpose stripeId secret', $pid, $description, $intent->id, $intent->client_secret));
}

/**
 * Create a donation payment through Stripe
 * @return {msg}: msg is an encrypted success or failure message
 */
function stripeTx($data) {
  extract(just('amount for purpose period coId pid stripeId coverCCFee coverFSFee', $data, NULL));
  require_once R_ROOT . '/rcron/rcron.inc';
  
  $flags = 
    ($for == FOR_DONATE ? u\bit(B_GIFT) : 0)
  | ($coverCCFee ? u\bit(B_COVERCC) : 0)
  | ($coverFSFee ? u\bit(B_COVERFS) : 0)
  | u\bit(B_CC);

  $info = compact(ray('amount period purpose flags stripeId')) 
        + ray('action from to payerType payer duration', 
        ACT_PAY, MATCH_PAYER, $coId, REF_PERSON, $pid, PER_1);
  $id = db\insert('tx_timed', $info); // create a timed record even if not recurring (for cron)
  $template = (array) db\row('tx_timed', compact('id')); // pass all fields to cron job
  $template += ray('lastRecur', 0);
  $queueId = cr\queue('recur', $template, TRUE);
  
  [$ok, $msg] = cr\recur($template);
  db\del('queue', ray('id', $queueId));
  if ($period == PER_1) db\del('tx_timed', compact('id')); // we don't need to keep timed records for one-timers

  return $ok ? ajaxMsg(u\ry('P', $msg)) : ajaxErr($msg);
}

function ajaxErr($msg) { http_response_code(500); return ajaxMsg($msg, 0); }
function ajaxMsg($message = '', $ok = 1) {return ajaxRet(compact('message'), $ok);}
function ajaxRet($data0, $ok = 1) {
  if (!isset($data0['which'])) u\loga('ajax', $data0); // don't log hundreds of choices (log data0 so secrets are hidden)
  return exitJson($data0 + compact('ok'));
//  $data = u\jsonize($data0 + compact('ok'));
//  exit($data);
}
function relPermErr($message, $reid, $v0) {
  return ajaxRet(compact(ray('message v0')), 0);
}
function latlonRet($latlon) {return ajaxRet(ray('latitude longitude', $latlon));}
