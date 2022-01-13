<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

/**
 * Download the member list for the current community
 */

function formDownloadMemberList($form, &$sta, $args = '') {
  global $base_url;

  extract($params = just(EXPORT_PARAMS, $args, NULL));
  if ($do == 'export') return export($params);

  if ($do == 'done') {
    $do = 'export';
    $params = http_build_query(compact(ray(EXPORT_PARAMS)));
///    return w\go("$base_url/sadmin/export-list/$params"); // debug(
    $download = item("<iframe src=\"$base_url/sadmin/export-list/$params\" class=\"hidden\"></iframe>");
  } else {$download = NULL;}
  
  global $mya;
  $cttyA = $mya->cttyA;

  $title = t('Download Member List');
  $expMo = r\rdoMonth();
  $subtext = t(<<<EOF
    <p>Generate a CSV format spreadsheet for accounts in your community (%cttyName). We recommend you use a service like MailChimp or MailerLite and import this list there before each mailing. Company accounts and members are marked, so there is generally no need to export them separately &mdash; you can do that separation within the mailing program (type contains "member" or "company" or "member company").</p>
    <p>Action links for individual accounts take the form %code. For example, a link from MailChimp to the democracy page would be:</p>
    <blockquote>$base_url/%eg1 (MailerLite) OR<br>
    $base_url/%eg2 (MailChimp)</blockquote>
    <p>Other action choices might be listed here someday. :)</p>
EOF
    , 'cttyName code eg1 eg2', $cttyA->fullName, '<site>/do/<op>~<code>', 'do/doEvents~{$code}', 'do/doEvents~*|code|*');
  
$chimp = boolFld(t('Export to email list?'), '', TRUE);
  
$allCttys = $mya->admRegion ? boxFld('allCttys', t('All communities:'), '', TRUE) : hidFld(FALSE); // temporary for CGF oversight
$status = w\radiosFld(t('Status:'), '', dft(2), ray('active not-yet-active both'));
$types = w\radiosfld(t('Types:'), '', dft(2), ray('individuals companies both'));
$anonymous = boxFld('anonymous', t('Anonymous:'), t('Export only non-identifying information (for data-mapping and analysis)'), FALSE);
$chimpSet = w\fieldSet('fields', compact(ray('allCttys status types anonymous')));
  
$submit = t('Download');
  
w\jsx('chimp');
return cgform(compact(ray('title subtext chimp chimpSet submit download')));
}

function formDownloadMemberList_submit($form, &$sta) {
  extract(just(EXPORT_PARAMS, $sta['input']));
  $do = 'done';
  u\setDft($anonymous, FALSE); // for a checkbox, unchecked results in not set
  return w\go('sadmin/export-list/' . http_build_query(compact(ray(EXPORT_PARAMS))), t('Download initiated.'));
}

/**
 * Download the member list for the current community
 */
define('EXPORT_PARAMS', 'do allCttys status types chimp anonymous');

function export($params) {
  extract(just(EXPORT_PARAMS, $params, NULL));
  global $mya;
  $cttyA = r\acct($mya->community);
  $solarUid = db\get('uid', 'users', "name='cooppower'");

  $filename = ($allCttys ? PROJECT : str_replace('.', '', $cttyA->name))
  . ($status == 0 ? t('-active') : ($status == 1 ? t('-notyet') : ''))
  . ($types == 0 ? t('-members') : ($types == 1 ? t('-orgs') : t('-accounts')))
  . '.csv';
  u\csvStart($filename);
  $delimiter = $chimp ? "\t" : ',';
  
  $fields = $anonymous ?
    'uid city state zip type'
      . ' created activated signed helper iCode balance rewards savingsAdd minimum share risk trust'
      . ' community co gross employees achMin pinThreshold lastiCode tenure moves mediaConx jid'
      . ' owns bankName slave closed notices secret'
  : ($chimp ? 'id name phone email city state zip community type code'
  : 'id fullName manages lastTx solar snap phone email address city state zip community type code modified created');
  u\csv($fields = ray($fields), $delimiter);

  // add actual accounts
  $where = $allCttys ? '1' : 'community=:cttyId';
  if ($status == 0) $where .= ' AND :IS_OK';
  if ($status == 1) $where .= ' AND NOT :IS_OK';

  if ($types == 0) $where .= ' AND NOT :IS_CO';
  if ($types == 1) $where .= ' AND :IS_CO';

  $sql = "SELECT uid AS id, fullName FROM users WHERE uid>:UID_CANON9 AND (NOT activated OR :IS_OK) AND $where ORDER BY :IS_OK, -signed, -created";
  
  $q = db\q($sql, ray('cttyId', $cttyA->id));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($id);
    $type = $a->ok ? 'member' : ($a->activated ? 'deactivated' : 'notyet');
    if ($a->co) $type .= ' company';
    
    if ($anonymous) {
      foreach ($fields as $k) $$k = $a->$k; // do this first. some will be null.
      $zip = $a->zip;
      foreach (ray('weekly monthly secret') as $k) $$k = $a->can(u\consta('b', $k));
      $notices = $monthly ? t('monthly') : ($weekly ? t('weekly') : t('daily'));
      $state = r\realstate($state);
      if ($routing = substr($a->bankAccount, 4, 9)) {
        $bankName = db\get('name', 'r_banks', 'route=:routing', compact('routing'));
      } else $bankName = '';
    } else {
      list ($firstname, $lastname, $mid) = $a->co ? [$a->fullName, '', ''] : u\parseName($a->fullName, TRUE);
      if (strlen($firstname) < 2 and strlen($mid) > 2) $firstname = $mid;
      $name = $firstname ?: $a->fullName; // Company name or person's first name
      $email = $a->email;
      $phone = $a->phone;
      list ($address, $city, $state, $zip) = $a->postalAddr
      ? u\parseAddr($a->postalAddr)
      : [$a->address, $a->city, $a->st, $a->zip];
      $phone = u\fmtPhone($phone);
      $id = $a->mainQid;
      $code = $a->doCode(); 
      $asof = $modified = u\fmtDate(now());
      $manages = db\get('fullName', 'u_relations r LEFT JOIN users u ON u.uid=r.main', 'r.permission>=:B_MANAGE-:B_RELATED AND u.:IS_CO AND r.other=:id', ['id' => $a->id]);
      $tx = $a->lastTx;
      $lastTx = empty($tx) ? '' : u\fmtDate($tx->created, TRUE);
      $solar = $solarUid ? r\relation(t("'solar'"), $solarUid, $a->id, ':IS_CUSTOMER') : '';
      $snap = $a->snap ? t('SNAP') : '';
      $special = '';
// (BROKEN)      $special = $a->special;
    }
    $community = r\cttyName($a->community, TRUE);
    $created = u\fmtDate($a->created);
//    if (is_numeric($zip) and !@$chimp) $zip .= " "; // prevent interpretation as a number (fails for MailChimp, so don't)
//    if (!@$chimp) foreach ($fields as $k) $$k = '"' . $$k . '"'; // keeps some imports from getting confused
    u\csv(array_values(compact($fields)), $delimiter);
  }
  
  exit();
}
