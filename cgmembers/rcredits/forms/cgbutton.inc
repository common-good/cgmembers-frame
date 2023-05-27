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

  $title = t('Create a CGPay Link/Button');
  $subtext = t(<<< X
    <p>Create a link or button to paste into your website or emails for customers to pay you through %PROJECT. If you need more complex functionality, use instead %advanced with your company code: %code</p>
    <p>When your customer (or donor) clicks the link or button, a payment form appears (try it!). Members enter their account ID (or email) and password to confirm the purchase. Nonmembers pay by credit card or ACH in the typical way. You are notified by email when payment is received.</p>
X
  , '_advanced code', 
  lnk('https://docs.google.com/document/d/1QpOW0WU7RiUV900PMekyH7ve3-_9yjCnQn627cWovOQ', t('our&nbsp;advanced&nbsp;interface')), 
  u\ry('P', $mya->emailCode) );
  
  $forVals = t('Donation,Purchase,Store Credit,Gift of Store Credit');
  $selling = explode("\n", $mya->selling)[0];

  $for = radiosFld(t('Purpose:'), '', dft(0), ray($forVals)); // default to gift
  $item = textFld(t('Item:'), [t('Item being sold'), t('Short description of item purchased (leave blank to let customer specify).')], dft($selling));
  $text = textFld(REQ . t('Link Text:'), [t('Text')], dft(t('Pay with %PROJECT')));
  $amount = numFld(t('Price/Amt:'), [t('Amount'), t('Leave blank to let the customer choose an amount (for donations, for example)')]);
  $credit = numFld(t('Store Credit:'), [t('Amount of credit')]);
  $ccOk = boxFld('ccOk', NBSP, t('Also accept credit card payments from non-members'), TRUE);
  $expires = dateFld(t('Expires:'), [t('Expiration date')]);
  $button = radiosFld(REQ . t('Create:'), '', dft(2), ray(t('Link,Text Button,Image Button')));
  $size = intFld(REQ . t('Size/Button:'), [t('Size'), t('Button height, in pixels')], dft(50) + vmin(1) + suffix(tr('<div id="button">%img</div>', 'img', t('(Button will appear here)'))));
  $example = item(t('(Link will appear here)'), t('Link:'));
  $html = areaFld(t('HTML:'), [t('(HTML will appear here)'), t('Copy this HTML into your website or copy the link or button into an email.')]);
  
  jsx('cgbutton', ray('qid forVals selling style showCcOk', $mya->mainQid, $forVals, $selling, TEXT_BUTTON_STYLE, !isPRODUCTION or in($mya->mainQid, 'NYAAVV NYAAVU')));
  
  return cgform(compact(ray('title subtext for item amount credit ccOk text expires button example size html')));
}
