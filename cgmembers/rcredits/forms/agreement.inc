<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Give member a chance to sign the Common Good Agreement. (agreement revised 2/5/2019)
 */
function formAgreement($form, &$sta) {
  global $mya;
  $title = t('%PROJECT Agreement');

  if (!$mya) {
    $signedBy = NULL;
  } elseif ($date = $mya->signed) {
    $date = u\fmtDate($date);
    say('signed agreement', compact('date'));
    $signedBy = item("Signed by <b>$mya->signedBy</b>, $date");
  } elseif (!$mya->co and !$mya->proSe and !$mya->admHelpSignup) { // can't sign for someone else!
    say('self must sign', 'signedBy');
  } elseif (!$mya->co) {
    $signedBy = NULL;
    $setupFoot = w\setupFoot(t('I agree'));
  } elseif (!$mya->trialco) {
    $signedBy = w\signedBy();
    $setupFoot = w\setupFoot(t('I agree'));
  } else $signedBy = NULL;
  $topics = ray('Members Member-Organizations CGC Dollar-Pool Backing Without-Limit Disputes Participation');
  foreach ($topics as $k) $args["_a$k"] = atag("/help/agreement#$k");

  $agreement = t(<<< X
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
  , $args);  

  $details = item(t('<%a>Details (how it works for you)</a>', '_a', atag('/help/agreement')));


  if (!$mya) {
    $subtext = t('If you haven\'t finished opening your account yet, <%aSignin>sign in</a> to sign this agreement and complete your account.', '_aSignin', atag('/signin'));
  } elseif ($mya->signed) {
    $subtext = NULL;
  } elseif ($mya->co) { // company
    $subtext = t('<p>I commit %coName to the terms of the <%a>%PROJECT Agreement</a>%same.</p>', 'coName _a same', $mya->fullName, 'a id="show-agreement"', $mya->trialco ? '' : t(' (the same agreement I signed as an individual)'));
    $agreement = "<div id=\"wrap-agreement\">$agreement</div>";
  } else { // individual
    $subtext = t('<p>In using the %PROJECT payment system, you agree to the following.</p>');
  }
//  if ($subtext) $subtext = '<div id="instructions" class="well">' . $subtext . '</div>';
      
  $agreement = "<div id=\"wrap-agreement\">$agreement</div>";
  $agreement = w\item($agreement, BARE);
  jsx('agree', ['show' => !$mya ?: $mya->trialco]);
  
  return cgform(compact(ray('title subtext agreement details signedBy')) + (nn($setupFoot) ?: []));
}

function formAgreement_validate($form, &$sta) {
  global $mya;
//  list ($i0, $i9) = $mya->co ? [1, R_AGREE_9 - 1] : [R_AGREE_0, R_AGREE_9]; // companies omit prolog and democracy question
//  if (!$mya->co and !$mya->admin) for ($i = $i0; $i <= $i9; $i++) if (!@$sta['input']["check$i"]) return say('incomplete agreement', "check$i");
//  if (w\badSignature($sta)) return;
}

function formAgreement_submit($form, &$sta) {
  global $mya;
  $signed = now();
//  extract(just('signedBy', $sta['input']));
  $aa = ($mya->admHelpSignup or ($mya->co and $mya->proSe)) ? $mya->helperA : $mya->agentA;
  $signedBy = $aa->fullName;
  $mya->update($info = compact('signed', 'signedBy'));
  return w\goNextStep('signup');
}
