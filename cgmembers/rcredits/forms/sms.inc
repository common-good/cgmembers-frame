<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

/**
 * Test the SMS interface (admins only and deprecated)
 */
function formSMS($form, &$sta) {
  $title = t('Test SMS');
  $myNumber = svar('lastSMS');
  $number = phoneFld(t('From (number): '), '', required($myNumber));
  $message = textFld(t('Message: '));
  $submit = t('Send it!');

  focusOn('message');
  $form = compact(ray('title number message submit'));
  return cgform($form);  
}

function formSMS_submit($form, $sta) {
  global $sms_devel; $sms_devel = TRUE;
  extract(just('number message', $sta['input']));
  $number = u\fmtPhone($number, '+n');
  svar('lastSMS', $number);
  \rsms_sms_incoming('process', $number, $message);
}