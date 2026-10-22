<?php
namespace CG\Web;
use CG as r;
use CG\Db as db;
use CG\Util as u;

/**
 * @file
 * Server-to-server signup endpoint for the SvelteKit cgpay app.
 *
 * The cgpay /signup page POSTs here through its /api/signup proxy. This
 * endpoint:
 *   1. verifies the shared secret
 *   2. validates the body (reusing the same checks the legacy PHP /signup form uses)
 *   3. creates a new Acct in the "unverified" state (no `signed` bit yet)
 *   4. generates a one-time verification code + emails it to the member
 *   5. returns {uid, qid, status: 'verify_email'}
 *
 * Verification + password-set happen on the companion /cgpay-signup-verify
 * endpoint. The SSO cookie is NOT set here - the account cannot act until it
 * is verified.
 *
 * Shared secret: config.json `cgpaySsoSecret` -> CGPAY_SSO_SECRET.
 */
function cgpaySignup() {
  global $channel; $channel = TX_API;

  if (nni($_SERVER, 'REQUEST_METHOD') !== 'POST') return exitJust(X_SYNTAX);

  $expected = CGPAY_SSO_SECRET;
  $provided = nni($_SERVER, 'HTTP_X_CG_INTERNAL_TOKEN');
  if (empty($expected) || empty($provided) || !hash_equals($expected, $provided)) {
    return exitJust(X_UNAUTH);
  }

  $body = json_decode(file_get_contents('php://input'), TRUE);
  if (!is_array($body)) return exitJust(X_SYNTAX);

  $fullName = trim((string) nni($body, 'fullName'));
  $email    = trim((string) nni($body, 'email'));
  $phone    = trim((string) nni($body, 'phone'));
  $zip      = trim((string) nni($body, 'zip'));
  $country  = (int) (nni($body, 'country') ?: US_COUNTRY_ID);
  $accepted = (bool) nni($body, 'agreementAccepted');

  // optional carry-through fields - preserve acquisition + inviter attribution
  $invitationCode = trim((string) nni($body, 'invitationCode'));
  $source         = trim((string) nni($body, 'source'));

  $errors = [];

  if ($fullName === '') {
    $errors['fullName'] = t('Please enter your full name.');
  } elseif ($err = u\badName($fullName)) {
    $errors['fullName'] = $err;
  } elseif (r\isCriminal($fullName, FALSE)) {
    // flagged name: tell admin, soft-fail with the FLN support code (mirrors legacy flow)
    r\tellAdmin(t('New potential member flagged as criminal'), $body);
    return exitJson(['errors' => ['fullName' => tr('code FLN')]], X_SYNTAX);
  }

  if ($phone === '') {
    $errors['phone'] = t('Please enter your phone number.');
  } elseif ($err = u\badPhone($phone)) {
    $errors['phone'] = $err;
  }

  if ($email === '') {
    $errors['email'] = t('Please enter your email address.');
  } elseif (!emailOkay($email, FALSE)) {
    $errors['email'] = t('That email address is not valid or is already in use.');
  } elseif (emailDupCo($email)) {
    $errors['email'] = t('That email is already registered to a company account.');
  }

  if ($zip === '') {
    $errors['zip'] = t('Please enter your ZIP code.');
  } elseif ($err = u\badZip($zip, $country)) {
    $errors['zip'] = $err;
  }

  if (!$accepted) $errors['agreementAccepted'] = t('You must agree to the Common Good Agreement to continue.');

  if ($errors) return exitJson(['errors' => $errors], X_SYNTAX);

  $fullName = u\normalizeCase($fullName);
  list ($city, $state, $latitude, $longitude) = r\zipCity($zip);

  $info = compact(ray('fullName email phone country zip city state'));
  if ($source !== '') $info['source'] = $source;
  if ($invitationCode !== '') $info['code'] = $invitationCode;

  $a = new r\Acct($info);
  if (!$a) return exitJust(X_SYNTAX);
  $uid = (int) $a->id;

  // mark email + extras the legacy form marks
  $a->update('email', u\cry('P', $email));
  $a->setRisk('rents');

  if ($invitationCode !== '') invitation($invitationCode, $uid);
  db\q('UPDATE r_invites SET invitee=:myid WHERE email=:email', ['myid' => $uid, 'email' => u\cry('P', $email)]);

  tellStaffNewMember($info + ['acctType' => CO_PERSONAL, 'qid' => $a->mainQid, 'name' => $a->name]);

  // Generate + email the verification code. Reuses `oneTimePass` which already
  // handles expiry; the cgpay-signup-verify endpoint checks against the same
  // field. Not the final password - member sets that on the verify step.
  $code = $a->oneTimePass();
  $a->tell('verify', ray('name qid code', $a->name, $a->mainQid, $code));

  return exitJson([
    'uid'    => $uid,
    'qid'    => $a->mainQid,
    'status' => 'verify_email',
  ], X_OK);
}
