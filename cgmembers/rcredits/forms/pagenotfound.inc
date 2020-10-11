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
  if ($page and $page[0] == '/') $page = substr($page, 1); // ignore leading slash
  if ($mya and !$page) return go('/console');
  
  if ($page and $page[0] == '-') {
    include_once __DIR__ . '/../forms/signup.inc';
    return formSignup('', $sta, 'code=' . substr($page, 1)); // invitation!
  }
  
  $title = t('Page Not Found');
  $subtext = t('The page you requested does not exist.');
  return cgform(compact(ray('title subtext')));
}