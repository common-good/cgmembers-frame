<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Cron as cr;

/*
  Give the person a chance to donate to CG from the current account.
  If they haven't chosen a recurring donation yet, a recurring donation is the only option
*/
function formCC($form, &$sta, $args = '') {
  extract(just('op code qid', $args, NULL));
  $ray = $code ? u\decryRay($code) : [];
  if ($op == 'done' and $ray) return thank($ray);
  
  extract(just('fullName email zip', $ray, '')); // for cancel, repopulate
//  global $mya; if ($mya) return go('community/donate', t('You are a %PROJECT member! Please use this page to donate.'));

  $giftLevels = array(
  // 10000 => t('Marble .. $10,000'),
  // 5000 => t('Granite ... 5,000'),
    2500 => t('Slate .... $2,500'),
    1000 => t('Glass .... $1,000'),
     500 => t('Iron ....... $500'),
     250 => t('Oak ........ $250'),
     100 => t('Two Bricks . $100'),
      50 => t('One Brick ... $50'),
      25 => t('Half Brick .. $25'),
//       0 => t('None for now . $0'),
      -1 => t('Water ... (other)'),
//      10 => t('Tile ........ $10'),
//       5 => t('Cloth ........ $5'),
  );

  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of');

  $img = item(tr('<img src="%BASE_URL/rcredits/images/icons/%url" />', 'url', 'h+$=cg-180.png'));
  $title = t('Donate to %PROJECT');
  $subtext = t('<p>Make a tax-deductible gift to %PROJECT of any amount. The most common amounts chosen are $50 and $25.</p><p>Thank you for your support!</p>');
  $fullName = textFld(REQ . t('Name:'), [t('Your full name')], dft($fullName));
//  $phone = phoneFld(REQ . t('Phone:'), [t('Your phone number')], dft($phone));
  $email = emailFld(t('Email:'), [t('Email')], dft($email));
  $zip = zipFld($zip);
  $amtChoice = selectFld(REQ . t('Donation:'), '', INDIVIDUAL_GIFT, $giftLevels);
  $amount = numFld(t('Donation:'), [t('Amount')]);
//  $permo = boxFld('permo', NBSP, t('Make this a monthly donation.'));
//  $honored = rendA('honored', textFld(BARE, [t('(optional)')]));
//  $honor = selectFld(t('Honoring:'), '', suffix($honored), $honors);
  $dft = ($qid and $co = r\acct($qid) and $co->id != CGID) ? t('for ') . $co->fullName : '';
  $comment = areaFld(t('Comment:'), '', dft($dft));
  list ($cq, $ca) = w\captcha();
  $submit = submit(t('Donate'), NBSP, '', suffix(lnk('', t('Add comment'))));;

  jsx('amtChoice');
  jsx('cc');
  return cgform(compact(ray('img title subtext fullName email zip amtChoice amount comment cq ca submit')));
}

function formCC_validate($form, &$sta) {
  extract(just('fullName email zip', $sta['input']));
  $fullName = u\normalizeCase($fullName);
  
//  if ($err = u\badPhone($phone)) return say($err, 'phone');
  if (!emailOkay($email, FALSE, '')) return;
  if ($err = u\badZip($zip)) ; // (allow international) return say($err, 'zip');

  if (!checkAmtChoice($sta)) return;
  if (badCaptcha($sta)) return;

  u\preray(compact(ray('fullName email zip')), $sta['input']);
}

function formCC_submit($form, &$sta) {
  extract($info = just('fullName email zip amount comment', $sta['input'], NULL));
  
  r\tellAdmin(t('CC donation initiated'), $info);
  unset($info['comment']); // don't send comment to PayPal

  $code = u\cryRay($info);
  $return = BASE_URL . "/cc/op=done&code=$code";
  u\EXPECT(strlen($return) <= 1024, 'cc return url too long');
  $cancel_return = str_replace('=done', '=cancel', $return);

  $cmd = '_donations';
  list ($first_name, $last_name) = u\parseName($fullName);
  $no_shipping = 1;
  $rm = 0; // 0 for GET, 2 for POST
  $business = 'info@CommonGood.earth'; // CGF_EMAIL
  $data = compact(ray('cmd business amount first_name last_name email zip return cancel_return no_shipping rm'));
//  if ($permo) $data += ray('a3 t3 p3 src', $amount, 'M', 1, 1); // t3=DWMY, p3=periods, src=1 for recur
//  if ($permo) unset($data['amount']);
  $url = 'https://www.paypal.com/donate';
  return u\postnGo($url, $data); // does not return
}

/**
 * Thank the donor and remember them.
 * @compare investinterest.inc
 */
function thank($ray) {
  global $mya;
  
  extract(just('fullName email zip amount', $ray));
  if (!$mya) $mya = r\acct($email);
    
  list ($city, $state, $latitude, $longitude) = r\zipCity($zip);
  $source = t('cc donation');

  $notes = r\stampNote(t('cc donation: $') . $amount) . ($mya ? $mya->notes : '');
  if ($mya) $fullName = $mya->fullName;
  r\tellAdmin(t('CC donation complete'), compact(ray('fullName email zip city state amount')) + ($mya ? [] : ray('NEW!', '1')));
  $msg = 'cc thanks';

  if ($mya) {
    $mya->update('notes', $notes);
  } else {
    $acctType = hidFld(CO_PERSONAL);
    $flags = u\bit(B_NONUDGE);
    $info = []; foreach (ray('fullName email flags zip notes source city state latitude longitude') as $k) if (isset($$k)) $info[$k] = $$k;
    if (!$a = new r\Acct($info)) return softErr(t('cannot create account'));
    $msg .= '|check it out';
    $a->tell($msg, [], t('Thank you for your donation!'));
  }
  return sayEmpty($msg);
}
