<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once R_ROOT . '/admin/admin.inc';

/**
 * Run acceptance tests.
 */
function formTests($form, &$sta) {
  if (!isDEV) return;

  \drupal_get_messages(); // clear messages
  db\q('TRUNCATE test'); // clear in advance, rather than after, so we can inspect results when tests fail
  $modules = u\prefixValues('rcredits/', ray('admin rvote rweb rcron rsmart api'));
//  jsx('test');
  test(TRUE);

  if (!getv('cgf_uid')) {
    a\setupBasicAccounts();
    return go('/tests/?' . $arg, t('redirecting after setting up basic accounts'), 'err');
  }

  include_once DRUPAL_ROOT . '/vendor/gherkin/test.php'; // some inline code and functions
  //  $_SESSION = [];
  if (ini_get("session.use_cookies")) { // from PHP's session_destroy page
    $params = session_get_cookie_params();
    setcookie(session_name(), '', now() - 42000,
              $params["path"], $params["domain"],
              $params["secure"], $params["httponly"]
              );
  }
  if (session_id() and session_status() == PHP_SESSION_ACTIVE and nn($_SESSION)) session_destroy();
  db\q('TRUNCATE sessions');
  return go('/sadmin/test-results/top=1'); // has to be a form available when logged out
}
