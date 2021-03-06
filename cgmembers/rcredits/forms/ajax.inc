<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Handle an arbitrary AJAX request.
 * GET parameters:
 * @param string op: what to do
 * @param string sid: session id
 * @todo: maybe all zero uid for some ops (by looking up two fields in sessions?)
 */

function ajax($args = NULL) {
  global $channel; $channel = TX_AJAX; // tell log and tellAdmin our environment
  if (!$args) $args = $_POST ?: $_GET; // not u\setDft ($args comes in as FALSE usually)
  extract(just('op sid data', $args, NULL));

  header('Content-Type: application/json');

  $myid = $aid = NULL;
  if (test()) extract(just('aid myid', $args)); else // skip the next block if testing (because session isn't recorded yet)
  if (!in($op, 'setNotice')) { // no sid if not signed in
    if (!nn($sid) or !$vars = w\sessionVars($sid)) return ajaxErr(t('You are signed out. Click <%a>here</a> to sign back in.', '_a', atag('/signin')));
    if (!$aid = $vars['uid'] or !$myid = $vars['myid']) return ajaxErr('not signed in');
  }

/**/  lug(compact(ray('op data myid aid')));
  $data = u\dejsonize($data); // must be passed encoded or jQuery post() passes null
  $mya = ($myid and $aid) ? r\acct($myid, $aid) : NULL; // usually relevant
  
  if ($op == 'qBtn') { // question mark help button
    include_once R_ROOT . '/help-scraps.inc';
    return ajaxRet(helpScrap($data['topic']));
  
  } elseif ($op == 'ssn') { // verify member's SSN
    if ($mya) $mya->ssnCheck();
    exit();
    
  } elseif ($op == 'cgPayCode') { // get cgPay code
    include_once R_ROOT . '/forms/code.inc';
    extract(just('fer expires', $data));
    $data['for'] = nni(ray('credit,gift,,donate'), $fer); // fer=2 (other) is ignored
    unset($data['fer']);
    if ($expires and $err = u\badDate($data['expires'], strtotime('tomorrow'))) return ajaxErr(tr($err));
    foreach ($data as $k => $v) if ($v == '') unset($data[$k]);
    $code = w\code($data + ray('account secret', $mya->mainQid, $mya->emailCode));
    return strhas($code, ' ') ? ajaxErr($code) : ajaxRet(compact('code'));
    
  } elseif ($op == 'suggestWho') { // get selections for a whoFld (see w\whoFldSubmit()
    $res = be\memberRay($myid, $aid, $data ? nni($data, 'restrict') : '');
    return exitJust(u\jsonize($res ? array_values($res) : []));
    
  } elseif ($op == 'setNotice') { // change notice preference for one notice type (code, type, freq)
    extract(just('code type freq', $data, ''));
    if (!$ray = u\decryRay($code)) return ajaxErr(t('Bad ID code.'));
    $idRay = just('pid uid', $ray);
    $tnm = nni($idRay, 'pid') ? 'people' : 'users';
    $notices = ray(db\get('notices', $tnm, $idRay));
    $notices[$type] = $freq;
    db\update($tnm, $idRay + ray('notices', u\deray($notices)), key($idRay));
    
  } elseif ($op == 'relations') {
    extract(just('name v', $data, ''));
    list ($fld, $reid) = explode('-', $name);
    $other = r\relation('other', $reid);
    $a = r\acct($myid, $other);
    
    if ($fld == 'delete' or $fld == 'permission') {
      list ($managePerm, $joinPerm) = [r\perm(B_MANAGE), r\perm(B_JOINT)];
      $v0 = r\relation('permission', $reid);
      if ($mya->co and $v < $managePerm and !db\exists('r_relations', 'main=:main AND reid<>:reid AND permission>=:perm', ray('main reid perm', $myid, $reid, $managePerm))) return relPermErr(t('need a manager'), $reid, $managePerm);
      if ($v == $joinPerm or $v0 == $joinPerm) { // handle joining or unjoining accounts
        if ($v == $joinPerm and db\exists('r_relations', 'main=:myid AND other<>:other AND permission=:joinPerm', compact(ray('myid other joinPerm')))) return relPermErr(t('too many joins'), $reid, $v0);
        $msg = r\setJoin($reid, $v == $joinPerm);
      } else { $msg = ''; }
      
      if ($fld == 'delete') {
        db\del('r_relations', compact('reid'));
        $subs = ray('otherName', db\get('fullName', 'users', ray('uid', $other)));
        $msg = u\joins($msg, '|', ajaxMsg(tr('deleted relation', $subs)));
      } else { $a->setRel($fld, $v); }
      return ajaxMsg($msg); // show messages, if any
    } else { $a->setRel($fld, $v); } // toggle
    
  } elseif ($op == 'changeCtty') { // change member's community
    extract(just('newCtty retro', $data));
    if (!$mya or !$mya->changeCtty($newCtty, $retro, $err)) return ajaxErr(t('Community change failed.|') . $err);
//    ajaxRet(['msg' => "uid is $uid, newCtty is $newCtty"]);

  } elseif ($op == 'getLocus') {
    u\FAIL('ajax getLocus unused');
    extract(just('location', $data));
    return ajaxRet(array_combine(ray('latitude longitude zip'), u\getLocus($location)));
    
  } elseif ($op == 'setBit') {
    extract(just('bit on', $data));
    if ($mya and $b = u\consta('b', $bit) and $b < B_ADMIN2) {
      $mya->setBit($b, $on);
    } else return ajaxErr(t('nope'));
    
  } elseif ($op == 'eval' and isDEV) {
    extract(just('jsCode', $data, ''));
    return ajaxRet(u\decryRay($jsCode));
    
  } elseif ($op == 'set') {
    extract(just('uid k v', $data));
    if ($a = r\acct($uid)) {
      if (in($k, 'minimum') and $err = u\badAmount($v)) return ajaxErr($err);
      $a->update($k, $v);
    } else return ajaxErr(t('nope'));
    
  } elseif ($op == 'dpOffset') {
    if (!$mya or !$mya->superAdmin) return ajaxErr(t('You do not have permission.'));
    extract(just('amount', $data));
    if (!is_numeric($amount) or $amount <= 0) return ajaxErr('bad dollar pool offset transfer amount: ' . $amount);
    // temporarily disabled until we remember what this is supposed to do! r\acct(1)->bank(-amount, '', [], $why);
    ajaxMsg($msg);
    
  } elseif ($op == 'who') {
    extract(just('who question amount selfErr restrict', $data));
    $amtDpy = u\fmtAmt(nn($amount) ?: 0);
    $who = be\identify($who, $mya, $selfErr, $restrict);
    if (is_string($who)) return ajaxErr(tr($who));
    if (is_object($whoA = $who)) ajaxRet(ray('who confirm', $whoA->id, $question ? tr($question, 'name amount', $whoA->fullName, $amtDpy) : ''));
    
    // $who must be an array, so create modal selection form
    $choice = selectFld(BARE, '', ['size' => 12], $who);
    $which = render($choice);
    $title = t($question ?: t('Which account?'), ray('name amount', t('whom'), $amtDpy));
    $message = '';
    ajaxRet(compact('which', 'title', 'message'));
    
  } elseif ($op == 'whoami') { // return one or more data fields
//    $data = just('fullName', ray($data)); // limit what we can get to what we need
    return $mya ? ajaxRet(ray('whoami', "$mya->fullName ($mya->mainQid)")) : ajaxErr('no current account!');
    
  } elseif ($op == 'bumpShout') {
    db\q('UPDATE u_shouters SET rating=IF(rating=3, 0, rating+1) where uid=:uid', ray('uid', r\qo($data['qid'])->id));
    
  } else return ajaxErr(t('Invalid op'));
  
  return ajaxMsg(t('Done!'));
}

function ajaxErr($msg) {return ajaxMsg($msg, 0);}
function ajaxMsg($message = '', $ok = 1) {return ajaxRet(compact('message'), $ok);}
function ajaxRet($data0, $ok = 1) {
  $data = u\jsonize($data0 + compact('ok'));
  if (!isset($data0['which'])) u\loga('ajax', $data); // don't log hundreds of choices
  exit($data);
}
function relPermErr($message, $reid, $v0) {
  return ajaxRet(compact(ray('message v0')), 0);
}
function latlonRet($latlon) {return ajaxRet(ray('latitude longitude', $latlon));}
