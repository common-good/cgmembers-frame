<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Create a CGPay Button.
 */
function formCGButton($form, &$sta, $args = '') {
  global $mya, $base_url;
  extract(just('nothing', $args));

  $title = t('Create a CGPay Link/Button');
  $subtext = t('<p>Complete this form to create HTML code to paste into your website or emails for customers to pay you through %PROJECT. If you need more complex functionality, use instead %twostep or ask your website person to modify dynamically the HTML created on this page.</p><p>When your customer (or donor) clicks the link or button, a form pops up for them to enter their Common Good account ID. When they submit that form, they receive an email with a link that they must click to confirm the purchase. Funds are then transferred to your company account automatically.</p>', '_twostep', lnk('https://docs.google.com/document/d/1QpOW0WU7RiUV900PMekyH7ve3-_9yjCnQn627cWovOQ', t('our&nbsp;two&#8209;step&nbsp;interface')));

  $button = boolFld(t('Type:'), '', FALSE, ray(t('Link Button')));
  $selling = explode("\n", $mya->selling)[0];
  $item = textFld(t('Item:'), [t('Item being sold'), t('Short description to appear on confirmation/invoice to the customer.')], required($selling));
  $text = textFld(t('Link Text:'), [t('Text')], required(t('Pay with %PROJECT')));
  $amount = textFld(t('Price/Amount:'), [t('Amount'), t('Leave blank to let the customer choose an amount (for donations, for example)')], digitsOnly());
  $size = textFld(t('Button Size:'), [t('Size'), t('Button height, in pixels')], required(50) + digitsOnly() + suffix("<div id=\"button\"><img src=\"https://cg4.us/images/buttons/cgpay.png\" /></div>"));
  $example = item('', t('Example:'));
  $html = areaFld(t('Result:'), [t('Result'), t('Copy this HTML into your website or copy the example into an email.')]);
  
  jsx('cgbutton', ['qid' => $mya->mainQid]);
  
  return cgform(compact(ray('title subtext button amount item text example size html')));
}
