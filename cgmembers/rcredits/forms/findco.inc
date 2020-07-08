<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once R_ROOT . '/rweb/co-list.inc';

/**
 * Show a list of participating businesses in a given region.
 * @param string $args: optional parameters from URL:
 *    where   community qid or start of zip pattern
 *    cos     part of company name or industry category
 *    nearby  include nearby businesses in the region?
 */
function formFindCo($form, &$sta, $args = '') {
  global $mya;
  extract(just('where cos nearby', $args, ''));

  $list = w\coList($cos, $where, $nearby, $place);
  
  $title = t('Companies ') . $place;
  if ((!$where and !$cos) or !strhas($list, '</div>')) { // nothing listed
    $subtext = $posttext = NULL;
    $cos = textFld(t('Search for:'), [t('Whatever'), t('Type part of company name or industry category')], dft($cos));
    $where = textFld(t('Where:'), [t('Start of postal code'), t('Type the first few characters of your postal code or your state abbreviation.<br>For example, type 013 for Franklin County, Massachusetts; 05 or VT for Vermont, etc.')], dft($where)); // or country name
    $nearby = boxFld('nearby', t('Wider Area:'), t('Include nearby businesses in the region (within %NEIGHBOR_MAX miles)?'), TRUE);
    $submit = t('Find');
  } else { // list whatever, with option to go back to search
    $cos = $where = $nearby = $submit = NULL;
    $subtext = $posttext = backButton();
  }

  $list = "<div class=\"cmpy-list\">$list</div>";

  return cgform(compact(ray('title subtext where cos nearby submit list posttext')));
}

function formFindCo_validate($form, &$sta) {
}

function formFindCo_submit($form, &$sta) {
  w\go('/community/find-company/' . http_build_query(just('where cos nearby', $sta['input'])));
}
