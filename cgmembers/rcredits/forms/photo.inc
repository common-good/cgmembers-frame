<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Upload a photo.
 * @later: maybe drag&drop (see http://hayageek.com/drag-and-drop-file-upload-jquery/)
 */
function formPhoto($form, &$sta, $arg = '') {
  global $base_url, $rUrl;
  global $mya;

  $chooserStyle = 'btn-md btn-primary';
  extract(just('ok err recrop', $arg, NULL));
  
  if ($ok) {
    say('got photo');
    return goNextStep('photo');
  }

  $recrop = $recrop ? "$base_url/account-photo/orig=1/" . now() : '';

  $body = t('I am attaching a photo of me for my %PROJECT account. &nbsp; - %fullName', 'fullName', $mya->fullName);
  $emailIt = t('<%a>Email a photo to us</a>', '_a', atag('mailto:' . r\regionField('email') . '?subject=photo&body=' . $body));
  $recropIt = $mya->admManageAccts ? w\btn('/settings/photo/recrop=1', t('Recrop')) : '';
  $progress = $nextStep = NULL; // may get set by extracting w\setupFoot()

  if ($err) say($err, 'err');
  if ($mya->hasPhoto) {
    extract(w\setupFoot()); // be sure new member has a chance to continue
  } else { $nextStep = item(w\skipButton()); }
  $desc = $mya->co
  ? t('<p>1. Choose a picture to represent your company in %PROJECT promotions. You will then resize and crop to fit it in the box (%R_PHOTO_WIDTHpx tall by %R_PHOTO_HEIGHTpx wide).</p>')
  : t('<p>1. Choose a recent color picture of yourself for seller to see when they scan your member QR Code. Your photo will be encrypted, unseen by other members unless you transact with them. <%b>Requirements</b>:
  <ul>
  <li>A color photo of you, and only you (you can crop other people out).</li>
  <li>It must show your full face clearly (no sunglasses, eyes open).</li>
  <li class="loud">It includes the top of your head and your chin, with just a little space above and below.</li></ul>', '_b', 'b class="loud"');    
  $desc .= t('<p>2. Drag to reposition, use the buttons to adjust size and rotation, then click the <%img>button to upload. Please feel free to %emailIt, if that is easier for you.', '_b _emailIt _img', 'b class="crop-button"', $emailIt, "img src=\"$rUrl/images/icons/green-checkmark.png\" height=\"20\" /");
  
  $title = $mya->co ? t('Company Photo') : t('Photo ID Picture');

  if ($mya->hasPhoto) {
    $choose = t('Choose a New Photo');
    $del = $mya->admDeletePhoto ? submit(t('Delete')) : NULL;
  } else {
    $choose = t('Choose a Photo File');
    $del = NULL;
  }

  $class = $mya->co ? ' class="co"' : '';
  w\svar('userAgent', nni($_SERVER, 'HTTP_USER_AGENT')); // for debugging spurious photo rotations

  $desc = item($desc);
  $printCard = $mya->admRegion ? f('a.cardLink', $mya->id) : '';
  $newPhoto = item(<<<EOF
<div id="choosePhotoWrap"><div class="btn $chooserStyle btn-success" id="choosePhoto">$choose</div> <div id="emailPhoto" class="btn btn-warning btn-xs">$emailIt</div> $recropIt $printCard</div>
<div class="cropHeaderWrapper">
  <div id="photoUp"$class></div>
</div>

EOF
  );

  js('croppic');
  js('crop-setup', 'recrop uid', $recrop, $mya->id);
  css('x/croppic.css');

  $form = compact(ray('title nextStep progress desc newPhoto del'));
  return cgform($form);
}

function formPhoto_submit($form, &$sta) {
  global $base_url;

  global $mya;
  
  if (op($sta) == 'del') {
    u\EXPECT($mya->admDeletePhoto and $mya->hasPhoto, 'del ghost photo');
    $mya->stepDone('photo', FALSE);
    $mya->update('photo', '');
    $mya->setBit(B_MEMBER, FALSE);
    $mya->setBit(B_OK, FALSE);
    say(t('photo removed, step undone, person set to non-member'));
    return w\go('/settings/photo');
  } else return w\goNextStep('photo');
}