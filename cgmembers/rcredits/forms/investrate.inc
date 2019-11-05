<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * View or rate an investment.
 */
function formInvestRate($form, &$sta, $args = '') {
  global $mya;
  extract(just('vestid clubqid', $args));
  if (!nn($vestid) or !nn($clubqid) or !$clubid = r\acct($clubqid)) return w\go('/page-not-found');
  
  $title = t('View or Rate Investment #%vestid', compact('vestid'));
  $subtext = t('Numeric ratings are on a scale of 0-100');

  $res = db\get('*', 'r_investments', compact('vestid'));
  extract($res);
  $coA = r\acct($coid);

  $ks = ray('company city area dob gross desc');
  $fields = ray('investment offering price return terms assets character strength web history soundness');
  $ds = ray(t('Company, City, Service area, Founded, Annual gross, Co description, Project/Purpose, Offering size, Share price, Predicted return, Terms, Company assets, Owner character, Financial strength, Web presence, Repayment history, Overall soundness'));
  $ds = array_combine(array_merge($ks, $fields), $ds);

  $vs = [$coA->fullName, $coA->city, $coA->serviceArea, u\fmtDate($coA->dob, TRUE), $coA->gross, $coA->coDesc()];
  $vs = array_combine($ks, $vs); 
  foreach ($fields as $k) $vs[$k] = $res[$k]; // add investment record fields to $vs in the order of $fields (don't use just())

  if (strpos($types, 'I') !== FALSE) {
    $ds['offering'] = t('Target');
    unset($ds['price']);
    unset($vs['price']);
    $ds['return'] = t('Interest Rate');
  }
  
  foreach (ray('gross assets offering') as $k) $vs[$k] = u\fmtAmt($vs[$k]);
  $vs['return'] = number_format($vs['return'], 2) . '%';
  foreach ($vs as $k => $v) $ds[$k] = item($vs[$k], $ds[$k] . ':');

  if ($mya->iclub or $shares = db\get('shares', 'r_shares', compact('vestid'))) { // is the club or already invested
    $value = item(u\fmtAmt(@$shares * $price), t('Value:'));
    $shares = item(u\fmtAmt(@$shares), t('Club\'s stake:'));
    if ($mya->iclub) {
      foreach (ray('good patronage') as $k) ${$k . 's'} = number_format(db\avg($k, 'r_ratings', compact('vestid')));
      $goods = item($goods, t('Common Goodness:'));
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
    $good = textFld(t('Common Goodness:'), [t('Common Goodness'), t('Rate this investment\'s benefit to the community and the common good (0-100)')], required());
    $patronage = textFld(t('Your patronage:'), [t('Monthly spending'), t('How much you will probably spend monthly, paying this company for its new or expanded services (estimate)')], required());
    $comment = areaFld(t('Comments:'), [t('Comments')]);
    $submit = t('Rate it');
  }

  $buttons = backButton();
  if ($mya->iclub) $buttons .= btn("/invest/buy-or-sell/vestid=$vestid", t('Buy or Sell This'), 'warning');
  $vestid = hidFld($vestid);
  $clubqid = hidFld($clubqid);
  
  return cgform(@compact(ray('title subtext buttons vestid goods patronages')) + $ds + @compact(ray('stake value good patronage comment submit clubqid')));
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
