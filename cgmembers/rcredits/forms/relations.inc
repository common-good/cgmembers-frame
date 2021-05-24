<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Give someone access to the current account or mark them as an employee or owner.
 */
function formRelations($form, &$sta, $args = '') {
  global $base_url;
  global $mya;
  $myid = $mya->id;
  extract(just('showCustomers', $args, FALSE)); // linked from an email

  //  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $acctIdentifiers ="$mya->fullName ($mya->mainQid)";
  $header = ray('The other account,Draw,Employee,Family,Permission');
  if ($co = $mya->co) $header[3] = 'Owns 10%+';
  $classes = ray('person draw employee owner permission');
  if ($showCustomers) {
    $header[] = t('Customer');
    $classes[] = 'customer';
  }
  $header[] = '';
  $classes[] = 'delete';
  $classes = u\prefixValues('rel-', $classes);

  if ($mya->hasCustomers) {
    $buttons = $showCustomers
    ? item(w\btn("/settings/relations", t('Hide Customers')))
    : item(w\btn('/settings/relations/showCustomers=1', t('Show Customers Too')));
  } else $buttons = NULL;
  
  $where = $showCustomers ? '1' : '(NOT s.:IS_CUSTOMER OR s.permission OR s.:IS_EMPLOYEE OR s.:IS_OWNER OR s.:IS_DRAW)';
  $sql = <<<EOF
    SELECT DISTINCT s.reid, s.main, s.other, IF(:myid=s.main, v.fullName, u.fullName) AS fullName,
      s.permission, s.:IS_EMPLOYEE AS employee, s.:IS_OWNER AS owner, s.:IS_CUSTOMER AS customer, s.:IS_DRAW AS draw
    FROM ((r_relations s LEFT JOIN users u ON u.uid=s.main)
      LEFT JOIN users v ON v.uid=s.other)
    WHERE s.main=:myid AND $where
    ORDER BY fullName, IF(:myid=s.main, s.other, s.main), (:myid=s.other)
EOF;
  $result = db\q($sql, compact('myid'));
  $rows = [];
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $a = r\acct($other);
    $otherCo = $a->co;
    $otherPerson = !$otherCo;

    $idLink = R_NONE; // default to no link
    if ($mya->co and !$otherCo and $permission >= r\perm(B_SCAN) and $permission != r\perm(B_JOINT)) {
      $text = str_replace(' ', '&nbsp;', $permission >= r\perm(B_BUY) ? t('request %PROJECT card') : t('request Cashier Card'));
      $idLink = "<a href=\"$base_url/request-employee-rcard/$other\">$text</a>";
    }

    $ii = $reid;
    $drawn = toggle('draw', $draw, $ii);
//    $employer = toggle('employeeOk', @$employeeOk2, $ii, @$employeeOk2 xor @$employee2);
    $employee = !$otherCo ? toggle('employee', $employee, $ii) : R_NONE;
    $owner = toggle('owner', $owner, $ii);
    $customer = toggle('customer', $customer, $ii);
    $permissions = $GLOBALS['share permissions'];
// NO    if (!$mya->can(B_JOINED)) $permissions[$permission = B_JOINT] .= '?'; // pending agreement
    if ($co or $otherCo or !$mya->ok or !$a->ok) unset($permissions[r\perm(B_JOINT)]); // no joint accounts with a company or nonmember
    $perm = $otherPerson ? dropdown('permission', $permission, $permissions, $ii) : R_NONE;
    $delete = btn('', '&cross;', 'default', '', ray('name', "delete-$ii"));
    
    $rows[] = $showCustomers
    ? [$fullName, $drawn, $employee, $owner, $perm, $customer, $delete] // what to show on screen
    : [$fullName, $drawn, $employee, $owner, $perm, $delete];
  }
  
  $aExact = "a href=\"$base_url/help/payment-exchanges\"";
  $_aInvite = atag('/community/invite');
  $aAgent = "a href=\"$base_url/agent\"";
  $help = t('Connect individual %PROJECT participants to your account &mdash; either as owners, employees, customer, or others, with limited or full access to the account. If they don\'t have an account yet, <%aInvite>invite them</a>! Each permission includes all the permissions above it.', compact(ray('aExact _aInvite aAgent')));
  //If they wish NOT to open a real account, create an <@aAgent>rPOS Sign-in Account</a> for them.

  $subtext = '<span id="account-label">' . t('Current Account') . ":</span> <span id='account-identifiers'>$acctIdentifiers</span><br><br>$help";

  $attributes = array('id' => 'relations');
  $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = t('no relations');
  $list = theme_table(compact(ray('header rows attributes caption colgroups sticky empty')));

  $title = t('Relations');
  $list = fld('item', '', '', array('markup' => $list));
  $newPerson = textFld(BARE, [t('Name, id, email, or phone')]);
  $submit = submit(BARE . t('Add'));
  $submitGroup = fieldset('submitGroup', compact(ray('newPerson submit')), t('Add Who:'));

  $showCustomers = hidFld($showCustomers);

  $form = compact(ray('title subtext buttons list submitGroup showCustomers'));

// no such step  if (!$mya->member and !$mya->stepIsDone('relations')) $form += w\setupFoot();
  w\whoFldSubmit('newPerson', t('Create a new relation with %name?'), 'self-relation', '');
  w\jsx('relations');

  return cgform($form, 'wider');
}

function formRelations_validate($form, &$sta) {
  global $mya;

  $uid = whoId($sta);
  if (r\relation(1, $mya->id, $uid)) return say('already related', 'newPerson');

/*  if ($newPerson = nn($sta['input']['newPerson'])) {
    $a = whois($newPerson, 'newPerson', $sta['input'], 'self-relation', TRUE);
    $id = $a ? $a->id : '';
    if ($id and r\relation(1, $mya->id, $id)) {
      say('already related', 'newPerson');
    } else $sta['input']['newPerson'] = $id; // save for submission handling
  }
  */
}

function formRelations_submit($form, &$sta) {
  global $mya;

  extract(just('showCustomers', $sta['input'], ''));
  $uid = whoId($sta);

  if ($other = $uid) { // adding a new relation
    $permission = r\perm(B_READ);
    $mya->newRelation(compact('other', 'permission'));
    $who = r\acct($other)->fullName;
    say('report new relation', compact('who'));
  }
  
  $show = $showCustomers ? 'showCustomers=1' : '';
  return w\go("/settings/relations/$show");
}
