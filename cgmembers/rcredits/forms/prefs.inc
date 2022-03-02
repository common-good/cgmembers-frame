<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

function formPrefs($form, &$sta) {
  global $base_url;
  global $mya;
  $title = t('Account Preferences');

  if (!$mya->co) {
    $roundup = w\roundupFld($mya);
    $list = 'CHIP, CNP, EAEDC, EITC, Fuel Assistance, Medicaid, LIHEAP, Section 8, SNAP, SSP, TAFDC, TANF, WIC';
    if ($mya->st == 'MA') $list .= ', HIP';
//    if ($mya->st == 'VT') $list .= ', XXX';
    $snap = boolFld(t('Food Assistance?'), t('If you struggle to afford healthy food, consider answering "Yes" here. This is a request for financial assistance from other %PROJECT members in your community. Your answer is private &mdash; only program administrators can tell who is receiving this assistance. Typically answer "Yes" if you are eligible for any of the following (whether or not you have actually applied): %list. (Food assistance may not yet be available in your area.)', compact('list')), $mya->snap);
  } else { $snap = $roundup = NULL; }
  
  $backing0 = u\fmtAmt(($mya->backing + 0) ?: 0); // don't default to 1 except on backing page
  $backingLink = lnk('/community/backing', t('change'));
  $backingEnd = $mya->backing > 0 ? t(' until ') . fmtDt($mya->backingDate + YEAR_SECS) : '';
  $backing = item($backing0, t('Backing:'), t('Backing allows you and your community to make grants. For the next 12 months, if there is a cash crunch, you agree to buy this much more %PROJECT credit. If the system fails utterly in that time, your %backing0 credit becomes a contribution.', compact('backing0')), suffix(" $backingEnd <small>[$backingLink]</small>"));
  
  $mediumRadios = array(
    t('I will accept electronic statements**.'), 
    t('I choose to receive printed statements by mail. <div>I agree to pay $%R_STATEMENT_COST per month to cover the postage and handling costs for this service.</div>')
  );
  $statements = radiosFld(t('Statements:'), '', dft((int) $mya->paper), $mediumRadios);

  $showAdvancet = item(t('<a>Show advanced options</a>'), NBSP); // different name from showAdvanced (for CSS)

  // Advanced settings
//  if ($mya->ok and $mya->share) $share = w\shareFld($mya); // don't show this during signup (it's on the donation page)

  $noticeRadios = array(t('daily'), t('weekly'), t('monthly'));

  $secretBal = boolFld(t('Secret Balance:'), t("Don't let merchants see my balance, even if I ask."), $mya->secret);
  // Double confirmation: by SMS, by email, either. require extra confirmation of crucial account changes
  $nosearch = $mya->co ? NULL : boolFld(t('No Search:'), t("Let members identify my account only by phone, email, or account ID &mdash; not by name."), $mya->nosearch);
  
  $desc = t('Use your %PROJECT card as a <i>credit</i> card, allowing your account balance to go negative, limited by your credit limit (currently %limit), based on your average monthly account activity. When you use your credit line  (that is, when your balance goes negative) it is a short-term loan from your %PROJECT community. <span class="loud">In choosing this option, you agree to bring your balance back up above zero within 30 days.</span>', 'limit', u\fmtAmt(-$mya->floor));
  if ((!$mya->hasBank or !$mya->refill) and !$mya->can(B_DEBT)) {
    $desc .= t(' <b>NOTE:</b> This setting is disabled unless you <%a>choose automatic refills from a bank account</a>, with a target balance.', '_a', atag('/settings/fund'));
    $extra = disabled() + suffix(t(' (see <b>NOTE</b>)'));
  } else list ($desc, $extra) = ['', []];
  
  $debtOk = $mya->underage ? NULL : boolFld(t('Debt Okay:'), $desc, $mya->can(B_DEBT), NULL, $extra);
  
  $posttext = item(t(<<<EOF
<h4>** Disclaimers required by the IRS:</h4>
<p>Your choice of how to receive statements applies to all statements you receive from now on. You can change your preference at any time by returning to this Preferences page (on the Settings menu) or by notifying the Regional Administrator by mail: %regName, %regAddr. Any change will be confirmed in writing (electronically if you accept electronic statements, otherwise by US Mail). You may also ask the Regional Administrator for a paper copy without changing your preference for electronic statements in the future.</p>
<p>To view your electronic statements, you need a typical computer or mobile device connected to the internet, including an email program and a web browser. To print the statements, you need a printer.</p>
<p>Your annual tax statements will be accessible online from January through December.</p>
EOF
  , ray('regName regAddr', r\regionfield('legalName'), r\regionfield('postalAddr'))));

  $advancedFields = 'achMin secretBal nosearch food backing';
  
//  $submit = submit(t('Update Preferences'));
  

  jsx('advanced-prefs', compact('advancedFields'));
  $info = []; foreach (ray("title roundup snap debtOk statements showAdvancet $advancedFields") as $k) if (isset($$k)) $info[$k] = $$k;
  return cgform($info + w\setupFoot() + compact('posttext'));
}

function formPrefs_validate($form, &$sta) {
  extract(just('food snap', $sta['input'], NULL));

  $food = $food ?: 0;
  if ($err = u\badAmount($food, '>=0', 1, 100)) return say($err, 'food');
  snapErr($snap); // just to remove punctuation
/// (no SNAP checking for now)  if ($snap and $err = snapErr($snap)) return say($err, 'snap');
  u\preray(compact(ray('food snap')), $sta['input']); // update trimmed percentage
}

function formPrefs_submit($form, $sta) {
  include_once __DIR__ . '/../admin/admin.inc';
  global $mya;

  extract(just('food snap roundup statements nosearch debtOk secretBal', $sta['input'], NULL));
  $fields = 'food snap';
  $food /= 100;

  foreach (ray($fields) as $k) $old["_old $k"] = $mya->$k;
  $old += ['_old flags' => u\bits($mya->flags)];
  $bits = ray('roundup paper secret debt nosearch', $roundup, $statements, $secretBal, $debtOk, $nosearch);
  $info = compact(ray($fields)) + $bits;

  if ($statements) r\tellAdmin(t('%who wants paper statements', 'who', $mya->fullName), ['topic' => t('paper')]);
  if ($snap) r\tellAdmin(t('%who has SNAP', 'who', $mya->fullName), ['topic' => t('SNAP')]);
  
  $mya->update($info);
  return w\goNextStep('preferences');
}

/**
 * Validate a SNAP number.
 * @param string $snap: the snap number as typed by the user
 *                      (RETURNED) just the relevant characters (digits and letters)
 * @return the error message if the number is invalid, else return FALSE.
 */
function snapErr(&$snap) {
  $snap = preg_replace('/[^A-Z\\d]/i', '', $snap);
  $c1 = strlen($snap)? $snap[0] : '';
  if (!preg_match('/(37|04|06|08|91|92|93|94|96)(\\d{3})(\\d{2})([A-Z\\d]\\d{2,5})/i', $snap, $m) or ($c1 == '9' and $m[3] != '22')) return t('That is not a valid SNAP number.');
  return FALSE;
}