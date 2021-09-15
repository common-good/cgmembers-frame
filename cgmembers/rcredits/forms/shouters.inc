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
    SELECT uid,rating,fullName,city,abbreviation AS st,quote,title AS orgTitle,org,website,community=:community AS mine
    FROM u_shout h 
    LEFT JOIN users u USING (uid) 
    LEFT JOIN r_states s ON s.id=u.state AND s.country_id=u.country
    WHERE IFNULL(quote, '')='' OR rating>0 OR uid=:id OR :admin
    ORDER BY IFNULL(org, '')='', IFNULL(quote, '')='', rating DESC, fullName
X;
  $subs = []; foreach (ray('community id admin') as $k) $subs[$k] = $mya ? $mya->$k : 0;
  $q = db\q($sql, $subs);

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

    list ($org, $orgTitle) = getAffiliation($org, $orgTitle, $website, $city, $st);
    $list .= u\ray2row(compact($dpyFlds));
  }
  
  return cgform(compact(ray('title ask signed list posttext')));
}

function raw() {
  $sql = <<< X
    SELECT uid,fullName,city,abbreviation AS st,org,title AS orgTitle,website,quote
    FROM u_shout h 
    LEFT JOIN users u USING (uid) 
    LEFT JOIN r_states s ON s.id=u.state AND s.country_id=u.country
    WHERE u.:IS_OK AND IFNULL(quote, '')<>'' AND rating>0
    ORDER BY rating DESC, RAND()
X;
  $q = db\q($sql);
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($org) {
      $nm = $fullName;
    } else {
      list ($first, $last, $middles) = u\parseName($fullName, TRUE);
      $nm = $first . ' ' . substr($last, 0, 1) . '.';
    }
    list ($org, $orgTitle) = getAffiliation($org, $orgTitle, $website, $city, $st);
    $res[] = [$quote, "$nm ($orgTitle$org)"];
  }
/**/  echo u\jsonize(nn($res));
  exit();
}

/**
 * Return the shouter's context
 */
function getAffiliation($org, $orgTitle, $website, $city, $st) {
  if ($org) {
    if ($orgTitle and $org) $orgTitle .= ', ';
    if ($org and $website) $org = lnk('http://' . $website, $org);
    return [$org, $orgTitle];
  } else { return [$st, "$city, "]; }
}
