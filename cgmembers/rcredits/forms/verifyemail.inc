<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

//include_once __DIR__ . '/changepassword.inc';
//include_once __DIR__ . '/resetpassword.inc';

/**
 * Send the new member a link and a code, to verify their email address.
 */
function formVerifyEmail($form, &$sta, $arg = '') {
  global $mya;

  $title = t('Verify Your Email Address');
  $subtext = t('sent email', 'email', $mya->email) . t('<p><%a>Resend or change email</a></p>', '_a', atag('/settings/resend'));
  return cgform(compact(ray('title subtext')));
}
