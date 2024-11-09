<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Complete a "no signin required" request
 *   called with:
 *     <baseurl>/do/<op>~<code> (for example: new.CommonGood.earth/do/doProxies~PabcDEF123
 *     <baseurl>/do/<code>
 *     <baseurl>/do/<error message (with a space in it)>
 * @param string $form: a form to complete without signing in
 * @param string code: an array encrypted with u\cryRay, possibly including:
 *   op:      what to do (REQUIRED). Must be of the form do<form> (for example doProxies) if the call is <op>~...
 *   qid:     the account QID
 *   ecode:   the account's email security code (deprecated. delete 10/1/2020)
 *   mo:      month that the link should expire, relative to first rdo (see rdoMonth()) (deprecated. delete 10/1/2020)
 *   v:       an amount or identifier, specific to the operation
 *   sql:     a database request, if any
 *   subs:    substitutions for the database request
 *   
 * Possible values for op:
 *   min: change minimum to $v
 *   sql: do the database operation in $sql
 *   addr: show an address for account QID $v (for shipping or thank-you card)
 *   inv: pay invoice number $v
 *   confirmInvite: confirm an invitation
 *   do<form>: allow member to visit one specific page
 * @see also acct::doLink()
 */
function rdo($code) {
/**/  flog(pr(compact(ray('code'))));
  global $channel;
  if (test()) foreach (['error', 'status'] as $k) u\deleteFile(f('t.dosayFilename', $k));

  if (strpos($code, ' ')) return doErr(plain($code));
  
  if (strhas($code, '~')) { // newsletter link (doWhatever~code)
    list ($op, $code) = explode('~', $code);
    if (!$code and $op == 'doDonate') return w\go('cc'); // special case for donations from nonmembers
    $info = u\decryRay($code);
    extract(just('qid expire v sql subs', $info, NULL));
    extract(just('op', $info)); // overwrite op, if op is specified
    if (!$qid or !$a = r\acct($qid)) {
      $nonMemberErr = t('That link is only for current members. <%a>Click here to sign up!</a>', '_a', atag('/signup'));
      if (!u\starts($op, 'do') and !in($op, 'survey')) return doErr($nonMemberErr); // doFormName works for nonmembers too (if allow='any')
    }
  } else { // normal email link
    if (!$info = u\decryRay($code)) return doErr(t('That link is archaic.'));
    extract(just('op qid expire v sql subs', $info, NULL));
    $a = $qid ? r\acct($qid) : NULL;
    if (in($op, 'nosecret debtok rsv sql addr inv confirmInvite vote yesno join') and !$a) return doErr(t('That link does not correspond to a %PROJECT account.'));
  }
  
  if (nn($expire) and now() >= $expire) return doErr(t('link expired'));

  $channel = TX_LINK;
  
  if (u\starts($op, 'do') and $formName = substr($op, 2)) {
    if ($op == 'doDonate' and !$a->ok) return w\go('cc'); // special case for donations from nonmembers
    $file = strtolower(R_ROOT . "/forms/$formName.inc");
    if (file_exists($file)) include_once $file;    
    return w\showForm($formName, nn($a) ? "qid=$a->mainQid&ecode=$a->emailCode&$v" : '');
  }

  if ($op == 'cancelTimed') $res = db\update('tx_timed', ray('id end', $v, now()));
  if ($op == 'nosecret') $res = $a->setBit(B_SECRET, FALSE);
  if ($op == 'debtok') $res = $a->setBit(B_DEBT, TRUE);
  if ($op == 'rsv') {return w\go("savings/qid=$qid&ecode=$ecode");}
  if ($op == 'sql') $res = db\q($sql, $subs);

  if ($op == 'survey') {
      extract(just('email amt', $_GET, NULL));
      if (!$amt or !is_numeric($amt)) return doErr(t('Survey click failed.'));
      if ($qid) $a->update('iintent', $amt); else r\tellAdmin(t('Investment Intent'), compact(ray('email amt')));
      return doSay(t('Your investment intent is noted. Thank you for responding! We will send you a note when it\'s time to invest and launch a club in your area.'));
  }
  
  if ($op == 'addr') {
    if (!$a2 = r\acct($v)) return doErr(t('That account no longer exists.'));
    $phone = $a2->co ? t('Phone: ') . u\fmtPhone($a2->phone) : '';
    $addr = t('Physical address: ') . $a2->location(TRUE);
    return doSay(tr('%nm<br>%postal<br>%addr<br>%phone<br>', 'nm postal addr phone', $a2->fullName, $a2->postalAddr, $addr, $phone));
  } 
  
  if ($op == 'inv') {
    if ($err = r\badUnpaidInv($v)) return doErr($err);
    return w\go("/handle-invoice/nvid=$v&code=$code");
  } 
  
  if ($op == 'confirmInvite') {
    $a->setBit(B_CONFIRMED);
    return doSay(t('Your invitation to %nm is confirmed!', 'nm', $a->fullName));
  }
  
  if ($op == 'vote' or $op == 'yesno') { // NOTE: about a third of the time, $_POST comes in empty (even from the same computer)
    $issue = $v;
    if ($issue != '20141220') return doErr('link expired');
//    $vote = 0;
//    foreach (ray('wl sl wd ed no') as $k => $vv) $vote |= u\setBit($vote, $k, isset($_POST[$vv]));
    extract(just('yes no', $args, NULL));
    $vote = $yes ? 1 : 0;
    $info = compact(ray('uid issue vote'));
    $vid = db\get('vid', 'r_votes', 'uid=:uid AND issue=:issue', compact('uid', 'issue'));
//    $vid = 0;
///  debug(compact(ray('op args issue yes no vote info vid')));
    if ($vid) db\update('r_votes', $info + compact('vid'), 'vid'); else db\insert('r_votes', $info, 'vid');
    $say = t('Thank you for your RSVP! ');
    if ($no) $say .= t('We will miss you. ');
    r\tellAdmin('RSVP!', ['coming' => $yes ? 'yes' : 'no'], $uid);
    $res = TRUE;
  }
  
  if ($op == 'join') {
    $otherId = $v;
    $joint = r\perm(B_JOINT);
    if  (r\relation(1, $otherId, $a->id, 'permission=:joint', compact('joint'))) { // inviter still wants it?
      $a->unjoinAllBut($otherId); // zap any other joins or join requests
      r\acct($a->id, $otherId)->setRel('permission draw', $joint, 0);
      $a->join(r\acct($otherId)); // remember who is joined
      $res = TRUE;
    }
  }

  if (!nn($res)) return doErr(t('Request failed.'));
  return doSay(nn($say) . t('Your request was successfully completed.'));
}

