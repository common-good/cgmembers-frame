<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Txs as x;

include __DIR__ . '/../cg-txs.inc';

/**
 * Show form 1099-K for the previous calendar (or fiscal) year.
 * @param int $year: the relevant calendar year (defaults to previous year)
 */
function formTaxInfo($form, &$sta, $year = '') {
  global $base_url;
  global $mya;

  $y1 = date('Y', min($mya->created, $mya->jA ? $mya->jA->created : PHP_INT_MAX));
  $y9 = date('Y');
  $range = range($y1, $y9);
  if (!$year or $year < $y1 or $year > $y9) $year = max($y1, $y9 - 1); // can't use setDft because cmdline param is never null
  $ending = strtotime('1/1/' . ($year + 1)) - 1;

  $regName = r\regionfield('legalName');
  $dbaName = r\regionfield('fullName');
  if ($dbaName != $regName) $regName .= t(' dba ') . $dbaName;
  $regPhone = u\fmtPhone(r\regionfield('phone'), '');
  $posttext = item(t('<p>This report is provided by %CGF_LEGALNAME dba %PROJECT (EIN #20-5280780), %CGF_POSTALADDR, %CGF_PHONE, info@%EMAIL_DOMAIN.</p><p>If you would rather receive reports by US Mail, you can change that setting in your <%aPrefs>Preferences</a>.</p>', 'thisYear _aPrefs', date('Y'), atag('/settings/preferences')));
//  $posttext = item(t('<p>This report is provided by %regName (EIN #%R_REGION_EIN). Address: %regAddr, %regPhone, %regEmail.</p><p>If you would rather receive reports by US Mail, you can change that setting in your <%aPrefs>Preferences</a>.</p>', 'regName regAddr regPhone regEmail thisYear _aPrefs', $regName, r\regionfield('postalAddr'), $regPhone, r\regionfield('email'), date('Y'), atag('/settings/preferences')));  

  $title = t('Your Form 1099-K') . ($year == $y9 ? t(' (tentative)') : '');
  $row = be\get1099('K', $mya->id, $ending); // last second of relevant year
  extract(just('gross noCard txCount', $row, 0));
  $year = selectFld(t('For calendar year:'), [t('Calendar Year')], dft($year) + short(), array_combine($range, $range));
  $jointTxt = $mya->jid ? t(' Your partner in the joint account receives a separate 1099-K.') : '';
  $subtext = t(<<<EOF
  <p>As a Payment Settlement Entity (PSE), the %PROJECT system is required to report (to the IRS and to state government) the total %PROJECT payments made to you in the past calendar year (on Form 1099-K) &mdash; unless the total is below $%MIN1099. This page provides you with that information even if your total is below the threshold.</p>
  <p class="loud">The IRS does not require you to file Form 1099-K, but some state governments do.</p>
  <p>The purpose of 1099-K is to report payment card transaction totals, not total income. So don't worry if some of the total listed here is something other than income (such as reimbursements or selling something at a loss). Also, you may receive a Form W2 or 1099-MISC from your employer or customers for some of the same money. Take care not to over-report your income by counting things twice.</p>
  <p>This report is for %nm (account %acct).%jointTxt</p>
EOF
  , 'nm acct jointTxt', $mya->legalName, $mya->mainQid, $jointTxt);
  
  $txCount = item(number_format($txCount), t('Payments (count):'));
  $gross = item(u\fmtAmt($gross, '$'), t('Total:'));
  $noCard = item(u\fmtAmt($noCard, '$'), t('Paid Online:'));
  $withheld = item('0.00', t('Income Tax Withheld:'));
  jsx('taxinfo');

  $form = compact(ray('title year subtext txCount gross noCard withheld'));
  for ($i = 1; $i <= 12; $i++) $form["mo$i"] = item(u\fmtAmt($row["amt$i"], '$'), date("F", mktime(0, 0, 0, $i)) . ':');
  return cgform($form + compact('posttext'), '', 'left');
}

function formTaxInfo_validate($form, &$sta) {
//  if ($err = u\badDate($sta['input']['ending'])) return say($err, 'ending');
}

function formTaxInfo_submit($form, &$sta) {
//  w\go('history/tax-info/' . $sta['input']['ending']);
}
