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
 */
function formFindCo($form, &$sta, $args = '') {
  global $mya;
  extract(just('where cos', $args, NULL));

  $list = w\coList(nn($cos), nn($where));
  $list = "<div class=\"cmpy-list\">$list</div>";
  
  $title = t('Companies in Your Region');
  $which = textFld(t('Search for:'), [t('Whatever'), t('Type part of company name or industry category')], dft($cos));
  $region = textFld(t('Where:'), [t('Start of postal code'), t('Type the first few characters of your postal code or your state abbreviation.<br>For example, type 013 for Franklin County, Massachusetts; 05 or VT for Vermont, etc.')], dft($where)); // or country name
  $submit = t('Find');

  return cgform(compact(ray('title region which submit list')));
}

function formFindCo_validate($form, &$sta) {
}

function formFindCo_submit($form, &$sta) {
  extract($sta['input']);
  w\go("community/find-company/where=$region&cos=$which");
//  svar('which_companies', trim($which));
//  svar('region', trim($region));
}
