<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const SIMPLE_CO_LIST = '%simple-list%';
const NO_STORE_CREDIT = 'INAACQ MIWAAY'; // who opted out of offering store credit (temporary)

/**
 * Return a formatted list of member companies
 * Call:
 *   coList(which, region) --  categorized
 *   coList()              --  no categories, each company appears just once, in alphabetical order
 * @param string $which: word search for business name or industry
 * @param mixed $region: community (the default), zip (maybe partial), state abbrev, or country (maybe partial)
 */
function coList($which = SIMPLE_CO_LIST, $region = '', $nearby = TRUE, &$place = '') {
  global $mya;

  list ($lat, $lon) = $mya ? [$mya->latitude + 0, $mya->longitude + 0] : [0, 0];
  if (!$region and $mya and !$lat and !$lon) list ($region, $nearby) = [$mya->zip, TRUE];
  $isZip = (is_numeric($region) or preg_match('/^[A-Z][0-9][A-Z] ?([0-9][A-Z][0-9])?$/i', $region));
  $subFlds = 'which value';
  $distCrit = tr('((latitude<>0 OR longitude<>0) AND %DISTANCE_BETWEEN<%NEIGHBOR_MAX)');
  
  if ($isZip) { // eg 013 or 01002
    list ($value, $crit) = [$region, 'u.zip LIKE :value'];
    if ($nearby) {
      extract(db\get('AVG(latitude) AS lat,AVG(longitude) AS lon', 'users u', $crit, ray('value', $value . '%')));
      if ($lat or $lon) $crit .= ' OR ' . strtr($distCrit, ray(':lat :lon', $lat ?: 0, $lon ?: 0)); else $nearby = FALSE;
    }
    $place = ($nearby ? t('near ') : t('in ')) . $region;
  } elseif (!$region and $mya) { // we always have lat and lon here; nearby is assumed (since no region specified)
    list ($value, $crit, $subFlds, $place) = [NULL, $distCrit, 'which lat lon', t('Near You')];
  } elseif (!$region) { // signed out and no region specified -- show all and hope we have an industry query
    list ($value, $crit, $place) = [1, ':value=1', t('Anywhere')];
  } elseif (preg_match('/[A-Z]{3}/', $region)) {  // eg NEW or MIW (nearby is irrelevant)
    list ($value, $crit) = [r\serverUid($region), 'u.uid BETWEEN -:value AND -:value+:R_REGION_MAX-1'];
    $place = t('in ') 
    . (db\get('fullName', 'r_regions', compact('region')) 
      ?: (r\fullState(db\get('st', 'r_regions', compact('region'))) 
      ?: t('Unknown Region')));
  } else { // eg MA or Germany (nearby is irrelevant)
    list ($value, $crit, $place) = [$region, 's.abbreviation=:value OR c.name=:value', t('in ') . r\fullState($region)];
  }

  if ($which == SIMPLE_CO_LIST) {
    list ($fields, $order) = ['u.uid', 'u.fullName'];
    $which = '';
  } else {
    $fields = 'u.uid, a.base, a.ancestor as selIid, a.ancestorIndustry as industry, abbreviation AS st';
    $order = 'industry, u.fullName';
    $which = u\ignoreSuffix($which, 'ants ant ian es ers ing er or ion s');
    $which = str_replace(' ', '%', \db_like(" $which ")); // allow abbreviations of each word
  }

  $sql = <<<EOF
    SELECT DISTINCT $fields
    FROM users u
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN ancestors a on a.base=ui.iid
    LEFT JOIN r_states s ON s.id=u.state
    LEFT JOIN r_countries c ON c.id=u.country
    WHERE :IS_CO AND u.uid>:CANONIC_ACCTS AND :IS_OK AND ($crit) AND (u.fullName LIKE :which OR a.baseIndustry LIKE :which)
    ORDER BY $order
EOF;

  $result = db\q($sql, compact(ray($subFlds)));
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
  if (!$rows) return t('No companies there yet. Invite them!');

  $model = <<< X
<div class="row">
  <div class="cmpy-img"><img src="%img" alt="profile picture" /></div>
  <div class="cmpy-name-does">
    <div class="cmpy-name">%name</div>
    <div class="cmpy-does">%desc</div>
  </div>
  <div class="cmpy-contact">%contact</div>
  <div class="cmpy-buttons">%buttons</div>
</div>
X;
  $iModel = '<div class="cmpy-ind-wrapper"><div class="cmpy-ind">%industry</div>';

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
      $industry = $row['industry'] ?: tr('Unclassified');
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
      $buttons = ($a->coType == CO_NONPROFIT xor $a->coCan(CO_FLIP))
      ? buttonHtml($a, t('Donate'), 'donate', 'success')
      : buttonHtml($a, t('Buy<br>Credit'), 'credit', 'primary');
      if (in($a->mainQid, NO_STORE_CREDIT)) $buttons = '';

      $contact = "$a->city, $a->st<br>$a->phoneDash";
      $new = ($a->activated > REQUEST_TIME - 9 * WEEK_SECS) ? $NEW : '';
      $img = $a->photoSrc(TRUE) ?: 'images/no-photo-available.jpg';
      $res .= tr($model, '_name _buttons desc _contact img', "$name$new", $buttons, strtolower($a->shortDesc)
      , $contact, $img) . "\n";
    }
  }
  if (! $firstTime) $res .= '</div>';
  return $res;
}

function buttonHtml($a, $text, $for, $style) {
  $class = "cgpay-$for";
  $code = w\authcode("company=$a->mainQid&cocode=$a->emailCode", FALSE);
  return w\btn("/pay-with-cg/company=$a->mainQid&code=$code&for=$for", $text, $style, '', compact('class') + away());
}
