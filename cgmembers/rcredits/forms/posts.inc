<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const POST_FLDS = 'type service item details cat exchange emergency radius end private'; // db fields other than contact info
const CONTACT_FLDS = 'fullName displayName address city state zip phone email method uid'; // db contact fields
const MESSAGE_FLDS = 'message postid sender'; // db message fields
const TOP_FLDS = 'locus radius latitude longitude zip'; // fields used for filtering list
const POST_INPUT_FLDS = 'service cat item private exchange details emergency radius end type'; // post input fields
define('METHOD_OPTS', t('email phone text')); // contact method options
const VIPID = 'vipid'; // name of confirmed-email cookie (the VIP's people record ID)
const POSTPAGE = '/community/posts';
const MODERATE_ZIPS = ''; // 3-digit zip code areas where moderation is mandatory (eg '010 013')
const POST_FRMS = 'top post editpost who message'; // possible values for $frm
const POST_TYPES = 'need offer tip';
define('POST_TYPES_A', 'a need, an offer, a tip');
define('POST_TYPE_NAMES1', t('Need Offer Tip'));
define('POST_TYPE_NAMES2', t('Needs Offers Tips'));
define('POST_TERMS', t('%i0 Free, %i1 Pay, %i2 Trade or Pay'));
const POST_TERM_ICONS = '<span class="glyphicon glyphicon-heart"></span>&nbsp;, <span class="forpay">$</span>, <span class="glyphicon glyphicon-retweet"></span>&nbsp;';
define('TERMS_CHOICES', t('-1:Free/Pay/Trade, 0:Free, 1:Pay, 2:Trade or Pay'));
define('POST_GORS', t('Service Goods'));
define('SORG_CHOICES', t('-1:G & S, 0:Goods, 1:Services'));
define('CAT_FLAG', '[CAT]'); // gets replaced programmatically
const MY_POSTS = 98; // select OPTION index for my posts

/*
  Features:
  - View the lists of tips, offers, and needs for a certain address and radius
  - Select a certain Category to limit the list
  - Click an item to view it in more detail, but without personal identifying information (more detail for administrators )
  - Reply to an item being viewed (replies to offers and needs are delivered by email; replies to tips are posted)
  - Post a tip, offer, or need
  - Click the confirmation link in the email you get after posting or replying (just the first time)
  - Receive a reply to something you posted
  - Cancel (terminate) a Post (immediately or much later) by clicking a link in the confirmation email or editing the post
  - Edit your post (or someone else's if you are an administrator)
  - Search for text
  - View just your own posts (or someone else's if you are an administrator)  
  - Moderation required in certain 3-digit zip areas
  - COVID 19 risk survey (self-vetting)
  - Automatic translation (in process)
  - Say when new things are posted in my area.
*/

/**
 * Exchange help with neighbors.
 
  @todo: 
  - link in every reply that will both cancel the post AND tell the replier thank you and no longer needed or offered.
  - send an email the next day after a reply, asking if the offer/need is still ok (with a link to cancel it). At the same time, change confirmed to a date (NULL is not confirmed, date is most recent confirmation date). Allow ongoing.f
  - For administrators, more fields in the summary list

  For St. Louis:
  - Reconcile the current categories with ones SESL currently uses and color-code them. And/or maybe implement key words.
 */
function formPosts($form, &$sta, $args = '') {
  global $base_url;
  extract(just('op code', $args, NULL));
  if ($code == '%code') u\FAIL('top');
  
  $confirming = in($op, 'confirm cancel edit');
  if ($code and !$confirming) $args = u\deurlify($code);
  extract(just(TOP_FLDS . ' postid thing email', $args, NULL));
  if (nn($zip) and !nn($radius)) $radius = DFT_RADIUS; // allow community link as domain/community/posts/zip=whatever

  foreach (ray(TOP_FLDS . ' email') as $k) {
    if (nn($$k)) r\setCook($k, $$k); else $$k = r\cook($k); // default from previous time (useful also in getWho)
    ${$k . '0'} = $$k; // remember initial values
  }

  if ($confirming) return confirm($op, $thing, $code);
  if ($op == 'post') return getPost();
  if ($op == 'show') return showPost($postid);
  if ($op == 'who') return getWho($args);
  if ($op == 'prefs') return getPrefs($code);

  $op = 'top';
  extract(just(TOP_FLDS, $code)); // overwrites same fields from $args

  jsx('posts', 'listView memoView myPosts noLocYet isMobile', t('List View'), t('Memo View'), MY_POSTS, !isset($latitude), u\isMobile());

  $title = t('Offers, Needs, & Tips');
  $subtext = t('<p>In our %PROJECT Economy we want to make sure everyone has what they need, with no one left behind. Contact us to comment or collaborate or for administrative access.</p>');
  
  // <p>Think about what you need and what you want to offer in each category. This only works if you also get a couple neighbors to post something, so do that too!</p>
  
  if (in(substr(nn($zip), 0, 3), '010 013')) $subtext .= t('<p>In Western Massachusetts see Community Action\'s excellent <%a>Look4Help.org</a> website for every type of help you can imagine from government, social services, and community organizations.</p>', '_a', atag('http://look4help.org'));
  
  $view = btn('', t('List View'), '', '', 'edit-view');
  $where = btn('', $radius ? t('Where') : t('Look Nearby'), '', '', 'edit-where');
  $post = btn(POSTPAGE . '/op=post', t('Post!'), '', '', 'edit-post');

  $type = rendA('type', selectFld(BARE, '', '', ray(t('-1:Any type,') . u\deray(POST_TYPE_NAMES2))));
  $choices = catChoices(t('Any Category'), [MY_POSTS => t('my posts')]);
  $cat = rendA('cat', selectFld(BARE, '', '', $choices));
  $terms = rendA('terms', selectFld(BARE, '', '', ray(TERMS_CHOICES)));
  $sorg = rendA('sorg', selectFld(BARE, '', '', ray(SORG_CHOICES)));

  $search = rendA('search', textFld(BARE, [t('Search')]));
  $go = btn('', '&#128269;', '', '', 'edit-go'); // magnifying glass
  $nogo = btn('', '&#128269;', '', '', 'edit-nogo'); // magnifying glass
  $searchGroup = '<div id="searchGroup">' . $search . $go . $nogo . '</div>';
    
  $rows = postList('', $latitude0, $longitude0, $zip0, $radius0);
  $none = tr('<div id="%id">%none</div>', 'id none', count($rows) < 4 ? 'none0' : 'none', t('There are no such posts (try within a larger distance?)'));
  $list = join("\n", $rows);

  $list = item(<<< X
<div id="list" class="memo form-group">
  <div class="container">
    $list
  </div>
  $none
</div>
X
  , BARE);

  $clear = '<div class="clear"></div>';

  // location fields
  $locus = rendA('locus', textFld(REQ . BARE, [t('Full Address (include zip)')], dft($locus ?: myLocus()) + suffix('+ ')));
  $radius = rendA('radius', textFld(REQ . BARE, [t('Radius')], dft($radius ?: DFT_RADIUS) + suffix(t('miles'))));
  $point = btn("$base_url/community/posts", t('Go'), '', '', 'edit-point');
  $locset = "<div id=\"locset\">$locus$radius$point</div>$clear";

  $latitude = hidFld($latitude ?: 0); // to be filled in on the fly (by JS)
  $longitude = hidFld($longitude ?: 0); // to be filled in on the fly
  $zip = hidFld($zip ?: ''); // to be filled in on the fly
  $submitter = submit(); // Drupal keeps other solutions from calling _submit function)

  $buttons = item("$view$where$post$type$cat$clear$locset$terms$sorg$searchGroup");
  
  $frm = hidFld('top');
  
  return cgform(compact(ray('title subtext buttons list frm latitude longitude zip submitter')), 'nolabs wide');
}

/**
 * Return a table of confirmed postings of the given type.
 * @param string $type: post, offer, or tip (default ALL)
 * @param numeric $lat: latitude of user
 * @param numeric $lon: longitude of user
 * @param string $zip: zip of user
 * @param numeric $radius: up to how far away user is interested
 */
function postList($type, $lat, $lon, $zip, $radius) {
  global $mya;
  
  $myPid = vip('pid') ?: '0';
  $admin = admin($zip) ? 1 : 0;
  $zipIrrelevant = ($mya and $mya->admin) ? 1 : 0;
  
  $q = getPosts($type, $lat, $lon, $zip, $radius, 0, $admin, $zipIrrelevant);
  return postRows($q);
}

/**
 * Input an offer or need.
 * @param string $type: 
 */
function getPost() {
//  extract(just('offercat needcat tipcat', $_COOKIE, NULL));

//  $need = ($type == 'need');
//  $postCat = ($id = nn($_COOKIE[$type . 'cat'])) ? db\get('cat', 'post_cats', compact('id')) : '';
//  $lastCat = $postCat ? t('Your last post was in the "%postCat" category.', compact(ray('type postCat'))) : '';
  $_confirm = vip('confirmed') ? '' : t('<p>You will also need to confirm your first post by email.</p>');
  
  $frm = hidFld('post');
  $title = t('Post Something');
  $subtext = t('<p>Think about what you want to post in each category, one at a time.</p><p>Do not put identifying information in the "What" or "Details". People contact you through the system, without knowing your full name or email address. You can be more specific when you reply to them.</p>%confirm', compact('_confirm'));
  extract(postInputFlds());
  $email = eFld();
  $submit = submit(t('Post'));
  
  return cgform(compact(ray('title subtext type service cat item private exchange details emergency radius end email submit frm')));
}

/**
 * Return input fields common to creating or editing a post.
 */
function postInputFlds($dfts = []) {
  extract(just(POST_INPUT_FLDS, $dfts, NULL));

  jsx('post-post', 'types', POST_TYPES);

  $type = radiosFld(REQ . t('Type:'), '', dft(isset($type) ? typeNum($type) : -1), ray(POST_TYPE_NAMES1));
  $service = radiosFld(REQ . t('Goods or Service:'), '', dft(nn($service, -1)), ray(POST_GORS));
  $choices = catChoices(t('Choose a category'));
  $cat = array_search($cat, $choices);
  $cat = selectFld(REQ . t('Category:'), '', dft($cat), $choices);
  
  $item = textFld(REQ . t('What:'), [t('What you\'re offering or need'), t('Include the price if you\'re selling.')], dft($item) + maxlen(40));
  $private = admin(nni($dfts, 'zip')) ? boolFld(t('Private:'), t('Seen only by the administrator'), dft((int) $private)) : hidFld(0);

  $terms = tr(POST_TERMS, array_combine(ray('_i0 _i1 _i2'), ray(POST_TERM_ICONS)));
  $exchange = radiosFld(REQ . t('Terms:'), t('If not Free, put the price or options for exchange in the Details, below.'), dft(nn($exchange, 0)), ray($terms));
  $details = areaFld(REQ . t('Details:'), [t('Describe the item'), t('Describe. Also include words you think people might use to search for this item. Max 500 characters')], dft($details) + maxlen(500));
  
  $emergency = boolFld(t('Emergency:'), t('This is an emergency need or applies only in an emergency.'), dft($emergency ? 1 : 0));
  
  $dft = isset($radius) ? dft(u\fmtAmt($radius, 's')) : [];
  $radius = numFld(REQ . t('Radius:'), [t('Visibility radius'), t('Up to how many miles away do you want this seen? 10 usually covers neighboring towns. Use zero for "anywhere".')], $dft + suffix(t(' miles')));
  $end = dateFld(t('End Date:'), [t('mm/dd/yyyy'), t('When to stop listing this (you can also cancel it at any time)')], dft($end) + ($dfts ? suffix(lnk('', t('end now'))) : []));

  return compact(ray(POST_INPUT_FLDS));
}

/**
 * Show details about a post.
 */
function showPost($postid) {
  global $mya;
  
  $distance = tr('%DISTANCE_BETWEEN');
  $noGeo = tr('%NO_DISTANCE');
  list ($lat, $lon) = [nni($_COOKIE, 'latitude', 0), nni($_COOKIE, 'longitude', 0)];
  
  $flds = strtr(<<< X
    pid,s.type,s.service,c.cat,s.item,s.private,s.details,s.exchange,s.radius,s.emergency,s.created,s.end,s.confirmed AS postOk,
    p.uid,p.displayName,p.fullName,p.phone,p.email,p.address,p.city,p.state,p.zip,p.method,p.created AS userSince,
    p.confirmed AS personOk,IF ($noGeo, '', $distance) AS dist
X
  , ray(':lat :lon', $lat, $lon));
  if (!$res = db\get($flds, 'posts s LEFT JOIN post_cats c ON c.id=s.cat LEFT JOIN people p USING (pid)', compact('postid'))) return softErr(t('That is not a need or offer.'));
  extract($res);
  
  $admin = admin($zip);
  $phone = u\fmtPhone($phone);
  if ($emergency and $type != 'need') $item = t('(In emergency) ') . $item;
  $ok = ($uid and db\get(':IS_OK', 'users', compact('uid')));
  $member = $ok ? t(' - member') : t(' - non-member');
  
  // set up display-only fields that are the same for user's own post and someone else's post
  $created = item(u\fmtDate($created, TRUE), t('Posted:'));
  $mi = fmtDist($dist, $zip);
  if ($mi) $mi = " ($mi mi)";
  $state = r\realState($state);
  $location = "$city, $state$mi<br>";

  if ($admin) {
    list ($userSince, $phone) = [u\fmtDate($userSince, TRUE), u\fmtPhone($phone)];
    if ($personOk) $personOk = t(' (confirmed)');
    if ($ok and $mya and $mya->cAdmin) $member = ' # ' . r\changeAcctLink($uid, r\qid($uid));
    
    $who = <<< X
      $fullName ("$displayName")<b>$member</b><br>
      $phone<br>
      $location<br>
      $email$personOk<br><br>
      Prefers: $method<br>
      User since $userSince
X;
  } else {
    $who = <<< X
      $displayName <b>$member</b><br>
      $location<br>
X;
  }

  $location = hidFld($location);
  $who = item($who, t('Who:'));

  // set up fields that are the different for user's own post and someone else's post
  
  if (vip('pid') == $pid or $admin) { // user's own post: allow editing or termination
    $frm = hidFld('editpost');
    $title = t('Edit Post');
    extract(postInputFlds($res));
    if ($admin) {
      $message = msgFld('');
      $email = hidFld($mya->email); // messages from admin always come from admin
      $subtext = $postOk ? t('Post CONFIRMED') : t('Post NOT confirmed');
    } else $subtext = $email = $message = NULL;
    
    $pid = hidFld($pid);
    $submit = t('Update');    
  } else { // someone else's post
    db\q('UPDATE posts SET hits=hits+1 WHERE postid=:postid', compact('postid'));
    $frm = hidFld('message');
    $title = t('Details');
    $cat = item($cat, t('Category:'));
    if ($emergency) {
      list ($label, $item) = $type == 'need' 
      ? [t('Urgent Need:'), $item] 
      : [typeName($type, 1) . ':', t('(In emergency) ') . $item];
    } else $label = typeName($type, 1) . ':';
    
    $item = item($item, $label);
    $details = item($details, t('Details:'));
    $service = $exchange = $emergency = $radius = $end = NULL;
    
    if ($type != 'tip') {
      $subtext = t('Send a message to the person who posted this %type. They will see your display name, city, and email address. You can include your full name and/or other contact information in the message (or not).', ['type' => lcfirst(typeName($type, 1))]);
      $email = eFld();
      $message = msgFld(REQ);
      $submit = t('Send Message');
    } else $subtext = $email = $message = $submit = NULL;

    $type = $pid = $private = NULL;
  // Keep in mind: the person you are contacting is almost certainly a neighbor, but might be a scoundrel pretending to live nearby.
  }
  $postid = hidFld($postid);

  return cgform(compact(ray('title subtext type service cat who created private item exchange details emergency radius end message email submit postid pid location frm')));
}

/**
 * Input name and contact information.
 */
function getWho($args) {
  mb_parse_str($args, $ray);
  $info = []; // needed if $args/$ray is empty
  foreach(just(POST_FLDS . ' email location ' . MESSAGE_FLDS, $ray) as $k => $v) $info[$k] = hidFld(${$k . 0} = $v);
  extract(just('email', $info)); // because it's in CONTACT_FLDS

  jsx('post-who', ray('moderateZips', MODERATE_ZIPS));

  if ($uid = isMember($email0)) {
    $a = r\acct($uid);
    foreach (ray('fullName address city state zip phone uid') as $k) $$k = hidFld($a->$k);
  } else {
    $uid = NULL;
    if ($locus = nni($_COOKIE, 'locus')) {
      list ($address, $city, $state, $zip) = u\parseAddr($locus);
      $state = r\stateNum($state);
    } else $address = $city = $state = $zip = '';

    $fullName = textFld(REQ . t('Name:'), [t('Your full name'), t('usename desc')]);
    $address = textFld(REQ . t('Street Address:'), [t('Physical address'), t('So people can search for offers and needs by neighborhood')], dft($address));
    $city = textFld(REQ . t('City:'), [t('City where you live')], dft($city));
    $state = stateFld($state ?: R_STATE_ID); // can't use required() here
    $zip = zipFld($zip);
    $phone = phoneFld(REQ . t('Phone:'), [t('Phone number'), t('Just in case we need to contact you. This is not shared with other users unless you choose "phone" or "text" as your preferred contact method. We have only ever needed to contact one person so far (as of %date), but it helped a lot to have their phone number.', 'date', u\fmtDate(NOW - DAY_SECS, TRUE))]);
  }
  
  $frm = hidFld('who');
  $title = t('Your Information');
  $subtext = t('<p>This information is required only the first time you post or contact someone.</p><p>Only your "display name" (first name or nickname) is shown to others with your city and state. Your full name, street address, email, and phone are not shown to anyone unless you choose "phone" or "text" as your preferred contact method (then just your phone number is displayed when someone asks to contact you about a specific post).</p>');
  $displayName = textFld(REQ . t('Display Name:'), [t('Your first name or nickname'), t('To be shown with your posts, so your neighbors will recognize you.')]);
  $method = w\radiosFld(REQ . t('Preferred Contact:'), t('The "phone" and "text" options apply only when a %PROJECT member contacts you (because we have verified the identity only of members). Otherwise you will receive replies by email.'), '', ray(METHOD_OPTS));

  $midtext = t('And now some COVID-19 self-assessment questions for 2020:');

  if (!isset($location)) $location = NULL;
  $submit = t('Submit');

  return cgform(compact(ray('title subtext ' . CONTACT_FLDS . ' midtext submit location frm')) + $info);
}

/**
 * Respond to user's click of confirm, cancel, or edit link in email.
 */
function confirm($op, $thing, $code) {
  $ray = u\decryRay($code);

  list ($tnm, $idnm) = $thing == 'post' ? ['posts', 'postid'] : ['messages', 'id'];
  extract(just($idnm . ' location', $ray, NULL));
  $idSub = compact($idnm);
  $successUrl = POSTPAGE . (nni($_COOKIE, 'radius') ? '/op=top' : '');

  if (db\exists($tnm, $idSub)) {
    $pid = db\get($thing == 'post' ? 'pid' : 'sender', $tnm, $idSub);
    
    if ($op == 'cancel') {
      $thing = t('cancelation'); // confirming cancelation not post
      db\update('posts', ray('postid end', $postid, now()), 'postid');
      return go($successUrl, t('Success! Your post is canceled.', compact('thing')));
      
    } elseif ($op == 'confirm') {
      $already = db\get('confirmed', $tnm, $idSub);
      db\update('people', ray('pid confirmed', $pid, 1), 'pid'); // might already be confirmed if this is a new device
      db\update($tnm, ['confirmed' => 1] + $idSub, $idnm);

      r\setCook(VIPID, $pid);

      if ($thing == 'message') {
        extract(db\get('postid,message,displayName,email', 'messages m JOIN people p ON pid=sender', compact('id')));
        postMsg($message, $displayName, $email, $location, $postid);      
        return go($successUrl, 'message success');
      } else { // post
        if ($already) {
          return go(POSTPAGE . '/op=show&postid=' . $idSub['postid']); // second confirmation means edit
        } else return go($successUrl, 'post success');
      }
      
    } elseif ($op == 'edit') {
      return go(POSTPAGE . '/op=show&postid=' . $idSub['postid']); // showing is editing, for the right person
      
    } else return u\FAIL('Bad do in confirm: ' . pr($ray));


  } else return softErr(t('That %thing does not exist.', compact('thing')));
  
}

function formPosts_validate($form, &$sta) {
  global $mya;
  
  $input = $sta['input'];
  extract(frmBools($frm = $input['frm'])); // make code a little easier to read

  if ($frmTop) {
    extract($topInfo = just(TOP_FLDS, $input, NULL));
    if ($err = u\badAmount($radius, '>=0', 6)) {$radius = DFT_RADIUS; say($err);}
    if ($locus and $locus == myLocus()) {
      foreach (ray('latitude longitude zip') as $k) $$k = $mya->$k;
    } else {
      $locus = ucwords($locus);
      list ($latitude, $longitude, $zip) = u\getLocus($locus);
      if ($latitude == 0 and $longitude == 0) {
        if (!$zip and !is_numeric($zip = u\parseOutZip($locus))) return say('bad locus', 'locus');
        // [better not to confuse user] say(zipOnlyMsg(t('you will see posts only'), $zip));
      }
    }
    $chgFlds = 'locus latitude longitude zip';
    
  } elseif ($frmPost or $frmEditpost) {
    extract(just('end email cat radius service exchange emergency type', $input, NULL));
    $type = ray(POST_TYPES)[typeNum($type)]; // convert to text (typeNum() is for tests -- no effect in production)
    if (!$cat) return say(t('You must choose a category.'), 'cat');
    if (isset($radius) and $err = u\badAmount($radius, '>=0', 6)) return say($err, 'radius');
    if ($end and $err = u\badDate($end, $frmPost ? strtotime('tomorrow') : today())) return say($err, 'end');
    if ($frmPost and !emailOkay($email, TRUE)) return;
    foreach (ray('service exchange emergency') as $k) $$k = (int) $$k;
    $chgFlds = 'email radius end service exchange emergency type';
    
  } elseif ($frmMessage) {
    extract(just('email', $input, NULL));
    if (!emailOkay($email, TRUE)) return;
    $chgFlds = 'email';
    
  } elseif ($frmWho) {
    extract(just('zip phone address city state zip type', $input, NULL));
    if ($err = u\badZip($zip)) return say($err, 'zip');
    if ($err = u\badPhone($phone)) return say($err, 'phone');
    if (u\poAddress($address)) return say('po in location', 'address');
    foreach (ray('address city') as $k) if (in($$k, '0 1', strtolower($$k), strtoupper($$k))) $$k = ucwords(strtolower($$k));
    
    list ($latitude, $longitude, $zot) = u\getLocus($address, $city, $state, $zip);
    if ($latitude == 0 and $longitude == 0) {
      // [better not to confuse user] if ($type == 'post') say(zipOnlyMsg(t('your post will be shown to anyone'), $zip));
    }
    $chgFlds = 'zip phone latitude longitude address city';
    
  } else return u\FAIL(t('bad form in validate: ' . $frm));
  
  u\preray(compact(ray($chgFlds)), $sta['input']);
}
      
/**
 * Process the results of various Offers & Needs forms:
 *   post, message, or contact info (this last including post or message info)
 */
function formPosts_submit($form, &$sta) {
  $input = just(POST_FLDS . ' frm pid location ' . MESSAGE_FLDS . ' ' . CONTACT_FLDS, $sta['input'], NULL);
  extract(just('frm email type message method pid zip location', $input), NULL);
  extract(frmBools($frm)); // make code a little easier to read

  if ($frmTop) return go(codedUrl('top', just(TOP_FLDS, $sta['input'])));
  
  if ($frmPost or $frmMessage) { // get pid for this post or message; if none, carry it through to the "who" form
    if (!$pid = db\get('pid', 'people', compact('email'))) return w\go(codedUrl('who', $input));
    if (!db\get('confirmed', 'people', compact('pid'))) { // never confirmed first thing
      say(t('NOTE: You also have a previous post or message that you never confirmed. If you can\'t find it, look in your Spam folder.'));
//      if (oldEmailLink($pid)) return say(t('You must confirm your first post or message before you can complete this one. We are sending you another confirmation email now. Leave this window open, click the link in the email, then come back here and refresh the screen to re-submit this. If you need help, <%a>contact us</a>!', '_a', atag(PROMO_URL . '/about-us/contact-us/')), 'err');
    }
  } elseif ($frmWho) {  // carrying info about post or message, as well as contact info
    $contactInfo = just(CONTACT_FLDS, $input, NULL);
    $method = ray(METHOD_OPTS)[$method];
    $notices = NOTICE_DFTS;

    u\EXPECT(!db\get('pid', 'people', compact('email')), 'duplicate people email in post submit');
    $pid = db\insert('people', compact(ray('method notices')) + $contactInfo, 'pid');
    if ($message) $frmMessage = TRUE; else $frmPost = TRUE; // now fall through to process the earlier data passed through by who form
  }

  // got pid, now process post or message
  $confirmed = (int) (vip('pid') == $pid and vip('confirmed') and (vip('email') == $email or $frmEditpost)); // get confirmation status of person posting, editposting, or messaging -- vip('confirmed') is for DEV and double-check
  if (admin(vip('zip'))) $confirmed = 1;
  $created = now();
  $err = FALSE;
  
  if ($frmPost or $frmEditpost) {
    extract($info = just(POST_FLDS . ' postid', $input, NULL));
    if ($frmEditpost and db\get('confirmed', 'posts', compact('postid'))) $confirmed = 1;
    if (!$end) $info['end'] = NULL; // this has to be here, not in validate
    foreach (ray('type item details cat pid') as $k) if (!$$k) $err = $k;
    if ($err) {
      r\tellAdmin('post save error: ' . $err, $input);
      return go(POSTPAGE, t('Apologies. There was an error saving your post. Our tech crew is looking into it.'), 'err');
    }
    
    if ($frmPost) {
      u\setDft($private, (int) moderate(db\get('zip', 'people', compact('pid'))));
      $info += compact('created');
    } else unset($info['created']); // don't change creation date or links stop working

    $postid = db\updateOrInsert('posts', compact(ray('type pid private confirmed')) + $info, 'postid');

    if ($confirmed) {
      if ($frmEditpost and $end and $end <= now()) {
        say(t('Your post is ended.'));
      } else say(tr('post will appear', 'when', $private ? t('when it is approved') : t('immediately')));
    } else emailLink('post', compact('postid'), $created, $type, $item, $pid);
    if ($frmEditpost and $input['message']) $frmMessage = TRUE; // handle message from admin
  }

  if ($frmMessage) {
    extract($info = just(MESSAGE_FLDS, $input, NULL));
    $sender = $pid;
    foreach (ray(MESSAGE_FLDS) as $k) if (!$$k) $err = $k;
    if ($err) {
      r\tellAdmin('msg save error', $input);
      return go(POSTPAGE, t('Apologies. There was an error creating your message. Our tech crew is looking into it.'), 'err');
    }
    $id = db\insert('messages', compact(ray('sender created confirmed')) + $info, 'id');
    db\q('UPDATE posts SET contacts=contacts+1 WHERE postid=:postid', compact('postid'));

    if ($confirmed) {
      $fromName = db\get('displayName', 'people', compact('pid'));
      postMsg($message, $fromName, $email, $location, $postid);
    } else {
      extract(db\get('type,item', 'posts', compact('postid')));
      emailLink('message', compact('id'), $created, $type, $item, $pid, compact('location'));
    }
  }
     
  w\go(POSTPAGE . '/op=top', 'info saved');
}

/**
 * Send a confirmation email.
 * @param string $thing: "post" or "message"
 * @param assoc $idSub: thing record ID name and value
 * @param int $created: thing record creation date/time
 * @param string $type: post type
 * @param string $item: brief description of post
 * @param int $pid: people record ID of person to send link to
 * @param assoc $extra: other values to encode in emailed link
 */
function emailLink($thing, $idSub, $created, $type, $item, $pid, $extra = []) {
  $code = w\getEmailCode($idSub, $extra);
  $index = "confirm-$thing";
  extract(pVal('fullName,email', $pid));
  $to = [$email => $fullName];
  $date = u\fmtDate($created, TRUE);
  $what = ray(t(POST_TYPES_A))[typeNum($type)];
  $noFrame = TRUE;
  r\rMail($index, $to, compact(ray('fullName date code what item noFrame')));
  $thing = $thing == 'post' ? t('post') : t('message'); // translatable
  say('confirm by email', compact('thing'));
}

/**
 * Resend the first confirmation link for the given unconfirmed person.
 *//*
function oldEmailLink($pid) {
  if ($res = db\get('postid, created, type, item', 'posts', ray('pid confirmed', $pid, 0))) {
    extract(just('postid created type item', $res, NULL));
    list ($thing, $idSub) = ['post', compact('postid')];
  } elseif ($res = db\get('id, created, postid', 'messages', ray('sender confirmed', $pid, 0))) {
    extract(just('postid created', $res, NULL));
    list ($thing, $idSub) = ['message', compact('id')];
    extract(db\get('type, item', 'posts', compact('postid')));
  } else {
    r\tellAdmin(t('Cannot send old email about people pid=') . $pid);
    return softErr(t('There is an unusual problem with your data. An administrator is looking into it. We apologize for the inconvenience. Your post or message did not succeed.'));
  }
  emailLink($thing, $idSub, $created, $type, $item, $pid);
}*/

/**
 * Send a message in response to a post.
 */
function postMsg($message, $fromName, $fromEmail, $fromLocation, $postid) {
  extract(db\get('type,item,s.created,fullName,email', 'posts s JOIN people p USING (pid)', compact('postid')));
  $to = [$email => $fullName];
  $date = u\fmtDate($created, TRUE);
  $code = w\getEmailCode(compact('postid')); // code for recipient to cancel post
  $noFrame = TRUE;
  r\rMail('post-message', $to, compact(ray('fullName fromName fromLocation type item date message code noFrame')), $fromEmail);
  say('message sent');
}

/**
 * Return a specified field from the people record of the last person verified on the current device.
 */
function vip($fld = 'pid') {
  if (!$pid = nni($_COOKIE, VIPID)) return FALSE;
  return $fld == 'pid' ? $pid : db\get($fld, 'people', compact('pid'));
}

/**
 * Say whether the current user is a member (signed in or supplied a member email).
 * @param string $email: email supplied by user, if any
 * @return the account ID or FALSE
 */
function isMember($email = '') {
  global $mya;

  if (!$a = $mya) {
    if (!$email) return FALSE;
    if (!$uid = db\get('uid', 'users', 'NOT :IS_CO AND email=:email', ['email' => u\cry('P', $email)])) return FALSE;
    $a = r\acct($uid);
  }
  return $a->agentA->stepIsDone('contact') ? $a->id : FALSE;
}

/**
 * Say whether the current user has administrative permission for the given zip.
 */
function admin($zip) {
  global $mya;
  static $adminZips;
  
  if (!$mya or !$mya->oAdmin) return FALSE;
  if ($mya->admin) return TRUE;
  if (isset($adminZips) and in_array($zip, $adminZips)) return TRUE;
  if (r\communityId($zip) == $mya->agentA->community) { $adminZips[] = $zip; return TRUE; }
  return FALSE;
}

/**
 * Return a result set for the given parameters
 * @param string $type: post, offer, or tip (default ALL)
 * @param numeric $lat: latitude of user
 * @param numeric $lon: longitude of user
 * @param string $zip: zip of user
 * @param numeric $radius: up to how far away user is interested
 * @param date $since: first post date to include (0 to include all dates)
 * @param bool $all: show all such posts, even mine
 * @return the query result set
 */
function getPosts($type, $lat, $lon, $zip, $radius, $myPid, $admin, $zipIrrelevant, $since = 0, $all = TRUE) {
  if (($radius ?: DFT_RADIUS) >= DFT_RADIUS  // looking broadly
      and $zips = r\communityId($zip, TRUE, 'zips')) { // from within an established CG community
    $cttyCrit = "p.zip RLIKE '$zips'";
    if (!$radius) $radius = DFT_RADIUS; // default to JUST ctty and DFT_RADIUS instead of all posts
  } else $cttyCrit = 0;
    
  $visible = $zipIrrelevant ? 1 
  : ($admin ? '(SELECT 1 FROM u_company WHERE :zip RLIKE zips AND uid<1 LIMIT 1)' : 0);
  $visible = "($visible OR (NOT s.private AND s.confirmed))";
  if (!$all) $visible .= ' AND pid<>:myPid';
  list ($distance, $noGeo) = [DISTANCE_BETWEEN, NO_DISTANCE];
  $postRadius = 'IFNULL(s.radius, 0)';
  $isMyPid = 'pid=:myPid';
  if (!$type) $type = ray(POST_TYPES);
///    $admin as admin, :radius as radius, $noGeo as noGeo, MID(p.zip,1,3) = MID(:zip,1,3) AS zipMatch, $zipIrrelevant as noz,

  $sql = <<<X
    SELECT IF ($noGeo, '', $distance) AS dist, s.postid, s.service, c.id AS catId, c.cat, s.item, s.details, s.exchange,
      s.emergency, $isMyPid AS mine, s.type as type,
      LEAST(IF($postRadius=0, 99999, s.radius), :radius) AS least, $postRadius AS postRadius, p.displayName, p.city,
      :zip AS zip
    FROM posts s
    LEFT JOIN post_cats c ON c.id=s.cat
    LEFT JOIN people p USING (pid)
    WHERE s.type IN (:type)
      AND s.created <= :NOW + :HOUR_SECS AND s.created >= $since
      AND IFNULL(s.end, :NOW) >= :NOW
      AND (
        $admin OR :radius='' OR
        IF($noGeo, 
          MID(p.zip,1,3) = MID(:zip,1,3) OR $zipIrrelevant, 
          $distance < LEAST(IF($postRadius=0, 99999, s.radius), :radius)
        ) OR $cttyCrit
      )
      AND $visible
    ORDER BY s.cat,s.created DESC
X;

///  showq($sql, compact(ray('type lat lon zip radius myPid')));
  return db\q($sql, compact(ray('type lat lon zip radius myPid')));
}

/**
 * Return the given post selection query as HTML rows of summary information.
 * @param q $q: query result set
 * @return the HTML rows (including header)
 */
function postRows($q) {
  foreach (ray(t('category item details')) as $k) $cells[] = "<div class=\"cell k-$k\">" . ucfirst($k) . '</div>';
  $rows = ['<div class="thead"><div class="row">' . join('', $cells) . '</div></div>'];
  $rows[] = '<div class="tbody">';
  
  while ($row = $q->fetchAssoc()) {
    extract(just('postid dist catId type cat service emergency exchange item mine details zip', $row));

    $type = typeNum($type);
    $terms = ray(POST_TERM_ICONS)[$exchange];
    if ($emergency) $item = '<span class="emergency">!!</span> ' . $item;
    $item = lnk('/community/posts/op=show&postid=' . $postid, $item);
    $rowClass = "row t$type s$service x$exchange c$catId" . ($mine ? ' mine' : '');
    if ($dist = fmtDist($dist, $zip)) { $cat .= " <small>$dist mi</small>"; } // must be after $rowClass=
    
    $rows[] = <<< X
    <div class="$rowClass">
      <div class="cell cat">$terms$cat</div>
      <div class="cell item">$item</div>
      <div class="cell details">$details</div>
    </div>
X;
  }
  
  $rows[] = '</div>';
  return $rows;
}

/**
 * Return url query arguments urlify'd
 * @param string $op: op parameter (doesn't get urlify'd)
 * @param assoc $params: other parameters
 */
function codedUrl($op, $params) {return POSTPAGE . "/op=$op&code=" . u\urlify(http_build_query($params));}
function fmtDist($dist, $zip) {return $dist ? (admin($zip) ? number_format($dist, 2) : ($dist < .5 ? '&frac12;' : u\vague($dist))) : '';} // don't show distances less than 1/2 mile
function catChoices($top, $extra = []) {return ['' => $top] + db\keyed('id,cat', 'post_cats', '1 ORDER BY sort') + $extra;}
function eFld() {global $mya; return emailFld(REQ . t('Your Email:'), [t('Your email address')], dft($mya ? $mya->email : (nni($_COOKIE, 'email') ?: vip('email'))));}
function pVal($fld, $pid) {return db\get($fld, 'people', compact('pid'));}
function zipOnlyMsg($specifics, $zip) {return t('We are unable to geolocate that address at this time, so %specifics in the "%zip3" Zip Code area.', 'specifics zip3', $specifics, substr($zip, 0, 3));}
function msgFld($req = REQ) {return areaFld($req . t('Message to Send:'), t('Max 200 characters'), maxlen(200));}
function postRet($msg) {return r\acct() ? softErr($msg) : doSay($msg);}
function moderate($zip) {return in(substr(trim($zip), 0, 3), MODERATE_ZIPS);}
function frmBools($frm) {foreach(ray(POST_FRMS) as $k) $res['frm' . ucfirst($k)] = ($frm == $k); return $res;}
function myLocus() {global $mya; return ($mya and $mya->agentA->stepIsDone('contact')) ? $mya->location(TRUE) : '';}
function typeName($type, $n) {return ray(constant("POST_TYPE_NAMES$n"))[typeNum($type)];}
function typeNum($type) {return is_numeric($type) ? $type : array_search($type, ray(POST_TYPES));}
