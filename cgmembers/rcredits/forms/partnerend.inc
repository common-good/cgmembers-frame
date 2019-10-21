<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once R_ROOT . '/forms/partnersignup.inc'; // for validation

/**
 * Finish a signup through a partner organization (for now, just show or link to the contract with Co-op Power).
 */
function formPartnerEnd($form, &$sta, $args = '') {
  global $mya;

  $title = t('Congratulations!');
  $subtext = t('<p>You have completed your solar subscription sign-up. We have emailed you a copy of the signed contract.</p><p>We will let you know when your array is ready to start delivering credits to your electric account and when we will start billing you for those credits. Let us know if you have any questions!</p>');
  $midtext = t('<p>Want to go further with %PROJECT? Click below to apply for your free %PROJECT Card or participate in the %PROJECT Democracy in your community.</p>');
  $continue = submit(t('Continue to %PROJECT'));
  
  if ($mya and $mya->stepIsDone('verifyemail')) { // process got interrupted, but now is finished
    ; // nothing to do here
  } elseif ($args) {
    signout(TRUE);
    extract(just('code', $args));
    if (!$data = r\doCode($code)) return softErr(t('That is not a valid link.'));
    extract(just('cgAccount pid customer', $data));
    if (!@$pid or !$pA = r\acct($pid) or !in($pA->name, SIGNUP_PARTNERS)) return softErr(t('bad partner'));
    
    $contractInfo = ray('partner customer', $pA->uid, $customer);
    if (!$dateSigned = db\get('created', 'r_contracts', $contractInfo)) {
      $created = ['created' => time()];
      emailContract($pA, 'signed', $data + $created);
      db\insert('r_contracts', $contractInfo + $created, 'id');
    }

    if ($cgAccount) {
      $mya = r\acct($cgAccount);
      $mya->stepDone('verifyemail');
      $mya->stepDone('partnerend');
      if ($dateSigned and $dateSigned < strtotime('today')) { // signed before today; no using the link just to sign in
        $midtext = t('Click below to sign in to your %PROJECT account.');
      } else $sta['uid'] = $mya->id; // sign in automatically
    } else $midtext = $continue = NULL;
    
    svar('partner', $pA->name); // tell page.html to use the partner's branding
    // no need to email partner, since they got a copy of the signed contract
  } else {
    if (!$partner = svar('partner')) {
      if (!$mya or $mya->ok or !$partner = db\get('u.name', 'r_relations r LEFT JOIN users u ON u.uid=r.main', 'r.other=:id AND r.:IS_CUSTOMER', ['id' => $mya->id])) return softErr(t('The final step page is not available right now.'));
    }
    
    svar('partner', $partner); // tell page.html to use the partner's branding

    $title = t('Community Solar Final Step');
    $subtext = t('<p>Just one step to go, to complete your Community Solar subscription:</p><p>We emailed you a contract. Please click the button in that email within 3 days, to sign the contract.</p><p>If you do not see the email, look for it in your Spam folder.</p><p>You can also preview the standard contract <%a>here</a>.', '_a', atag("/rcredits/images/contracts/$partner.pdf"));
    $midtext = $continue = NULL;
  }
  
  return cgform(compact(ray('title subtext midtext continue')));
}

function formPartnerEnd_submit($form, &$sta, $args = '') {
  setAcct($sta['uid']);
  return go('summary');  
}
