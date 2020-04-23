<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Ask about paying a little extra
 */
function formStepUp($form, &$sta, $args = '') {
  global $mya; $myid = $mya->id;
  
  extract(just('er', $args, NULL));
  $dfts = $er ? urlq2ray(u\deurlify($er)) : []; // get defaults from error or from tests
  rendA("amt7", amtFld(3838));
  
  jsx('stepup');
  
  $title = t('Step Up');
  $subtext = t('<p>Support your favorite causes by paying a little extra on your %PROJECT purchases.</p><p>Choose organizations from the list below and/or add your own choices. Type a dollar amount or percentage to add to each purchase &mdash; to be donated to that organization. You can also set a maximum donation per transaction or choose to donate periodically instead.</p><p>If you would like to see your nonprofit or project on this list, get %STEPUP_MIN people to write it in.</p>', '_a', atag('/community/invite', away()));
  
  $mine = 'from=:myid';
  foreach (ray('amount portion amtMax') as $k) $set[] = "IF(r.$mine, r.$k, NULL) AS `$k`";
  $set = join(', ', $set);
  
  $sql = <<< X
    SELECT * FROM (
      SELECT 0 AS xly, CONCAT_WS('-', 'r', uid, r.$mine) AS recK, r.to, id, action,
        IF(r.portion>0, IF(amtMax>0, 'pctmx', 'pct'), 'tx$') AS `when`, uid, fullName, r.$mine AS mine, $set
      FROM tx_rules r LEFT JOIN users u ON u.uid=r.to
      WHERE r.action=:ACT_SURTX AND r.from>0 AND r.to>0 AND r.payeeType=:REF_ANYCO AND IFNULL(r.end, :NOW+1) > :NOW
      UNION 
      SELECT 1 AS xly, CONCAT_WS('-', 't', uid, t.$mine) AS recK, t.to, id, action,
        t.period AS `when`, uid, fullName, t.$mine AS mine, amount, t.portion, amtMax
      FROM tx_templates t LEFT JOIN users u2 ON u2.uid=t.to
      WHERE t.action=:ACT_PAY AND t.from>0 AND t.to>0 AND t.period<>'once' AND IFNULL(t.end, :NOW+1) > :NOW
    ) x    
    GROUP BY mine,x.to,action HAVING COUNT(id)>=LEAST(1, :STEPUP_MIN) OR mine
    ORDER BY mine, fullName, uid, xly
X;

  $classes = ray('org amt when max');
  $hdrs = u\ray2row(ray(t('Organization, Amount, When, Max')), $classes);
  $q = db\q($sql, compact('myid'));

  if ($recs = $q->fetchAllAssoc('recK')) foreach ($recs as $rec) {
    extract((array) $rec);
    if (!isset($recs[$recK])) continue; // unsetting doesn't stop it from being included

    if (!$mine and $myK = preg_replace('/-0$/', '-1', $recK) and $my = (array) nn($recs[$myK], [])) { // chose a standard
      foreach (ray('amount portion amtMax') as $k) $$k = $my[$k]; // get the amounts for the standard choice
      unset($recs[$myK]); // show it as standard and not as custom
    } 
    
    foreach (ray('amount portion amtMax') as $k) $$k += 0;
    
    $amt = $amount ? u\fmtAmt($amount, '') : ($portion ? str_replace('%', '', u\fmtAmt($portion, '%')) : '');
    $max = $amtMax ? u\fmtAmt($amtMax) : '';
    $uids[] = $uid;
    $xlys[] = $xly;
    if (nn($dfts["amt-$xly-$uid"])) foreach (ray('amt max') as $k) $$k = nn($dfts["$k-$xly-$uid"]); // what the user intended
    $rows[] = u\ray2row([$fullName, rendA("amt-$xly-$uid", amtFld($amt)), rendA("when-$xly-$uid", whenFld($when)), rendA("max-$xly-$uid", maxFld($max))], $classes);
  }
  
  for ($i = 0; $i < 10; $i++) { // add some blank fields for custom choices
    $uids[] = $i;
    $xlys[] = 'x';
    foreach (ray('org amt when max') as $k) {
      $func = ($k . 'Fld');
      $$k = rendA("$k-x-$i", f("w.{$k}Fld", nn($dfts["$k-x-$i"])));
    }
    $rows[] = u\ray2row([$org, $amt, $when, $max], $classes);
  }
  
  $uids = hidFld(join(' ', $uids));
  $xlys = hidFld(join('', $xlys)); // character array
  $list = join('', $rows);
  
  $list = <<< X
<div class="container">
  <div class="thead">
    $hdrs
  </div>
  <div class="tbody">
    $list
  </div>
</div>
X;

  return cgform(compact(ray('title subtext uids xlys list')) + setupFoot());
}

function formStepUp_validate($form, &$sta) {
  global $mya;
  extract(just('uids xlys', $input = $sta['input']));

  foreach ($uids = ray($uids) as $i => $uid) {
    $xly = $xlys[$i];
    foreach (ray('org amt when max') as $k) $$k = trim(nn($input["$k-$xly-$uid"]));
    
    if ($uid >= 0 and $uid < CANONIC_ACCTS) { // lookup organization
      if (empty($org) and !empty($amt)) return er($sta, t('You must choose an organization if there is an amount.'), "org-xly-$uid");
      if (!$fullName = $org) {unset($uids[$i]); continue;}

      if (!$id = db\get('uid', '(SELECT * FROM users ORDER BY trust DESC) u', compact('fullName'))) return er($sta, t('This is not a member organization: %fullName. Organization names are suggested automatically when you type 3 or more letters in the name. Please try again. For the list of participating organizations, <%a>click here</a>.', 'fullName _a', $fullName, atag(PROMO_URL . '/businesses')), "org-$xly-$uid");
      
      $xly = in($when, PERIODS) ? 1 : 0;
      foreach (ray('amt when max') as $k) $input["$k-$xly-$id"] = $$k;
      $uids[$i] = $uid = $id;
      si($xlys, $i, $xly);
      
    } elseif (!$amt) { continue; }
    
    $amt = strtr($amt, ray('$ %', '', ''));
    
    foreach (ray('amt max') as $fld) if ($$fld) { // amt is not empty, skip max if empty
      $$fld = str_replace('$', '', $$fld); // max might have a $
      if ($err = u\badAmount($$fld, '>=0', 1, 100)) return er($sta, $err, "$fld-$xly-$uid");
      $input["$fld-$xly-$uid"] = $$fld;
    }
  }
  $uids = join(' ', $uids);
  
  u\preray(compact(ray('uids xlys')) + $input, $sta['input']); // update trimmed percentage
  for ($i = 0; $i < 10; $i++) foreach (ray('amt when max') as $k) unset($sta['input']["$k-x-$i"]); // clean up unused flds
}

function formStepUp_submit($form, &$sta) {
  global $mya; $myid = $mya->id;
  
  $changeable = 'amount portion amtMax'; // fields user can adjust from time to time
  $periods = 1; // if the donation recurs at all, it's once per period
  extract(just('uids xlys', $input = $sta['input']));

  foreach ($uids = ray($uids) as $i => $uid) { // for each organization, create a rule for the amount
    $xly = $xlys[$i];
    foreach(ray('amt:amt,period:when,amtMax:max') as $k => $v) $$k = $input["$v-$xly-$uid"] ?: NULL;
    $pct = strhas($period, 'pct');
    $match = ray($changeable, $pct ? 0 : $amt, $pct ? $amt / 100 : 0, $period == 'pctmx' ? $amtMax : NULL);
    
    if ($xly) { // periodic donation
      $tnm = 'tx_templates';
      $info = ray('action from to periods', ACT_PAY, $myid, $uid, 1);
      $matchFlds = $changeable . ' period';
      $match += compact('period');
      $purpose = t('donation (%xly)', 'xly', whenly($period));
    } else { // donation per transaction
      $tnm = 'tx_rules';
      $info = ray('payer payerType payeeType from to action', $myid, REF_ACCOUNT, REF_ANYCO, $myid, $uid, ACT_SURTX);
      $matchFlds = $changeable;
      $purpose = $pct ? str_replace('3%', u\fmtAmt($amt / 100, '%s'), STEPUP_3PCT) : STEPUP_DESC;
    }
      
    if ($res = db\get('id,amount,`portion`,amtMax', $tnm, u\where($info) . ' AND IFNULL(end, :NOW+1) > :NOW', $info)) {
      if ($match == just($matchFlds, $res)) continue; // no change; done with this organization
      db\update($tnm, ray('id end', $res['id'], now()), 'id'); // cancel the old rule, to start a new one
    }
    $info += $match + ray('purpose start', $purpose, now());
    if ($amt) db\insert($tnm, $info); // setting amt to zero ends the old rule without creating a new one
  }
  
  return w\goNextStep('stepup');
}

/**
 * Given a "when" choice, return its description.
 * @return a description or a map of all descriptions, if no "when" choice is specified.
 */
function whenly($when = '') {
  $whenMap = array_combine(ray('pct, tx$, pctmx'), ray(t('% per tx, $ per tx, % / tx up to'))) + array_combine(ray(PERIODS), ray(PERIODLY));
  foreach (ray('once forever') as $k) unset($whenMap[$k]);

  return $when ? $whenMap[$when] : $whenMap;
}
  
function orgFld($dft = '') {return textFld(BARE, [t('Organization name')], dft($dft));}
function amtFld($dft = '') {return textFld(BARE, [t('Amount')], dft($dft));}
function whenFld($dft = '') {return selectFld(BARE, '', dft($dft), whenly());}
function maxFld($dft = '') {return textFld(BARE, [t('Max')], dft($dft));}
