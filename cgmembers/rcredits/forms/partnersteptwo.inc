<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * In partner signup, having collected bank account information, tell and ask for whatever else is needed:
 * Sign the Common Good Agreement (revised 2/5/2019), choose roundups
 */
function formPartnerStepTwo($form, &$sta) {
  global $mya;
  $title = t('%PROJECT Account Setup');
  $subtext = t('<p>Great! You chose to <b>pay</b> for your community solar subscription through %PROJECT.</p><p>%PROJECT is a not-for-profit payment system designed to give you and your community millions of dollars in funding for whatever you decide together is important, when enough people and businesses participate.</p><p>You may want to round all your payments up to a whole dollar amount and donate the cents toward building a %PROJECT Economy in your area, by choosing "Yes" below.');
  $midtext = t('<p>For more information about %PROJECT &mdash; or to request a free %PROJECT card, accept payments, or invite your friends and local businesses (after completing this final step in your community solar signup), sign in to your account at %CG_DOMAIN or call us!</p><h3>The %PROJECT Agreement</h3><p>In using the %PROJECT payment system, you agree to the following.</p>');
  $roundup = w\boolFld(t('Round Up!'));

  $submit = t('I agree');
  $topics = ray('Members Member-Organizations CGC Dollar-Pool Backing Without-Limit Disputes Participation');
  foreach ($topics as $k) $args["_a$k"] = atag("/help/agreement#$k");
  $args['_aDetails'] = atag('/help/agreement');

  $agreement = item(t(<<< X
<div id="agreement">
<ol>
  <li><b>Who.</b> I make this agreement with all %PROJECT <%aMembers>Members</a> and <%aMember-Organizations>Member Organizations</a> everywhere &mdash; especially with Members and Member Organizations in my <%aCGC>%PROJECT Community</a>.</li>
  <li><b>Community control.</b> I understand we can use the Common Good System, as a democratic community, to reclaim control of our local economy for the common good. I am willing to participate with other Members to do that, and to support other communities to do the same.</li>
  <li><b>Investing together.</b> I understand whenever I put money in my %PROJECT account by buying %PROJECT credit, there is more money in the <%aDollar-Pool>Dollar Pool</a>, so my community has more money to invest while I use my credit for purchases.</li>
  <li><b>Backing together.</b> I understand my %PROJECT credit is <%aBacking>backed</a> 100% or more &mdash; partly by money in the Dollar Pool and partly by Members and Member Organizations.</li>
  <li><b>Accepting payments.</b> I will accept %PROJECT credit as payment, <%aWithout-Limit>without limit or surcharge</a>.</li>
  <li><b>Account Balance.</b> If I spend more than the balance in my %PROJECT account, resulting in a negative balance, I will bring my balance up to zero or more within 30 days.</li>
  <li><b>Disputes.</b> When there is a dispute, I will follow the <%aDisputes>%PROJECT Dispute Resolution Process</a> and will honor its outcome.</li>
  <li><b>Changes.</b> I understand I will have the <%aParticipation>opportunity to participate</a> in any decision to change this Agreement, and if I use my account after changes have been approved, that means I agree to those changes.</li>
</ol>
</div>
X
  , $args));  
  
  return cgform(compact(ray('title subtext roundup midtext agreement submit')));
}

function formPartnerStepTwo_validate($form, &$sta) {}

function formPartnerStepTwo_submit($form, &$sta) {
  global $mya;

  extract(just('roundup', $sta['input']));
  $signed = now();
  $signedBy = $mya->contact ?: $mya->agentA->fullName;
  $mya->update(compact(ray('signed signedBy roundup')));
  $mya->stepDone('agree');
  $mya->stepDone('preferences');
  return w\goNextStep('partnersteptwo');
}
