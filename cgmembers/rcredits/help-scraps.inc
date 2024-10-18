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
  case 'qr': return hret(t('QR Codes'), t('<p>Your QR code represents your account ID and security code. Member businesses and other members can scan here or from your CGPay mobile app.</p><p>Similarly, you can use your device to scan another member\'s %PROJECT QR code by clicking the "Scan QR" link here or on your CGPay mobile app.</p><p>You then have the option to pay them or charge them.</p><p>Show your QR code only to people and businesses you trust.</p>'));

  case 'signup-why': return hret(t('Why Sign Up?'), t('<ul><li><b>It\'s FREE!</b> Credit card payments cost organizations and businesses in your county many thousands of dollars every year. Member payments through %PROJECT are free.</li><li><b>It takes about 5 minutes to sign up.</b> Then you save time every time you donate or pay a %PROJECT member.</li><li><b>Plus your community gets to invest</b> the ton of money floating around in the system and grant the profits. You and your community decide together what to invest in and what to fund &mdash; for the common good!</li></ul>'));
  
  default: u\FAIL('bad help scrap');
  }
}

function hret($title, $body) {return compact('title', 'body');}

