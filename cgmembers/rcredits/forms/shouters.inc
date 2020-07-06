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

  extract(just('raw', $args, NULL));
  if ($raw) return raw();

  jsx('shouters');
  $title = t('Endorsements');
  $ask = item(tr('%SHOUT_TEXT'));
  $footnote = '<b class="footnote">*</b>';
  $signed = item(t('Signed, ' . $footnote));
  $posttext = $footnote . ORG_DISCLAIMER;

  $sql = <<< X
    SELECT uid,rating,fullName,city,abbreviation AS st,quote,title AS orgTitle,org,website,community=:ctty AS mine
    FROM u_shouters h 
    LEFT JOIN users u USING (uid) 
    LEFT JOIN r_states s ON s.id=u.state AND s.country_id=u.country
    ORDER BY IFNULL(org, '')='', IFNULL(quote, '')='', rating DESC, fullName
X;
  $q = db\q($sql, ray('ctty', $mya ? $mya->community : 0));

  $dpyFlds = ray('fullName orgTitle org quote');
  $list = '';
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($uid);
    
    if ($mya and $mya->admin) {  // rating: 0=unrated 1=poor 2=normal 3=great
      $rating = <<< X
        <div class="rating" qid="$a->mainQid" rating="$rating">*** </div>
X;
      $quote = $rating . w\lnk("/change-account/acct=$a->mainQid&page=" . u\urlify('/community/invite/edit=1'), $quote); // editable for admins
    } elseif (!$rating) continue;

    $fullName .= ', ';

    if ($org) {
      if ($orgTitle and $org) $orgTitle .= ', ';
      if ($org and $website) $org = lnk('http://' . $website, $org);
    } else { list ($orgTitle, $org) = ["$city, ", $st]; }
      
    $list .= u\ray2row(compact($dpyFlds));
  }
  
  return cgform(compact(ray('title ask signed list posttext')));
}

function raw() {
  $sql = <<< X
    SELECT uid,fullName,city,abbreviation AS st,quote
    FROM u_shouters h 
    LEFT JOIN users u USING (uid) 
    LEFT JOIN r_states s ON s.id=u.state AND s.country_id=u.country
    WHERE u.:IS_OK AND IFNULL(quote, '')<>'' AND rating>0
    ORDER BY rating DESC, RAND()
X;
  $q = db\q($sql);
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
    list ($first, $last, $middles) = u\parseName($fullName, TRUE);
    $nm = $first . ' ' . substr($last, 0, 1) . '.';
    $res[] = [$quote, "$nm ($city, $st)"];
  }
/**/  echo json_encode(nn($res));
  exit();
}
