<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Show a list of participating businesses in a given region.
 * @param string $args: optional parameters from URL:
 *    where   community qid or start of zip pattern
 *    cos     part of company name or industry category
 */
function formFindCo($form, &$sta, $args = '') {
  global $mya;
  extract(just('where cos', $args));

  $list = w\directoryList(nn($cos), nn($where));
  $list = "<div class=\"cmpy-list\">$list</div>";
  
  $title = t('Companies in Your Region');
  $which = textFld(t('Search for:'), [t('Whatever'), t('Type part of company name or industry category')], dft(@$cos));
  $region = textFld(t('Where:'), [t('Start of postal code'), t('Type the first few characters of your postal code<br>For example, use 013 for Franklin County, Massachusetts; 05 for Vermont, etc.')], dft(@$where)); // or country name
  $submit = t('Find');

  return cgform(compact(ray('title which region submit list')));
}

function formFindCo_validate($form, &$sta) {
}

function formFindCo_submit($form, &$sta) {
  extract($sta['input']);
  w\go("community/find-company/where=$region&cos=$which");
//  svar('which_companies', trim($which));
//  svar('region', trim($region));
}

/**
 * Return a formatted, categorized list of businesses
 * @param string $which: word search for business name or industry
 * @param mixed $region: community (the default), zip (maybe partial), state abbrev, or country (maybe partial)
 */
function directoryList($which, $region) {
  global $mya;

  $stateCrit = '(s.abbreviation=:region OR c.name=:region)';
  
  list ($region, $crit) =
    !$region ? [($mya and $mya->id != 1) ? $mya->community : r\serverUid(), 'u.uid=:region'] // blank: show ctty or region
  : (preg_match('/[A-Z]{3}/', $region) ? [$region . '.', 'u.name=:region'] // eg NEW or MIW
  : (preg_match('/[0-9]{1,5}/', $region) ? [$region . '%', 'u.zip LIKE :region'] // eg 013 or 01002
  : [$region, $stateCrit] // eg MA or Germany
  ));

  $table = 'users u LEFT JOIN r_states s ON s.id=u.state LEFT JOIN r_countries c ON c.id=u.country';
  $res = db\get('latitude AS lat,longitude AS lon', $table, $crit, compact('region'));
  if (!$res) return participatingCompanies(''); // nothing in that area
  
  extract($res); // get lat,lon
  $regionCrit = ':DISTANCE_BETWEEN<:NEIGHBOR_MAX OR ' . $stateCrit;

  $which = u\ignoreSuffix($which, 'ants ant ian es ers ing er or ion s');
  $which = str_replace(' ', '%', \db_like(" $which ")); // allow abbreviations of each word

  $sql = <<<EOF
    SELECT DISTINCT u.uid, i.iid, parent.iid as selIid, IFNULL(parent.industry, i.industry) as selIndustry, abbreviation AS st
    FROM users u 
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN r_industries i ON i.iid=ui.iid
    LEFT JOIN r_industries parent on i.parent = parent.iid
    LEFT JOIN r_states s ON s.id=u.state
    LEFT JOIN r_countries c ON c.id=u.country
    WHERE :IS_CO AND u.uid>:CANONIC_ACCTS AND :IS_OK AND ($regionCrit) AND (u.fullName LIKE :which OR i.industry LIKE :which)
    ORDER BY selIndustry, u.fullName
EOF;

  $result = db\q($sql, compact(ray('region which lat lon')));
  $rows = [];
  foreach ($result as $row) {
    if (!nn($xrow) or $xrow->uid != $row->uid) $rows[] = $row; // eliminate dups within class (DISTINCT fails)
    $xrow = $row;
  }
  return participatingCompanies((array) $rows);
}

/**
 * Return a list of participating companies for the given community.
 * @param assoc $rows: information to display (either on members site or on promo site)
 */
function participatingCompanies($rows) {
  $model = <<< X
<div class="row">
  <div class="cmpy-img"><img src="%img" alt="profile picture" /></div>
  <div class="cmpy-name-does">
    <div class="cmpy-name">%name</div>
    <div class="cmpy-does">%desc</div>
  </div>
  <div class="cmpy-contact">%contact</div>
</div>
X;
  $iModel = '<div class="cmpy-ind-wrapper"><div class="cmpy-ind">%industry</div>';

  if (!$rows) return t('No companies there yet. Invite them!');
  $res = '';
  $_newClass = 'class="company-listing"';
  $NEW = t(' <small %newClass> NEW!</small>', compact('_newClass'));

  $lastSelIid = -1;  // negative record ids are, we hope, unlikely
  $firstTime = TRUE;
  foreach ($rows as $rowObject) {
    $row = (array)$rowObject;
    $uid = $row['uid'];
    $a = r\acct($uid);
    if (!($a->coCan(CO_PRIVATE))) {
      $industry = $row['selIndustry'] ?: tr('Unclassified');
      $selIid = $row['selIid'] ?: -2;
      if ($selIid != $lastSelIid) {
        if (! $firstTime) {
          $res .= '</div>';
        }
        $firstTime = false;
        $res .= tr($iModel, 'industry', $industry) . "\n";
        $lastSelIid = $selIid;
      }
      $website = $a->website ? "http://$a->website" : "/member/$a->name";
      $name = w\lnk($website, $a->bestName);
      $contact = "$a->city, $a->st<br>$a->phoneDash";
      $new = ($a->activated > REQUEST_TIME - 9 * WEEK_SECS) ? $NEW : '';
      $img = $a->photoSrc(TRUE) ?: 'images/no-photo-available.jpg';
      $res .= tr($model, '_name desc _contact img', "$name$new", $a->shortDesc, $contact, $img) . "\n";
    }
  }
  if (! $firstTime) $res .= '</div>';
  return $res;
}