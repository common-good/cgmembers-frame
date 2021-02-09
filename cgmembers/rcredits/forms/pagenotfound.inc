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

  $page = $_SERVER['REQUEST_URI'];
  $subtext = t('The page you requested does not exist (%page).', compact('page'));

  if ($page and $page[0] == '/') $page = substr($page, 1); // ignore leading slash
  if ($mya) return $page ? go($mya->cAdmin ? '/sadmin' : '/dashboard', $subtext) : go('/signup');
  
  if (!$page or $page[0] == '-') return go('/signup/code=' . substr($page, 1)); // invitation!
  
  $title = t('Page Not Found');
  return cgform(compact(ray('title subtext')));
}