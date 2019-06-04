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
  } elseif (!$mya->co and !$mya->proSe and !$mya->cAdmin) { // can't sign for someone else!
    say('self must sign', 'signedBy');
  } else {
    $signedBy = w\signedBy();
    $setupFoot = w\setupFoot();
  }    
  $topics = ray('Members Member-Organizations CGC Dollar-Pool Backing Without-Limit Disputes Participation');
  foreach ($topics as $k) $args["_a$k"] = atag("/help/agreement#$k");
  $args['_aDetails'] = atag('/help/agreement');

  $agreement = t(<<< X
<div id="agreement">
<ol>
  <li>[1] <b>Who.</b> I make this agreement with all %PROJECT <%aMembers>Members</a> and <%aMember-Organizations>Member Organizations</a> everywhere &mdash; especially with Members and Member Organizations in my <%aCGC>%PROJECT Community</a>.</li>
  <li>[2] <b>Community control.</b> I understand we can use the Common Good System, as a democratic community, to reclaim control of our local economy for the common good. I am willing to participate with other Members to do that, and to support other communities to do the same.</li>
  <li>[3] <b>Investing together.</b> I understand whenever I put money in my %PROJECT account by buying %PROJECT credit, there is more money in the <%aDollar-Pool>Dollar Pool</a>, so my community has more money to invest while I use my credit for purchases.</li>
  <li>[4] <b>Backing together.</b> I understand my %PROJECT credit is <%aBacking>backed</a> 100% or more &mdash; partly by Dollars and partly by Members and Member Organizations.</li>
  <li>[5] <b>Accepting payments.</b> I will accept %PROJECT credit as payment, <%aWithout-Limit>without limit or surcharge</a>.</li>
  <li>[6] <b>Account Balance.</b> If I spend more than the balance in my %PROJECT account, resulting in a negative balance, I will bring my balance up to zero or more within 30 days.</li>
  <li>[7] <b>Disputes.</b> When there is a dispute, I will follow the <%aDisputes>%PROJECT Dispute Resolution Process</a> and will honor its outcome.</li>
  <li>[8] <b>Changes.</b> I understand I will have the <%aParticipation>opportunity to participate</a> in any decision to change this Agreement, and if I use my account after changes have been approved, that means I agree to those changes.</li>
</ol>
<br><p class="full center"><%aDetails>Details (how it works for you)</a></p>
</div>
X
  , $args);  

  if (!$mya) {
    $subtext = t('If you haven\'t finished opening your account yet, <%aSignin>sign in</a> to sign this agreement and complete your account.', '_aSignin', atag('/signin'));
  } elseif ($mya->signed) {
    $subtext = NULL;
  } elseif ($mya->co) { // company
    $subtext = t('<p>I commit %coName to the terms of the <%a>%PROJECT Agreement</a> (the same agreement I signed as an individual).</p>', 'coName _a', $mya->fullName, 'a id="show-agreement"');
    $agreement = "<div id=\"wrap-agreement\">$agreement</div>";
  } else { // individual
    $subtext = t('<h4>Instructions</h4><p>Please <b>READ THIS AGREEMENT</b>. Put a check mark by each section to indicate you have read and understand it. Then sign your full legal name at the bottom.</p>');
    if ($mya->cAdmin) $subtext .= t('<p><b>ADMINISTRATORS</b> need not put check marks in the boxes.</p>');
  }
  if ($subtext) $subtext = '<div id="instructions" class="well">' . $subtext . '</div>';
      
  $model = @\render(boxFld('checkNUMBER'));
  $hasBoxes = ($mya and !$mya->co and $mya->can(B_MANAGE) and !$mya->signed);
  for ($i = R_AGREE_0; $i <= R_AGREE_9; $i++) {
    $agreement = str_replace("[$i]", $hasBoxes ? str_replace('checkNUMBER', "check$i", $model) : '', $agreement);
  }

  $agreement = w\item($agreement);
  jsx('agree');
  
  return cgform(compact(ray('title subtext agreement signedBy')) + (@$setupFoot ?: []));
}

function formAgreement_validate($form, &$sta) {
  global $mya;
  list ($i0, $i9) = $mya->co ? [1, R_AGREE_9 - 1] : [R_AGREE_0, R_AGREE_9]; // companies omit prolog and democracy question
  if (!$mya->co and !$mya->cAdmin) for ($i = $i0; $i <= $i9; $i++) if (!@$sta['input']["check$i"]) return say('incomplete agreement', "check$i");
  if (w\badSignature($sta)) return;
}

function formAgreement_submit($form, &$sta) {
  global $mya;
  $signed = now();
  extract(just('signedBy', $sta['input']));
  $mya->update($info = compact('signed', 'signedBy'));
  return w\goNextStep('sign');
}