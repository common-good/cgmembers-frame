<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Describe Common Good's response to COVID-19.
 */
function formCovid($form, &$sta, $args = '') {
  global $mya;
  extract(just('nothing', $args));

  $title = t('%PROJECT response to COVID-19');
  $subtext = t('It\'s up to us. Let\'s support each other. And support our local businesses and nonprofits!');

  $actions = ray(t('Use our Offers & Needs board, Buy Credit, Make Step-up Contributions, Increase Your Backing, Invite Someone'));
  $links = ray('/community/posts /community/buy-credit /community/stepup /community/backing /community/invite');

  foreach ($actions as $i => $text) $choices[] = lnk($links[$i], $text);
  $choices = item('<ul><li>' . join("</li>\n<li>", $choices) . '</li></ul>');
  
  return cgform(compact(ray('title subtext choices')));
}
