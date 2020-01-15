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

  $myCats = db\q('SELECT iid FROM r_user_industries WHERE uid=:myid', compact('myid'))->fetchCol();
  $links = t('<%a>See your public %PROJECT web page</a> (in a new window)', 
    '_a', atag("/member/$mya->name", ['id' => 'show-webpage'] + w\away()));
  $cgbutton = item(t('Click to create a <%aSimple>simple CGPay button</a> for your website or use this code &mdash; %code &mdash; for an <%aAdvanced>advanced CGPay button</a>.', 'code _aSimple _aAdvanced', $mya->emailCode, atag('/cgbutton'), atag('https://docs.google.com/document/d/1QpOW0WU7RiUV900PMekyH7ve3-_9yjCnQn627cWovOQ')), t('CGPay Button:'));

  $canDescs = ray(APP_CAN_BIT_DESCS);
  $permissions = '';
  if (!$mya->stepDone['company'] and !$mya->coCan(APP_CANS + APP_CAN_BITS)) foreach (ray(CO_DFTS) as $i) $mya->setCoBit(APP_CANS + $i); // set defaults if doing initial account setup AND manager has no permission to charge custs yet
  
  foreach ($canDescs as $i => $desc) {
    $outbox = <<<EOF
<div class="form-item form-type-checkbox">
  <input type="checkbox" name="can[$i]" CHECKED class="form-checkbox" />
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

  $title = t('Company Information');
  $links = item($links, ' ');
  $name = item($mya->fullName, t('Company name:'));
  $photo = item(t('<%a>Change your company photo</a>', '_a', atag('/settings/photo?' . rand())), t('Photo:'));
  $private = boxFld('private', t('Private:'), t('Hide this company from the public (omit from public listings)'), $mya->coCan(CO_PRIVATE) ?: 0);
  $categories = selectFld(t('Categories:'), t('Hold down the Ctrl key to select more than one'), ['multiple' => TRUE] + dft($myCats), coCats());
  $selling = w\sellingFld($mya->selling);
  $can2 = item($permissions, t('App permissions:'), t('Download the CGPay app from Google Play Store. Limit app permissions here, or go to <%aRel>Relations</a> settings to limit them for each agent.', '_aRel', atag('/settings/relations')));
//    $shareRpos = boolFld(t('Share rPOS:'), t('Allow other companies to use our POS device(s).'), $mya->coCan(CO_SHARE_RPOS) ?: 0);
//    $requireCashier = boolFld(t('Must scan in:'), t('Require a cashier to sign in before using rPOS (this also allows you to share the device with another company).'), $mya->coCan(CO_REQUIRE_CASHIER) ?: 0);

  $dob = $mya->hasDob ? NULL : ssnFields(FALSE, required())['dob'];
  $website = textFld(t('Website:'), [t('Website address'), t('Especially if you want to accept payments on your website, be sure to list it here.')], dft($mya->website));
  $descPreface = item(t('<br>What does the company do? This description will appear on your %PROJECT web page.')); //, '', ['class'=>'help-block']);
//  $description = fld('text_format', t('Description:'), [t('Arbitrarily complex description')], ['format' => 'filtered_html'] + dft(strtr($mya->description, ["\r" => '', "\n" => '', '<br>' => PHP_EOL, "\0" => ''])));
  $description = areaFld(t('Description:'), t('Arbitrarily complex description, using <%a>markdown</a>. Use "&#92;" for line breaks.', '_a', atag('http://commonmark.org/help/')), dft($mya->description));
  $shortDesc = textFld(t('Short Desc:'), [t('Short description'), t('Short description of your company\'s products and services. Maximum 35 characters.')], attrib(['maxlength'=>35]) + required($mya->shortDesc));
  $employees = textFld(t('Employees:'), [t('Employee count'), t('How many employees do you have?')], required($mya->employees));
  $gross = textFld(t('Annual Gross:'), [t('Approximate annual gross income')], required($mya->gross));
  $staleNudge = textFld(t('Nudge Every:'), [t('Number of days between nudges'), t('Wait this many days before nudging a customer to pay an invoice you sent them (zero means never send a reminder).')], required($mya->staleNudge ?: STALE_NUDGE));
  $tips = boolFld(t('Tips:'), t('After each %PROJECT card sale, ask the customer to choose a tip percentage (or no tip)'), $mya->coCan(CO_TIP) ?: 0);
  if ($mya->cAdmin) {
    if (FALSE) $fast = boolFld(t('No ID Chk:'), t('Never ask customer for photo ID (set by admin)'), $mya->coCan(CO_FAST) ?: 0); // no exemptions
    $atm = boolFld(t('ATM service:'), t('Allow company to trade %RCREDITS for cash even if their balance is less than their rewards (in-person only).'), $mya->coCan(CO_ATM) ?: 0);
  }
//  $submit = submit();
  return cgform(@compact(ray('title links cgbutton photo name private categories selling shortDesc employees gross dob website descPreface description can2 staleNudge tips fast atm')) + w\setupFoot(''));
}

function formCompany_validate($form, &$sta) {
  $fields = 'selling employees gross staleNudge';
  extract(just("$fields dob", $sta['input']));
  if ($err = zBadSelling($selling)) return say($err, 'selling');
  if ($err = u\badAmount($employees, '>=0', 0)) return sayFieldErr($err, 'employees');
  if ($err = u\badAmount($gross, '', 2, '')) return sayFieldErr($err, 'gross');
  if ($err = u\badAmount($staleNudge, '>=0', 0, 365)) return sayFieldErr($err, 'staleNudge');
  if (nn($dob) and !formBasic_validate($form, $sta, FALSE, 'dob')) return; // validate internalizes dob

  u\preray(compact(ray($fields)), $sta['input']);
}

function formCompany_submit($form, &$sta) {
  global $mya;

  extract($info = just('private description shortDesc website selling categories can requireCashier employees gross staleNudge dob tips fast atm', $sta['input']));
  
  $website = preg_replace('~https?://~i', '', @$website);

  $mya->setCoCans(@$can);
  //$mya->setBits(APP_CAN_BITS, $can, 'co');
// (keep this) $mya->setBit(CO_REQUIRE_CASHIER, @$requireCashier, 'coFlags');
  $mya->setCoBit(CO_PRIVATE, @$private);
  $mya->setCoBit(CO_TIP, $tips);
  if ($mya->cAdmin) $mya->setCoBit(CO_FAST, @$fast);
  if ($mya->cAdmin) $mya->setCoBit(CO_ATM, $atm);
  $mya->update(@compact(ray('description shortDesc website selling employees gross staleNudge dob')));
  
  if (@$categories) $mya->setCategories($categories);

//  $mya->stepDone('company', $info);
//  return $mya->ok ? say('options saved') : w\go('status', 'options saved|step completed');
  return w\goNextStep('company', NULL, $info);
}
