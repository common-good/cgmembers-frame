<?php
namespace CG;
use CG as r;

/**
 * @file
 * Text messages to members. A thin layer over twilioSend() (cg-tx-notify.inc) that says what to send
 * and what to do when it cannot be sent. Every message wording lives here, passed through t().
 */

require_once R_ROOT . '/cg-tx-notify.inc';

/**
 * Send a text message to a member.
 * @param acct $a: the member
 * @param string $text: what to say
 * @return bool: TRUE if Twilio took it; FALSE if the member has no phone, texting is not set up, or the send failed
 *   (including the member having replied STOP, Twilio error 21610), in which case the caller should use email instead.
 */
function sms($a, $text) {
  if (!$a->phone or !twilioConfigured() or test()) return FALSE; // tests never text real phones
  try {
    return twilioSend($a->phone, $text) !== '';
  } catch (\Throwable $e) {
    if (!strhas($e->getMessage(), '21610')) r\tellAdmin('sms-failed', ray('uid err', $a->id, $e->getMessage())); // 21610 means the member opted out, which is routine
    return FALSE;
  }
}

/** Text a password reset code. Same return as sms(). */
function smsResetCode($a, $code) {
  return sms($a, t('Common Good: Your password reset code is %code. It expires in 10 minutes. Never share this code. Reply STOP to opt out.', compact('code')));
}
