<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Do whatever.
 */
function formSign($form, &$sta, $args = '') {
  global $mya;
  extract(just('code', $args, NULL));

  $title = t('Sign a Document');
  $file = $canvas = $dummy = $submit = NULL;
  $im = new \Imagick();

  if (!$args) {
    //    $subtext = t('Upload a PDF document to email to someone to be signed electronically. For example, to sign it yourself, choose the file and enter your own account ID or email.');
    $subtext = t('Upload a PDF document to sign electronically.');
    $file = fileFld(t('PDF Document:'));
    $dummy = submit(' '); // required even if not used
    jsx('sign');
  } elseif ($code) {
    $subtext = t('Push the document around to find where you want to sign. Then drag your electronic signature to that spot. Repeat for the date.');
    $canvas = item('<canvas id="canvas" width="400" height="400" placeholder="loading..."></canvas>');
    $submit = t('Download Signed Document');
    js('x/fabric.min');
    jsx('sign2', compact('code'));
  }
  
  return cgform(compact(ray('title subtext file canvas dummy submit')));
}

function formSign_validate($form, &$sta) {
  global $mya;
}

function formSign_submit($form, &$sta) {
  global $mya;

  $fileData = uploadFile(NULL, 'application/pdf');
  if (!is_array($fileData)) return err($fileData, 'file');

  return go('community/sign/code=' . u\cryRay($fileData));
}
