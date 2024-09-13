<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

include_once R_ROOT . '/admin/admin.inc';

/**
 * Show a list of members in the current account's community
 */
function formMemberList($form, &$sta) {
  global $base_url;
  global $mya;

  $cttyA = $mya->cttyA;

  $title = ((!$mya->proSe and $cttyA) ? $cttyA->fullName : '') . t(' Member List');
  $sql = 'SELECT uid, login, access, fullName, notes, :IS_CO AS co FROM users WHERE uid>:UID_CANON9 AND community=:cttyId AND NOT :IS_NONUDGE ORDER BY :IS_OK, IF(:IS_OK, fullName, 0-login)'; // -login fails because it gets treated as a string
  $q = db\q($sql, ray('cttyId', $cttyA->id));
  $list = "<table id='memberlist'>\n";
  $emails = '';
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $nameClass = $co ? 'name co' : 'name';
    $notes = str_replace("\n", "<br>\n", nn($notes));
    $notes = str_replace('do:', '<b class="memberlist-notes">do:</b>', $notes);
    $a = r\acct($uid);
    $phone = str_replace('+1 ', '', u\fmtPhone($phone0 = $a->phone));
    $postalAddr = $a->postalAddr;
    $email = $a->email ?: t('BAD EMAIL');
    $fullName = "<a href=\"$base_url/change-account/acct=$a->mainQid\">$fullName</a>";
    $steps = a\showSteps($a);
    $date = fmtDt($a->ok ? $access : $login);

    $bits = []; foreach (ray('member ok hasBank') as $k) if ($a->$k) $bits[] = $k;
    $bits = $bits ? ('<span class="memberlist-bits"> ' . join(' ', $bits) . "</span><br>\n") : '';
    
    $one = "<tr><td colspan=4 class=\"memberlist-line2\">$date &mdash; $steps$bits$notes</td></tr>\n";
    $list .= <<<EOF
      <tr>
      <td class="$nameClass">$fullName</td>
      <td><a href="mailto:$email" target="_blank">$email</a></td>
      <td>$phone</td>
      <td>$postalAddr</td>
      </tr>
      $one
EOF;
  }
  $list .= "</table>\n";
  
  return cgform(compact(ray('title list')), 'wide nolabs');
}
