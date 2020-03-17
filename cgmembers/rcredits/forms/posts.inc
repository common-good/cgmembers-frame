<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

define('ITEM_FLDS', 'type item details cat exchange emergency end uid'); // db fields other than contact info
define('CONTACT_FLDS', 'displayName fullName address city state zip phone email method'); // db contact fields
define('MESSAGE_FLDS', 'message from to'); // db message fields
define('METHOD_OPTS', 'email phone text'); // contact method options
define('VIPID', 'vipid'); // name of confirmed-email cookie (the VIP's people record ID)

/*    $buttons = btn('/community/posts/op=offer', t('Post an Offer'))
             . btn('/community/posts/op=need', t('Post a Need'));  */

/**
 * Exchange help with neighbors.
 */
function formPosts($form, &$sta, $args = '') {
  global $base_url;
  extract(just('op code zip id thing', $args, NULL));

  if (in($op, 'confirm cancel')) return confirmThing($op, $thing, $code);
  if ($op == 'who') return contactInfo($args, $id);
  if (in($op, 'offer need')) return postForm($op);
  if ($op == 'detail') return showDetails($op, $id);

  jsx('posts', 'active', $op);

  $title = t('Offers & Needs');
  $subtext = t('<p>%PROJECT Economy doesn\'t have to be about money. Let\'s just make sure everyone in the neighborhood has what they need, with no one left behind, so we can thrive and have fun together as a community. What\'s one thing that would make your life better? How might you help your neighbors?</p>
    <p>Offer only <i>gifts</i> of assistance (no ads for paid help). If you are interested in free STUFF, try <%a>freecycle.org</a>)</p>
    <p>If you know someone in need who has no Internet access, feel free to ask them if they want you to post something for them.</p>
    <p>Please email comments and suggestions to us at %CGF_EMAIL.</p>', '_a', atag('http://freecycle.org'));

  u\setDft($zip, nn($_COOKIE['vipzip'])); // default to zip from previous time
  $op = hidFld('gotzip');
  $zip = ziptogoFld($zip0 = $zip);
  
  if (!$zip0) { // user's first time ever
    $submit = submit(t('fake'), '', '', clas('invisible')); // Drupal keeps other solutions from calling _submit function)
    return cgform(compact(ray('title subtext zip submit op')));
  }

  // op = gotzip or whatever
  $cat = selectFld('&nbsp;', '', clas('limit-list'), catFld());

  foreach (ray('offer need') as $type) ${$type . 's'} = postList($type, $zip0);
  
  $tabs = item(<<< X
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
  );

  return cgform(compact(ray('title subtext zip cat tabs op')));
}

/**
 * Return a table of confirmed postings for the given type and category.
 */
function postList($type, $zip) {
/*  $cat = 'any';
  $all = $cat = 'any' ? '1' : '0';
      AND ($all OR c.cat=:cat) */
  
  $sql = <<<X
    SELECT s.id, c.cat, s.item, s.details, s.emergency FROM posts s
    LEFT JOIN post_cats c ON c.id=s.cat
    LEFT JOIN people p USING (pid)
    WHERE s.type=:type
      AND s.created <= :NOW
      AND IFNULL(s.end, :NOW) >= :NOW
      AND s.confirmed
      AND p.zip=:zip
    ORDER BY s.type,s.item,s.created DESC
X;
  $q = db\q($sql, compact(ray('type zip')));
  $rows[] = u\tableRow(ray(t('category item details')));;
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($emergency) $item = '<span class="emergency">!!</span> ' . $item;
    $item = lnk('/community/posts/op=detail&id=' . $id, $item);
    $rows[] = "<tr class=\"$cat\"><td>$cat</td><td>$item</td><td>$details</td></tr>\n";
  }

  return join("\n", $rows);
}

/**
 * Input an offer or need.
 */
function postForm($op) {
  $need = ($op == 'need');
  $title = $need ? t('Post a Need') : t('Post an Offer');
  $cat = selectFld(REQ . t('Category:'), '', '', catFld());
  $item = textFld(REQ . t('What:'), [$need ? t('What you need') : t('What you\'re offering')], maxlen(40));
  $details = areaFld(REQ . t('Details:'), t('max 200 characters'), maxlen(200));
  
  $exchange = $need ? boolFld(t('Exchange:'), t('Are you offering something in exchange? (include it in the details too)')) : hidFld(0);
  $emergency = boolFld(t('Emergency:'), $need ? t('This need is a short-term emergency.') : t('This offer is only for emergencies (such as 2 weeks of social distancing).'));
  $end = textFld(t('End Date:'), [t('mm/dd/yyyy'), t('if any')]);
  $uid = uidFld();
  $submit = submit(t('Post'));
  $op = $type = hidFld($op);
  
  return cgform(compact(ray('title cat item details exchange emergency end uid submit type op')));
}

/**
 * Show details about a post.
 */
function showDetails($op, $id) {
  global $mya;
  
  if (!$res = db\get('s.type,c.cat,s.item,s.details,s.emergency,p.displayName,p.fullName,p.phone', 'posts s LEFT JOIN post_cats c ON c.id=s.cat LEFT JOIN people p USING (pid)', 's.id=:id', compact('id'))) return softErr(t('That is not a need or offer.'));
  extract($res);
  
  $phone = u\fmtPhone($phone);
  if ($mya and $mya->admin) $displayName .= " (for admin only: $fullName $phone)";
  $need = ($type == 'need');
  if (!$need and $emergency) $item = t('(In emergency) ') . $item;
  
  $op = hidFld($op);
  $id = hidFld($id);

  $title = t('Details');
  $subtext = t('Use this form to send a message to the person who posted this %type. Your message will be sent by email with your display name and email address. Your full name and other contact information will not be sent unless you include it in the message.', compact('type'));
  $type = item($type, t('Type:'));
  $cat = item($cat, t('Category:'));
  $who = item($displayName, t('Who:'));
  $item = item($item, $need ? ($emergency ? t('Urgent Need:') : t('Need:')) : t('Offer:'));
  $details = item($details, t('Details:'));
  
  $message = areaFld(t('Message to Send:'), t('max 200 characters'), maxlen(200));
  $uid = uidFld();
// Keep in mind: the person you are contacting is almost certainly a neighbor, but might be a scoundrel pretending to live nearby.

  $submit = t('Send Message');

  return cgform(compact(ray('title subtext type cat who item details message uid submit id op')));
}

/**
 * Input name and contact information.
 */
function contactInfo($args, $id) {
  parse_str($args, $ray);
  foreach($ray as $k => $v) $info[$k] = hidFld($v);
  extract(just('op email', $args));
  
  $title = t('Your Information');
  $submit = t('Submit');
  $op = hidFld($op);
  $id = hidFld($id);
  return cgform(compact('title') + contactFlds($email) + compact(ray('submit op id')) + $info);
}

/**
 * Return contact fields.
 */
function contactFlds($email) {
  $subtext = t('<p>This information is required only the first time you post or contact someone.</p><p>Only your "display name" (first name or nickname) is shown to others. Your full name, address, email, and phone are not shown to anyone unless you choose "phone" or "text" as your preferred contact method (then just your phone number is displayed when someone asks to contact you about a specific post).</p>');
  $displayName = textFld(REQ . t('Display Name:'), [t('Your first name or nickname'), t('To be shown to publicly with your posts')]);
  $fullName = textFld(REQ . t('Name:'), [t('Your full name'), t('usename desc')]);
  $address = textFld(REQ . t('Street Address:'), [t('Physical address'), t('So people can search for offers and needs by neighborhood')]);
  $city = textFld(REQ . t('City:'), [t('City where you live')]);
  $state = stateFld(R_STATE_ID); // can't use required() here
  $zip = zipFld();
  $phone = textFld(REQ . t('Phone:'), [t('Phone number'), t('Just in case we need to contact you. This is not shared with other users unless you choose "phone" or "text" as your preferred contact method.')]);
  $method = w\radiosFld(REQ . t('Preferred Contact:'), t('The "phone" and "text" options apply only when a %PROJECT member contacts you (because we have verified the identity only of members). Otherwise you will receive replies by email.'), '', ray(METHOD_OPTS));
  $email = hidFld($email);

  return compact(ray('subtext ' . CONTACT_FLDS));
}

/**
 * Respond to user's click of confirmation and cancelation link in email.
 */
function confirmThing($op, $thing, $code) {
  global $mya;
  $code = u\decry('P', CRYPT_FLAG . $code);
  extract(just('id created', $code, NULL));
  
  $tnm = $thing == 'post' ? 'posts' : 'messages';
  if (db\exists($tnm, compact(ray('id created')))) {
    if ($op == 'cancel') {
      $thing = t('cancelation'); // confirming cancelation not post
      db\update('posts', ray('id end', $id, time()), 'id');
    } elseif ($op == 'confirm') {
      db\update($tnm, ray('id confirmed', $id, TRUE), 'id');
      $pid = db\get($thing == 'post' ? 'pid' : '`from`', $tnm, compact('id'));
      db\update('people', ray('pid confirmed', $pid, TRUE), 'pid');

      r\setCook(VIPID, $pid);

      if ($thing == 'message') {
        extract(db\get('postid,message,displayName,email', 'messages m JOIN people p ON pid=`from`', compact('id')));
        postMsg($message, $displayName, $email, $postid);      
      }
    } else return u\FAIL('Bad do in confirmThing: ' . pr($code));
    $msg = t('Success! Your %thing is confirmed.', compact('thing'));
  } else $msg = t('That %thing does not exist.', compact('thing'));
  
  return $mya ? w\go('empty', $msg) : doSay($msg);
}

function formPosts_validate($form, &$sta) {
  global $mya;
  extract(just('op type uid end zip phone address', $sta['input'], NULL));
  if ($op == 'gotzip') return;
  
  if ($uid) { // item form
    if (FALSE and $a = r\acct($uid)) { // NYI
      $uid = $a->id;
    } elseif (!emailOkay($uid, TRUE)) return;
    if ($end and $err = u\badDate($end, NOW)) return say($err, 'end');
    u\preray(compact(ray('uid end')), $sta['input']);
  } else { // contact form
    if ($err = u\badZip($zip)) return say($err, 'zip');
    if ($op == 'who') {
      if ($err = u\badPhone($phone)) return say($err, 'phone');
      if (u\poAddress($address)) return say('po in location', 'address');
    }
    u\preray(compact(ray('zip phone')), $sta['input']);
  }  
}

/**
 * Process the results of various Offers & Needs forms:
 *   offer/need, details (message), contact info (including offer/need or message info)
 */
function formPosts_submit($form, &$sta) {
  global $mya;
  $input = $sta['input'];
  extract($itemInfo = just(ITEM_FLDS, $input, NULL));
  extract($contactInfo = just(CONTACT_FLDS, $input, NULL));
  extract($msgInfo = just(MESSAGE_FLDS, $input, NULL));
  extract(just('op id message', $input));
  if ($zip) r\setCook('vipzip', $zip);
  
//  if ($op == 'gotzip') return go("community/posts/op=gotzip&zip=$zip");
  if (isset($method)) $method = ray(METHOD_OPTS)[$method];

  if ($op == 'who') { // from contact form (fall through)
    u\EXPECT((bool) $contactInfo, 'missing contactInfo in posts submit');
  } elseif (in($op, 'offer need detail')) { // from offers/needs or details/message form (show contact form or get contact info pid and fall through)

    if ($uid == vip('email')) {
      $email = $uid;
      $pid = db\get('pid', 'people', compact('email'));
    } elseif (strpos($uid, '@')) { // uid is an email
      $email = $uid;
      if ($contactInfo = db\get(join(',', ray('pid confirmed ' . CONTACT_FLDS)), 'people', 'email=:uid', compact('uid'))) {
        if (!$contactInfo['confirmed']) return go('community/posts', t('That email has not been confirmed. If you cannot find the confirmation email we sent, look in your spam folder.'), 'err');
        extract(just('pid method', $contactInfo)); // no need to save people record
      } else return w\go('/community/posts/op=who&' . http_build_query(ray('email uid id', $uid, 0, nn($id)) + $itemInfo + $msgInfo)); // one of ($itemInfo, $msgInfo) is null, $id is for messages only
        
    } elseif ($uid) { // uid is account ID, vetted in _validate
      u\FAIL('uid in posts submit'); // NYI
      $a = r\acct($uid);
      if (!$pid = db\get('pid', 'posts', compact('uid'))) { // first post or message for this member
        list ($displayName, $zot) = u\parseName($a->fullName);
        $contactInfo = ray('displayName method confirmed', $displayName, 'email', TRUE);
        foreach (ray(CONTACT_FLDS) as $k) if ($a->$k) $contactInfo[$k] = $a->$k;
      }
    }

  } else return u\FAIL(t('Bad op in posts submit: ') . pr($sta['input']));
  
  $created = time();

//  foreach (ray('uid end') as $k) if (!$$k) unset($itemInfo[$k]); // let these default to NULL if blank
  foreach (ray('uid end') as $k) if (!$$k) $itemInfo[$k] = NULL;
  $itemInfo['pid'] = $pid = nn($pid) ?: db\insert('people', compact(ray('method email created')) + $contactInfo, 'pid');
  $msg = 'info saved';
  
  if ($item) {
    $newId = db\insert('posts', $itemInfo, 'id');
  } else {
    $to = db\get('pid', 'posts', compact('id'));
    $newId = db\insert('messages', ray('postid from to message', $id, $pid, $to, $message), 'id');
  }
  
  if (vip('email')) {
    db\update($tnm, ray('id confirmed', $id, TRUE), 'id');
    if (!$item) { // message
      $fromName = db\get('displayName', 'people', compact('pid'));
      postMsg($message, $fromName, $email, $pid);
    }
  } else { // first time posting or messaging
    $code = str_replace(CRYPT_FLAG, '', u\cry('P', http_build_query(ray('id created word', $newId, $created, R_WORD))));
    $index = $item ? 'confirm-post' : 'confirm-message';
    $to = "$fullName <$email>";
    if ($message) extract(db\get('type,item,s.created as date', 'posts s JOIN people p USING (pid)', 'id=:pid', compact('pid')));
    $date = u\fmtDate($date, TRUE);
    $what = $type == 'need' ? t('a need') : t('an offer');
    $noFrame = TRUE;
    r\rMail($index, $to, compact(ray('fullName what item date code noFrame')));

    $msg .= '|confirm by email';
  }
  
  w\go('/community/posts', $msg);
}


/**
 * Send a message in response to a post.
 */
function postMsg($message, $fromName, $fromEmail, $postId) {
  extract(db\get('type,item,s.created,fullName,email', 'posts s JOIN people p USING (pid)', 'id=:postId', compact('postId')));
  $to = "$fullName <$email>";
  $date = u\fmtDate($created, TRUE);
  $noFrame = TRUE;
  r\rMail('item-message', $to, compact(ray('fullName fromName type item date message noFrame')), $fromEmail);
}

function catFld() {return ['' => t('Choose a category')] + db\keyed('id,cat', 'post_cats', '1 ORDER BY id');}
function ziptogoFld($zip = '') {return textFld('&nbsp;', [REQ . t('Your Zip Code')], dft($zip) + clas('ziptogo') + suffix(' ' . btn(BASE_URL . '/community/posts', t('Go'))));}
//function uidFld() {return textFld(REQ . t('Email or ID:'), [t('Email or account ID')], dft(nn($_COOKIE['uid'])));}
function uidFld() {return textFld(REQ . t('Your Email:'), [t('Your email address')], dft(vip('email')));}
function vip($fld) {return ($pid = nn($_COOKIE[VIPID])) ? db\get($fld, 'people', compact('pid')) : '';}
