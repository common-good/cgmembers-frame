<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

define('POST_FLDS', 'type item details cat exchange emergency radius end private'); // db fields other than contact info
define('CONTACT_FLDS', 'displayName fullName address city state zip phone email method'); // db contact fields
define('MESSAGE_FLDS', 'message postid sender'); // db message fields
define('TOP_FLDS', 'locus radius latitude longitude zip'); // fields used for filtering list
define('POST_INPUT_FLDS', 'cat item private details exchange emergency radius end type'); // post input fields
define('METHOD_OPTS', 'email phone text'); // contact method options
define('VIPID', 'vipid'); // name of confirmed-email cookie (the VIP's people record ID)
define('HEALTH_RATINGS', 'excellent good ok indisposed'); // health self-assessment
define('POSTPAGE', '/community/posts');
define('MODERATE_ZIPS', ''); // 3-digit zip code areas where moderation is mandatory (eg '010 013')
define('POST_FRMS', 'top tabs post editpost who message'); // possible values for $frm

/*
Current features
- View the lists of Posts and Needs for a certain address and radius (say Greenfield, MA 01301 with a 30 mile radius)
- Select a certain Category to limit the list
- Click an item to view it in more detail, but without personal identifying information (more detail for administrators )
- Reply to an item being viewed
- Post an Offer
- Post a Need
- Click the confirmation link in the email you get after posting or replying (just the first time)
- Receive a reply to something you posted (for example by replying to your own post)
- Post or Reply to a Post after doing either thing once before
- Cancel (terminate) a Post (immediately or much later) by clicking a link in the confirmation email.

New features:
- Edit your post (or someone else's if you are an administrator)
- Search for text
- View just your own posts (or someone else's if you are an administrator)  
- For administrators, more fields in the summary list
- Moderation required in 013 zip area
- COVID 19 risk survey (self-vetting) in 013 zip area
- Automatic translation (in process)
*/

/**
 * Exchange help with neighbors.
 
 @todo: add a link in every reply that will both cancel the post AND tell the replier thank you and no longer needed or offered.
 - send an email the next day after a reply, asking if the offer/need is still ok (with a link to cancel it). At the same time, change confirmed to a date (NULL is not confirmed, date is most recent confirmation date). Allow ongoing.f
 - say when new things are posted in my area.
 
 */
function formPosts($form, &$sta, $args = '') {
  /* $title = t('Offers & Posts');
  $subtext = t('We\'re sorry. This page is under maintenance for a few minutes. Try again a bit later please.');
  return cgform(compact(ray('title subtext'))); */
  
  global $base_url;
  extract(just(TOP_FLDS . ' op code postid thing email', $args, NULL));
  if ($code == '%code') u\FAIL('top');

  foreach (ray(TOP_FLDS . ' email') as $k) {
    if (nn($$k)) r\setCook($k, $$k); else $$k = nn($_COOKIE[$k]); // default from previous time (useful also in getWho)
    ${$k . '0'} = $$k; // remember initial values
  }

  if (in($op, 'confirm cancel edit')) return confirm($op, $thing, $code);
  if (in($op, 'offer need')) return getPost($op);
  if ($op == 'show') return showPost($postid);
  if ($op == 'who') return getWho($args);

  $title = t('Mutual Aid Offers & Needs'); // for top or tabs
  
  if ($op == 'top') {
    jsx('post-tabs');
    $back = btn('/community/posts', t('<big>&laquo;</big> Back to Top'), 'warning');
    $search = textFld(BARE, [t('Search for words')]);

    $choices = catChoices(t('Category (all)'), [-1 => t('(search)')]) + [99 => t('my posts')];
    $cat = rendA('cat', selectFld(BARE, '', clas('limit-list'), $choices));

    foreach (ray('offer need') as $type) {
      $k = $type . 's';
      $rows = postList($type, $latitude0, $longitude0, $zip0, $radius0);
      ${$k . 'None'} = count($rows) < 4 ? tr('<div class="none">%none</div>', 'none', t('There are not yet any %types within %radius miles of that address.', compact(ray('type radius')))) : '';
      $$k = join("\n", $rows);
      $$k = preg_replace(t('/Category/'), $cat, $$k, 1);
      $$k = preg_replace('/edit-cat/', "\$0-$type", $$k, 1);
    }

    $tabs = tabFlds($offers, $needs, $offersNone, $needsNone);
    $frm = hidFld('tabs');

    return cgform(compact(ray('title back search tabs frm')));
  }

  // no op
  jsx('posts');

  $subtext = t('<p>In our %PROJECT Economy we want to make sure everyone has what they need, with no one left behind.  Enter your address, then <span class="loud">think about what you need and what you want to offer in each category</span>. Then get two neighbors to do the same! You will be delighted by the serendipity that results.</p>
    <p>If you know someone in need who has no Internet access, offer to post something for them.</p>
    <p>Community organizers and programmers, contact us to collaborate or to get administrative access. For an example of how this works in practice, try "Greenfield, MA 01301" with a radius of 10. Email comments and suggestions to %CGF_EMAIL.</p>');

  $locus = textFld(REQ . t('Where:'), [t('Full address OR city/state/zip'), t('Your address (including zip) or just city, state, and zip.')], dft($locus));
  $radius = textFld(REQ . t('Radius:'), [t('Max distance from address')], dft($radius ?: 10) + suffix(t('miles') . btn("$base_url/community/posts", t('Go'), '', 'sm')));

  $latitude = hidFld($latitude ?: 0); // to be filled in on the fly
  $longitude = hidFld($longitude ?: 0); // to be filled in on the fly
  $zip = hidFld($zip ?: ''); // to be filled in on the fly
  $submit = submit(t('fake'), '', '', clas('invisible')); // Drupal keeps other solutions from calling _submit function)
  $frm = hidFld('top');
  
  return cgform(compact(ray('title subtext locus radius latitude longitude zip submit frm')));
}

/**
 * Return the field with all the information and action choices.
 */
function tabFlds($offers, $needs, $offersNone, $needsNone) { 
  global $base_url;

  return item(<<< X
<div id="tabs">
  <ul>
    <li><a href="#tab-offers">Offers</a></li>
    <li><a href="#tab-needs">Needs</a></li>
    <li><a id="offer" href="$base_url/community/posts/op=offer">Post an Offer</a></li>
    <li><a id="need" href="$base_url/community/posts/op=need">Post a Need</a></li>
  </ul>
  <div id="tab-offers">
    <div class="container">
      $offers
    </div>
    $offersNone
  </div>
  <div id="tab-needs">
    <div class="container">
      $needs
    </div>
    $needsNone
  </div>
</div>
X
  , BARE);
}

/**
 * Return a table of confirmed postings for the given type and category.
 */
function postList($type, $lat, $lon, $zip, $radius) {
  $distance = tr('%DISTANCE_BETWEEN');
  $myPid = vip('pid') ?: '0';
  $isMyPid = 'pid=:myPid';
  
  $visible = admin() ? 1 : 'NOT s.private';

  $sql = <<<X
    SELECT $distance AS dist, s.postid, c.cat, s.item, s.details, s.emergency, $isMyPid AS mine
    FROM posts s
    LEFT JOIN post_cats c ON c.id=s.cat
    LEFT JOIN people p USING (pid)
    WHERE s.type=:type
      AND s.created <= :NOW
      AND IFNULL(s.end, :NOW) >= :NOW
      AND p.confirmed
      AND IF((latitude=0 AND longitude=0) OR (:lat=0 AND :lon=0), 
            MID(p.zip,1,3) = MID(:zip,1,3), 
            $distance < LEAST(IFNULL(s.radius, :radius), :radius)
          )
      AND $visible
    ORDER BY s.cat,s.created DESC
X;

///     debug(u\showq($sql, compact(ray('type lat lon zip radius'))));
  $q = db\q($sql, compact(ray('type lat lon zip radius myPid')));
  foreach (ray(t('category item details')) as $k) $rows[] = "<div class=\"cell k-$k\">" . ucfirst($k) . '</div>';
  $rows = ['<div class="thead"><div class="row">' . join('', $rows) . '</div></div>'];
  $rows[] = '<div class="tbody">';
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
///      debug($dist);
    if ($emergency) $item = '<span class="emergency">!!</span> ' . $item;
    $item = lnk('/community/posts/op=show&postid=' . $postid, $item);
    $rowClass = "row $cat" . ($mine ? ' mine' : '');
    $rows[] = <<< X
    <div class="$rowClass">
      <div class="cell cat">$cat</div>
      <div class="cell item">$item</div>
      <div class="cell details">$details</div>
    </div>
X;
  }
  
  $rows[] = '</div>';
  return $rows;
}

/**
 * Input an offer or need.
 * @param string $type: 
 */
function getPost($type) {
  extract(just('offercat needcat', $_COOKIE, NULL));

  jsx('post-post', 'type', $type);

  $need = ($type == 'need');
  $postCat = ($id = nn($_COOKIE[$type . 'cat'])) ? db\get('cat', 'post_cats', compact('id')) : '';
  $lastCat = $postCat ? t('Your last %type post was in the "%postCat" category.', compact(ray('type postCat'))) : '';
  $_confirm = vip('confirmed') ? '' : t('<p>You will also need to confirm your first post by email.</p>');
  
  $frm = hidFld('post');
  $title = $need ? t('Post a Need') : t('Post an Offer');
  $typeThink = $need ? $type : t('want to offer');
  $subtext = t('<p>Think about what you %typeThink in each category, one at a time. %lastCat</p><p>Do not put identifying information in the "What" or "Details". People contact you through the system, without knowing your full name or email address. You can be more specific when you reply to them.</p>%confirm', compact(ray('typeThink lastCat _confirm')));
  extract(postInputFlds($type));
  $email = emailFld();
  $submit = submit(t('Post'));
  
  return cgform(compact(ray('title subtext cat item private details exchange emergency radius end email submit type frm')));
}

/**
 * Return input fields common to creating or editing a post.
 */
function postInputFlds($type0, $dfts = []) {
  extract(just(POST_INPUT_FLDS, $dfts, ''));
  $type = $type0;
  $need = ($type == 'need');
  
  $choices = catChoices(t('Choose a category'));
  $cat = array_search($cat, $choices);
  $cat = selectFld(REQ . t('Category:'), '', dft($cat), $choices);
  
  $item = textFld(REQ . t('What:'), [$need ? t('What you need') : t('What you\'re offering')], dft($item) + maxlen(40));
  $private = admin() ? boolFld(t('Private:'), t('Seen only by the administrator'), dft((int) $private)) : hidFld(0);

  $details = areaFld(REQ . t('Details:'), t('Max 500 characters'), dft($details) + maxlen(500));
  
  $exchange = $need ? boolFld(t('Exchange:'), t('Are you offering something in exchange? (include it in the details too)'), dft($exchange)) : hidFld(0);
  $emergency = boolFld(t('Emergency:'), $need ? t('This need is a short-term emergency.') : t('This offer is only for emergencies (such as 2 weeks of social distancing).'), dft($emergency));
  $radius = $need ? NULL : textFld(REQ . t('Radius:'), [t('Visibility radius'), t('Up to how many miles away do you want this seen? 10 usually covers neighboring towns. Use zero for "anywhere".')], dft($radius ?: 10) + suffix(t(' miles')));
  $end = textFld(t('End Date:'), [t('mm/dd/yyyy'), t('When to stop listing this (you can also cancel it later)')], dft($end ? u\fmtDate($end, TRUE) : '') + suffix(lnk('', t('end now'))));

  $type = hidFld($type);

  return compact(ray(POST_INPUT_FLDS));
}

/**
 * Show details about a post.
 */
function showPost($postid) {
  if (!$res = db\get('pid,s.type,c.cat,s.item,s.private,s.details,s.exchange,s.emergency,s.created,s.end,p.displayName,p.fullName,p.phone,p.email,p.address,p.city,p.state,p.zip,p.method,p.created AS userSince', 'posts s LEFT JOIN post_cats c ON c.id=s.cat LEFT JOIN people p USING (pid)', compact('postid'))) return softErr(t('That is not a need or offer.'));
  extract($res);
  
  $phone = u\fmtPhone($phone);
  $need = ($type == 'need');
  if (!$need and $emergency) $item = t('(In emergency) ') . $item;
  
  $postid = hidFld($postid);

  // set up display-only fields that are the same for user's own post and someone else's post
  $created = item(u\fmtDate($created, TRUE), t('Posted:'));

  if (admin()) {
    list ($userSince, $phone, $state) = [u\fmtDate($userSince, TRUE), u\fmtPhone($phone), r\realState($state)];
    $who = <<< X
      $fullName ("$displayName")<br>
      $phone<br>
      $city, $state $zip<br>
      $email<br><br>
      Prefers: $method<br>
      User since $userSince
X;
  } else $who = $displayName;

  $who = item($who, t('Who:'));

  // set up fields that are the different for user's own post and someone else's post
  
  if (vip('pid') == $pid or admin()) { // user's own post: allow editing or termination
    $frm = hidFld('editpost');
    $title = t('Edit Post');
    $subtext = NULL;
    extract(postInputFlds($type, $res));
    if ($a = admin()) {
      $message = msgFld('');
      $email = hidFld($a->email); // messages from admin always come from admin
    } else $email = $message = NULL;
    
    $pid = hidFld($pid);
    $submit = t('Update');    
  } else { // someone else's post
    $frm = hidFld('message');
    $title = t('Details');
    $subtext = t('Use this form to send a message to the person who posted this %type. Your message will be sent by email with your display name and email address. Your full name and other contact information will not be sent unless you include it in the message.', compact('type'));
    $type = item($type, t('Type:'));
    $cat = item($cat, t('Category:'));
    $item = item($item, $need ? ($emergency ? t('Urgent Need:') : t('Need:')) : t('Offer:'));
    $details = item($details, t('Details:'));
    $exchange = $emergency = $radius = $end = NULL;
    
    $message = msgFld(REQ);
    $email = emailFld();
    $pid = $private = NULL;
  // Keep in mind: the person you are contacting is almost certainly a neighbor, but might be a scoundrel pretending to live nearby.

    $submit = t('Send Message');
  }
  
  return cgform(compact(ray('title subtext type cat who created private item details exchange emergency radius end message email submit postid pid frm')));
}

/**
 * Input name and contact information.
 */
function getWho($args) {
  mb_parse_str($args, $ray);
  foreach(just(POST_FLDS . ' email ' . MESSAGE_FLDS, $ray) as $k => $v) $info[$k] = hidFld($v);
  extract(just('email', $info)); // because it's in CONTACT_FLDS

  jsx('post-who', ray('moderateZips', MODERATE_ZIPS));
  
  $frm = hidFld('who');
  $title = t('Your Information');
  $subtext = t('<p>This information is required only the first time you post or contact someone.</p><p>Only your "display name" (first name or nickname) is shown to others. Your full name, address, email, and phone are not shown to anyone unless you choose "phone" or "text" as your preferred contact method (then just your phone number is displayed when someone asks to contact you about a specific post).</p>');
  $displayName = textFld(REQ . t('Display Name:'), [t('Your first name or nickname'), t('To be shown to publicly with your posts')]);
  $fullName = textFld(REQ . t('Name:'), [t('Your full name'), t('usename desc')]);
  $address = textFld(REQ . t('Street Address:'), [t('Physical address'), t('So people can search for offers and needs by neighborhood')]);
  $city = textFld(REQ . t('City:'), [t('City where you live')]);
  $state = stateFld(R_STATE_ID); // can't use required() here
  $zip = zipFld();
  $phone = textFld(REQ . t('Phone:'), [t('Phone number'), t('Just in case we need to contact you. This is not shared with other users unless you choose "phone" or "text" as your preferred contact method.')]);
  $method = w\radiosFld(REQ . t('Preferred Contact:'), t('The "phone" and "text" options apply only when a %PROJECT member contacts you (because we have verified the identity only of members). Otherwise you will receive replies by email.'), '', ray(METHOD_OPTS));

  $midtext = t('And now some COVID-19 self-assessment questions for 2020:');
  $days = textFld(t('Distancing Days:'), [t('Days practicing social distancing'), t('About how long have you been practicing social distancing consistently (staying at least 6 feet away from all but a handful of people)? Thank you! Your voluntary inconvenience could save thousands of lives &mdash; especially your friends, family, and neighbors.')]);
  $washes = textFld(t('Hand-washing:'), [t('Hand-washes per day'), t('About how many times a day do you wash your hands for 20 seconds or more? (This data will help us convince other people to do it more.)')]);
  $health = radiosFld(t('Health:'), t('If you are not feeling completely well, please <%a>take this quiz from the CDC</a> right away, and follow its advice.', '_a', atag('https://www.cdc.gov/coronavirus/2019-ncov/if-you-are-sick/steps-when-sick.html')), dft(2), ray(HEALTH_RATINGS));

  $submit = t('Submit');

  return cgform(compact(ray('title subtext ' . CONTACT_FLDS . ' midtext days washes health submit frm')) + $info);
}

/**
 * Respond to user's click of confirm, cancel, or edit link in email.
 */
function confirm($op, $thing, $code) {
  $code = u\decry('P', CRYPT_FLAG . $code);

  
  list ($tnm, $idnm) = $thing == 'post' ? ['posts', 'postid'] : ['messages', 'id'];
  extract(just($idnm . ' created', $code, NULL));
  $idSub = compact($idnm);

  if (db\exists($tnm, $idSub + compact('created'))) {
    $pid = db\get($thing == 'post' ? 'pid' : 'sender', $tnm, $idSub);
    
    if ($op == 'cancel') {
      $thing = t('cancelation'); // confirming cancelation not post
      db\update('posts', ray('postid end', $postid, time()), 'postid');
      return postRet(t('Success! Your post is canceled.', compact('thing')));
      
    } elseif ($op == 'confirm') {
      $already = db\get('confirmed', 'people', compact('pid'));
      db\update('people', ray('pid confirmed', $pid, TRUE), 'pid');

      r\setCook(VIPID, $pid);

      if ($thing == 'message') {
        extract(db\get('postid,message,displayName,email', 'messages m JOIN people p ON pid=sender', compact('id')));
        postMsg($message, $displayName, $email, $postid);      
        return postRet('message success');
      } else { // post
        if ($already) {
          return go(POSTPAGE . '/op=show&postid=' . $idSub['postid']); // second confirmation means edit
        } else return postRet(t('post success'));
      }
      
    } elseif ($op == 'edit') {
      return go(POSTPAGE . '/op=show&postid=' . $idSub['postid']); // showing is editing, for the right person
      
    } else return u\FAIL('Bad do in confirm: ' . pr($code));


  } else return postRet(t('That %thing does not exist.', compact('thing')));
  
}

function formPosts_validate($form, &$sta) {
  $input = $sta['input'];
  extract(frmBools($frm = $input['frm'])); // make code a little easier to read

  if ($frmTop) {
    extract($topInfo = just(TOP_FLDS, $input, NULL));
    if ($err = u\badAmount($radius, '>=0', 6)) {$radius = 10; say($err);}
    $locus = ucwords($locus);
    list ($latitude, $longitude, $zip) = u\getLocus($locus);
    if ($latitude == 0 and $longitude == 0) {
      if (!$zip and !is_numeric($zip = u\parseOutZip($locus))) return say('bad locus', 'locus');
      // [better not to confuse user] say(zipOnlyMsg(t('you will see posts only'), $zip));
    }
    $chgFlds = 'locus latitude longitude zip';
    
  } elseif ($frmPost or $frmEditpost) {
    extract(just('end email cat radius', $input, NULL));
    if (!$cat) return say(t('You must choose a category.'), 'cat');
    if (isset($radius) and $err = u\badAmount($radius, '>=0', 6)) return say($err, 'radius');
    if ($end and $err = u\badDate($end, $frmPost ? strtotime('tomorrow') : today())) return say($err, 'end');
    if ($frmPost and !emailOkay($email, TRUE)) return;
    $chgFlds = 'email radius end';
    
  } elseif ($frmMessage) {
    extract(just('email', $input, NULL));
    if (!emailOkay($email, TRUE)) return;
    $chgFlds = 'email';
    
  } elseif ($frmWho) {
    extract(just('zip phone address city state zip type days washes', $input, NULL));
    if ($err = u\badZip($zip)) return say($err, 'zip');
    if ($err = u\badPhone($phone)) return say($err, 'phone');
    if (u\poAddress($address)) return say('po in location', 'address');
    foreach (ray('address city') as $k) if (in($$k, '0 1', strtolower($$k), strtoupper($$k))) $$k = ucwords(strtolower($$k));
    if (moderate($zip)) foreach (ray('days washes') as $k) if ($err = u\badAmount($$k, '>=0')) return say($err, $k);
    
    list ($latitude, $longitude, $zot) = u\getLocus($address, $city, $state, $zip);
    if ($latitude == 0 and $longitude == 0) {
      // [better not to confuse user] if ($type == 'post') say(zipOnlyMsg(t('your post will be shown to anyone'), $zip));
    }
    $chgFlds = 'zip phone latitude longitude days washes address city';
    
  } else return u\FAIL(t('bad form in validate: ' . $frm));
  
  u\preray(compact(ray($chgFlds)), $sta['input']);
}
      
/**
 * Process the results of various Offers & Needs forms:
 *   post, message, or contact info (this last including post or message info)
 */
function formPosts_submit($form, &$sta) {
  $input = just(POST_FLDS . ' frm pid days washes health ' . MESSAGE_FLDS . ' ' . CONTACT_FLDS, $sta['input'], NULL);
  extract(just('frm email type method pid days washes health zip', $input), NULL);
  extract(frmBools($frm)); // make code a little easier to read

  if ($frmTop) return go('community/posts/' . http_build_query(['op'=>'top'] + just(TOP_FLDS, $sta['input'])));

  if ($frmPost or $frmMessage) { // get pid for this post; if none, carry it through to the "who" form
    $pid = db\get('pid', 'people', compact('email'));
    if (!$pid) return w\go('/community/posts/' . http_build_query(['op'=>'who'] + $input));
  } elseif ($frmWho) {  // carrying info about post or message, as well as contact info
    $contactInfo = just(CONTACT_FLDS, $input, NULL);
    $method = ray(METHOD_OPTS)[$method];
    $health = moderate($zip) ? ($days+0) . ' ' . ($washes+0) . ' ' . ray(HEALTH_RATINGS)[$health] : '';

    $pid = db\insert('people', compact(ray('method health')) + $contactInfo, 'pid');
    if ($type) $frmPost = TRUE; else $frmMessage = TRUE; // now fall through to process the earlier data passed through by who form
  }

  // got pid, now process post or message
  $confirmed = (int) ($email == vip('email') or $frmEditpost); // set confirmation status of record we're about to create
  $created = time();
  $err = FALSE;
  
  if ($frmPost or $frmEditpost) {
    extract($info = just(POST_FLDS . ' postid', $input, NULL));
    if (!$end) $info['end'] = NULL; // this has to be here, not in validate
    foreach (ray('type item details cat pid') as $k) if (!$$k) $err = $k;
    if ($err) {
      r\tellAdmin('post save error', $info);
      return go(POSTPAGE, t('Apologies. There was an error saving your post. Our tech crew is looking into it.'), 'err');
    }
    if ($frmPost) {
      u\setDft($private, (int) moderate(db\get('zip', 'people', compact('pid'))));
      $info += compact('created');
    } else unset($info['created']); // don't change creation date or links stop working
    $postid = db\updateOrInsert('posts', compact(ray('pid private')) + $info, 'postid');
    if (!$confirmed) emailLink('post', compact('postid'), $created, $type, $item, $pid);
    if ($frmEditpost and $input['message']) $frmMessage = TRUE;
  }

  if ($frmMessage) {
    extract($info = just(MESSAGE_FLDS, $input, NULL));
    $sender = $pid;
    foreach (ray(MESSAGE_FLDS) as $k) if (!$$k) $err = $k;
    if ($err) {
      r\tellAdmin('msg save error', $info);
      return go(POSTPAGE, t('Apologies. There was an error creating your message. Our tech crew is looking into it.'), 'err');
    }
    $id = db\insert('messages', compact(ray('sender created')) + $info, 'id');
    if ($confirmed) {
      $fromName = db\get('displayName', 'people', compact('pid'));
      postMsg($message, $fromName, $email, $postid);
    } else {
      extract(db\get('type,item', 'posts', compact('postid')));
      emailLink('message', compact('id'), $created, $type, $item, $pid);
    }
  }
     
  w\go('/community/posts/op=top', 'info saved');
}

/**
 * Send a confirmation email.
 * @param string $thing: "post" or "message"
 * @param assoc $idSub: thing record ID name and value
 * @param int $created: thing record creation date/time
 * @param string $type: post type
 * @param string $item: brief description of post
 * @param int $pid: people record ID of person to send link to
 */
function emailLink($thing, $idSub, $created, $type, $item, $pid) {
  $code = getCode($thing, $idSub);
  $index = "confirm-$thing";
  extract(pVal('fullName,email', $pid));
  $to = [$email => $fullName];
  $date = u\fmtDate($created, TRUE);
  $what = $type == 'need' ? t('a need') : t('an offer');
  $noFrame = TRUE;
  r\rMail($index, $to, compact(ray('fullName date code what item noFrame')));
  say('confirm by email', compact('thing'));
}

/**
 * Return a code to confirm, cancel, or edit the given post or message.
 * @param string $thing: post or message
 * @param assoc $idSub: [record ID field name => record ID]
 */
function getCode($thing, $idSub) {
  $created = db\get('created', $thing . 's', $idSub);
  return str_replace(CRYPT_FLAG, '', u\cry('P', http_build_query($idSub + ray('created word', $created, R_WORD))));
}

/**
 * Send a message in response to a post.
 */
function postMsg($message, $fromName, $fromEmail, $postid) {
  extract(db\get('type,item,s.created,fullName,email', 'posts s JOIN people p USING (pid)', compact('postid')));
  $to = [$email => $fullName];
  $date = u\fmtDate($created, TRUE);
  $code = getCode('post', compact('postid')); // code for recipient to cancel post
  $noFrame = TRUE;
  r\rMail('post-message', $to, compact(ray('fullName fromName type item date message code noFrame')), $fromEmail);
  say('message sent');
}

function vip($fld = 'pid') {
  if (!$pid = nn($_COOKIE[VIPID])) return FALSE;
  return $fld == 'pid' ? $pid : db\get($fld, 'people', compact('pid'));
}

function catChoices($top, $extra = []) {return ['' => $top] + $extra + db\keyed('id,cat', 'post_cats', '1 ORDER BY sort');}
function emailFld() {return textFld(REQ . t('Your Email:'), [t('Your email address')], dft(nn($_COOKIE['email']) ?: vip('email')));}
function pVal($fld, $pid) {return db\get($fld, 'people', compact('pid'));}
function zipOnlyMsg($specifics, $zip) {return t('We are unable to geolocate that address at this time, so %specifics in the "%zip3" Zip Code area.', 'specifics zip3', $specifics, substr($zip, 0, 3));}
function msgFld($req = REQ) {return areaFld($req . t('Message to Send:'), t('Max 200 characters'), maxlen(200));}
function postRet($msg) {return r\acct() ? w\go('empty', $msg) : doSay($msg);}
function moderate($zip) {return in(substr(trim($zip), 0, 3), MODERATE_ZIPS);}
function frmBools($frm) {foreach(ray(POST_FRMS) as $k) $res['frm' . ucfirst($k)] = ($frm == $k); return $res;}
function admin() {global $mya; return ($mya and $mya->admin) ? $mya : FALSE;}
