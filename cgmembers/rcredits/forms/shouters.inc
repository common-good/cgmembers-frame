<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * List people who have agreed to give us a public shout out.
 */
function formShouters($form, &$sta, $args = '') {
  global $mya;

//  $title = t('Who Signed the Invitation');
  $ask = item(tr('<h3>%SHOUT_TEXT</h3>'));
  $footnote = '<b class="footnote">*</b>';
  $signed = item(t('Signed, ' . $footnote));
  $posttext = $footnote . ORG_DISCLAIMER;

  $sql = <<< X
    SELECT fullName,city,abbreviation AS st,quote,title,org,website,community=:ctty AS mine
    FROM u_shouters h 
    LEFT JOIN users u USING (uid) 
    LEFT JOIN r_states s ON s.id=u.state AND s.country_id=u.country
    ORDER BY org IS NULL, fullName
X;
  $q = db\q($sql, ray('ctty', $mya->community));
  
  $dpyFlds = ray('fullName title org quote');
  while ($row = $q->fetchAssoc()) {
    extract($row);

    $fullName .= ', ';

    if ($org) {
      if ($title and $org) $title .= ', ';
      if ($org and $website) $org = lnk('http://' . $website, $org);
    } else { list ($title, $org) = ["$city, ", $st]; }
    $list[] = u\ray2row(compact($dpyFlds));
  }
  
  $list = join('', nn($list, []));
  return cgform(compact(ray('ask signed list posttext')));
}
