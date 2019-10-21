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
  
  if ($mya and $mya->stepIsDone('verifyemail')) { // process got interrupted, but now is finished
    ; // nothing to do here
  } elseif ($args) {
    signout(TRUE);
    extract(just('code', $args));
    if (!$data = r\doCode($code)) return softErr(t('That is not a valid link.'));
    extract(just('cgAccount pid', $data));
    if (!@$pid or !$pA = r\acct($pid) or !in($pA->name, SIGNUP_PARTNERS)) return softErr(t('bad partner'));
    
    emailContract($pA, 'signed', $data + ['date' => time()]);

    if ($cgAccount) {
      $mya = r\acct($cgAccount);
      $mya->stepDone('verifyemail');
      if ($mya->nextStep() != 'partnerend') return w\goNextStep('');
      $mya->stepDone('partnerend');
    }
    
    svar('partner', $pA->name); // tell page.html to use the partner's branding
    // no need to email partner, since they got a copy of the contract
  } else {
    if (!$partner = svar('partner')) {
      if (!$mya or $mya->ok or !$partner = db\get('u.name', 'r_relations r LEFT JOIN users u ON u.uid=r.main', 'r.other=:id AND r.:IS_CUSTOMER', ['id' => $mya->id])) return softErr(t('The final step page is not available right now.'));
    }
    
    svar('partner', $partner); // tell page.html to use the partner's branding

    $title = t('Community Solar Final Step');
    $subtext = t('<p>Just one step to go, to complete your Community Solar subscription:</p><p>We emailed you a contract. Please click the button in that email within 3 days, to sign the contract.</p><p>If you do not see the email, look for it in your Spam folder.</p><p>You can also preview the standard contract <%a>here</a>.', '_a', atag("/rcredits/images/contracts/$partner.pdf"));
  }
  
  return cgform(compact(ray('title subtext')));
}
