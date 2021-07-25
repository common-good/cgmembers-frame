<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/invest.inc';

/**
 * View or rate an investment.
 */
function formInvestRate($form, &$sta, $args = '') {
  global $mya;
  extract(just('vestid clubqid', $args));
  if (!nn($vestid) or !$res = db\get('*', 'r_investments', compact('vestid')) or !nn($clubqid) or !$clubA = clubA($clubqid)) return w\go('/page-not-found');
  
  $title = t('Investment #%vestid', compact('vestid'));
  if ($mya->id != $clubA->id) $title = t('View or Rate ') . $title;
  $subtext = t('Numeric ratings are on a scale of 0-100');

  $equity = (strpos($res['types'], 'I') === FALSE);
  $res['type'] = $equity ? t('Equity') : t('Loan');
  extract($res);
  $coA = r\acct($coid);

  $ks = ray('company city area founded gross desc');
  $fields = ray('investment type offering price return terms assets character strength web history soundness');
  $ds = ray(t('Company, City, Service area, Founded, Annual gross, Co description, Project/Purpose, Type, Offering size, Share price, Predicted return, Terms, Company assets, Owner character, Financial strength, Web presence, Repayment history, Overall soundness'));
  $ds = array_combine(array_merge($ks, $fields), $ds);

  $vs = [$coA->fullName, $coA->city, $coA->zips, $coA->founded ? u\fmtDate($coA->founded, TRUE) : '?', $coA->gross, $coA->coDesc()];
  $vs = array_combine($ks, $vs); 
  foreach ($fields as $k) $vs[$k] = $res[$k]; // add investment record fields to $vs in the order of $fields (don't use just())

  if (!$equity) {
    $ds['offering'] = t('Target');
    unset($ds['price']);
    unset($vs['price']);
    $ds['return'] = t('Interest Rate');
  }
  
  foreach (ray('gross assets offering') as $k) $vs[$k] = u\fmtAmt($vs[$k]);
  $vs['return'] = u\fmtAmt($vs['return'], '%2');
  foreach ($vs as $k => $v) $ds[$k] = item($vs[$k], $ds[$k] . ':');
  $goods = $patronages = NULL;

  if ($mya->iclub or $shares = db\get('shares', 'r_shares', compact('vestid')) ?: 0) { // is the club or already invested
    u\setDft($shares, 0);
    $value = item(u\fmtAmt($shares * $price), t('Value:'));
    $shares = item(u\fmtAmt($shares), t('Club\'s stake:'));
    if ($mya->iclub) {
      foreach (ray('good patronage') as $k) ${$k . 's'} = number_format(db\avg($k, 'r_ratings', compact('vestid')));
      $goodsMed = (int) db\med('good', 'r_ratings', compact('vestid'));
      $goods = item(t('%goods (med %goodsMed)', compact(ray('goods goodsMed'))), t('Common Goodness:'));
      $patronages = item($patronages, t('Patronage:'));
      $comment = '<ul><li>' . db\lst('comment', 'r_ratings', compact('vestid'), NULL, "</li>\n<li>") . '</li></ul>';
      $comment = item($comment, t('Comments:'));
    }
  } elseif ($res = db\get('good,patronage,comment', 'r_ratings', ray('vestid uid', $vestid, $mya->id))) { // rated
    extract($res);
    $good = item($good, t('Your rating:'));
    $patronage = item($patronage, t('Your patronage:'));
    $comment = item($comment, t('Your comment'));
  } else { // not rated yet
    $good = intFld(REQ . t('Common Goodness:'), [t('Common Goodness'), t('Rate this investment\'s benefit to the community and the common good (0-100)')]);
    $patronage = numFld(REQ . t('Your patronage:'), [t('Monthly spending'), t('How much you will probably spend monthly, paying this company for its new or expanded services (estimate)')]);
    $comment = areaFld(t('Comments:'), [t('Comments')]);
    $submit = t('Rate it');
  }

  $buttons = backButton();
  if ($mya->iclub) $buttons .= btn("/invest/buy-or-sell/vestid=$vestid", t('Buy or Sell This'), 'warning');
  $vestid = hidFld($vestid);
  $clubqid = hidFld($clubqid);
  
  $info = compact(ray('title subtext buttons vestid goods patronages')) + $ds;
  foreach (ray('stake value good patronage comment submit clubqid') as $k) if (isset($$k)) $info[$k] = $$k;
  return cgform($info);
}

function formInvestRate_validate($form, &$sta) {
  extract(just($fields = 'good patronage', $sta['input']));
  if ($err = u\badAmount($good, '>=0', 0, 100)) return sayFieldErr($err, 'good');
  if ($err = u\badAmount($patronage, '>=0', 2, '')) return sayFieldErr($err, 'patronage');
  u\preRay(compact(ray($fields)), $sta['input']);
}

function formInvestRate_submit($form, &$sta) {
  global $mya;
  
  $vs = just('vestid good patronage comment', $sta['input']);
  db\insert('r_ratings', $vs + ray('uid', $mya->id), 'ratingid');
  $clubqid = $sta['input']['clubqid'];
  return w\go("/invest/list/clubqid=$clubqid", t('rating successful', 'num', $vs['vestid']));
}
