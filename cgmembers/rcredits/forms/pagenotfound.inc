<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Handle page not found error (404).
 */
function formPageNotFound($form, &$sta, $arg = '') {
  global $mya;

  $page = plain($_SERVER['REQUEST_URI']);
  $subtext = t('The page you requested does not exist (%page).', compact('page'));

  if ($page and $page[0] == '/') $page = substr($page, 1); // ignore leading slash
  if ($mya) {
    if (!$page) return go('dashboard');
    if (in(substr($page, -4), '.jpg jpeg .png')) {
      r\tellAdmin('page not found', compact('page'));
    } else return go('dashboard', $subtext);
  }
  
  if (!$page) return go('signin');
  if ($page[0] == '-') return go('/signup/code=' . substr($page, 1)); // invitation!
  
  $title = t('Page Not Found');
  return cgform(compact(ray('title subtext')));
}