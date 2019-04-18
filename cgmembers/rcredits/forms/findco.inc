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

  $list = w\directoryList(@$cos, @$where ?: @$mya->community);
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
 * @param mixed $region: community (the default), zip (maybe partial), or country (maybe partial)
 * @param bool $withButtons: include "Pay" and "Chg" buttons
 */
function directoryList($which, $region, $withButtons = FALSE) {
  global $base_url, $mya;

  if ($withButtons and $mya) {
    $canBuy = $mya->can(B_BUY);
    $canSell = $mya->can(B_SELL);
  } else $canBuy = $canSell = FALSE;

  list ($symbolMember, $symbolRTrader, $symbolNonMember) = array('m', 'R', '-');

  if (!$region) {
    $region = r\serverUid();
    $regionCrit= ($mya and $mya->state) ? "state=:$mya->state" : 1;
  } else if (is_numeric($region) and $region < 0) {
    $regionCrit = 'community=:region';
  } else {
    $region = \db_like($region) . '%';
    $regionCrit = '(zip LIKE :region OR country LIKE :region)';
  }

  $which = u\ignoreSuffix($which, 'ants ant ian es ers ing er or ion s');
  $which = str_replace(' ', '%', \db_like(" $which ")); // allow abbreviations of each word

  $sql = <<<EOF
    SELECT DISTINCT u.uid, i.iid, if (ISNULL(parent.industry), i.industry, parent.industry) as selIndustry, parent.iid as selIid
    FROM users u 
    LEFT OUTER JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT OUTER JOIN r_industries i ON i.iid=ui.iid
    LEFT OUTER JOIN r_industries parent on i.parent = parent.iid
    WHERE :IS_CO AND u.uid>:CANONIC_ACCTS AND :IS_OK AND $regionCrit AND (u.fullName LIKE :which OR i.industry LIKE :which)
    ORDER BY selIndustry, u.fullName
EOF;
  $result = db\q($sql, compact(ray('region which')));
  $rows = [];
  foreach ($result as $row) {
    if (nn($xrow) and $xrow->uid != $row->uid) $rows[] = $row; // eliminate dups within class (DISTINCT fails)
    $xrow = $row;
  }
  return participatingCompanies((array) $rows, FALSE);
}

/**
 * Return a list of participating companies for the given community.
 * @param assoc $rows: information to display
 * @param bool $email: <format for use in emails and show just new companies> (else format for promo site or Find Co page)
 */
function participatingCompanies($rows, $email = FALSE) {
  $model = $email 
  ? '<tr><td width="200">%name</td><td style="margin-left:10px;">%desc</td></tr>'
  : '<div class="row"><div class="cmpy-img"><img src="%img" alt="profile picture" /></div><div class="cmpy-name-does"><div class="cmpy-name">%name</div><div class="cmpy-does">%desc</div></div></div>';
  $iModel = '<div class="cmpy-ind-wrapper"><div class="cmpy-ind">%industry</div>';

  if (!$rows) return $email ? t('no new companies this month') : t('No companies there yet. Invite them!');
  $res = '';
  $_newClass = $email ? 'style="color:red; font-weight:bold;"' : 'class="company-listing"';
  $NEW = t(' <small %newClass> NEW!</small>', compact('_newClass'));

  $lastSelIid = -1;  // negative record ids are, we hope, unlikely
  $firstTime = true;
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
      $name = $a->website ? w\lnk("http://$a->website", $a->bestName) : $a->bestName;
      $new = ($a->activated > REQUEST_TIME - 9 * WEEK_SECS) ? $NEW : '';
      $img = $a->photoSrc(true) ?: 'images/no-photo-available.jpg';
      $res .= tr($model, '_name desc img', "$name$new", $a->shortDesc, $img) . "\n";
    }
  }
  if (! $firstTime) $res .= '</div>';
  return $res;
}