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
  global $mya;
  extract(just('er', $args, NULL));
  $dfts = $er ? urlq2ray(u\deurlify($er)) : []; // get defaults from error or from tests
  debug($dfts);

  jsx('tithe-out');
  
  $title = t('Step Up');
  $subtext = t('<p>Support your favorite causes by paying a little extra on your %PROJECT purchases.</p><p>Choose organizations from the list below and/or add your own choices. Type a dollar amount or percentage to add to each purchase &mdash; to be donated to that organization. And a maximum donation per transaction, if any.</p><p>If you would like to see your nonprofit or project on this list, get %STEPUP_MIN people to write it in.</p>', '_a', atag('/community/invite', away()));
  
  $sql = <<< X
    SELECT uid, fullName
    FROM tx_rules r LEFT JOIN users u ON u.uid=r.to
    WHERE r.from>0 AND r.to>0
    GROUP BY r.to HAVING COUNT(id)>=:STEPUP_MIN
X;


  $q = db\q($sql);
  $hdrs = ray2row(ray(t('Organization, $ or %, Max')));
  
  while ($rec = $q->fetchAssoc()) {
    extract($rec);
    $uids[] = $uid;
    $rows[] = ray2row([$fullName, rendA("amt$uid", amtFld()), rendA("max$uid", maxFld())]);
  }
  
  for ($i = 0; $i < 10; $i++) {
    $uids[] = $i;
    foreach (ray('org amt max') as $k) {
      $func = ($k . 'Fld');
      $$k = rendA("$k$i", f("w.{$k}Fld", nn($dfts["$k$i"])));
    }
    $rows[] = ray2row([$org, $amt, $max]);
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
    $amt = trim($input["amt$uid"]);
    $max = trim($input["max$uid"]);
    
    if ($uid > 0 and $uid < CANONIC_ACCTS) { // lookup organization
      $fullName = trim($input["org$uid"]);
      if (empty($amt) xor empty($fullName)) return er($sta, t('You must have both an organization and an amount or neither.'), "org$uid");
      if (!$fullName) {unset($uids[$i]); continue;}
      
      if (!$id = db\get('uid', 'users', compact('fullName'))) return er($sta, t('This is not a member organization: %fullName.', compact('fullName')), "org$uid");
      $input["amt$id"] = $amt;
      $input["max$id"] = $max;
      $uids[$i] = $uid = $id;
    }

    if (!$amt) {unset($uids[$i]); continue;}
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
  
  $changeable = 'amount portion amtLimit'; // fields user can adjust from time to time
  $paleGrpId = db\get('id', 'u_groups', "name=':PALE_TARGETS'");
  extract(just('uids', $input = $sta['input']));

  foreach ($uids = ray($uids) as $i => $uid) { // for each organization, create a rule for the amount
    list ($amt, $pct, $amtLimit) = [$input["amt$uid"], $input["pct$uid"], $input["max$uid"] ?: NULL];

    $amts = ray($changeable, $pct ? 0 : $amt, $pct ? $amt / 100 : 0, $max);
    $info = ray('payer payerType from to action', $myid, REF_ACCOUNT, $myid, $uid, ACTION_PAYMENT);
    if ($res = db\get('id, amount, portion, amtLimit', 'tx_rules', u\where($info) . ' AND IFNULL(end, :NOW+1) > :NOW')) {
      if ($amts == just($changeable, $res)) continue; // no change; done with this organization
      db\update('tx_rules', ray('id end', $res['id'], NULL), 'id'); // cancel the old rule, to start a new one
    }
    $info += $amts + ray('purpose start', t('donation (by step-up)'), time());
    
    db\insert('tx_rules', $info);
  }
  
  return w\goNextStep('stepup');
}

/**
 * Convert the given array to an HTML row.
 */
function ray2row($row) {
  $row = join('</div><div class="cell">', $row);
  return '<div class="row"><div class="cell">' . $row . "</div></div>\n";
}

function orgFld($dft = '') {return textFld(BARE, [t('Organization name')], dft($dft));}
function amtFld($dft = '') {return textFld(BARE, [t('$ or %')], dft($dft));}
function maxFld($dft = '') {return textFld(BARE, [t('Max')], dft($dft));}
