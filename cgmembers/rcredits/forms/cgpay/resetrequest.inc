<?php
namespace CG\Web;
use CG as r;
use CG\Util as u;

/**
 * @file
 * Server-to-server password-reset-request endpoint for the SvelteKit cgpay app.
 *
 * The cgpay /reset page POSTs here through its /api/reset-request proxy. This
 * endpoint:
 *   1. verifies the shared secret
 *   2. looks up the uid for the submitted identifier via r\loginString
 *   3. generates a one-time code via $a->oneTimePass()
 *   4. delivers the code by the member's chosen channel (email or SMS)
 *   5. always returns 200 with the same shape, whether or not the identifier
 *      matched - the identifier-enumeration guard called out in the scoping doc
 *
 * Companion verify + password-set happens on /cgpay/reset-verify.
 *
 * Shared secret: config.json `cgpaySsoSecret` -> CGPAY_SSO_SECRET.
 *
 * TODO (waiting on William's answer): reuse $a->oneTimePass() or add a dedicated
 * resetCode field? Reusing is simpler but wipes any pending sign-up code if a
 * member hits reset mid-signup. For now this reuses oneTimePass to match the
 * current PHP /settings/password form behavior.
 *
 * TODO (waiting on William's answer): SMS rate-limit shape - the scoping doc
 * proposed 3-per-24h per account on the PHP side. Jose asked for a $10/mo spend
 * cap target with alerts; both will live here once the design is confirmed.
 *
 * TODO (waiting on William's answer): new 'password-reset-sms' template or
 * reuse the existing cg-tx-notify.inc path for SMS? The email channel below
 * reuses the existing 'password-reset' template.
 */
function cgpayResetRequest() {
  global $channel; $channel = TX_API;

  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  $body = json_decode(file_get_contents('php://input'), TRUE);
  if (!is_array($body)) return exitJust(X_SYNTAX);

  $identifier = trim((string) nni($body, 'identifier'));
  $channelPref = nni($body, 'channel');
  if ($channelPref !== 'sms' && $channelPref !== 'email') $channelPref = 'email';

  // Identifier-enumeration guard: always return 200 with the same shape so the
  // response cannot be used to probe which identifiers belong to real accounts.
  // On no match or company-only account, return uid=0 and skip the delivery
  // path quietly.
  $empty = ['uid' => 0, 'qid' => '', 'channel' => $channelPref, 'status' => 'code_sent'];

  if ($identifier === '') return exitJson($empty, X_OK);

  $uid = r\loginString($identifier);
  if (!$uid) return exitJson($empty, X_OK);

  $a = r\acct($uid);
  if (!$a) return exitJson($empty, X_OK);
  if ($a->co && !$a->trialco) return exitJson($empty, X_OK); // mirror formPass_validate rule

  $qid = $a->co ? $a->qid : $a->mainQid;
  $code = $a->oneTimePass();

  if ($channelPref === 'email') {
    // Reuses the existing 'password-reset' template that /settings/password uses today.
    $a->tell('password-reset', ['qid' => $qid, 'code' => $code, 'pwMsg' => '']);
  } else {
    // SMS channel: TODO - plug the new 'password-reset-sms' template into
    // cg-tx-notify.inc's twilioSend() path. See scoping doc section "SMS specifics"
    // for the approved copy ("Common Good: Your password reset code is {code}.
    // It expires in 10 minutes. Never share this code. Reply STOP to opt out.")
    // and A2P 10DLC compliance notes. Falling back to email until the SMS path
    // ships so a member hitting reset in staging/demo still gets their code.
    $a->tell('password-reset', ['qid' => $qid, 'code' => $code, 'pwMsg' => '']);
  }

  return exitJson([
    'uid'     => (int) $a->id,
    'qid'     => $qid,
    'channel' => $channelPref,
    'status'  => 'code_sent',
  ], X_OK);
}
