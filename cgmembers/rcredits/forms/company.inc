<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

function formCompany($form, &$sta, $arg = '') {
  global $base_url;
  global $mya;
  $myid = $mya->id;
//  if ($arg == 'gotPhoto') say('got photo');

  $selfServe = r\cook('selfServe') ? 0 : 1;
  jsx('company', compact(ray('selfServe')));
  
  $myCats = db\q('SELECT iid FROM r_user_industries WHERE uid=:myid', compact('myid'))->fetchCol();
  $links = item(join('<br>', array_filter([ // array_filter to omit null entries
    lnk("/member/$mya->name", t('See your public %PROJECT web page'), w\away()),
    lnk('/co/cgbutton', t('Create a CGPay button for your website (or emails)')),
    lnk('/settings/photo?' . rand(), t('Change your company photo')),
    lnk('/settings/dft-acct', t('%doWhat to this account upon sign-in', 'doWhat', r\cook('dftAcct') == $mya->mainQid ? t('Stop defaulting') : t('Default'))),
    r\cook('scanner') ? lnk('', $selfServe ? t('Set this device to self-serve mode') : t('End self-serve mode on this device'), clas('selfServe')) : NULL,
  ])), t('Shortcuts:') );

  $canDescs = ray(APP_CAN_BIT_DESCS);
  $permissions = '';
  if (!$mya->stepIsDone('company') and !$mya->coCan(APP_CANS + APP_CAN_BITS)) $mya->setDftAppCans(); // set defaults if doing initial account setup AND manager has no permission to charge custs yet
  
  foreach ($canDescs as $i => $desc) {
    $outbox = <<<EOF
<div class="form-item form-type-checkbox">
  <input type="checkbox" name="can[$i]" CHECKED class="form-checkbox">
  <div class="description"><div class="box"></div></div>
</div>
EOF;
    $j = $i + APP_CAN_BITS; // convert signed out to signed in index
    $inbox = str_replace("can[$i]", "can[$j]", $outbox);
    if (!$mya->coCan(APP_CANS + $i)) $outbox = str_replace('CHECKED ', '', $outbox); // uncheck if not set
    if (!$mya->coCan(APP_CANS + $i + APP_CAN_BITS)) $inbox = str_replace('CHECKED ', '', $inbox);
    $permissions .= "<tr><th>$desc</th><td>$inbox</td><td>$outbox</td></tr>\n";
  }

  $permissions = <<<EOF
<div id="permissions"><table>
<tr id="perm-head"><td></td><th>Signed IN</th><th>Signed OUT</th></tr>
$permissions
</table></div>
EOF;

  $title = t('Company Settings');
  $name = item($mya->fullName, t('Name:'));
  $private = boxFld('private', t('Private:'), t('Hide this company from the public (omit from public listings)'), $mya->coCan(CO_PRIVATE) ?: 0);
  $categories = selectFld(t('Categories:'), t('Hold down the Ctrl key to select more than one'), ray('multiple size', TRUE, 8) + dft($myCats), coCats());
  $selling = w\sellingFld($mya->selling);
//  $can2 = item($permissions, t('App permissions:'), t('Download the CGPay app from Google Play Store. Limit app permissions here, or go to <%aRel>Relations</a> settings to limit them for each agent.', '_aRel', atag('/settings/relations')));
//    $shareRpos = boolFld(t('Share rPOS:'), t('Allow other companies to use our POS device(s).'), $mya->coCan(CO_SHARE_RPOS) ?: 0);
//    $requireCashier = boolFld(t('Must scan in:'), t('Require a cashier to sign in before using rPOS (this also allows you to share the device with another company).'), $mya->coCan(CO_REQUIRE_CASHIER) ?: 0);

  $founded = foundedFld($mya->founded, REQ);
  $website = textFld(t('Website:'), [t('Website address'), t('Especially if you want to accept payments on your website, be sure to list it here.')], dft($mya->website));
  $logo = textFld(t('Logo URL:'), [t('Logo web address')], $mya->hasLogo ? suffix(logoHtml($mya)) : []);
  $descPreface = item(t('<br>What does the company do? This description will appear on your %PROJECT web page (optional).')); //, '', ['class'=>'help-block']);
//  $description = fld('text_format', t('Description:'), [t('Arbitrarily complex description')], ['format' => 'filtered_html'] + dft(strtr($mya->description, ["\r" => '', "\n" => '', '<br>' => PHP_EOL, "\0" => ''])));
  $description = areaFld(t('Describe:'), t('Arbitrarily complex description, using <%a>markdown</a>. Use "&#92;" for line breaks.', '_a', atag('http://commonmark.org/help/')), dft($mya->description));
  $shortDesc = textFld(REQ . t('Short Desc:'), [t('Short description'), t('Short description of your company\'s products and services, all lowercase. Maximum 35 characters.')], maxlen(35) + dft($mya->shortDesc));
  $employees = intFld(t('Employees:'), [t('Employee count'), t('How many employees do you have?')], required($mya->employees));
  $gross = numFld(t('Annual Gross:'), [t('Approximate annual gross income')], required($mya->gross));
  $staleNudge = intFld(t('Nudge Every:'), [t('Number of days between nudges'), t('Wait this many days before nudging a customer to pay an invoice you sent them (zero means never send a reminder).')], required($mya->staleNudge ?: STALE_NUDGE));
  $giftThanks = areaFld(t('Gift Thanks:'), t('Arbitrarily complex additional text at the bottom of automatic emails thanking donors. Use <%a>markdown</a> with "&#92;" for line breaks.', '_a', atag('http://commonmark.org/help/')), dft($mya->giftThanks));
//  $tips = boolFld(t('Tips:'), t('After each %PROJECT card sale, ask the customer to choose a tip percentage (or no tip)'), $mya->coCan(CO_TIP) ?: 0);
  if ($mya->admManageAccts) {
    $fast = NULL; // boolFld(t('No ID Chk:'), t('Never ask customer for photo ID (set by admin)'), $mya->coCan(CO_FAST) ?: 0); // no exemptions
//    $atm = boolFld(t('ATM service:'), t('Allow company to trade %RCREDITS for cash even if their balance is less than their rewards (in-person only).'), $mya->coCan(CO_ATM) ?: 0);
  } else { $fast = $atm = NULL; }
  
  $showAdvanced = item(btn('', t('Advanced Settings'), 'default'), NBSP);
  $advanced = fieldSet(NBSP, compact(ray('logo descPreface description staleNudge giftThanks')));

  return cgform(compact(ray('title links name private categories selling shortDesc employees gross website founded showAdvanced advanced')) + w\setupFoot(''));
}

function formCompany_validate($form, &$sta) {
  global $mya;
  extract(just($fields = 'selling employees gross staleNudge website logo founded', $sta['input'], NULL));

  if ($err = zBadSelling($selling)) return say($err, 'selling');
  if ($err = u\badAmount($employees, '>=0', 0)) return sayFieldErr($err, 'employees');
  if ($err = u\badAmount($gross, '', 2, '')) return sayFieldErr($err, 'gross');
  if ($err = u\badAmount($staleNudge, '>=0', 0, 365)) return sayFieldErr($err, 'staleNudge');
  if ($err = u\badDob($founded, TRUE)) return say($err, 'founded');

  $website = preg_replace('~https?://~i', '', nn($website));
  if ($website and !u\urlExists("https://$website")) return sayFieldErr(t('That website URL does not exist.'), 'website'); // disabled until external file_get_contents debugged    
  if ($logo) {
    if (!is_dir(LOGO_DIR)) mkdir(LOGO_DIR, 0755) or u\FAIL('cannot create photo ID directory'); // required for tests
    if (!preg_match('~https?://~i', $logo)) $logo = 'https://' . $logo;
    if (!u\copyFile($logo, LOGO_DIR . '/' . $mya->mainQid)) return sayFieldErr(t('That logo URL does not exist.'), 'logo');
    $logo = '';
  }
  
  u\preray(compact(ray($fields)), $sta['input']);
}

function formCompany_submit($form, &$sta) {
  global $mya;

  extract($rawInfo = just('private description shortDesc website logo selling categories requireCashier employees gross staleNudge giftThanks founded', $sta['input'], NULL));
  
//  $mya->setCoCans(nn($can));
  $mya->setCoBit(CO_PRIVATE, nn($private));
//  $mya->setCoBit(CO_TIP, $tips);
//  if ($mya->admManageAccts) $mya->setCoBit(CO_FAST, nn($fast));
//  if ($mya->admManageAccts) $mya->setCoBit(CO_ATM, nn($atm));
  $info = []; foreach (ray('description shortDesc website logo selling employees gross staleNudge giftThanks founded') as $k) if (isset($$k)) $info[$k] = $$k;
  $mya->update($info);
  if (nn($categories)) $mya->setCategories($categories);
  return w\goNextStep('company', NULL, $rawInfo);
}
