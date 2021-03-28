<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Return the title and body of a help popup for the given topic
 * Called when user clicks a question mark icon (a qBtn)
 * @param string $topic: topic of the help wanted
 * @return assoc [title, body]
 */
function helpScrap($topic) {
  switch ($topic) {
  case 'qr': return hret(t('QR Codes'), t('<p>Your QR code is a visual representation of your account ID and security code, that appears on your %PROJECT card. Member businesses and other members can scan your QR code from your card or directly from your mobile device.</p><p>Similarly, you can use your device to scan another member\'s %PROJECT card (or a QR code displayed on their device) by clicking the "Scan a QR code" link.</p><p>You then have the option to pay them or charge them.</p><p>Show your QR code only to people and businesses you trust.</p>'));
  default: u\FAIL('bad help scrap');
  }
}

function hret($title, $body) {return compact('title', 'body');}

