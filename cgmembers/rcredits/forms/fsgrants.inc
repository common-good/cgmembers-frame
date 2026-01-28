<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const FSGRANTS_FLDS = 'id uid pid xid amount by ckNum ckDate documented received created'; // just the fields we handle here (and omitting pid)
require_once R_ROOT . '/forms/tx.inc'; // for txErr()
require_once R_ROOT . '/forms/people.inc'; // for PEOPLE_FLDS

/**
 * List a sponsored partner's expected grants (or for all partners) and provide a form for entering more.
 */
function formFSGrants($form, &$sta, $args = '') {
  global $mya; $myid = $mya->id;
  
  $args = justNOT('crit', urlq2ray($args)); // prevent code injection (redundantly); need both urlq2ray() and just()
  $seeAll = strhas($_SERVER['REQUEST_URI'], '/fsgrants');
  if (!$seeAll) $args['crit'] = "uid=$myid";

//  $order = tr('amount<%UNDOC_GRANT_MAX OR documented>0, -IFNULL(received, %PHP_INT_MAX)');
  extract($zot = crud('grants', $args, 'id', FSGRANTS_FLDS, 'created DESC')); // get ret or [id, title, orig, url]
  if (nn($ret)) return $ret;
  
  if ($pid) {
    extract(just(PEOPLE_FLDS, $pRec = db\row('people', compact('pid'))));
  } else foreach (ray(PEOPLE_FLDS) as $k) $$k = '';

  $title = t('Expected Grants');
  $subtext = t('Please complete as much information as you can for the grantor. For grants of %max or more, we also need a copy of the grant agreement.', ray('max', u\fmtAmt(UNDOC_GRANT_MAX))) . '<br>' . NBSP;

  $fullName = textFld(REQ . t('Grantor Name:'), [t('Grantor name properly capitalized')], dft($fullName));
  $email = emailFld(t('Email:'), [t('Email address')], dft($email));
  $phone = phoneFld(t('Phone:'), [t('Phone number')], dft($phone));
  $address = textFld(REQ . t('Postal Addr:'), [t('Number and street')], dft($address));
  $city = textFld(REQ . t('City:'), [t('Postal city')], dft($city));
  $state = stateFld($state);
  $zip = zipFld($zip);
  $methods = ray(METHOD_OPTS);

  $amount = numFld(REQ . t('Expected Amount:'), [t('Amount')], dft($amount) + vmin(.01));

  $opts = ray(tr(PAYBY_OPTS));
  $optNums = array_flip($opts);
  $by = radiosFld(REQ . t('Method:'), '', dft($by ? $optNums[$by] : B_ACH), $opts);
  
  $ckNum = $mya->admin ? textFld(t('Ck Number:'), '', dft($ckNum)) : hidFld($ckNum);
  $ckDate = $mya->admin ? dateFld(t('Ck Date:'), '', dft($ckDate)) : hidFld($ckDate);

  if ($received) {
    $got = item(fmtDt($received), t('Received:'));
  } else $got = $mya->admin ? boxFld('got', t('Received:'), '', FALSE) : NULL;
  
  $documented = dateFld(t('Documented:'), t('Date the sponsored organization emailed a copy of the grant agreement to %PROJECT'), dft($documented));
  
  $submit = submit();
  $orig = hidFlds($orig);
  foreach (ray('url seeAll received') as $k) $$k = hidFld($$k);
  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost

  $form = compact(ray('title subtext fullName email phone address city state zip amount got by ckNum ckDate documented received submit orig url seeAll'));
        
  jsx('contact');
  jsx('fsgrants', ray('byCheck undocMax', B_CHECK, UNDOC_GRANT_MAX));

  return cgform($form);
}

function formFSGrants_validate($form, &$sta) {
  global $mya;

  $info = just('amount purpose fullName address city state zip phone email by ckNum ckDate documented received seeAll', $sta['input'], NULL);
  extract($info);

  $orig = hidFlds($sta, 'orig');
  if ($seeAll and !$orig['uid']) return txErr(t('To add a grant record, go to the organization account and choose Expected Grants on the company menu.'));

  if ($err = u\badName($fullName)) return txErr($err, ['field' => 'fullName'], 'fullName');
  if ($zip and $err = u\badZip($zip)) return txErr($err, 'zip');
  if ($phone and $err = u\badPhone($phone)) return txErr($err, 'phone');
  if ($email and !u\validEmail($email)) return txErr('bad email', 'email');
  if ($err = u\badAmount($amount, '>0')) return txErr($err, 'amount');

  foreach (ray('ckDate documented') as $k) if (!$$k) $$k = NULL; elseif ($err = u\badDate($$k)) return txErr("$k: $err", $k);
  
  if ($received and !$by) return txErr(t('You must specify a payment method.'));
  if ($received and $by == B_CHECK and !($ckDate and $ckNum)) return txErr(t('You must provide check number and date.'));
  
  u\preray(compact(ray('fullName zip phone ckDate received documented amount')), $sta['input']);
}

function formFSGrants_submit($form, &$sta) {
  global $mya;

  $orig = hidFlds($sta, 'orig');
  $input = $sta['input'] + just(FSGRANTS_FLDS, $orig, NULL); // new values trump orig values

  extract($new = just(FSGRANTS_FLDS . ' got url fullName uid', $input, NULL));
  $received0 = $received;
  if ($got) $received = now();

  $cat = [NULL, r\nick2cat('D-FBO')];
  [$paying, $purpose, $isGift] = [FALSE, t('grant'), TRUE];
  $doTx = (!$xid and $received and ($documented or $amount < UNDOC_GRANT_MAX)); // time to credit the account
  $uid = $orig['uid'] ?: $mya->id;
  foreach (ray('documented received ckDate') as $k) if (!$$k) $$k = $orig[$k] ?: NULL; // redundant, but doing it in _validate fails for <input>

  $DBTX = \db_transaction();
  $xid0 = $xid;
  [$pid, $xid] = be\fboTx($input + compact(ray('cat paying purpose isGift')), r\acct($uid), $doTx);
  if ($xid) {
    if ($doTx) say('funds distributed');
  } else $xid = $xid0; // don't lose existing transaction record

  $by = ray(tr(PAYBY_OPTS))[$by];
  $created = $orig['created'] ?: now();
  if ($received and !$received0 and !$documented and $amount >= UNDOC_GRANT_MAX) {
    $a = r\acct($uid);
    [$grantorName, $fullName, $amt] = [$fullName, $a->fullName, u\fmtAmt($amount)];
    $a->tell('grantdoc-required', compact(ray('fullName grantorName amt')));
    say('sponsee notified');
  }

  db\updateOrInsert('grants', compact(ray(FSGRANTS_FLDS)), 'id');
  say('info saved');
  unset($DBTX);

  return go($url);
}
