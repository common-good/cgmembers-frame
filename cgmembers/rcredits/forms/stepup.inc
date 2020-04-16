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
  $subtext = t('<p>Support your favorite causes by paying a little extra on your %PROJECT purchases.</p><p>Choose organizations from the list below and/or add your own choices. Type a dollar amount or percentage to add to each purchase &mdash; to be donated to that organization. And a maximum donation per transaction, if any.</p><p>If you would like to see your nonprofit or project on this list, get %STEPUP_MIN people to write it in.</p>', '_a', atag('/community/invite', away()));
  
  $mine = 'r.from=:myid';
  foreach (ray('amount portion amtMax') as $k) $set[] = "MAX(IF($mine, `$k`, NULL)) AS `$k`";
  $set = join(', ', $set);
  
  $sql = <<< X
    SELECT CONCAT_WS('-', uid, $mine) AS recK, uid, fullName, $mine AS mine, $set
    FROM tx_rules r LEFT JOIN users u ON u.uid=r.to
    WHERE r.from>0 AND r.to>0 AND payeeType=:REF_ANYCO AND IFNULL(end, :NOW+1) > :NOW
    GROUP BY mine,r.to HAVING COUNT(id)>=:STEPUP_MIN OR mine
    ORDER BY mine, fullName, uid
X;


  $hdrs = u\ray2row(ray(t('Organization, $ or %, Max')));
  $q = db\q($sql, compact('myid'));

  if ($recs = $q->fetchAllAssoc('recK')) foreach ($recs as $rec) {
    extract((array) $rec);
    if (!isset($recs[$recK])) continue; // unsetting doesn't stop it from being included

    if ($mine) {
      $fullName = rendA("org$uid", orgFld($fullName));
    } elseif ($myK = str_replace('-0', '-1', $recK) and $my = (array) nn($recs[$myK], [])) { // user chose a standard
      foreach (ray('amount portion amtMax') as $k) $$k = $my[$k]; // get the amounts for the standard choice
      unset($recs[$myK]); // show it as standard and not as custom
    } 
    
    foreach (ray('amount portion amtMax') as $k) $$k += 0;
    
    $amt = $amount ? u\fmtAmt($amount, '$') : ($portion ? u\fmtAmt($portion, '%') : '');
    $max = $amtMax ? u\fmtAmt($amtMax) : '';
    $uids[] = $uid;
    if (nn($dfts["amt$uid"])) foreach (ray('amt max') as $k) $$k = nn($dfts["$k$uid"]); // what the user intended
    $rows[] = u\ray2row([$fullName, rendA("amt$uid", amtFld($amt)), rendA("max$uid", maxFld($max))]);
  }
  
  for ($i = 0; $i < 10; $i++) { // add some blank fields for custom choices
    $uids[] = $i;
    foreach (ray('org amt max') as $k) {
      $func = ($k . 'Fld');
      $$k = rendA("$k$i", f("w.{$k}Fld", nn($dfts["$k$i"])));
    }
    $rows[] = u\ray2row([$org, $amt, $max]);
  }
  
  $uids = hidFld(join(' ', $uids));
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

  return cgform(compact(ray('title subtext uids list')) + setupFoot());
}

function formStepUp_validate($form, &$sta) {
  global $mya;
  
  extract(just('uids', $input = $sta['input']));
  
  foreach ($uids = ray($uids) as $i => $uid) {
    foreach (ray('org amt max') as $k) $$k = trim(nn($input["$k$uid"]));
    
    if ($uid >= 0 and $uid < CANONIC_ACCTS) { // lookup organization
      if (empty($amt) xor empty($org)) return er($sta, t('You must have both an organization and an amount or neither.'), "org$uid");
      if (!$fullName = $org) {unset($uids[$i]); continue;}

      if (!$id = db\get('uid', '(SELECT * FROM users ORDER BY trust DESC) u', compact('fullName'))) return er($sta, t('This is not a member organization: %fullName.', compact('fullName')), "org$uid");
      $input["amt$id"] = $amt;
      $input["max$id"] = $max;
      $uids[$i] = $uid = $id;
    } elseif (!$amt) {unset($uids[$i]); continue;}
    
    $input["pct$uid"] = $pct = (substr($amt, -1, 1) == '%');
    if (!$pct and $amt[0] != '$') return er($sta, t('Your amounts must begin with $ or end with %.'), "amt$uid");
    $amt = strtr($amt, ray('$ %', '', ''));
    
    foreach (ray('amt max') as $fld) if ($$fld) { // amt is not empty, skip max if empty
      $$fld = str_replace('$', '', $$fld); // max might have a $
      if ($err = u\badAmount($$fld, '>=0', 1, 100)) return er($sta, $err, "$fld$uid");
      $input["$fld$uid"] = $$fld;
    }
  }
  $uids = join(' ', $uids);
  
  u\preray(compact('uids') + $input, $sta['input']); // update trimmed percentage
}

function formStepUp_submit($form, &$sta) {
  global $mya; $myid = $mya->id;
  
  $changeable = 'amount portion amtMax'; // fields user can adjust from time to time
  extract(just('uids', $input = $sta['input']));

  foreach ($uids = ray($uids) as $i => $uid) { // for each organization, create a rule for the amount
    list ($amt, $pct, $amtMax) = [$input["amt$uid"], $input["pct$uid"], $input["max$uid"] ?: NULL];

    $amts = ray($changeable, $pct ? 0 : $amt, $pct ? $amt / 100 : 0, $amtMax);
    $info = ray('payer payerType payeeType from to action', $myid, REF_ACCOUNT, REF_ANYCO, $myid, $uid, ACT_SURTX);
    if ($res = db\get('id, amount, `portion`, amtMax', 'tx_rules', u\where($info) . ' AND IFNULL(end, :NOW+1) > :NOW', $info)) {
      if ($amts == just($changeable, $res)) continue; // no change; done with this organization
      db\update('tx_rules', ray('id end', $res['id'], now()), 'id'); // cancel the old rule, to start a new one
    }
    $info += $amts + ray('purpose start', STEPUP_DESC, now());
    
    db\insert('tx_rules', $info);
  }
  
  return w\goNextStep('stepup');
}

function orgFld($dft = '') {return textFld(BARE, [t('Organization name')], dft($dft));}
function amtFld($dft = '') {return textFld(BARE, [t('$ or %')], dft($dft));}
function maxFld($dft = '') {return textFld(BARE, [t('Max')], dft($dft));}
