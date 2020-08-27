<?php
namespace CG\Web;
use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Web as w;
use CG\Vote as v;

include_once R_ROOT . '/rvote/rvote.inc';
include_once R_ROOT . '/rvote/rvote-ballot.inc';
include_once R_ROOT . '/rvote/rvote-results.inc';

/**
 * Submit a community funding proposal.
 * @param string $args:
 *   ctty: community's qid
 */
function formProposal($form, &$sta, $args = '') {
  global $base_url, $mya;
	
  extract(just('ctty', $args, NULL));
	if (!$ctty or !$cttyA = r\acct($ctty)) return w\go(''); // don't even bother with an error message if no ctty
	list ($ctty, $cttyName) = [$cttyA->id, $cttyA->fullName];
	list ($cttyEmail, $cttyPhone) = [$cttyA->email, u\fmtPhone($cttyA->phone)];
  
	$where = ($mya and $mya->cAdmin) ? ':NOW>start ORDER BY end DESC' : ':NOW BETWEEN start AND end';
	if (!$res = db\get('end,event,id AS eid', 'r_events', "ctty=:ctty AND type='P' AND $where", compact('ctty'))) {
    w\softErr(t('We\'re very sorry. The deadline for submitting proposals has passed. Try again in the next funding cycle!'));
	}
  
	extract($res);
	$end = strftime('%B %e, %G', $end); // Month d, YYYY
	$max = preg_replace('/[^0-9]/', '', $event);
	list ($idealAmt, $max) = [u\fmtAmt(round($max/10)), u\fmtAmt($max)];
  $options = ray(t('Sustainability / Renewable Energy, Small Business / Economic Development, Social and Economic Justice, Food Systems, Arts & Culture'));
  $optionKeys = ray(t('energy business justice food arts'));
  $criteria = db\q('SELECT text,points FROM r_criteria WHERE ctty=0')->fetchAllKeyed();
  $criteria = ['' => t('Points')] + $criteria + [
    ' ' => '____',
    t('Total') => 100,
	'  ' => '',
	t('<b>Extra Credit (up to 25 points):</b> Will the project bring <i>more</i> money into the Community Fund?') => '', 
  ];
/*  [
    ,
    t('How well does the project support our %PROJECT community investment priorities?') => 15,
	t('How well does the project promote systemic change?') => 15,
    t('Overall, how clearly doable is the project?') => 15,
    t('Ability to manage the project and funds effectively.') => 10,
	t('How quickly, surely, and voluminously will the project bring funds back into our Common Good Community Dollar Pool?') => 10,
    t('Evaluation plan.') => 5,
    t('How close is the request to the ideal amount (%idealAmt)?', compact('idealAmt')) => 5,
    t('What fraction of the total project budget are you requesting? (50% is ideal)') => 5,
    t('How much of the total project budget has been raised/committed so far? (half is ideal)?') => 5,
    t('Ability to implement project with less funding than requested.') => 5,
    t('How soon does the project begin? (soon after funding is best)') => 2.5,
    t('How soon does the project end? (soon after funding is best)') => 2.5,
    t('How local is the applicant?') => 2.5,
    t('%PROJECT member sponsorship of the project.') => 2.5,
    ' ' => '____',
    t('Total') => 100,
		'  ' => '',
		t('<b>Extra Credit (up to 25 points):</b> Will the project bring <i>more</i> money into the Community Fund?') => '', 
  ];
*/
  foreach ($options as $k) $_list[] = "<li>$k</li>\n";
  $_list = '<ul>' . join('', $_list) . '</ul>';

  foreach ($criteria as $text => $points) {
	  $text = tr($text, compact('idealAmt'));
    $text = str_replace(t('is the applicant'), t('are you'), $text);
    $crit[] = "<div class=\"row\"><div>$points</div><div>$text</div></div>\n";
  }

  $crit = '<div id="critChart" class="table">' . join('', $crit) . '</div>';
  $critPop = item(w\modal('criteria', t('Evaluation Criteria for Community Investment Proposals'), '', $crit, 'X'));
  jsx('funding-criteria');

  $title = t('Application/Proposal for Community Investment');
	$event = hidFld($eid);
  $_aInfo = w\atag(PROMO_URL);
  $_aCrit = 'a class="critLink"'; //  href="javascript:;
  
  $subtext = t(<<< EOF
<p>Complete this form by %end, to submit a proposal for funding from %cttyName. 
Requests can be made for up to %max (smaller requests are more likely to be funded). *</p>

<p>Please keep your responses concise -- 5 short paragraphs at most. If you can answer a question in a single sentence, even better! Proposals will be evaluated following <%aCrit>these criteria</a>.</p>
<p><%aInfo>Click here for more information about %PROJECT</a> &mdash; the payment card for community power. Amazingly, the %PROJECT system generates funding for local community projects without cost to any participant.</p>

<p>Questions? Email %cttyEmail or call %cttyPhone.</p>

<p>* <b>Note:</b> Funds will be disbursed using the %PROJECT payment system, so successful applicants will need to open a free %PROJECT account to receive funding.</p>
EOF
  , compact(ray('end cttyName max _list _aInfo _aCrit cttyEmail cttyPhone')));
 
  $project = textFld(REQ . t('Project title:'), [t('Project Title')]);
  $overview = areaFld(REQ . t('What:'), [t('Project Description'), t('Describe your project briefly. What specific actions will be taken?')]);
  $categories = boxFlds('cats', t('Categories'), t('What category or categories are most applicable to this project:'), array_combine($optionKeys, $options));

  $purpose = areaFld(REQ . t('Purpose:'), [t('Purpose of Project'), t('Describe the Project\'s purpose. How does it promote the common good? How does it address each of the categories you checked above?')]);
	$systemic = areaFld(REQ . t('Change:'), [t('Systemic Change'), t('%cttyName funds systemic change. In our society today, people suffer. Work that eases that suffering here and now is very important. It is also important to change the world to prevent such suffering from happening year after year. How does your project promote systemic change?', compact('cttyName'))]);

  $where = textFld(REQ . t('Where:'), [t('Project Zip Code'), t('Where will the project take place? (zip code)')]);
  $when = dateFld(REQ . t('When:'), [t('Project Start Date'), t('When will the project begin?')]);
  $until = dateFld(REQ . t('Until:'), [t('Project End Date'), t('When will the project be completed?')]);
  $how = areaFld(REQ . t('How:'), [t('Implementation Plan'), t('Provide details of HOW your project will be implemented.')]);
  $amount = numFld(REQ . t('Amount:'), [t('Amount to Fund'), t('How much funding are you proposing from the Community Fund? Be sure to read the <%aCrit>Evaluation Criteria</a>.', compact('_aCrit'))]);
/*  $typeDesc = <<<EOF
"Incentive" means a program of incentive rewards to %PROJECT members, to encourage a specific behavior; for example: voting, using alternative energy, or inviting people to join %PROJECT. If "Other", explain above under "What" (the project description).
EOF;
  $typeOptions = ray('Investment, Loan, Incentive, Grant, Other (explain)');
  $type = radiosFld(t('Type:'), $typeDesc, required(), $typeOptions);
	*/
	
	$recovery = boxesFld(t('Recovery:'), t('How will some or all of these funds come back to the Community Fund? For grant requests, this "coming back" may happen indirectly. (check all that apply)'), [], ray(RECOVERY_OPTS));

  $budgetTotal = numFld(REQ . t('Budget:'), [t('Total Project Budget'), t('What is the Total Project Expense Budget? Type or paste a complete Income and Expense budget below. Include your top 3 sources and amounts of committed funds. Please do not show subtotals.')]);
  $budget = areaFld(REQ, [t('Itemized Income and Expense Budget')]);
  //   $budget = htmlFld('', [t('Itemized Income and Expense Budget')], '', required());
  $committed = numFld(REQ . t('Committed:'), [t('Amount Already Raised'), t('How much of the project budget has already been raised?')]);
  $contingency = areaFld(REQ . t('Contingency:'), [t('Contingency Plan, for Partial Funding'), t('Funds are limited, and projects may not receive the full amount requested. If you receive less than the amount requested, how will that impact your ability to conduct the project?')]);
  $qualifications = areaFld(REQ . t('Qualifications:'), [t('Skills, Experience, and Qualities'), t('Who will complete the project? What are their specific skills, experience, or qualities that assure success?')]);
  $evaluation = areaFld(REQ . t('Evaluation:'), [t('Evaluation Plan'), t('How will you measure and evaluate the project\'s success?')]);

  $name = textFld(REQ . t('Applicant:'), [t('Applicant Name'), t('Type the name of the organization or individual who will receive the funding (properly capitalized).')]);
  $contact = textFld(REQ . t('Contact:'), [t('Contact Name'), t('If you are applying on behalf of yourself, type "self". Otherwise type the full name of the contact person for this project. If the contact person is not you, add your name in parenthesis &mdash; for example, "Jane Dough (per Millie Baker)".')]);
  $phone = phoneFld(REQ . t('Phone:'), [t('Contact Telephone Number')]);
  $email = emailFld(REQ . t('Email:'), [t('Contact Email Address')]);
  $sponsor = textFld(REQ . t('Sponsor:'), [t('%PROJECT Member(s) Sponsoring this Funding Proposal'), t('Name(s) of %PROJECT member(s) sponsoring this funding proposal, if any (or "self" if you are a member)')]);
  $submitProposal = submit(t('Submit Proposal'));
  return cgform(compact(ray('title subtext project overview categories purpose systemic where when until how amount recovery budgetTotal budget committed contingency qualifications evaluation name contact phone email sponsor submitProposal critPop event')));
} 
  
function formProposal_validate($form, &$sta) {
  extract(just('cats when until amount budgetTotal phone email', $sta['input'], NULL));
  if (!$cats) return say(t('You must choose at least one category.'), 'categories');
  if ($err = u\badDate($when, now() - 30 * DAY_SECS)) return sayFieldErr($err, 'when');
  if ($err = u\badDate($until, max($when, now()))) return sayFieldErr($err, 'until');
  foreach (ray('amount budgetTotal committed') as $k) if ($err = amtErr($k, $sta)) return sayFieldErr($err, $k);
  if ($err = u\badPhone($phone)) return say($err, 'phone');
  if (!u\validEmail($email)) return say('bad email', compact('email'), 'email');
  u\preray(compact(ray('when until phone')), $sta['input']); // amounts get updated in $sta by amtErr()
}

function formProposal_submit($form, &$sta) {
  $input = $sta['input'];
  $dbFields = 'event project categories overview purpose systemic where when until how amount recovery budgetTotal budget committed contingency qualifications evaluation name contact phone email sponsor';
  $input['categories'] = join(' ', array_keys($input['cats']));
	
	foreach (nni($input, 'recovery') ?: [] as $k => $v) u\setBit($recs, $k);
	$input['recovery'] = nn($recs) ?: 0;
  db\insert('r_proposals', $info = just($dbFields, $input));
  r\tellCAdmin('funding proposal received', $info);
  w\go('community', t('Your funding application has been submitted! The process of evaluating proposals may take several weeks. You will hear from us whether or not your proposal is chosen for funding.'));
}