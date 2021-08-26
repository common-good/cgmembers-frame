<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const PEOPLE_FLDS = 'displayName fullName email phone address city state zip method source notes'; // just the fields we handle here (and omitting pid)
const PEOPLE_URL = 'sadmin/people/pid';

/**
 * Display contact info for a non-member, for editing by admin.
 */
function formPeople($form, &$sta, $args = '') {
  global $mya;

  extract(crud('people', $args, PEOPLE_URL, PEOPLE_FLDS)); // get ret or [title, orig, id, and PEOPLE_FLDS]
  if (nn($ret)) return $ret;

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

  $form = compact(ray('title ' . PEOPLE_FLDS . ' submit orig'));
        
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
  if ($email and !emailOkay($email, $mya->co, $mya)) return;
  if ($phone and $err = u\badPhone($phone)) return say($err, 'phone');
//  if (u\poAddress($address)) return say('po in location', 'address'); // address can be either physical or postal

  u\preray(compact(ray($normals . ' email phone')), $sta['input']);
}

function formPeople_submit($form, &$sta) {
  global $mya;
  extract($info = just(PEOPLE_FLDS, $sta['input'], NULL));
  extract(just('pid address city state zip notices', hidFlds($sta, 'orig'), ''), EXTR_PREFIX_ALL, 'o');
  if ($o_pid) $info['pid'] = $o_pid;
  
  if ("$address $city $state $zip" != "$o_address $o_city $o_state $o_zip") $info += ray('latitude longitude', 0, 0); // regeolocate
  $info['method'] = ray(METHOD_OPTS)[$method];
  if (!$o_notices) $info['notices'] = NOTICE_DFTS;
  
  db\updateOrInsert('people', $info, 'pid');
  say('info saved');
  return go(PEOPLE_URL);
}
