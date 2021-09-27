<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once __DIR__ . '/coupons.inc';

/**
 * Offer a trial company a way to get their customers signed up.
 */
function formDiscount($form, &$sta) {
  global $mya;

  foreach (ray(COUPON_FLDS) as $k) $$k = NULL;
  
  $title = t('Get Your Customers Signed Up');
  $subtext = t(<<< X
    <p>%PROJECT saves you credit card fees AND it funds wonderful community initiatves. But for that all to work, your customers have to sign up for %PROJECT!</p>
    <p>Fill out this form to create a lmiited-time discount for %PROJECT members. The discount will be used by %PROJECT as an incentive for new members to sign up and as an incentive for current members to buy from your business!</p>
    <p>You can also advertise this discount to reward customer loyalty and to get all your customers to pay with %PROJECT (saving you thousands of dollars on credit card fees in the longer term).</p>
    <p>If you skip this step, you can find it again on the Company menu.</p>
X
  );
  //  A printable sheet of coupons will be displayed (to be cut in half three times). <-- No. Mail the coupons to them.

  $amount = textFld(REQ . t('Discount:'), [t('Amount or Percentage'), t('For example $20 or 10%')]);
  $minimum = numFld(REQ . t('Minimum:'), [t('Minimum purchase'), t('Minimum purchase amount, to get the discount.')], dft(0));
  $end = dateFld(t('Valid until:'), [t('End date'), t('Leave blank for no end.')], dft(strtotime('+6 months')));
  $useMax = intFld(t('Limit:'), t('Leave blank for unlimited uses.'), dft(5) + suffix(t(' uses per member')));
  
  $submit = submit(t('Create'), '', '', suffix(' ' . skipButton())); // allow user to skip this step forever
  
  $start = hidFld(u\fmtDate(now(), TRUE));
  $automatic = hidFld(TRUE);
  $type = hidFld('discount');
  
  return cgform(compact(ray('title subtext ' . COUPON_FLDS . ' submit')));
}

function formDiscount_validate($form, &$sta) {return formCoupons_validate($form, $sta);}

function formDiscount_submit($form, &$sta) {formCoupons_submit($form, $sta, TRUE);}
