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
  if (!$mya = w\eLinkAcct($sta, $args, 'acct')) return;
  $myid = $mya->id;
  
  extract(just('er', $args, NULL));
  $dfts = $er ? urlq2ray(u\deurlify($er)) : []; // get defaults from error or from tests
  
  jsx('stepup');
  
  $title = t('Step Up');
  $subtext = t('<p>Support your favorite causes by paying a little extra on your %PROJECT purchases.</p><p>Choose organizations from the list below and/or add your own choices. Type a dollar amount or percentage to add to each purchase &mdash; to be donated to that organization. You can also set a maximum donation per transaction or choose to donate periodically instead.</p><p>If you would like to see your nonprofit or project on this list, get 10 people to write it in (of course it has to have a %PROJECT account first).</p>', '_a', atag('/community/invite', away()));
  
  $fromMe = '`from`=:myid';
  foreach (ray('amount portion amtMax') as $k) $set[] = "IF($fromMe, `$k`, NULL) AS `$k`";
  $set = join(', ', $set);
  list ($distance, $noGeo) = [DISTANCE_BETWEEN, NO_DISTANCE];
  list ($lat, $lon, $zip) = [$mya->latitude, $mya->longitude, $mya->zip];

  // xly means recurring with some periodicity: daily, weekly, etc. ("x"-ly)
  $sql = <<< X
    SELECT DISTINCT x.*,u.uid,u.fullName FROM (
    
      SELECT 0 AS xly, $fromMe AS mine, CONCAT_WS('-', 'r', r.to, $fromMe) AS recK,
        r.to, id, IF($fromMe, IF(r.portion>0, IF(amtMax>0, 'pctmx', 'pct'), 'tx$'), 'pct') AS `when`, $set
      FROM tx_rules r
      WHERE r.action=:ACT_SURTX AND r.from>0 AND r.to>0 AND r.payeeType=:REF_ANYCO AND IFNULL(r.end, :NOW+1) > :NOW

      UNION 
      SELECT IF($fromMe, 1, 0) AS xly, $fromMe AS mine, CONCAT_WS('-', IF($fromMe, 't', 'r'), t.to, $fromMe) AS recK,
        t.to, id, IF($fromMe, t.period, 'pct') AS `when`, $set
      FROM tx_timed t
      WHERE t.action=:ACT_PAY AND t.from>0 AND t.to>0 AND t.period<>:PER_1 AND IFNULL(t.end, :NOW+1) > :NOW
      
    ) x 
    LEFT JOIN users u ON u.uid=x.to
    LEFT JOIN u_company c USING(uid)
    WHERE (mine AND NOT xly)
      OR (u.:IS_CO AND c.coType=:CO_NONPROFIT AND (c.coFlags & (1<<:CO_PRIVATE))=0 AND (
        IF($noGeo, LEFT(u.zip,3) = LEFT(:zip,3), $distance < :NEIGHBOR_MAX * 2) OR u.uid IN (:globalOrgs)
      ))
    GROUP BY mine,x.to,xly 
    HAVING COUNT(id)>=:STEPUP_MIN OR mine
    ORDER BY mine, fullName, uid, xly
X;

  $info = ray('myid globalOrgs lat lon zip', $myid, [CGID, r\foodFundId()], $mya->latitude, $mya->longitude, $mya->zip);
  $q = db\q($sql, $info);

  $classes = ray('org amt when max');
  $hdrs = u\ray2row(ray(t('Organization, Amount, When, Max')), $classes);

  if ($recs = $q->fetchAllAssoc('recK')) foreach ($recs as $rec) {
    extract((array) $rec);
    if (!$mine and (
      isset($recs[preg_replace('/r-(.*)-0$/', 'r-$1-1', $recK)]) or 
      isset($recs[preg_replace('/r-(.*)-0$/', 't-$1-1', $recK)])
    )) continue; // ignore the blank standard because it will appear as one of mine
    
    foreach (ray('amount portion amtMax') as $k) $$k += 0;
    
    $amt = $amount ? u\fmtAmt($amount, '') : ($portion ? str_replace('%', '', u\fmtAmt($portion, '%')) : '');
    $max = $amtMax ? u\fmtAmt($amtMax, '') : '';
    $uids[] = $uid;
    $xlys[] = $xly;
    if (nni($dfts, "amt-$xly-$uid")) foreach (ray('amt max') as $k) $$k = nni($dfts, "$k-$xly-$uid"); // what the user intended
    $rows[] = u\ray2row([$fullName, rendA("amt-$xly-$uid", amtFld($amt)), rendA("when-$xly-$uid", whenFld($when)), rendA("max-$xly-$uid", maxFld($max))], $classes);
  }
  
  for ($i = 0; $i < 10; $i++) { // add some blank fields for custom choices
    $uids[] = $i;
    $xlys[] = 'x';
    foreach (ray('org amt when max') as $k) {
      $func = ($k . 'Fld');
      $$k = rendA("$k-x-$i", f("w.{$k}Fld", nni($dfts, "$k-x-$i")));
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

  return cgform(compact(ray('title subtext uids xlys list')) + setupFoot(), 'nolabs');
}

function formStepUp_validate($form, &$sta) {
  extract(just('uids xlys', $input = $sta['input']));

  foreach ($uids = ray($uids) as $i => $uid) {
    $xly = $xlys[$i];
    foreach (ray('org amt when max') as $k) $$k = trim(nni($input, "$k-$xly-$uid"));
    
    if ($uid >= 0 and $uid <= UID_CANON9) { // lookup organization (if uid is an index, not an account ID)
      if (empty($org) and !empty($amt)) return er($sta, t('You must choose an organization if there is an amount.'), "org-xly-$uid");
      if (!$fullName = $org) {unset($uids[$i]); continue;}

      if (!$id = db\get('uid', '(SELECT * FROM users ORDER BY trust DESC) u', compact('fullName'))) return er($sta, t('This is not a member organization: %fullName. Organization names are suggested automatically when you type 3 or more letters in the name. Please try again. For the list of participating organizations, <%a>click here</a>.', 'fullName _a', $fullName, atag(PROMO_URL . '/businesses')), "org-$xly-$uid");

      foreach (ray('amt when max') as $k) $input["$k-$xly-$id"] = $$k;
      $uids[$i] = $uid = $id;
            
    } elseif (!$amt) { continue; }
    
    $amt = strtr($amt, ray('$ %', '', ''));
    
    foreach (ray('amt max') as $fld) if ($$fld) { // amt is not empty, skip max if empty
      $$fld = str_replace('$', '', $$fld); // max might have a $
      if ($err = u\badAmount($$fld, '>=0', 1, strhas($when, 'pct') ? 100 : '')) return er($sta, $err, "$fld-$xly-$uid");
      $input["$fld-$xly-$uid"] = $$fld;
    }
  }
  $uids = join(' ', $uids);
  
  u\preray(compact(ray('uids xlys')) + $input, $sta['input']); // update trimmed percentage
  for ($i = 0; $i < 10; $i++) foreach (ray('amt when max') as $k) unset($sta['input']["$k-x-$i"]); // clean up unused flds
}

function formStepUp_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta);
  $myid = $mya->id;
  
  $periods = 1; // if the donation recurs at all, it's once per period (for now)
  extract(just('uids xlys', $input = $sta['input']));

  foreach ($uids = ray($uids) as $i => $uid) { // for each organization, create a rule or template for the amount
    $xly = $xlys[$i];
    foreach(ray('amt:amt,period:when,amtMax:max') as $k => $v) $$k = nni($input, "$v-$xly-$uid") ?: NULL;
    $pct = strhas($period, 'pct');
    $matchFlds = 'amount,portion,amtMax'; // fields user can adjust from time to time
    $match = ray($matchFlds, $pct ? 0 : $amt, $pct ? $amt / 100 : 0, $period == 'pctmx' ? $amtMax : NULL);
    $purpose = STEPUP_PURPOSE; // description gets elaborated in be\getTxs()
    
    if ($xly = in($period, PERIODS)) { // periodic donation
      $tnm = 'tx_timed';
      $info = ray('action from to periods', ACT_PAY, $myid, $uid, 1);
      $matchFlds .= ',period';
      $match += compact('period');
    } else { // donation per transaction
      $tnm = 'tx_rules';
      $info = ray('payer payerType payeeType from to action', $myid, REF_ACCOUNT, REF_ANYCO, $myid, $uid, ACT_SURTX);
    }
    
    $getFlds = str_replace('portion', '`portion`', $matchFlds);
    if ($res = db\get('id,' . $getFlds, $tnm, u\where($info) . ' AND IFNULL(end, :NOW+1) > :NOW', $info)) {
      if ($match == just($matchFlds, $res)) continue; // no change; done with this organization
      r\endRule($xly ? 'tm' : 'ru', $res['id']); // cancel the old rule, to start a new one
    }
    $info += $match + ray('purpose start', $purpose, now());
    if ($amt) db\insert($tnm, $info); // setting amt to zero ends the old rule without creating a new one
  }
  
  return r\acct() ? w\goNextStep('stepup') : w\doSay();
}

/**
 * Given a "when" choice, return its description.
 * @return a description or a map of all descriptions, if no "when" choice is specified.
 */
function whenly($when = '') {
  $whenMap = array_combine(ray('pct, tx$, pctmx'), ray(t('% per tx, $ per tx, % / tx up to'))) + r\recurRay();
  return $when ? $whenMap[$when] : $whenMap;
}
  
function orgFld($dft = '') {return textFld(BARE, [t('Organization name')], dft($dft));}
function amtFld($dft = '') {return numFld(BARE, [t('Amount')], dft($dft));}
function whenFld($dft = '') {return selectFld(BARE, '', dft($dft), whenly());}
function maxFld($dft = '') {return numFld(BARE, [t('Max')], dft($dft) + vmin(.01) + prefix('$'));}
