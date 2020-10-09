<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once R_ROOT . '/rweb/co-list.inc';

/**
 * Show a list of participating businesses in the current region, to show "Buy Credit" buttons.
 */
function formBuyCredit($form, &$sta, $args = '') {
  global $mya;
  extract(just('where cos', $args));

  $list = w\coList(nn($cos), nn($where));
  $list = "<div class=\"cmpy-list\">$list</div>";
  
  $title = t('Support Local Companies');
  $subtext = t('<p>During the COVID-19 crisis, our local businesses and nonprofits need our support. For each business listed here, think about how much you might want to spend there over a few months or over the next year or two. Click the "Buy Credit" button, to buy yourself some credit at that business.</p><p>Once the business reopens (hooray!), every time you pay with your Common Good card <span class="loud">you automatically get 50% off until the credit is used up</span>. You end up paying the regular price and the business gets the regular amount &mdash; they just get some of it now instead of later, which is exactly what they need.</p><p>Or click a "Donate" button to support a local nonprofit. (To make regular purchases, sign in and click "Pay".)</p>');

  return cgform(compact(ray('title subtext list')), 'nolabs');
}
