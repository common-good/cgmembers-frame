<?php
namespace CG\Web;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Txs as x;
use CG\Admin as a;
use CG\Risk as k;
use CG\Web as w;

/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

require_once __DIR__ . '/rweb-subs.inc';
require_once __DIR__ . '/../cg-backend.inc';
//include_once __DIR__ . '/rweb-txs.inc';

define('UPLOAD_DIR', dirname($_SERVER['DOCUMENT_ROOT']) . '/public_ftp/incoming');

/**
 * Show a form called from a form linked to in an email or during signup, without signing in.
 * For example, /do/doEvents~NEWAAA~abcd123~ (in MailChimp: /do/doEvents~*|ID|*~*|ECODE|*~)
 */
function formProx($form, &$sta, $args) {
  extract(just('page', $args0 = $args));
/*  foreach (ray('Event Democracy Proposal') as $k) {
    if (strpos($page, $k) !== FALSE) include_once DRUPAL_ROOT . R_PATH . '/rvote/rvote-web.inc';
  } */
  $file = strtolower(R_ROOT . "/forms/$page.inc");
  if (file_exists($file)) include_once $file;
  
  $func = 'CG\\Web\\form' . ucwords($page);
  $form = $func($form, $sta, $args0);
  foreach (['validate', 'submit'] as $k) {
    if (!nni($form, "#$k") and function_exists($func . "_$k")) $form["#$k"] = [$func . "_$k"];
  }
  return $form;
}

/**
 * Print a JSON array of communities
 */
function cttyList() {
  $cttys = db\q('SELECT uid,fullName FROM users WHERE uid<0 ORDER BY fullName')->fetchAllKeyed();
  $cttys[1] = trim(str_replace(PROJECT, '', r\regionField('fullName')));
  $cttys[0] = t('ALL communities');
  foreach ($cttys as $k => $v) $cttys[$k] = trim(str_replace(PROJECT, '', $v));
  asort($cttys);
  unset($cttys[$seed = r\serverUid()]);
  $cttys[$seed] = t('Seedpack (no community yet)'); // last
/**/ echo json_encode($cttys);
  exit();
}

/**
 * Sections with submenus.
 */
function formHistory($form, &$sta) {return w\subMenu(t('Account History'), 'history');}
// Invest has its own page
function formCommunity($form, &$sta) {return w\subMenu(t('Community'), 'community');}
function formCo($form, &$sta) {return w\subMenu(t('Company Options'), 'co');}
function formSettings($form, &$sta) {return w\subMenu(t('Settings'), 'settings');}
function formAdmin($form, &$sta) {return w\subMenu(t('Admin Options'), 'sadmin');}

/**
 * Show an example invitation email.
 */
function inviteExample() {
  global $rUrl;
  global $mya;
  $s = file_get_contents("$rUrl/templates/invite.html");
  $note = str_replace("\n", '', t('invite default', 'name', ''));
/**/  echo tr($s, '_personalNote signed fullName phone CODE', $note, u\fmtDate($mya->signed), $mya->fullName, u\fmtPhone($mya->phone), '');
  exit();
}

function formVideos($form, &$sta) {
  $title = t('%PROJECT Member Videos');
  $subtext = t('The videos may not be available until mid-January. Try again in a few days!');
  $form = compact(ray('title subtext'));
  return cgform($form);
}

function formUser($form, &$sta) {return \user_login($form, $sta);} // called from w\go() in Summary

/**
 * Recalculate risk.
 * @param string $type: type of entity: acct or tx
 * @param int $id: record ID for the entity
 * @param bool $show: <show the risk calculation>
 */
function formRerisk($form, &$sta, $type = '', $id = '', $show = FALSE) {
  include_once __DIR__ . '/../cg-risk.inc';
  global $showRiskCalc; 
  
  $showRiskCalc = '';

  if ($type == 'acct') {
    k\acctRisk1($id, getv('riskThresholds', [0,0,0,0]), $risk, $calc);
    $w = ''; $m = K_RED;
  } elseif ($type == 'tx') {
    $x = r\x($id);
    k\txRisk1($x->xid, $risk, $calc);
    $w = u\fmtAmt($x->amount);
    $m = t('@K_RED / @K_THRESHOLD');
  }

  $risk = round($risk);
  
  $showRiskCalc = <<< EOF
  <table id="riskCalc">
  <tr><th>Risk Name</th><th>Weight</th><th>Multiplier</th><th>Risk</th><th>Set Flag?</th></tr>
  $showRiskCalc
  <tr><td><b>TOTAL</b></td><td>$w</td><td>$m</td><td>$calc</td><td>= $risk</td></tr>
  </table>
EOF;
  say($showRiskCalc);
  w\go("sadmin/$type-info/$id", t('info saved'));
}

/**
 * Output an account photo or thumbnail if the caller is authorized.
 * @param string $page: account-photo or account-thumb
 * @param string $args: other parameters, including:
 *   $qid: account letter-code
 *   $orig: show the original photo before it was cropped
 * @return the photo or thumb if the account is a company or the cardCode is valid. Otherwise a generic picture.
 */
function formAccountPhoto($form, &$sta, $page = '', $args = '') {
  global $mya;
  extract(just('qid orig', $args, NULL));
  list($zot, $type) = explode('-', $page);

  $a = nn($qid) ? r\acct($qid, $mya ? $mya->agentId : '') : ($mya ?: FALSE);
  if ($a and ($a->co or $a->can(B_READ))) $photo = $orig ? u\getFile($a->tempPhotoFilename) : $a->$type;

  $imgFlnm = ($mya and $mya->co) ? 'placeholder-co.png' : 'placeholder.png';
  header("Content-type: image/jpeg");
/**/  echo nn($photo) ?: \file_get_contents(R_ROOT . "/images/icons/$imgFlnm");
  return exit();
}

//function formIcon($orig = FALSE) {return memberPhoto(@r\acct()->mainQid, TRUE, $orig);} // Output a small version of the current account's photo.

/**
 * Handle the uploaded photo: crop, resize, rotate, and save.
 * @param int $uid: current account record ID
 * @param string $sid: session ID
 * POSTed: parameters from croppic
 * @return json success or error message            
 */
function photoUpload($uid = '', $sid = '') {
  global $channel; $channel = TX_AJAX;
  global $base_url;
  $f0 = ray('imgUrl imgInitW imgInitH imgW imgH imgY1 imgX1 cropH cropW rotation uid sid');
  $f = ray('url w0 h0 w h y1 x1 cropH cropW angle uid sid');
  $params = $_POST; // don't sanitize this, it only comes from us!
  foreach ($params2 = array_combine($f0, $f) as $k0 => $k) $$k = $params[$k0];
  
  if (!nn($uid) or !nn($sid) or !$sessionVars = w\sessionVars($sid)) return w\photoRet(t('bad sid'));
  $mya = r\acct($uid);
  $tempFilename = $mya->tempPhotoFilename;
  $quality = $mya->co ? 100 : 50; // jpeg compression/quality (0=worst quality, 100=best quality)
  if ($mya->co) foreach (ray('w h x1 y1 cropH cropW') as $k) $$k *= CO_PHOTO_FACTOR; // higher quality for businesses

  if ($recrop = u\starts($url, 'http')) $url = $tempFilename; // picture is already saved and is just being recropped

  try {
    $what = @getimagesize($url);
  } catch (\Exception $e) {$what = FALSE;}
  if (!$what) return w\photoRet(t('That is not an image file.'));
  
  list ($actualW, $actualH, $type, $zotAttr) = $what;
  if ($actualW != $w0 or $actualH != $h0) r\tellAdmin('photo problem?', $_POST + compact('actualW', 'actualH', 'uid'));
  if (!(imagetypes() & $type)) return w\photoRet(t('That image format is not recognized.'));
  $ext = image_type_to_extension($type, FALSE);
  $func = 'imagecreatefrom' . ($ext == 'bmp' ? 'wbmp' : $ext);
///   flog(compact(ray('uid sid f0 f params params2 sessionVars quality cropH cropW what actualW actualH type zotAttr ext func')));
  if (!$img0 = $func($url)) return w\photoRet(t('image creation error'));

  try {
    if (!$recrop) imagejpeg($img0, $tempFilename, 100); // save for a few days in case we have to recrop
  } catch (\Exception $e) {
/**/    lug('failed to save photo file ' . $tempFilename);
  }

  $img = u\alterImg($img0, $w, $h, $x1, $y1, $cropW, $cropH, $angle);

  try {
      $photo = u\img2string($img, $quality);
      $mya->update(compact('photo')); // do not combine this with line above
  } catch (\Exception $e) {
    return w\photoRet(t('Your photo could not be uploaded (%err). Try a smaller version or <%a>email it</a> to us?', ray('_err _a', $e->getMessage(), w\emailtag(NULL, t('Here is a photo for my %PROJECT card')))));
  }
  
  return w\photoRet("$base_url/account-photo", 'success');
}

/**
 * Account Selector form
 * appears above the whole Members Section
 */
function formAccounts($form, &$sta) {
  global $base_url, $rUrl, $mya;

  if (!$mya) return []; //['signin' => item('<a href="' . "$base_url/signin" . '">Sign in</a>')];
  $myid = $mya->id; // may be a non-existent account if tests are run while looking at .ZZA, for example
  
  if ($mya->cAdmin2 or $mya->isAdmin2) { // admin: show a search box instead of a list of choices
    $newAcct = textFld(t('Switch to account:'), [t('Type part of name, phone, email, record ID, or qid.')]);
    $submit = submit('Go Account!!');
    $accounts = fieldSet('edit-accounts', compact(ray('newAcct submit')));
    // can't use w\whoFldSubmit here because it fights with some pages (Pay, Charge, etc.)
    js('x/typeahead.bundle.min');
    jsx('new-acct');
    
    return compact('accounts');
  }
  
  if (!$accounts = be\accountChoices()) return [];
//  if (!in_array($myid, @$accounts) and $mya->can()) array_splice($accounts, 1, 0, $myid); // admin or cadmin
  $choices = $ltrs = [];

  foreach ($accounts as $uid => $choice) {
    $a = r\acct($uid);
    if (!$a and isDEV) continue; // ignore this if testing -- "current" account has been undermined is all
    if ($mya->cAdmin2 or $mya->isAdmin2) {
      $qid = $a->mainQid;
      if ($mya->cAdmin2 and !$mya->admin) $qid = substr($qid, -2, 2); // show just last two letters for cAdmins
      if ($a->isCtty) { // show communities in a separate category
//        $class = $a->id < 0 ? 'cgc' : '';
        $choice = r\cttyName($uid);
        $ltrs['_'][$uid] = "<small>$qid</small> $choice";
      } else {
        $pos = substr($choice, 1, 2) == '. ' ? 3 
        : (substr($choice, 1, 1) == ' ' ? 2
        : (mb_strtoupper(substr($choice, 0, 4)) == 'THE ' ? 4 : 0));
        $ltr = mb_strtoupper(substr($choice, $pos, 1));
        $ltrs[$ltr][$uid] = "<small>$qid</small> $choice";
        $choice =  "$qid $choice";
      }
    }
    $choices[$uid] = $choice;
  }

  $links = '';
  foreach ($choices as $k => $v) {
    $one = $k == $mya->id ? $v : r\changeAcctLink($k, $v);
    $links .= "<li>$one</li>\n";
  }
  $accounts = item("<ul>$links</ul>");
  
  return compact('accounts');
}

function formAccounts_validate($form, &$sta) {
  extract(just('newAcct accounts', $sta['input']));
//  t('Type part of name, phone, email, record ID, or qid.')
  if (mb_strpos($newAcct, '@') and !u\validEmail($email = $newAcct)) return say('bad email', compact('email'), 'newAcct');
  if (!$a = w\whois($newAcct, 'newAcct', [], t('You are already in that account.'), FALSE)) return FALSE;
  u\preray(compact('a'), $sta['input']); // ['uid' => $a->id]
}

function formAccounts_submit($form, &$sta) {
  extract(just('a', $sta['input']));
  changeWho('', 'acct=' . $a->id, $sta);
}

/**
 * Change the current account.
 * @param string $zot: gets basename passed from URL
 */
function changeWho($zot, $args = '', &$sta = '') {
  global $mya;
  extract(just('acct page', $args, NULL));

  if (test()) $page = u\urlify(str_replace(',', '/', nn($page)));
  $page = $page ? u\deurlify($page) : 'summary'; // go to summary, unless admin

  if (nn($acct) == 'signout') return signout();
  if (!$mya and !isDEV) return w\go($page, t('No previous account. Try again.')); // get old current account
  $mya = r\acct($acct, $mya->agentId); // tentative new account (agent is the same, so adminishness is the same)
  if (!$mya->cAdmin2 and !$mya->isAdmin2 and !$mya->can()) return hack('accts');
  if ($mya->cAdmin and !$mya->admin and !$mya->isAdmin2 and $mya->community != $mya->agentA->community) return w\go($page, t('You can switch only to accounts in your community.')); // cAdmin check is a bug fix
  r\setAcct($mya, $sta); // set new current account

  return w\go($page);
}

/**
 * Let there be no current user.
 * @param bool $continue: <don't go to Promo site after signout> (can't be a "go there" parameter, because it defaults wrong)
 */
function signout($continue = FALSE) {
  if (isDEV and !test()) db\q('TRUNCATE test');
  if ((string) $continue == 'timedout') list ($continue, $timeout) = [FALSE, '&timedout=1']; else $timeout = '';
  
  $sid = session_id();
  if ($continue) {
    db\update('sessions', ray('sid uid acct session', $sid, 0, 0, serialize('')), 'sid');
  } else {
    db\del('sessions', 'ssid', $sid);
    session_unset();
  }
  global $mya; $mya = NULL;
  r\Acct::_clear(); // empty the acct cache
  if (isDEV and !test() and !$continue) return w\go('signin');
  if (!$continue) return w\go(PROMO_URL . '?region=' . R_SERVER_ID . $timeout);
}

/**
 * Say the site is down
 */
function formSystemDown($form, &$sta) {return array('zot' => item(t('system down')));}

/**
 * Pay or charge a nonmember (continued from Charge page)
 */
function formTxNonmember($form, &$sta, $args = '') {
  if (!$args or !$info = unserialize(u\deurlify($args))) return w\go('', t('Url Error'), 'err');
  extract($info = just('who amount goods purpose', $info));
  if (count($info) != 4) return w\go('', t('Url Error'), 'err');
  
  $title = t('Invoice a Non-member');
  $subtext = t('Sending an invoice to %email (%amount for %purpose)', 'email amount purpose', $who, u\fmtAmt($amount), $purpose);
  $name = textFld(t('Recipient Name:'), [t('Recipient\'s full name')], required());
  $submit = t('Send Invoice');
  $args = hidFld($args);
  return cgform(compact(ray('title subtext name submit args')));
}

function formTxNonmember_submit($form, &$sta) {
  extract(just('name args', $sta['input']));
  $info = unserialize(u\deurlify($args));
  be\invoiceNonmember($name, $info);
}

function formMenu($form, &$sta) {return cgform(['title' => t('Menu')]);}

/**
 * Present an empty slate for status and error messages.
 * @param mixed $form: usually unused (but required by Drupal); if string, output it.
 * @param string $args: say=1 (if called from doSay) or err=1&msg=<msg> (or empty)
 */
function formEmpty($form, &$sta = '', $args = '') {
  if ($form and !is_array($form)) say(plain($form));
  extract(just('say err msg', $args, NULL));
  
  $title = t('Miscellaneous'); // this is suppressed with CSS
  if ($msg) say(plain($msg), $err);
  $subtext = $say ? t('<p>To protect your account, <%span>no sign-in was required</span> for this action. To sign in safely, always go to %CG_DOMAIN.</p><p>You may now close this browser page.</p>', '_span', 'span class="loud"') : NULL;
  return cgform(compact(ray('title subtext')));
}

/**
 * Return a one-use auth code
 */
function authcode() {
  extract($args = just('company cocode', $_GET, NULL));
  if (!$cocode or !u\isQid($company) or !($co = r\acct($company)) or !$co->co or $co->emailCode != $cocode) exit(t('bad company account ID or cocode'));
  exit(u\cryRay(compact(ray('cocode now r', $cocode, now(), u\randomString(1)))));
}

/**
 * Certify that a business is indeed a Common Good participant.
 */
function formCgBiz($form, &$sta, $id) {
  if (!nn($id) or !$a = r\acct(strtoupper($id)) or !$a->ok) return w\go('', t('That is not a %PROJECT Business.'));
  w\go('member/' . $a->name, t('<big><b>%fullName</b> is a %PROJECT Business since %activated.</big> For more information about Common Good, <%a>click here</a>.', 'fullName activated _a', $a->fullName, u\fmtDate($a->activated), atag(r\promoLink(''))));
}

/**
 * Display a list of the member's invitees.
 */
function formInvitedWhom($form, &$sta) {
  global $mya;
  $uid = $mya->cAdmin ? $mya->id : $mya->agentId;
  
  $title = t('Your Invitees');
  $subtext = t('<p>If the status of someone you invited is "<span class="invite-incomplete">Not Complete</span>", they have not finished setting up an account and may need your help.</p><p>If the status is "<span class="invite-expired">expired</span>", you may want to call them and send another invitation.</p>');
  $old = strtotime((R_INVITE_DAYS + 2) . ' days ago');
  $sql = <<<EOF
    SELECT DISTINCT IFNULL(u1.fullName, i.email) AS who, invited, invited<:old AS expired,
      u1.:IS_MEMBER AS member, u1.:IS_OK AS ok, u1.:IS_CO AS co
    FROM r_invites i LEFT JOIN users u1 ON u1.uid=i.invitee
    WHERE i.inviter=:uid ORDER BY invited DESC
EOF;
  $q = db\q($sql, compact('uid', 'old')); // IFNULL fails for u1.flags (not sure why -- it comes out 0)

  $list = "<table id='invitelist'>\n<tr><th>Invited</th><th>Whom</th><th>Status</th></tr>\n";
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $invited = u\fmtDate($invited);
    if (!$started = mb_strpos($who, ' ') and u\starts($who, CRYPT_FLAG . 'P')) $who = u\decry('P', $who); // decrypt email
    $status = ($member or $ok) ? t('is a Member!') : ($started ? t('account Not Complete') : ($expired ? t('expired') : '(no response yet)'));
    $color = $member ? 'member' : ($started ? 'incomplete' : ($expired ? 'expired' : 'other'));
    $nameClass = $co ? 'name co' : 'name';
    $list .= "<tr><td>$invited</td><td>$who</td><td class=\"invite-$color\">$status</td></tr>\n";
  }
  $list .= "</table>\n";
  
  $list = item($list);
  return cgform(compact(ray('title subtext list')));
}

define('R_BASIC_FLDS', 'fullName federalId dob');

/**
 * Validate the data that affects social security number verification.
 * called from signup validation and ssn validation, as well as from formBasic()
 * @param string/bool $co: is this a company account ('mya' means use the current account's type)
 *   'mya' also means we are retrying basic information (the AccountInfo step) during signup
 */
function formBasic_validate($form, &$sta, $co = 'mya', $fields = R_BASIC_FLDS) {
  extract(just($fields, $sta['input']));
  if ((string) $co == 'mya') {
    $mya = w\eLinkAcct($sta);
    $co = $mya->co;
  }
  if (isset($fullName) and $err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');

  $what = $co ? t('federal ID') : t('social security number');
  if (isset($federalId) and $err = u\badSsn($federalId, nn($dob))) return say($err, compact('what'), 'federalId');
  if (isset($dob) and $err = u\badDob($dob, $co)) return say($err, 'dob');

  foreach (ray($fields) as $k) if (isset($$k)) $sta['input'][$k] = $$k;
  return TRUE;
}

/**
 * Show Common Good's accountant (or any employee) our Deposits page, without the buttons
 */
function formShowDeposits($form, &$sta) {
  global $mya;
  if (!$mya or $mya->id != CGID) return w\softErr(t('You are not authorized to view that page.'));
  include_once __DIR__ . '/../admin/admin-forms.inc';
  return w\formDeposits($form, $sta);
}

/**
 * Show a page when someone scans a Common Good card not with the CGPay app, for example with QRDroid.
 * RC4.ME and RC2.ME must do some interpretation and redirection (NYI on those sites)
 */
function formI($form, &$sta, $who = '') {
  $a = r\acct($who);
  if ($a and $a->co) return w\go("member/$a->name");
  w\go(r\promoLink('')); return NULL;
//  return w\go(($a and $a->co) ? "member/$a->name" : r\promoLink(''));
}

/**
 * Show raw data for charting.
 * This data is typical requested by cg4.us/charts.php for display in an iframe on the community/graphs page (etc)
 * @param string $args: URL query string with
 *   ctty    community to provide data for
 * @return: does not return. Prints the data (json-encoded) and exits.
 */
function chartData($args = '') {
  extract(just('ctty', $args));
  
  $data = showAllCharts(nn($ctty, 0));
/**/  if (!nn($data)) die(t('No such chart data.')); // die not u\FAIL or u\EXPECT
/**/  echo $data;
  exit();
}

function showAllCharts($ctty = 0) {
  foreach (ray('days weeks months quarters years') as $period) $res[$period[0]] = showCharts(nn($ctty, 0), $period);
  return json_encode($res);
}

function formFooter() {
  global $base_url, $rUrl;
  $year = date('Y');

  global $mya;
  $extraLinks = ($mya and $mya->can(B_MANAGE)) 
  ? t('|<%aDonate>Donate</a> | <%aInvite>Invite Someone</a> | ',
    '_aDonate _aInvite', atag('/community/donate'), atag('/community/invite'))
  : ($mya ? '' : t('|<%aSignin>Sign in</a> |', '_aSignin', atag('/signin')));

  $helpType = $mya ? '' : 'other';
  
  $markup = "<hr><div id=\"footer-menu\">$extraLinks" . t(<<<EOF
|   <%aSite>About %PROJECT</a> |
    <%aAgree>The Agreement</a> |
    <%aSecurity>Privacy & Security</a> |
    <%aHelp>Help</a>
<br>&nbsp;
</div>
copyright &copy; %year <%aSite>%PROJECT<sup> &reg;</sup></a><span id="footer-details">, a nonprofit organization<br/>
%CGF_POSTALADDR<br/>
%CGF_PHONE &nbsp; <%aEmail>%CGF_EMAIL</a><br>
<br>
IP: %ip</span>
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>&nbsp;
EOF
  , '_aSite _aAgree _aSecurity _aHelp year _aEmail ip', atag(r\promoLink('about-us')), atag('/community/agreement'), atag(r\promoLink('about-us/privacy-and-security')), atag("/help/$helpType"), $year, u\emailTag(CGF_EMAIL), $_SERVER['REMOTE_ADDR']);
//    <%aCttys>Communities</a> |

  $footerText = item($markup); // unique name for this page
//  $sid = db\get('sid', 'sessions', 'uid=:uid', ['uid' => $mya ? $mya->agentA->id : 0]);
  
  js('parse-query');
  js('misc', 'baseUrl sid life isSafari', BASE_URL, session_id(), ($mya and !isDEV) ? SESSION_LIFE : 0, isSAFARI);
  return compact('footerText');
}

/**
 * Display of participating companies (for promo site) and exit.
 */
function whosin($ctty = '') {
  $ctty = '!' . $ctty; // promo site omits the leading '!' from the community's QID
  if (!$uid = r\qo($ctty)->id or !$list = r\companies($uid, FALSE)) exit();
/**/  echo $list;
  exit();
}

