<?php
/**
 * @file
 * Admin forms
 */
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

include_once R_ROOT . '/admin/admin.inc';
include_once R_ROOT . '/admin/admin-web.inc';

/**
 * Log back in to admin automatically (called after testing, in case session got destroyed).
 */
function formTestResults($form, &$sta, $args = '') {
  global $channel; $channel = TX_WEB;
  if (!isDEV) return;
  extract(just('top', $args, NULL));
  
  r\setAcct(UID_SUPER, $sta);
  superSignin(a(UID_SUPER));
  test(FALSE);
  jsx('tests');

  $q = db\q('SELECT * FROM test WHERE type IN (:types) ORDER BY id', ['types' => ray('status error warning')]);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $list[] = "<div class=\"$type messages\">$value</div>\n";
  }
  $list = item(join('', nn($list, [])), BARE);
//  \drupal_get_messages(); // throw out any existing messages
//  $title = $top ? t('Tests') : NULL;

  return cgForm(compact(ray('list')), 'wide');
}

/**
 * Delete an account
 * @param mixed $uid: account to delete (uid, qid, acct or 'this')
 */
function deleteAccount($uid) {
  global $mya;

  if (is_object($a = $uid)) $uid = $a->id;
  if ($uid == 'this') {
    $uid = $mya->id;
  } elseif (!is_numeric($qid = $uid) and $a = r\acct(strtoupper($qid))) $uid = $a->id;
  
  if (!$uid) return w\softErr(t('You must not delete account zero.'));
  if ($uid == $mya->id) r\setAcct($mya->agentId); // don't leave us on a deleted account (switch to agent)
  a\deleteUid($uid);
  return w\go($mya->admPanel ? '/sadmin/panel' : '');
}

/**
 * Check database sanity.
 */
function checkSanity() {
  require R_ROOT . '/cg-sanity.inc';
  \CG\Sanity\checkSanity();
  say('Sanity check complete');
  return w\go('sadmin');
}

/**
 * Run a utility function.
 */
function util($arg) {

  switch ($arg) {
  case 'reset-menu':
    w\uncacheSubmenus();
    db\q("DELETE FROM menu_router WHERE page_callback LIKE 'CG%'"); // otherwise we get leftover junk
    db\q("DELETE FROM menu_links WHERE module='rweb'");
    \menu_rebuild();
    break;
  case 'remove-test-data':
    if (isDEV) {
      f('t.clear'); 
      say(t('All test data has been deleted.'));
    } else { a\setupBasicAccounts(); }
    break;
  case 'clear':
    a\clearCache();
    break;
  case 'php':
    w\go('devel/php');
    break;
  default:
    break;
  }
  say("DONE with \"$arg\".");
  return w\go('sadmin');
}

function formHandy($form, &$sta) {
  global $base_path, $base_url, $base_root;

  $links = [
    'sadmin/check-sanity|Check database integrity|Sanity Check|ok',
    'sadmin/util/reset-menu|Rebuild menus based on cg-menu.inc.|Redo MENUS|menu-hamburger',
    'sadmin/util/remove-test-data|Remove test data|REMOVE test data|fast-backward',
    'sadmin/util/clear|Empty the cache and sessions tables|Clear Cache & Sessions|unchecked',
    'sadmin/variables|Edit and delete site variables.|Variables|copy',
  ];

  foreach ($links as $link) {
    list ($url, $detail, $label, $icon) = u\padRay(explode('|', $link), 4);
    if (!$label) $label = $detail;
    $handy[$url] = ['', $label, '', ANY, '', $detail, $icon];
  }
  return w\subMenu(t('<h2>Handy Links</h2>'), $handy, 'handy');
}

/**
 * Display variables
 */
function formVariables() {
  $header = array(
    'name' => array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc'),
    'value' => array('data' => t('Value'), 'field' => 'value'),
    'length' => array('data' => t('Length'), 'field' => 'length'),
    'edit' => array('data' => t('Operations')),
  );

  $rows = db\rows('name,LENGTH(value) AS length', 'variable', '1 ORDER BY name');

  foreach ($rows as $row) {
    // $variables[$row->name] = '';
    $options[$row->name]['name'] = check_plain($row->name);
    $value = getv($row->name);
    $complex = (is_array($value) or is_object($value));
    if ($complex) {
      $value = str_replace("\n", "<br>\n", pr($value));
    } else {$value = check_plain($value);}

    $options[$row->name]['value'] = $value;
    $options[$row->name]['length'] = $row->length;
    $options[$row->name]['edit'] = $complex ? '' : l(t('Edit'), "sadmin/variables/edit/$row->name");
  }
  $form['variables'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No variables.'),
  );

  return $form;
}

function formVariables_submit($form, &$form_state) {
  $deletes = array_filter($form_state['values']['variables']);
  array_walk($deletes, 'variable_del');
  if (count($deletes)) {
    drupal_set_message(format_plural(count($deletes), 'One variable deleted.', '@count variables deleted.'));
  }
}

function formVariableEdit($form, &$form_state, $name) {
  
  $title = t('Edit Variable');
  $subtext = $name;
  
  $value = getv($name, t('not found'));
  
  if (is_array($value) or is_object($value)) {
    $value = item(t('Sorry, complex variable types may not be edited yet.'), t('New value'));
    $name = $new = $submit = NULL;
  } else {
    $value = pr($value);
    $name = hidFld($name);
    $new = areaFld(t('New Value:'), '', dft($value . ''));
    $value = item($value . '', t('Old value:'));
    $submit = submit();
  }
  return cgform(compact(ray('title subtext name value new submit')));
}

function formVariableEdit_submit($form, &$sta) {
  extract(just('name new', $sta['input']));
  setv($name, $new);
  say(t('Saved new value for %name.', compact('name')));
}
