<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Invite someone to sign up for rCredits (on behalf of the account, not the agent).
 * NOTE: We allow agents to send invitations on behalf of a company, but be aware that companies are not people and there is a danger in giving companies permission to act like people. On this invitation form, the company is "trusting" whoever it invites -- something usually reserved for humans.
 */
function formInvite($form, &$sta, $args = '') {
  if (!$mya = w\eLinkAcct($sta, $args, 'acct')) return;
  global $base_url;
//  if ($agent = r\agent()) {
  if ($mya->cAdmin and !$mya->proSe) say(t('Note: Admins cannot invite on behalf of someone.'), 'err');
  
  $title = t('Invite Someone to %PROJECT');
  if (!$mya->member) {
    $subtext = t('We\'re sorry, you must finish opening your account before you can invite someone.');
    return cgform(compact(ray('title subtext')));
  }
  
  $_a2 = w\atagB('/community/invite/invited-whom');
  $_a2 = r\acct() ? t('<%a2>Who you have invited</a>', compact('_a2')) : '';
  $where = r\location($mya->cttyA); // currently UNUSED
  $inviteCode = $mya->iCardCode(IBY_EMAIL);
  $_inviteLink = PROMO_URL . "/signup/code=$inviteCode";
  $warning = $mya->co ? t('<p>Note you are sending an invitation on behalf of a company. The company is "trusting" whoever you invite. We at %PROJECT recognize that companies are not people, and that it\'s a bit odd to allow them a sentiment usually reserved for humans.</p>') : '';

  $_aMailInvites = atag('mailto:' . CGF_EMAIL . '?subject=' . t('%PROJECT Invitation Card request') . '&body=' . t('Please send me some more %PROJECT invitation cards. Thanks!') . " -- $mya->fullName (account #$mya->mainQid)");
  $region = $mya->qo->region;
  foreach (ray('NEW MIW') as $k) if ($region == $k) {
    list ($nm, $zot) = explode('@', $mya->cttyA->email);
//    $aBack = str_replace('-back', '-back-' . $nm, $aBack);
  }
  $friend = $mya->co ? t('customer, employee, or supplier') : t('friend');
  $_size = $mya->co ? '4.25" &times; 2.75"' : '3.5" &times; 2"';
  
//  $subtext2 = $mya->cttyRewardy ? t('<p>Get a $%R_HELPER_BONUS reward for each %friend you invite, who signs up!</p>', compact('friend')) : '';
  $subtext2 = t(<<<EOF
<p>Invite <b>only people you trust</b> who live, shop, or work in your community. You may need to help or nudge the people you invite. %a2</p>
  <br>
  <p>Here are three different ways to invite someone:</p>
    <ul>
      <li><b>Best: Complete the form below</b> to send an invitation now. For each person you invite in your community, an anonymous donor contributes <span class="loud">$2</span> to your Community Fund!</li>
      <li><b>Tell your %friend</b> to sign up at %CG_DOMAIN or send them this link: <div id=inviteLink>%inviteLink</div></li>
      <li><b><%aMailInvites>Ask us</a></b> to mail you some promo cards at no charge.</li>
    </ul>
EOF
  , compact(ray('_a2 _size where _aMailInvites _inviteLink inviteCode friend')));
//  (Later, you will need to confirm you know and trust them.)
  $subtext = $subtext2 . $warning;
//  $contact = textFld(t('Email or Cell:'), t('An email address or cell number for the person you want to invite'), required());

  $email = textFld(t('Email:'), [t('Their email address'), t('An email address for the person you want to invite')]);
//  $emails = areaFld(t('Email(s):'), t('A comma-separated list of email addresses to invite'));
  $dupsOk = $mya->cAdmin ? boolFld(t('Dups Okay?'), '', FALSE) : NULL;
  $trusted = boolFld(t('Trusted?'), t('If you lent this person (or people) $250, would you trust them to pay it back?'), $mya->cAdmin ?: NULL);
  $zip = textFld(t('Postal Code:'), [t('Physical location postal code'), t('Where does the person live or work?')], required());
  $subject = textFld(t('Subject:'), t('The subject of your message'), required($mya->fullName . t(' invites you to %PROJECT')));

  $default = @$mya->data['inviteBody'] ?: t('invite default', 'name', $mya->fullName);
  $messageHelp = t('Invite them to sign up, and say why. A <%a>description of %PROJECT</a> will follow your message.', '_a', atag('/community/invite/example', w\away()));
  $message = areaFld(t('Message:'), $messageHelp, required($default));
  $submit = ($mya->proSe or !$mya->cAdmin) ? t('Send') : NULL; // don't let cAdmins send on behalf of anyone
  jsx('invite-link');
  
  return cgform(compact(ray('title subtext email dupsOk trusted zip subject message')) + w\setupFoot($submit));
}

function formInvite_validate($form, &$sta) {
  extract(just('email emails trusted zip', $sta['input']));
  if (op($sta) == 'submitN') {
    if (!$emails) return say('bad email', compact('emails'), 'emails');
    foreach (ray($emails) as $one) if (!u\validEmail($one)) return say(t('bad email: %one', compact('one')), 'emails');
  } elseif (!u\validEmail($email)) return say('bad email', compact('email'), 'email');
  if (!@$trusted) return say('must trust', 'trusted');
  if ($err = u\badZip($zip)) return say($err, 'zip');
}

function formInvite_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta); 
  global $base_url;
  extract(just('email emails subject message dupsOk zip', $sta['input']));

  if (op($sta) == 'submitN') {
//    foreach (ray($email) as $one) sendInvite($one, $subject, $message, @$dupsOk);
    $code = ''; // get a random code just once
    foreach (ray($emails) as $email) {
      $code = r\invite($email, $mya->id, $code, $subject, $message, $zip);
      $links[] = "$base_url/signup/code=$code/e=$email";
    }
    $links = join('<br>', $links);
    say('invite links', compact('links'));
  } else {
    $email = mb_strtolower($email); // otherwise constantSubs breaks on cap after @
    sendInvite($mya, $email, $subject, $message, @$dupsOk, $zip);
    $data = ray('inviteBody', mb_substr($message, 0, 10000)) + $mya->data;
    $mya->update(compact('data'));
  }
  if (!r\acct()) return w\doSay();
  return w\goNextStep('invite');
}