<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Get information about member's calling and work life.
 */
function formGroups($form, &$sta, $args = '') {
  global $mya;
  extract(just('grpId end', $args, NULL));

  if ($end) {
    db\q('UPDATE u_groupies SET end=:NOW WHERE grpId=:grpId AND uid=:end', compact(ray('grpId end')));
    go("/sadmin/groups/grpId=$grpId"); // don't automatically delete the next addition, for example
  }
  
  $title = t('Groups');
  
  $hdrs = u\ray2row(ray('Id Group'));
  $q = db\q('SELECT * FROM u_groups ORDER BY name');

  while ($row = $q->fetchAssoc()) {
    extract($row);
    $list[] = u\ray2row([$id, lnk("/sadmin/groups/grpId=$id", $name)]);
  }
  $list = join("\n", nn($list) ?: []);
  $list = <<< X
<div class="container">
<div class="thead">$hdrs</div>
<div class="tbody">$list</div>
</div>
X;

  $new = textFld(t('New Group:'));
  
  if ($grpId) {
    $sql = 'SELECT * FROM u_groupies gu JOIN users u USING(uid) WHERE grpId=:grpId AND end IS NULL ORDER BY fullName';
    $q = db\q($sql, compact('grpId'));
    while ($row = $q->fetchAssoc()) {
      extract($row);
      $members[] = u\ray2row([$fullName, btn("/sadmin/groups/grpId=$grpId&end=$uid", t('end'), 'xs', 'default')]);
    }
    if (nn($members)) {
      $hdrs = u\ray2row([t('Member'), '']);
      $members = join("\n", $members);
      $members = <<< X
<div class="container">
<div class="thead">$hdrs</div>
<div class="tbody">$members</div>
</div>
X;
    } else $members = t('none');
    $members = item($members, t('Members:'));
    $newMember = textFld(t('New Member:'));
    $new = NULL;
    jsx('groups');
    
  } else $members = $newMember = NULL;
  
  $grpId = hidFld($grpId);

  $submit = t('Save');
  
  return cgform(compact(ray('title list members newMember new submit grpId')));
}

function formGroups_validate($form, &$sta) {
}

function formGroups_submit($form, &$sta) {
  extract(just('grpId new newMember', $sta['input'], NULL));
  
  if ($new) return db\insert('u_groups', ['name' => $new], 'id');
  if ($newMember) {
    if (!$uid = db\get('uid', 'users', ['fullName' => $newMember])) return say(t('Member does not exist: ') . $newMember, 'newMember');
    u\EXPECT($grpId, 'no group id');
    return db\insert('u_groupies', ray('uid grpId isMember', $uid, $grpId, 1), 'id');
  }
}
