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
require_once R_ROOT . '/cg-backend.inc';
//include_once __DIR__ . '/rweb-txs.inc';

define('UPLOAD_DIR', dirname($_SERVER['DOCUMENT_ROOT']) . '/public_ftp/incoming');

/**
 * Show a form called from a form linked to in an email or during signup, without signing in.
 * For example, /do/doEvents~<code> (in MailChimp: /do/doEvents~*|CODE|*)
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
  $cttys = r\cttyList();
//  $cttys[1] = trim(str_replace(PROJECT, '', r\regionField('fullName'))); // should go just before the seedpack (which is last)
  return exitJust(u\jsonize($cttys));
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
  return exitJust(tr($s, '_personalNote signed fullName phone CODE', $note, fmtDt($mya->signed), $mya->fullName, u\fmtPhone($mya->phone), ''));
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
  include_once R_ROOT . '/cg-risk.inc';
  global $showRiskCalc; 
  
  $showRiskCalc = '';

  if ($type == 'acct') {
    k\acctRisk1($uid = $id, getv('riskThresholds', [0,0,0,0]), $risk, $calc);
    $w = ''; $m = K_RED;
  } elseif ($type == 'tx') {
    k\txRisk1($xid = $id, $risk, $calc);
    $w = u\fmtAmt(db\get('amt', 'txs', compact('xid')));
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

  if (!$mya) return [];
  $myid = $mya->id; // may be a non-existent account if tests are run while looking at .ZZA, for example
  
  if ($mya->admSeeAccts) { // admin: show a search box instead of a list of choices
    $newAcct = textFld(t('Switch to account:'), [t('Type part of name, phone, email, record ID, or qid.')]);
    $submit = submit('Go Account!!');
    $accounts = fieldSet('edit-accounts', compact(ray('newAcct submit')), BARE);
    // can't use w\whoFldSubmit here because it fights with some pages (Pay, Charge, etc.)
    js('x/typeahead.bundle.min');
    jsx('new-acct');
    return compact('accounts');
  }
  
  if (!$accounts = be\accountChoices(r\acct())) return [];
  $choices = [];

  foreach ($accounts as $uid => $choice) {
    $a = r\acct($uid);
    if (!$a and isDEV) continue; // ignore missing account if testing -- "current" account has been undermined is all
    if ($mya->admSeeAccts) {
      $qid = $a->mainQid;
      if (!$mya->admRegion) $qid = substr($qid, -2, 2); // show just last two letters for cAdmins
      if ($a->isCtty) { // show communities in a separate category
//        $class = $a->id < 0 ? 'cgc' : '';
        $choice = r\cttyName($uid);
      } else {
        $pos = substr($choice, 1, 2) == '. ' ? 3 
        : (substr($choice, 1, 1) == ' ' ? 2
        : (mb_strtoupper(substr($choice, 0, 4)) == 'THE ' ? 4 : 0));
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

function formAccounts_submit($form, &$sta) {
  $uid = whoId($sta);
  return go("change-account/acct=$uid");
}

/**
 * Return the given account's logo in an HTTP stream (called from /account-logo).
 */
function logo($qid = '') {
  if (!$qid or !$a = r\acct($qid) or !$a->hasLogo or !$content = \file_get_contents(LOGO_DIR . '/' . $a->mainQid)) return '';
  header("Content-type: image/jpeg");
  return exitJust($content);
}

/**
 * Change the current account.
 * @param string $zot: gets basename passed from URL
 */
function changeWho($zot, $args = '') {
  global $mya;
  extract(just('acct page then', $args, NULL));

  if (test()) $page = u\urlify(str_replace(',', '/', nn($page)));
  $page = $page ? u\deurlify($page) : 'dashboard'; // go to dashboard, unless admin

  if (nn($acct) == 'signout') return signout();
  if (!$mya and !isDEV) return softErr(t('Your session timed out. Please <%a>Sign back in</a> and try again.', '_a', atag('/signin')));
  if (!$mya = r\acct($acct, $agt = $mya ? $mya->agentId : NULL)) return hack('change to nonexistent', ray('args mya agt', pr($args), pr($mya), pr($agt))); // tentative new account (agent is the same, so adminishness is the same)

  if (!$mya->can()) {
    if (!$mya->admin) return hack('change to illegal', ray('args mya', pr($args), pr($mya)));
    if ($mya->admin and !$mya->admRegion and $mya->community != $mya->agentA->community) return w\go($page, t('You can switch only to accounts in your community.'));
  }

  r\setAcct($mya); // set new current account

  if ($then) return then($then);

  return go($page);
}

/**
 * Let there be no current user.
 * @param bool/string $continue: <don't go to Promo site after signout> (can't be a "go there" parameter, because it defaults wrong)
 */
function signout($continue = FALSE) {
  global $mya;
  
  $from = basename(nni($_SERVER, 'HTTP_REFERER'));
  r\deleteCook('qboRefresh');
  r\deleteCook('vKeyPw'); // no longer admV, if we ever were

//  if ($from == 'app' and $continue === 'timedout' and $a = getScannerA() and !$a->co) return go('app'); // timeout for members using app just resets to app home
  
  if (isDEV and !test()) db\q('TRUNCATE test');
  if ((string) $continue === 'timedout') list ($continue, $timeout) = [FALSE, '&timedout=1']; else $timeout = '';
  
  $sid = session_id();
  if ($continue) {
    db\update('sessions', ray('sid uid acct session', $sid, 0, 0, serialize('')), 'sid');
  } else {
    db\del('sessions', 'ssid', $sid);
    session_unset();
  }
  
  if ($mya and !$mya->admin and !$continue) r\setCook('trust', NULL);
  r\setAcct(NULL); // but $mya still points to the signed-out account
  r\Acct::_clear(); // empty the acct cache
  
  if ($from == 'app' and $mya and r\cook('scanner') == $mya->mainQid) return w\go('app'); // if signing out from app, stay there
  if (isDEV and !test() and !$continue) return w\go('signin', $timeout ? tr('You were automatically signed out, to protect your account.') : '');
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
  if (!$args or !$info = unserialize(u\deurlify($args))) return w\softErr(t('Url Error'));
  extract($info = just('who amount goods purpose', $info));
  if (count($info) != 4) return w\softErr(t('Url Error'));
  
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
 * Return a one-use auth code
 */
function authcode($args = '', $exit = TRUE) {
  extract($args = just('company cocode', $args ?: $_GET, NULL));
  if (!$cocode or !u\isQid($company) or !($co = r\acct($company)) or !$co->co or $co->emailCode != $cocode) return exitJust(t('bad company account ID or cocode'));
  $ray = ray('cocode now r', $cocode, now(), u\randomString(1));
  if ($exit) exitRay($ray, TRUE); else return u\cryRay($ray);
}

/**
 * Certify that a business is indeed a Common Good participant.
 */
function formCgBiz($form, &$sta, $id) {
  if (!nn($id) or !$a = r\acct(strtoupper($id)) or !$a->ok) return w\softErr(t('That is not a %PROJECT Business.'));
  w\go('member/' . $a->name, t('<big><b>%fullName</b> is a %PROJECT Business since %activated.</big> For more information about %PROJECT, <%a>click here</a>.', 'fullName activated _a', $a->fullName, fmtDt($a->activated), atag(r\promoLink(''))));
}

/**
 * Display a list of the member's invitees.
 */
function formInvitedWhom($form, &$sta) {
  global $mya;
  $uid = $mya->admin ? $mya->id : $mya->agentId;
  
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
    $invited = fmtDt($invited);
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
 */
function formBasic_validate($form, &$sta, $fields = R_BASIC_FLDS) {
  extract(just($fields, $sta['input']));

  if (isset($fullName) and $err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');
  if (isset($federalId) and $err = u\badEin($federalId, FALSE)) return say($err, 'federalId');
  if (isset($dob) and $err = u\badDob($dob, FALSE)) return say($err, 'dob');

  foreach (ray($fields) as $k) if (isset($$k)) $sta['input'][$k] = $$k;
  return TRUE;
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
  return exitJust($data);
}

function showAllCharts($ctty = 0) {
  foreach (ray('days weeks months quarters years') as $period) $res[$period[0]] = showCharts(nn($ctty, 0), $period);
  return u\jsonize($res);
}

function formFooter() {
  global $mya, $base_url, $rUrl;

  js('parse-query');
  js('misc', 'baseUrl sid life isSafari', BASE_URL, session_id(), (isDEV or !$mya) ? 0 : SESSION_LIFE, isSAFARI);

  if (in(PAGE, 'app card')) return [];
  
  $year = date('Y');
  $helpType = $mya ? '' : 'other';
  $helpModal = modal('help-modal', 'Help', '', '', 'X'); // 'Help' gets replaced, but makes the HTML correct

  if (!$mya) {
    $links[] = lnk('/signin', t('Sign in'));
  } elseif ($mya->can(B_MANAGE)) {
    $links[] = lnk('/community/donate', t('Donate'));
    $links[] = lnk('/community/invite', t('Invite Someone'));
  }

  $links[] = lnk(r\promoLink('about-us'), t('About Us')) . (u\isMobile() ? '<br>' : '');
  $links[] = lnk('/community/agreement', t('The Agreement'));
  $links[] = lnk(r\promoLink('about-us/privacy-and-security'), t('Security'));
  $links[] = lnk("/help/$helpType", t('Help'));
  $links = join(' | ', $links);
  list ($copyright, $nonprofit) = ray(t('copyright, a nonprofit organization'));
  
  $markup = tr(<<< X
|<hr><div id="footer-menu">
$links
<br>&nbsp;
</div>
<span class="copyright">$copyright &copy; $year </span><%aSite>%PROJECT<sup> &reg;</sup></a><span class="no-print">, $nonprofit</span><br>
%CGF_POSTALADDR<br>
%CGF_PHONE (messages) &nbsp; <%aEmail>%CGF_EMAIL</a><br>
<div class="no-print"><br>IP: %ip
$helpModal</div>
X
  , '_aSite _aEmail ip', atag(r\promoLink()), u\emailTag(CGF_EMAIL), $_SERVER['REMOTE_ADDR']);
  
//    <%aCttys>Communities</a> |

  $footerText = item($markup); // unique name for this page
//  $sid = db\get('sid', 'sessions', 'uid=:uid', ['uid' => $mya ? $mya->agentA->id : 0]);
  
  return compact('footerText');
}

/**
 * Display of participating companies (for promo site) and exit.
 */
function whosin($ctty = '') {
  $ctty = '!' . $ctty; // promo site omits the leading '!' from the community's QID
  if (!$uid = r\qo($ctty)->id or !$list = r\companies($uid, FALSE)) exit(); // empty list
  return exitJust($list);
}
