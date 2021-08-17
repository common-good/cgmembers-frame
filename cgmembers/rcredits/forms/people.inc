<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Display contact info for a non-member, for editing by admin.
 */
function formPeople($form, &$sta, $args = '') {
  global $mya;

  extract(just('pid', $args, NULL));
  if ($pid and $orig = db\get('*', 'people', compact('pid'))) extract($orig); else $orig = [];
  
  $title = t('Contact Information');
  $displayName = textFld(t('Display Name:'), '', dft($displayName));
  $fullName = textFld(REQ . t('Full Name:'), '', dft($fullName));
  $email = emailFld(t('Email:'), '', dft($email));
  $phone = phoneFld(t('Phone:'), '', dft($phone));
  $address = textFld(t('Street Address:'), '', dft($address));
  $city = textFld(t('City:'), '', dft($city));
  $state = stateFld($state ?: R_STATE_ID);
  $zip = zipFld($zip);

  $method = w\radiosFld(t('Preferred Contact:'), '', dft($method), ray(METHOD_OPTS));
  $source = textFld(t('Source:'), '', dft($source));
  $notes = areaFld(t('Notes:'), '', dft($notes));
  
  $submit = submit();
  $orig = hidFlds($orig);
  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost

  $form = compact(ray('title displayName fullName email phone address city state zip method source notes submit orig'));
        
  jsx('contact');

  return cgform($form);
}

function formPeople_validate($form, &$sta) {
  global $mya;

  extract(u\normalizeCase(just($normals = 'fullName city address', $sta['input'])));
  extract(just('email phone zip orig', $sta['input']));
//  extract(just('fullName', hidFlds($orig)), EXTR_PREFIX_ALL, 'o');

  if ($err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');

  if ($err = u\badZip($zip)) return say($err, 'zip');
  if (!emailOkay($email, $mya->co, $mya)) return;
  if ($err = u\badPhone($phone)) return say($err, 'phone');
//  if (u\poAddress($address)) return say('po in location', 'address'); // address can be either physical or postal

  u\preray(compact(ray($normals . ' email phone')), $sta['input']);
}

function formPeople_submit($form, &$sta) {
  global $mya;
  global $partnerInfo; // information used within eachPartner function argument (so must be global)
  extract($info = just('displayName fullName email phone address city state zip method source notes orig', $sta['input'], NULL));
  extract(just('pid address city state zip', hidFlds($orig), ''), EXTR_PREFIX_ALL, 'o');
  if ($pid) $info += compact('pid');
  
  if ("$address $city $state $zip" != "$o_address $o_city $o_state $o_zip") $info += ray('latitude longitude', 0, 0); // regeolocate
  
  db\updateOrInsert('people', $info, 'pid');
  say('info saved');
}