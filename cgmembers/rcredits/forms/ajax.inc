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

function ajax() {
  global $channel; $channel = TX_AJAX; // tell log and tellAdmin our environment
  extract(just('op sid data', $args = $_POST ?: $_GET, NULL));
  if (nn($_SERVER['HTTP_X_REQUESTED_WITH']) != 'XMLHttpRequest') {
    return ajaxErr('cross-site'); 
    r\tellAdmin('ajax cross-site', $args + $_SERVER);
  }

  header('Content-Type: application/json');
  $myid = $aid = NULL;

  if ($op != 'presignup') { // no sid if not signed in
    if (!nn($sid) or !$vars = w\sessionVars($sid)) return ajaxErr('bad sid');
    if (!$aid = $vars['uid'] or !$myid = $vars['myid']) return ajaxErr('not signed in');
  }
//  $mya = r\acct($myid, $aid);
/**/  lug(compact(ray('op data myid aid')));
  $data = json_decode($data); // must be passed encoded or jQuery post() passes null
  $mya = ($myid and $aid) ? r\acct($myid, $aid) : NULL; // usually relevant
  
  if ($op == 'ssn') { // verify member's SSN
    if ($mya) $mya->ssnCheck();
    exit();
  } elseif ($op == 'typeWho') { // get selections for suggestWho
    exit(json_encode(be\memberRay($myid, $aid, $data))); // data=1 means companies only
  } elseif ($op == 'relations') {
    extract(just('name v', $data, ''));
    list ($fld, $reid) = explode('-', $name);
    $other = r\relation('other', $reid);
    $a = r\acct($myid, $other);
    
    if ($fld == 'delete' or $fld == 'permission') {
      list ($managePerm, $joinPerm) = [r\perm(B_MANAGE), r\perm(B_JOINT)];
      $v0 = r\relation('permission', $reid);
      if ($mya->co and $v < $managePerm and !db\exists('r_relations', 'main=:main AND other<>:other AND permission>=:perm', ray('main other perm', $myid, $other, $managePerm))) return relPermErr(t('need a manager'), $reid, $managePerm);
      if ($v == $joinPerm or $v0 == $joinPerm) { // handle joining or unjoining accounts
        if ($v == $joinPerm and db\exists('r_relations', 'main=:myid AND other<>:other AND permission=:joinPerm', compact(ray('myid other joinPerm')))) return relPermErr(t('too many joins'), $reid, $v0);
        $msg = r\setJoin($reid, $v == $joinPerm);
      } else { $msg = ''; }
      
      if ($fld == 'delete') {
        db\del('r_relations', compact('reid'));
        $subs = ray('otherName', db\get('fullName', 'users', ray('uid', $other)));
        $msg = u\joins($msg, '|', ajaxMsg(tr('deleted relation', $subs)));
      } else { $a->setRel($fld, $v); }
      if ($msg) return ajaxMsg($msg); // show messages, if any
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
  } elseif ($op == 'presignup') {
    $info = just('preid fullName legalName email phone', $data);
    db\update('signup', $info, 'preid');
  } elseif ($op == 'dpOffset') {
    if (!$mya or !$mya->superAdmin) return ajaxErr(t('You do not have permission.'));
    extract(just('amount', $data));
    if (!is_numeric($amount) or $amount <= 0) return ajaxErr('bad dollar pool offset transfer amount: ' . $amount);
    // temporarily disabled until we remember what this is supposed to do! r\acct(1)->bank(-amount, '', [], $why);
    ajaxMsg($msg);
  } elseif ($op == 'who') {
    extract(just('who question amount', $data));
    $amtDpy = u\fmtAmt(nn($amount) ?: 0);
    if (!is_array($whoA = be\identify($who, $mya, mb_stripos(" $question", t(' pay ')) !== FALSE, 'no self-trading'))) {
      ajaxRet(ray('who confirm', $whoA->mainQid, $question ? tr($question, 'name amount', $whoA->fullName, $amtDpy) : ''));
    }
    list ($msg, $args, $choices) = $whoA;
    $message = t($msg, $args);
    if (empty($choices)) return ajaxErr($message);
    foreach ($choices as $uid => $fullName) {
      $choices[$qid = r\qid($uid)] = "$qid $fullName";
      unset($choices[$uid]);
    }

    // create modal selection form
    $choice = selectFld('', '', ['size' => 12], $choices);
    $which = render($choice);
    $title = t($question ?: t('Which account?'), ray('name amount', t('whom'), $amtDpy));
    ajaxRet(compact('which', 'title', 'message'));
  } elseif ($op == 'whoami') { // return one or more data fields
//    $data = just('fullName', ray($data)); // limit what we can get to what we need
    return $mya ? ajaxRet(ray('whoami', "$mya->fullName ($mya->mainQid)")) : ajaxErr('no current account!');
  } elseif ($op == 'bumpShout') {
    db\q('UPDATE u_shouters SET rating=IF(rating=3, 0, rating+1) where uid=:uid', ray('uid', r\qo($data->qid)->id));
  } else return ajaxErr(t('Invalid op'));
  return ajaxMsg(t('Done!'));
}

function ajaxErr($msg) {return ajaxMsg($msg, 0);}
function ajaxMsg($message = '', $ok = 1) {return ajaxRet(compact('message'), $ok);}
function ajaxRet($data0, $ok = 1) {
  $data = json_encode($data0 + compact('ok'));
  if (!isset($data0['which'])) u\loga('ajax', $data); // don't log hundreds of choices
  exit($data);
}
function relPermErr($message, $reid, $v0) {
  return ajaxRet(compact(ray('message v0')), 0);
}
function latlonRet($latlon) {return ajaxRet(ray('latitude longitude', $latlon));}
