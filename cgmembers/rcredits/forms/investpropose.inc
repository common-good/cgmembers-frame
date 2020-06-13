<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

define('INVEST_COFLDS', 'fullName city serviceArea dob gross'); // investment flds in users table
define('INVEST_FLDS', 'clubid investment offering price return types terms assets character strength web history soundness'); // investment flds in r_investments
define('INVEST_ALLFLDS', 'company ' . INVEST_COFLDS . ' bizCats ' . INVEST_FLDS);

/**
 * Propose a new Investment.
 */
function formInvestPropose($form, &$sta, $args = '') {
  global $mya;
  
  $title = t('Propose a New Investment');
  $subtext = t('Complete this form if you have been assigned by your Investment Club steering committee to research the soundness of a proposed investment. <%a>Here is a printable worksheet</a> you can use to gather the necessary information.', '_a', atag('http://cg4.us/doc/investment-worksheet.pdf'));
  $company = textFld(REQ . t('Company:'), [t('Company to invest in'), t('Choose a %PROJECT member company (or type its account ID) or <%a>click here</a> to add them.', '_a', 'a id="add-co"')]);
  $fullName = textFld(t('Company:'), [t('The company\'s full name')]);
  $city = textFld(t('City:'), [t('City where company is located')]);
  $serviceArea = textFld(t('Service area:'), [t('Company service area')]);
  $dob = dateFld(t('Founded:'), [t('Founding date'), t('When did this company begin, approximately?')]);
  $gross = numFld(t('Annual Gross:'), [t('Gross income')]);
  $bizCats = selectFld(t('Biz categories:'), t('Hold down the Ctrl key to select more than one'), ['multiple' => TRUE], coCats());

  $investment = areaFld(REQ . t('Investment:'), [t('Description of investment/loan')]);
  
  $equity = boolFld(t('Equity:'), t('Is this an equity investment (as opposed to a loan)? Answer Yes if the interest rate depends on the business\'s success (regardless of whether the club will own part of the company).'), TRUE);
  
  // equity set
  $offering = numFld(REQ . t('Offering size:'), [t('Offering size'), t('Total size of offering, in dollars')]);
  $price = numFld(REQ . t('Share price:'), [t('Price per share')]);
  $return = numFld(REQ . t('Return:'), [t('Predicted APR')] + step(.001) + suffix('%'));
//  $types = textFld(REQ . t('Types:'), [t('Type codes'), t('D=dividends I=interest T=tax-exempt interest')]);
  $equitySet = fieldSet('equitySet', compact(ray('offering price return')));

  // loan set
  $offering = numFld(REQ . t('Target:'), [t('To be borrowed'), t('How much is the company borrowing from all lenders, in dollars')]);
  $price = hidFld(1);
  $return = numFld(REQ . t('Interest Rate:'), [t('Interest rate')] + step(.001) + suffix('%'));
  $exempt = boolFld(t('Exempt:'), t('Is the interest tax-exempt?'));
  $loanSet = fieldSet('loanSet', compact(ray('offering price return exempt')));
//  $types = textFld(REQ . t('Types:'), [t('Type codes'), t('D=dividends I=interest T=tax-exempt interest')]);
  
  $terms = areaFld(REQ . t('Terms:'), [t('Terms of the investment/loan'), t('Minimum investment/loan, minimum length of time, and any other details about the deal.')]);
  $assets = numFld(REQ . t('Company assets:'), [t('Company assets'), t('Company assets, bond, or collateral (dollar value)')]);
  $character = textFld(REQ . t('Owner character:'), [t('Character of owners'), t('What is your assessment of the integrity and determination of the owners')]);
  $strength = intFld(REQ . t('Strength:'), [t('Financial strength'), t('Rate the company\'s financial strength, based on quality and content of financial statements (0 to 100)')], vmax(100));
  $web = intFld(REQ . t('Web presence:'), [t('Web presence'), t('How impressive does the company look on Internet buzz (0 to 100)')], vmax(100));
  $history = intFld(REQ . t('History:'), [t('Repayment history'), t('Rate the company\'s past success repaying loans and investments (0 to 100, where 50 means no history)')], vmax(100));
  $soundness = intFld(REQ . t('Soundness:'), [t('Overall soundness'), t('Overall, how sound is this investment (0-100)')], vmax(100));

  $submit = t('Propose');

  w\whoFldSubmit('company', t('Propose this investment in %name?'), ray('coOnly', TRUE));
  
  jsx('invest-proposal');
  
  return cgform(compact(ray('title subtext company fullName city serviceArea dob gross bizCats investment equity equitySet loanSet terms assets character strength web history soundness submit')));
}

function formInvestPropose_validate($form, &$sta) {
  global $mya; $clubid = $mya->id;
  extract($info = just(INVEST_ALLFLDS, $sta['input'], NULL));

  if ($company) {
    if (!$a = whois($company, 'company', $info, t('no self-investing'))) return FALSE;
    $company = $a->id;
  } else {
    if ($err = u\badDate($dob, '', now())) return say($err, 'dob');
    if ($err = u\badAmount($gross, '>=0', 2, '')) return sayFieldErr($err, 'gross');
  }
  $toRate = 'strength web history soundness';
  foreach (ray('offering price return assets ' . $toRate) as $k) {
    $max = in($$k, $toRate) ? 100 : ''; // '' not NULL (no limit)
    if ($err = u\badAmount($$k, '>=0', 2, $max)) return sayFieldErr($err, $k);
  }
  $types = ''; // not actually used until submit
  
  foreach (ray(INVEST_ALLFLDS) as $k) if (isset($$k)) $sta['input'][$k] = $$k; // update changed fields
}

function formInvestPropose_submit($form, &$sta) {
  global $mya; 
  
  extract(just(INVEST_ALLFLDS . ' equity exempt', $sta['input'], NULL));
  list ($clubid, $clubqid) = [$mya->id, $mya->mainQid];
  $types = $equity ? 'D' : 'I';
  if ($exempt) $types .= 'T';
  
  if ($fullName) {
    $a = new r\Acct(compact(ray(INVEST_COFLDS)));
    $a->setCategories($bizCats);
  } else $a = r\acct($company);
  list ($coid, $proposedBy) = [$a->id, $mya->agentId];
  $return /= 100; // convert % to fraction
  
  db\insert('r_investments', compact(ray('coid proposedBy ' . INVEST_FLDS)), 'vestid');
  w\go("/invest/list/clubqid=$clubqid", t('investment proposal saved'));
}
