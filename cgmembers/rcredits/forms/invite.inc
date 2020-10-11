<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Invite someone to sign up for Common Good (on behalf of the account, not the agent).
 * NOTE: We allow agents to send invitations on behalf of a company, but be aware that companies are not people and there is a danger in giving companies permission to act like people. On this invitation form, the company is "trusting" whoever it invites -- something usually reserved for humans.
 */
function formInvite($form, &$sta, $args = '') {
  extract(just('edit example code', $args, NULL));

  if (!$mya = w\eLinkAcct($sta, $args, 'acct')) return;
  if ($mya->co) say(t('To endorse %PROJECT, use your personal account.'));

  $quote = db\get('quote', 'u_shouters', ray('uid', $mya->id)) ?: SHOUT_TEXT;
  if (!$mya->co and (!$mya->shouter or $edit)) return endorse($mya, $quote, $edit);

  $fullName = $mya->fullName;
  $phone = $mya->phoneDash;
  $signed = u\fmtDate($mya->signed, TRUE);
  $code = $mya->iCardCode(IBY_EMAIL); // this code is different from the one emailed in form
  $info = compact(ray('fullName quote phone signed'));
  $buttons = btn('/community/invite/all', t('Recent Endorsements'));

  if ($example) return exampleInvite($info, $code);

  return inviteOne($mya, $code, $info, $buttons, $args); // nonperson or already signed
}

function formInvite_validate($form, &$sta) {
  extract(just('sawVideo org position', $sta['input'], ''));

  if (isset($sawVideo)) { // community invitation (can't use isset($sign))
    if ($position and !$org) return say(t('If you are not associated with an organization, leave your title blank.'), 'title');
  } else { // individual invitation
    extract(just('email', $sta['input']));
    if (!u\validEmail($email)) return say('bad email', compact('email'), 'email');
  }
}

function formInvite_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta); 
  global $base_url;

  if (isset($sta['input']['sawVideo'])) { // community invitation
    extract($input = just('sign quote org position website usePhoto postPhoto sawVideo whyNot', $sta['input'], NULL));
    if ($sign) {
      foreach (ray('usePhoto postPhoto sawVideo') as $k) $$k = $$k ? 1 : 0;
      $website = preg_replace('~http(s?)://~', '', $website);
      list ($uid, $title) = [$mya->id, $position];
      db\updateOrInsert('u_shouters', compact(ray('uid quote org title website usePhoto postPhoto sawVideo')), 'uid');
      say(t('Thank you for endorsing %PROJECT!')); // might be new or edited
    } else {
      r\tellAdmin(t('non-endorsement'), $input + ray('fullName', $mya->fullName));
      if ($whyNot) say(t('Thank you for telling us! We will use your feedback to improve.'));
    }
  } else { // individual invitation
    extract(just('email subs', $sta['input'], NULL));
    $email = mb_strtolower($email); // otherwise constantSubs breaks on cap after @
    $subject = t('%nm invites you to %PROJECT', 'nm', $mya->fullName);
    $subs = unserialize($subs) + ray('subject uid noFrame', $subject, $mya->id, TRUE);
    $code = r\invite($email, $mya->id, '', $subject, '', $mya->zip);
    r\rMail('invite', $email, $subs + compact(ray('code')), $mya);
    say('invite sent|repeat invite', compact('email'));
    if (!$mya->admin) r\tellAdmin('invited', $subs + compact('email'), $mya->id);
  }

  return r\acct() ? w\goNextStep('invite') : w\doSay();
}

/**
 * Display the standard invitation that will be sent for this member.
 */
function exampleInvite($subs, $code, $emailCoded = '') {
  header_remove('Content-Security-Policy'); // allow the template's CSS
/**/  if (!$res = file_get_contents(R_ROOT . '/templates/invite.html')) die('missing invitation template');
/**/  echo tr($res, $subs + compact(ray('code emailCoded')));
  exit();
}

/**
 * Form to invite one person
 */
function inviteOne($mya, $code, $info, $buttons, $args) {
  $title = t('Invite Someone');

  if ($mya->shouter) $buttons .= btn('/community/invite/edit=1', t('Edit Yours'));
  if (db\exists('r_invites', 'inviter=:id', ray('id', $mya->id))) $buttons .= btn('/community/invite/invited-whom', t('Who you invited'));

  $_aInvite = atag(BASE_URL . "/signup/code=$code");
  $_aStd = atag("/community/invite/example=1&allow=acct&qid=$mya->mainQid&ecode=$mya->emailCode", away());
  $friend = $mya->co ? t('customer, employee, or supplier') : t('friend');
  $subs = hidFld(serialize($info));
  
  $subtext = t('<p>Invite a %friend</b> to sign up for a free %PROJECT card! Type an email address below and we\'ll send a message <%aStd>[like this]</a>. Or copy this link: [<%aInvite>Sign up for %PROJECT</a>] into an email you write yourself (or copy the whole example into an email).</p><p>Some people may need your help signing up &mdash; especially uploading or emailing a photo.</p>'
  , compact(ray('_aStd _aInvite friend')));

  $email = emailFld(t('Email:'), [t('Their email address'), t('An email address for the person you want to invite')]);
  $submit = t('Send');
  $foot = ($mya->proSe or !$mya->cAdmin) ? w\setupFoot($submit) : []; // don't let cAdmins send on behalf of anyone
  jsx('invite-link');

  return cgform(compact(ray('buttons title subtext email subs')) + $foot);
}    

/**
 * Community invitation
 */
function endorse($mya, $quote, $edit) {
  $fields = 'quote, org, title, website, usePhoto, postPhoto, sawVideo';
  $info = db\get($fields, 'u_shouters', ray('uid', $mya->id));
  if ($info) extract($info); else foreach (ray($fields) as $k) $$k = NULL;
  $position = $title;
  
  $title = t('Invite People');
  $subtext = t('<p>On average, the %PROJECT Community Fund grows by thousands of dollars with every participant. Let\'s get all of %city signed up and have a  multi-million dollar fund to help the community through this pandemic crisis and beyond.</p><p>If you have not yet seen our video, please see it now, before finishing this page: %videoBtn</p><p>Then join other thought leaders signing this public endorsement (you can also add your own words after you click Yes):</p><h5>%SHOUT_TEXT<br><small>&mdash; (your name here, along with many others)</small></h5>', 'nm city _videoBtn', $mya->fullName, $mya->city, btn(VIDEO_URL, t('Video'), 'success'));
  
  $sawVideo = boolFld(REQ . t('Saw Video?'), '', $edit ? $sawVideo : 0, NULL, clas('mlab-left'));
  $sign = $edit ? hidFld(1) : boolFld(REQ . t('Sign:'), '', -1, NULL, clas('mlab-left'));
  $whyNot = textFld(t('Why?'), [t('Reason'), t('If you\'re willing, please say why you\'re not endorsing %PROJECT.')]);
  $question = item(t('Why do you want people to sign up? How do you see %PROJECT benefiting you, your family, the community, or your organization\'s mission? When people are thinking about signing up, they will read this as you-talking-to-them. So you may want to tell them explicitly to sign up.'));
  $quote = areaFld(t('Answer:'), [t('Your answer to the question above'), t('Your answer will appear below your name in the list. You can edit it any time. (max 4000 chars)')], dft($quote) + maxlen(4096));
  $org = textFld(t('Organization (if any):'), [t('Organization name'), t('Sometimes your voice carries more weight when you hold a responsible position in society. The list will say explicitly: <i>"%ORG_DISCLAIMER"</i>.')], dft($org));
  $position = textFld(t('Title:'), [t('Your position there')], dft($position));
  $website = textFld(t('Website:'), [t('Organization website URL')], dft($website));
  $usePhoto = boxFld('usePhoto', t('Include Photo:'), t('Include my photo in the collage of members inviting the public to sign up.'), $edit ? $usePhoto : 1);
  $postPhoto = boxFld('postPhoto', NBSP, t('You may also include my photo and my answer (above) in a media post.'), $edit ? $postPhoto : 0);

  jsx('invite', 'edit', $edit);
  return cgform(compact(ray('title subtext sign sawVideo whyNot question quote org position website usePhoto postPhoto')) + w\setupFoot());
}
