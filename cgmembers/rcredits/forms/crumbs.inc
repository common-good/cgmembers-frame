<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Ask about crumbs and other tithing on payments received
 */
function formCrumbs($form, &$sta, $args = '') {
  global $mya;

  $title = t('Share When You Receive');
  $subtext = t('<p>Contribute a small percentage of every payment you receive. For example, 1% means donate one penny of each dollar you receive. Smaller amounts such as 0.5% are also very welcome (the average amount people choose is about 1.1%).</p><p>Your gift supports democratic community funding of whatever we decide together is important. PLUS if you accept credit cards, this gift even just in the short term allows %PROJECT to save you (and everyone else) about 2% on credit card fees in the long term. If you do not wish to contribute crumbs, type a zero.</p>');
  $crumbs = crumbsFld($mya->crumbs, $mya->co ? REQ : '');

  return cgform(compact(ray('title subtext crumbs')) + setupFoot(), 'nolabs');
}

function formCrumbs_validate($form, &$sta) {
  global $mya;
  extract(just('crumbs', $sta['input'], NULL));

  if (!$crumbs = str_replace('%', '', $crumbs)) $crumbs = 0;
  if ($err = u\badAmount($crumbs, '>=0', 1, 100)) return say($err, 'crumbs');
  u\preray(compact('crumbs'), $sta['input']); // update trimmed percentage
  return TRUE; // for when called from prefs.inc
}

function formCrumbs_submit($form, &$sta) {
  global $mya;
  extract(just('crumbs', $sta['input']));

  $crumbs /= 100; // convert percentage to decimal
  $mya->update(compact('crumbs'));

  return w\goNextStep('crumbs');
}