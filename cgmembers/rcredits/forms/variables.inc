<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

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
