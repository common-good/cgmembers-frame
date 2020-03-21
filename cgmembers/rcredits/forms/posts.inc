<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

define('POST_FLDS', 'type item details cat exchange emergency radius end'); // db fields other than contact info
define('CONTACT_FLDS', 'displayName fullName address city state zip phone email method'); // db contact fields
define('MESSAGE_FLDS', 'message postid sender'); // db message fields
define('TOP_FLDS', 'locus radius latitude longitude zip'); // fields used for filtering list
define('METHOD_OPTS', 'email phone text'); // contact method options
define('VIPID', 'vipid'); // name of confirmed-email cookie (the VIP's people record ID)

/*    $buttons = btn('/community/posts/op=offer', t('Post an Offer'))
             . btn('/community/posts/op=need', t('Post a Need'));  */

/**
 * Exchange help with neighbors.
 
 @todo: add a link in every reply that will both cancel the post AND tell the replier thank you and no longer needed or offered.
 - send an email the next day after a reply, asking if the offer/need is still ok (with a link to cancel it). At the same time, change confirmed to a date (NULL is not confirmed, date is most recent confirmation date). Allow ongoing.f
 - say when new things are posted in my area.
 
 */
function formPosts($form, &$sta, $args = '') {
  global $base_url;
  extract(just(TOP_FLDS . ' op code postid thing email', $args, NULL));

  foreach (ray(TOP_FLDS . ' email') as $k) {
    if ($$k) r\setCook($k, $$k); else $$k = nn($_COOKIE[$k]); // default from previous time (useful also in getWho)
    ${$k . '0'} = $$k; // remember initial values
  }

  if (in($op, 'confirm cancel')) return confirm($op, $thing, $code);
  if (in($op, 'offer need')) return getPost($op);
  if ($op == 'show') return showPost($postid);
  if ($op == 'who') return getWho($args);

  // op = top or nothing
  jsx('posts', 'active', $op);

  $title = t('Mutual Aid Offers & Needs');
  $subtext = t('<p>In our %PROJECT Economy we want to make sure everyone has what they need, with no one left behind.  What help do <i>you</i> need? What can you offer others (free! no ads for paid help)?</p>
    <!--p>If you are interested in giving or receiving free STUFF, try <%a>freecycle.org</a></p-->
    <p>If you know someone in need who has no Internet access, offer to post something for them. And tell your neighbors about this board!</p>
    <p>Please email comments and suggestions to us at %CGF_EMAIL.</p>', '_a', atag('http://freecycle.org'));

  $locus = textFld(REQ . t('Where:'), [t('Full address OR City,ST'), t('Type an address or just City, ST.')], dft($locus));
  $radius = textFld(REQ . t('Radius:'), [t('Max distance from address')], dft($radius ?: 10) + suffix(t('miles') . btn("$base_url/community/posts", t('Go'), '', 'sm')));
  $cat = rendA('cat', selectFld(BARE, '', clas('limit-list'), catChoices(t('Category (filter)'))));

  foreach (ray('offer need') as $type) {
    $k = $type . 's';
    $$k = preg_replace(t('/Category/'), $cat, postList($type, $latitude0, $longitude0, $zip0, $radius0), 1);
    $$k = preg_replace('/edit-cat/', "\$0-$type", $$k, 1);
  }
  $tabs = $locus0 ? tabFlds($offers, $needs) : NULL;

  $latitude = hidFld($latitude ?: 0); // to be filled in on the fly
  $longitude = hidFld($longitude ?: 0); // to be filled in on the fly
  $zip = hidFld($zip ?: ''); // to be filled in on the fly
  $submit = submit(t('fake'), '', '', clas('invisible')); // Drupal keeps other solutions from calling _submit function)
  $frm = hidFld('top');
  
  return cgform(compact(ray('title subtext locus radius tabs latitude longitude zip submit frm')));
}

/**
 * Return the field with all the information and action choices.
 */
function tabFlds($offers, $needs) { 
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
    <table>
      $offers
    </table>
  </div>
  <div id="tab-needs">
    <table>
      $needs
    </table>
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

  $sql = <<<X
    SELECT $distance AS dist, s.postid, c.cat, s.item, s.details, s.emergency FROM posts s
    LEFT JOIN post_cats c ON c.id=s.cat
    LEFT JOIN people p USING (pid)
    WHERE s.type=:type
      AND s.created <= :NOW
      AND IFNULL(s.end, :NOW) >= :NOW
      AND s.confirmed
      AND 1 or IF((latitude=0 AND longitude=0) OR (:lat=0 AND :lon=0), 
            MID(p.zip,1,3) = MID(:zip,1,3), 
            $distance < LEAST(IFNULL(s.radius, :radius), :radius)
          )
    ORDER BY s.type,s.item,s.created DESC
X;

///     debug(u\showq($sql, compact(ray('type lat lon zip radius'))));
  $q = db\q($sql, compact(ray('type lat lon zip radius')));
  $rows[] = u\tableRow(ray(t('category item details')));;
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
///      debug($dist);
    if ($emergency) $item = '<span class="emergency">!!</span> ' . $item;
    $item = lnk('/community/posts/op=show&postid=' . $postid, $item);
    $rows[] = "<tr class=\"$cat\"><td>$cat</td><td>$item</td><td>$details</td></tr>\n";
  }
  
  if (count($rows) == 1) $rows[] = tr('<tr class="none"><td colspan="3">%none</td></tr>', 'none', t('There are not yet any %types within %radius miles of that address.', compact(ray('type radius'))));

  return join("\n", $rows);
}

/**
 * Input an offer or need.
 */
function getPost($op) {
  $need = ($op == 'need');
  $title = $need ? t('Post a Need') : t('Post an Offer');
  $subtext = t('<p>Do not put identifying information in the "What" or "Details". People contact you through the system, without knowing your full name or email address. You can be more specific when you reply to them.</p><p>You will also need to confirm your first post by email.</p>');
  $cat = selectFld(REQ . t('Category:'), '', '', catChoices(t('Choosed a category')));
  $item = textFld(REQ . t('What:'), [$need ? t('What you need') : t('What you\'re offering')], maxlen(40));
  $details = areaFld(REQ . t('Details:'), t('Max 200 characters'), maxlen(200));
  
  $exchange = $need ? boolFld(t('Exchange:'), t('Are you offering something in exchange? (include it in the details too)')) : hidFld(0);
  $emergency = boolFld(t('Emergency:'), $need ? t('This need is a short-term emergency.') : t('This offer is only for emergencies (such as 2 weeks of social distancing).'));
  $radius = $need ? NULL : textFld(REQ . t('Radius:'), [t('Visibility radius'), t('Up to how many miles away do you want this seen? 10 usually covers neighboring towns. Use zero for "anywhere".')], dft(10) + suffix(t(' miiles')));
  $end = textFld(t('End Date:'), [t('mm/dd/yyyy'), t('When to stop listing this (you can also cancel it later)')]);
  $email = emailFld();
  $submit = submit(t('Post'));
  $type = hidFld($op);
  $frm = hidFld('post');
  
  return cgform(compact(ray('title subtext cat item details exchange emergency radius end email submit type frm')));
}

/**
 * Show details about a post.
 */
function showPost($postid) {
  global $mya;
  
  if (!$res = db\get('s.type,c.cat,s.item,s.details,s.emergency,p.displayName,p.fullName,p.phone', 'posts s LEFT JOIN post_cats c ON c.id=s.cat LEFT JOIN people p USING (pid)', compact('postid'))) return softErr(t('That is not a need or offer.'));
  extract($res);
  
  $phone = u\fmtPhone($phone);
  if ($mya and $mya->admin) $displayName .= " (for admin only: $fullName $phone)";
  $need = ($type == 'need');
  if (!$need and $emergency) $item = t('(In emergency) ') . $item;
  
  $postid = hidFld($postid);

  $title = t('Details');
  $subtext = t('Use this form to send a message to the person who posted this %type. Your message will be sent by email with your display name and email address. Your full name and other contact information will not be sent unless you include it in the message.', compact('type'));
  $type = item($type, t('Type:'));
  $cat = item($cat, t('Category:'));
  $who = item($displayName, t('Who:'));
  $item = item($item, $need ? ($emergency ? t('Urgent Need:') : t('Need:')) : t('Offer:'));
  $details = item($details, t('Details:'));
  
  $message = areaFld(REQ . t('Message to Send:'), t('Max 200 characters'), maxlen(200));
  $email = emailFld();
// Keep in mind: the person you are contacting is almost certainly a neighbor, but might be a scoundrel pretending to live nearby.

  $submit = t('Send Message');
  $frm = hidFld('message');

  return cgform(compact(ray('title subtext type cat who item details message email submit postid frm')));
}

/**
 * Input name and contact information.
 */
function getWho($args) {
  mb_parse_str($args, $ray);
  foreach(just(POST_FLDS . ' email ' . MESSAGE_FLDS, $ray) as $k => $v) $info[$k] = hidFld($v);
  extract(just('email', $info)); // because it's in CONTACT_FLDS
  
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

  $submit = t('Submit');
  $frm = hidFld('who');

  return cgform(compact(ray('title subtext ' . CONTACT_FLDS . ' submit frm')) + $info);
}

/**
 * Respond to user's click of confirmation and cancelation link in email.
 */
function confirm($op, $thing, $code) {
  global $mya;
  $code = u\decry('P', CRYPT_FLAG . $code);
  
  list ($tnm, $idnm) = $thing == 'post' ? ['posts', 'postid'] : ['messages', 'id'];
  extract(just($idnm . ' created', $code, NULL));
  $idSub = compact($idnm);

  if (db\exists($tnm, $idSub + compact('created'))) {
    if ($op == 'cancel') {
      $thing = t('cancelation'); // confirming cancelation not post
      db\update('posts', ray('postid end', $postid, time()), 'postid');
    } elseif ($op == 'confirm') {
      db\update($tnm, $idSub + ['confirmed' => TRUE], $idnm);
      $pid = db\get($thing == 'post' ? 'pid' : 'sender', $tnm, $idSub);
      db\update('people', ray('pid confirmed', $pid, TRUE), 'pid');

      r\setCook(VIPID, $pid);

      if ($thing == 'message') {
        extract(db\get('postid,message,displayName,email', 'messages m JOIN people p ON pid=sender', compact('id')));
        postMsg($message, $displayName, $email, $postid);      
      }
    } else return u\FAIL('Bad do in confirm: ' . pr($code));
    $msg = t('Success! Your %thing is confirmed.', compact('thing'));
  } else $msg = t('That %thing does not exist.', compact('thing'));
  
  return $mya ? w\go('empty', $msg) : doSay($msg);
}

function formPosts_validate($form, &$sta) {
  global $mya;
  extract(just('frm', $sta['input'], NULL));

  if ($frm == 'top') {
    extract(just(TOP_FLDS, $sta['input'], NULL));
    if ($err = u\badAmount($radius, '>=0', 6)) return say($err, 'radius');
    if (!$latitude and !$longitude) list ($latitude, $longitude, $zip) = u\parseLoc($locus);
    if ($latitude == LOCUS_ZIP) {
      $latitude = $longitude = 0;
      say(zipOnlyMsg(t('you will see posts only'), $zip));
    }
    if ($latitude == LOCUS_BAD) return say(t('That is not a recognizable full address or city,st. Try again.'), 'locus');
    $chgFlds = 'latitude longitude zip';
  } elseif ($frm == 'post') {
    extract(just('end email cat', $sta['input'], NULL));
    if (!$cat) return say(t('You must choose a category.'), 'cat');
    if (!emailOkay($email, TRUE)) return;
    if (isset($radius) and $err = u\badAmount($radius, '>=0', 6)) return say($err, 'radius');
    if ($end and $err = u\badDate($end, NOW)) return say($err, 'end');
    $chgFlds = 'email end';
  } elseif ($frm == 'message') {
    extract(just('email', $sta['input'], NULL));
    if (!emailOkay($email, TRUE)) return;
    $chgFlds = 'email';
  } elseif ($frm == 'who') {
    extract(just('zip phone address city state zip', $sta['input'], NULL));
    if ($err = u\badZip($zip)) return say($err, 'zip');
    if ($err = u\badPhone($phone)) return say($err, 'phone');
    if (u\poAddress($address)) return say('po in location', 'address');
    list ($latitude, $longitude, $zot) = u\getLocus($address, $city, $state, $zip);
    if (in($latitude, [LOCUS_ZIP, LOCUS_BAD])) {
      $latitude = $longitude = 0;
      if ($type == 'post') say(zipOnlyMsg(t('your post will be shown to anyone'), $zip));
    }
    $chgFlds = 'zip phone latitude longitude';
  } else return u\FAIL(t('bad form in validate: ' . $frm));
  
  u\preray(compact(ray($chgFlds)), $sta['input']);
}
      
/**
 * Process the results of various Offers & Needs forms:
 *   post, message, or contact info (this last including post or message info)
 */
function formPosts_submit($form, &$sta) {
  global $mya;
  $input = just(POST_FLDS . ' frm ' . MESSAGE_FLDS . ' ' . CONTACT_FLDS, $sta['input'], NULL);
  extract(just('frm email type method', $input), NULL);

  if ($frm == 'top') return go('community/posts/' . http_build_query(['op'=>'top'] + just(TOP_FLDS, $sta['input'])));

  if ($frm == 'post' or $frm == 'message') { // get pid for this post; if none, carry it through to the "who" form
    $pid = db\get('pid', 'people', compact('email'));
    if (!$pid) return w\go('/community/posts/' . http_build_query(['op'=>'who'] + $input));
  } elseif ($frm == 'who') {  // carrying info about post or message, as well as contact info
    $contactInfo = just(CONTACT_FLDS, $input, NULL);
    $method = ray(METHOD_OPTS)[$method];
    $pid = db\insert('people', compact('method') + $contactInfo, 'pid');
    $frm = $type ? 'post' : 'message'; // now fall through to process the earlier data passed through by who form
  }

  // got pid, now process post or message
  $confirmed = $email == vip('email') ? 1 : 0; // set confirmation status of record we're about to create
  $created = time();
  
  if ($frm == 'post') {
    extract($info = just(POST_FLDS, $input, NULL));
    if (!$end) $info['end'] = NULL; // this has to be here, not in validate
    $postid = db\insert('posts', compact(ray('pid confirmed created')) + $info, 'postid');
    if (!$confirmed) emailLink('post', compact('postid'), $created, $type, $item, $pid);
  } elseif ($frm == 'message') {
    extract($info = just(MESSAGE_FLDS, $input, NULL));
    $sender = $pid;
    $id = db\insert('messages', compact(ray('sender confirmed created')) + $info, 'id');
    if ($confirmed) {
      $fromName = db\get('displayName', 'people', compact('pid'));
      postMsg($message, $fromName, $email, $postid);
    } else {
      extract(db\get('type,item', 'posts', compact('postid')));
      emailLink('message', compact('id'), $created, $type, $item, $pid);
    }
  } else return u\FAIL(t('bad form at bottom of submit: ' . $frm));
     
  w\go('/community/posts', 'info saved');
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
  $code = str_replace(CRYPT_FLAG, '', u\cry('P', http_build_query($idSub + ray('created word', $created, R_WORD))));
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
 * Send a message in response to a post.
 */
function postMsg($message, $fromName, $fromEmail, $postid) {
  extract(db\get('type,item,s.created,fullName,email', 'posts s JOIN people p USING (pid)', compact('postid')));
  $to = [$email => $fullName];
  $date = u\fmtDate($created, TRUE);
  $noFrame = TRUE;
  r\rMail('post-message', $to, compact(ray('fullName fromName type item date message noFrame')), $fromEmail);
  say('message sent');
}

function vip($fld = 'pid') {
  if (!$pid = nn($_COOKIE[VIPID])) return FALSE;
  return $fld == 'pid' ? $pid : db\get($fld, 'people', compact('pid'));
}

function catChoices($top) {return ['' => $top] + db\keyed('id,cat', 'post_cats', '1 ORDER BY sort');}
function emailFld() {return textFld(REQ . t('Your Email:'), [t('Your email address')], dft(vip('email')));}
function pVal($fld, $pid) {return db\get($fld, 'people', compact('pid'));}
function zipOnlyMsg($specifics, $zip) {return t('We are unable to geolocate that address at this time, so %specifics in the "%zip3" Zip Code area.', 'specifics zip3', $specifics, substr($zip, 0, 3));}
