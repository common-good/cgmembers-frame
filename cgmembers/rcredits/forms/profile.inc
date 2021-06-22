<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Display the member company's profile.
 */
function formProfile($form, &$sta, $company = '', $args = '') {
  $uid = db\get('uid', 'users', 'name=:company', compact('company'));
  $a = r\acct($uid);
  if (!$uid or !$a->co) return w\softErr(t('That is not a %PROJECT company profile page.'));

  $pic = item($a->photoHtml());
  $state = $a->st;
  $country = r\realCountry($a->country);
  $siteList = '';
  if ($website = $a->website) foreach (ray($website) as $one) {
    $atag = atag("http://$one");
    $siteList .= "<$atag>$one</a><br>";
  }
    
  $title = $a->fullName;
  $phone = 'Phone: ' . u\fmtPhone($a->phone, '+');
  $physical = "$a->address<br>$a->city, $state $a->zip";
  $mailing = str_replace('<br>', ', ', $physical) == $a->postalAddr ? '' : "Mailing address: $a->postalAddr<br>";

  $contact = item("$physical $country<br>$mailing<br>
    $phone<br>
    $siteList");
  $desc = item($a->coDesc());  

  return cgform(compact(ray('pic title contact desc')));
}