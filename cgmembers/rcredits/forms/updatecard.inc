<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Let a non-member donor replace the card used for their recurring card donations, from a link we send them.
 * The link (see cardLink()) identifies the donor, so no sign-in is needed.
 */
function formUpdateCard($form, &$sta, $args = '') {
  extract(just('code', $args, ''));
  if (!$p = cardPerson($code)) return softErr(t('That link is invalid or has expired. Please ask us for a new one.'));
  if (!$recurs = cardRecurs($p->pid)) return softErr(t('You have no recurring card donations to update.'));

  require_once R_ROOT . '/stripe.inc';
  $intent = ($setupId = nni($sta['input'], 'setupId')) // Drupal rebuilds the form on submit, so don't make another SetupIntent then
  ? obj('id client_secret', $setupId, '')
  : stripe('setup', ray('stripeCid description', $p->stripeCid, t('card update')));
  if (!is_object($intent)) {
    r\tellAdmin(t('Stripe setup failed for card update'), ray('pid erMsg', $p->pid, $intent));
    return softErr(t('We could not reach our card processor. Please try again later.'));
  }

  foreach ($recurs as $x) $list[] = u\fmtAmt($x->amount) . ' ' . r\recurDesc($x->period, $x->periods) . t(' to ') . r\acct($x->to)->bestName . ($x->failed ? t(' (paused)') : '');
  $title = t('Update Your Card');
  $subtext = t('<p>Hello %nm. The card you enter here will be used for your recurring donations:</p>', 'nm', $p->fullName) . u\ul($list);
  $payment = item('', t('Card:'));
  $paymentErr = item('', NBSP);
  $submit = submit(t('Update'));
  foreach (ray('code secret setupId', $code, $intent->client_secret, $intent->id) as $k => $v) $$k = hidFld($v);

  js('https://js.stripe.com/v3/');
  jsx('update-card', ray('stripePublicKey', STRIPE_PUBLIC));
  return cgform(compact(ray('title subtext payment paymentErr submit code secret setupId')));
}

function formUpdateCard_submit($form, &$sta) {
  extract(just('code setupId', $sta['input'], ''));
  if (!$p = cardPerson($code)) return softErr(t('That link is invalid or has expired. Please ask us for a new one.'));

  require_once R_ROOT . '/stripe.inc';
  $intent = stripe('getSetup', ray('stripeId stripeCid', $setupId, $p->stripeCid));
  if (!is_object($intent) or $intent->customer != $p->stripeCid or $intent->status != 'succeeded') {
    r\tellAdmin(t('Card update failed'), ray('pid setupId erMsg', $p->pid, $setupId, is_object($intent) ? $intent->status : $intent));
    return softErr(t('Your new card could not be saved. Please try again.'));
  }

  foreach ($recurs = cardRecurs($p->pid) as $x) db\update('tx_timed', ray('id stripeId failed', $x->id, $setupId, NULL), 'id');
  r\tellAdmin(t('Donor updated card'), ray('pid fullName recurs', $p->pid, $p->fullName, count($recurs)));
  return sayEmpty(t('Thank you! Your new card will be used for your future donations.'));
}

/**
 * Return a link for the donor to update their card (valid for CARD_LINK_DAYS days).
 * The Stripe customer ID in the code keeps a link from working for any other person or after the customer record changes.
 */
function cardLink($pid, $stripeCid) {
  return BASE_URL . '/update-card/code=' . u\cryRay(ray('pid cid expires', $pid, $stripeCid, now() + CARD_LINK_DAYS * DAY_SECS));
}

/**
 * Return the people record identified by a valid, unexpired card-update code (else NULL).
 */
function cardPerson($code) {
  extract(just('pid cid expires', u\decryRay($code) ?: [], NULL));
  if (!$pid or !$cid or $expires < now() or !$p = db\row('people', 'pid=:pid', compact('pid'))) return NULL;
  return $p->stripeCid == $cid ? $p : NULL;
}

function cardRecurs($pid) {
  return db\rows('id,amount,period,periods,`to`,failed', 'tx_timed', 'payerType=:REF_PERSON AND payer=:pid AND stripeId IS NOT NULL AND period<>:PER_1 AND IFNULL(end, :NOW + 1) > :NOW', compact('pid'));
}
