<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Create a CGPay Button.
 * BE CAREFUL! Inline styles are permitted on this page.
 */
function formCGButton($form, &$sta, $args = '') {
  global $mya, $base_url;
  extract(just('nothing', $args));

  $fbo = ($mya->legalName == r\acct(CGID)->legalName); // fiscal sponsoring

  $title = t('Create a CGPay Link/Button');
  $subtext = t(<<< X
    <p>Create a link or button to paste into your website or emails for customers to pay you through %PROJECT. If you need more complex functionality, use instead %advanced with company code: %code</p>
    <p>When your customer (or donor) clicks the link or button, a payment form appears (try it!). The customer must enter their account ID (or email) and password to confirm the purchase. You are notified by email when payment is received.</p>
X
  , '_advanced code', 
  lnk('https://docs.google.com/document/d/1QpOW0WU7RiUV900PMekyH7ve3-_9yjCnQn627cWovOQ', t('our&nbsp;advanced&nbsp;interface')), 
  u\ry('P', $mya->emailCode) );
  
  $forVals = t('Credit,Gift of Credit,Other');
  if ($mya->admRegion and $mya->legalName == r\acct(CGID)->legalName) $forVals .= t(',Donate to CG acct');
  $selling = explode("\n", $mya->selling)[0];

  $button = radiosFld(REQ . t('Type:'), '', dft(2), ray(t('Link,Text Button,Image Button')));
  $for = radiosFld(t('For:'), '', dft(2), ray($forVals));
  $item = textFld(t('Item:'), [t('Item being sold'), t('Short description of item purchased (leave blank to let customer specify).')], dft($selling));
  $text = textFld(REQ . t('Link Text:'), [t('Text')], dft(t('Pay with %PROJECT')));
  $amount = numFld(t('Price/Amt:'), [t('Amount'), t('Leave blank to let the customer choose an amount (for donations, for example)')]);
  $credit = numFld(t('Store Credit:'), [t('Amount of credit')]);
  $expires = dateFld(t('Expires:'), [t('Expiration date')]);
  $size = intFld(REQ . t('Size/Button:'), [t('Size'), t('Button height, in pixels')], dft(50) + vmin(1) + suffix(tr('<div id="button">%img</div>', 'img', t('(Button will appear here)'))));
  $example = item(t('(Link will appear here)'), t('Link:'));
  $html = areaFld(t('HTML:'), [t('(HTML will appear here)'), t('Copy this HTML into your website or copy the link or button into an email.')]);
  
  $page = $fbo ? 'community/donate' : 'cgpay';

  jsx('cgbutton', ray('qid forVals selling style page', $mya->mainQid, $forVals, $selling, TEXT_BUTTON_STYLE, $page));
  
  return cgform(compact(ray('title subtext button for item amount credit text expires example size html')));
}
