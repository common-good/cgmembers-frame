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
  $name = 'checkNUMBER';
//  $id = "edit-$name";
//  $check = boxFld('checkNUMBER', '', '', attrib(compact('id')));
  $model = @\render(boxFld('checkNUMBER'));
  $_addendum = '';

  if (!$mya) {
    // nothing to be done if signed out
  } elseif ($date = $mya->signed) {
    $date = u\fmtDate($date);
    say('signed agreement', compact('date'));
    $signedBy = item("Signed by <b>$mya->signedBy</b>, $date");
  } elseif (!$mya->co and !$mya->proSe and !$mya->cAdmin) { // can't sign for someone else!
    say('self must sign', 'signedBy');
  } else {
    $signedBy = w\signedBy();
    $setupFoot = w\setupFoot();
    $_addendum = $mya->cAdmin ? t('ADMINISTRATORS need not put checkmarks in the boxes.') : '';
  }    
  $topics = ray('Members Member-Organizations CGC Dollar-Pool Backing Without-Limit Disputes Participation');
  foreach ($topics as $k) $args["_a$k"] = atag("/help/agreement#$k");

  $agreement = t(<<< X
<div id="instructions" class="well">
<p><b>Instructions:</b> Put a check mark by each section to indicate you have read and understand it. %addendum</p>
</div>
<div id="agreement">
<ol>
  <li>[1] <b>Who.</b> I understand this is an agreement among all %PROJECT <%aMembers>Members</a> and <%aMember-Organizations>Member Organizations</a>, in my <%aCGC>%PROJECT Community</a> and elsewhere.</li>
  <li>[2] <b>Community control.</b> I understand the %PROJECT System is designed to take back control of our local economy  from big business and distant government. I am willing to participate with other Members to bring about this shift.</li>
  <li>[3] <b>Investing together.</b> I understand whenever I put money in my %PROJECT account by buying %PROJECT credit, there is more money in the <%aDollar-Pool>Dollar Pool</a> so my community has more money to invest.</li>
  <li>[4] <b>Backing together.</b> I understand my %PROJECT credit is <%aBacking>backed</a> 100% or more -- partly by Dollars and partly by Members and Member Organizations.</li>
  <li>[5] <b>Accepting payments.</b> I will accept %PROJECT credit as payment, <%aWithout-Limit>without limit or surcharge</a>.</li>
  <li>[6] <b>Account Balance.</b> If I spend more than the balance in my %PROJECT account, resulting in a negative balance, I will bring my balance up to zero or more within 30 days.</li>
  <li>[7] <b>Disputes.</b> When there is a dispute, I will follow the <%aDisputes>%PROJECT Dispute Resolution Process</a> and will honor its outcome.</li>
  <li>[8] <b>Changes.</b> I understand I will have the <%aParticipation>opportunity to participate</a> in any decision to change this Agreement, and using my account after changes have been approved implies agreement to those changes.</li>
</ol>
</div>
X
  , compact('_addendum') + $args);

  for ($i = R_AGREE_0; $i <= R_AGREE_9; $i++) {
    $box = str_replace('checkNUMBER', "check$i", $model);
    $agreement = str_replace("[$i]", (!$mya or !$mya->can(B_MANAGE) or @$date) ? '' : $box, $agreement);
  }
//  $agreement = fld('item', '', '', $agreement);
  $agreement = w\item($agreement);
  
  return cgform(compact(ray('title agreement signedBy')) + (@$setupFoot ?: []));
}

function formAgreement_validate($form, &$sta) {
  global $mya;
  list ($i0, $i9) = $mya->co ? [1, R_AGREE_9 - 1] : [R_AGREE_0, R_AGREE_9]; // companies omit prolog and democracy question
  if (!$mya->cAdmin) for ($i = $i0; $i <= $i9; $i++) if (!@$sta['input']["check$i"]) return say('incomplete agreement', "check$i");
  if (w\badSignature($sta)) return;
}

function formAgreement_submit($form, &$sta) {
  global $mya;
  $signed = r\rTime();
  extract(just('signedBy', $sta['input']));
  $mya->update($info = compact('signed', 'signedBy'));
  return w\goNextStep('sign');
}