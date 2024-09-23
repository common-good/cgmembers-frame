<?php
use CG as r;
use CG\Util as u;
use CG\Web as w;

/**
 * @file
 * rCredits template functions, called from the rCredits theme's template.php stub
 * (run Handy|Empty Cache to activate a new function here)
 */
function rcredits_preprocess_html(&$variables) {
  // Add variables for path to theme.
  $variables['base_path'] = base_path();
  $variables['path_to_rcredits_theme'] = drupal_get_path('theme', 'rcredits');
  
  // Add body classes if certain regions have content.
  if (!empty($variables['page']['accounts'])) { // CGF addition
    $variables['classes_array'][] = 'accounts';
  }
  if (!empty($variables['page']['featured'])) {
    $variables['classes_array'][] = 'featured';
  }

  if (!empty($variables['page']['triptych_first'])
    || !empty($variables['page']['triptych_middle'])
    || !empty($variables['page']['triptych_last'])) {
    $variables['classes_array'][] = 'triptych';
  }

  if (!empty($variables['page']['footer_firstcolumn'])
    || !empty($variables['page']['footer_secondcolumn'])
    || !empty($variables['page']['footer_thirdcolumn'])
    || !empty($variables['page']['footer_fourthcolumn'])) {
    $variables['classes_array'][] = 'footer-columns';
  }
}

/**
 * Override or insert variables into the page template for HTML output.
 */
function rcredits_process_html(&$variables) {
  // Hook into color.module.
  if (module_exists('color')) {
    _color_html_alter($variables);
  }
}

/**
 * Override or insert variables into the page template.
 */
function rcredits_process_page(&$variables) {
  // Hook into color.module.
  if (module_exists('color')) {
    _color_page_alter($variables);
  }

  // Since the title and the shortcut link are both block level elements,
  // positioning them next to each other is much simpler with a wrapper div.
  if (!empty($variables['title_suffix']['add_or_remove_shortcut']) && $variables['title']) {
    // Add a wrapper div using the title_prefix and title_suffix render elements.
    $variables['title_prefix']['shortcut_wrapper'] = array(
      '#markup' => '<div class="shortcut-wrapper clearfix">',
      '#weight' => 100,
    );
    $variables['title_suffix']['shortcut_wrapper'] = array(
      '#markup' => '</div>',
      '#weight' => -99,
    );
    // Make sure the shortcut link is the first item in title_suffix.
    $variables['title_suffix']['add_or_remove_shortcut']['#weight'] = -100;
  }
}

/**
 * Implements hook_preprocess_maintenance_page().
 */
function rcredits_preprocess_maintenance_page(&$variables) {
  // By default, site_name is set to Drupal if no db connection is available
  // or during site installation. Setting site_name to an empty string makes
  // the site and update pages look cleaner.
  // @see template_preprocess_maintenance_page
  if (!$variables['db_is_active']) {
    $variables['site_name'] = '';
  }
  drupal_add_css(drupal_get_path('theme', 'rcredits') . '/css/maintenance-page.css');
}

/**
 * Override or insert variables into the maintenance page template.
 */
function rcredits_process_maintenance_page(&$variables) {}

/**
 * Override or insert variables into the node template.
 */
function rcredits_preprocess_node(&$variables) {
  if ($variables['view_mode'] == 'full' && node_is_page($variables['node'])) {
    $variables['classes_array'][] = 'node-full';
  }
}

/**
 * Override or insert variables into the block template.
 */
function rcredits_preprocess_block(&$variables) {
  // In the header region visually hide block titles.
  if ($variables['block']->region == 'header') {
    $variables['title_attributes_array']['class'][] = 'element-invisible';
  }
}

/**
 * Implements theme_menu_tree().
 */
function rcredits_menu_tree($variables) {
  return '<ul class="menu clearfix">' . $variables['tree'] . '</ul>';
}

/**
 * Returns HTML for a form.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties of the element.
 *     Properties used: #action, #method, #attributes, #children
 *
 * @ingroup themeable
 */
function rcredits_form($variables) {
  global $base_url;
  $element = $variables['element'];
  if (isset($element['#action'])) {
    if ($element['#action'][0] == '/') $element['#action'] = $base_url . $element['#action']; // CGF
    $element['#attributes']['action'] = drupal_strip_dangerous_protocols($element['#action']);
  }

  $frmName = str_replace('cgwebform', '', $element['#id']);
  if ($frmName == 'prox') {
    $args = basename($_SERVER['REQUEST_URI']);
    extract(just('page', $args, ''));
    $frmName = strtolower($page);
  }
  $element['#id'] = "frm-$frmName"; // CGF
  element_set_attributes($element, array('method', 'id'));
  if (empty($element['#attributes']['accept-charset'])) {
    $element['#attributes']['accept-charset'] = "UTF-8";
  }
  // Anonymous DIV to satisfy XHTML compliance.
  return w\tags('form', w\tags('div', $element['#children']), $element['#attributes']);
//  return '<form' . u\tribs($element['#attributes']) . '><div>' . $element['#children'] . '</div></form>';
}

function rcredits_links($variables) {
  include_once R_ROOT . '/cg-menu.inc';
  global $rUrl, $base_url;
  $links = $variables['links'];
  $attributes = $variables['attributes'];
  $attributes['class'][] = 'nav navbar-nav'; // cgf
  $menuLinks = '';
  $num_links = count($links);
  $i = 1;

  foreach ($links as $key => $item) { // for each top-level menu item
    extract($item, EXTR_PREFIX_ALL, 'm'); // get title and href

    $id = "menu-$m_href";
    $class = 'topmenu';
    if ($i == 1) $class .= ' first';
    if ($i == $num_links) $class .= ' last';
    if ($m_href == $_GET['q'] or ($m_href == '<front>' && \drupal_is_front_page())) $class .= ' active';
    
    if (in($m_href, SUBMENUS)) {
      $class .= ' submenu';
      $menu = w\subMenuRaw($m_href);
      $link = <<<EOF
      <div class="popmenu">$menu</div>
      <a href="#" data-toggle="popover" data-html="true" data-trigger="manual" data-animation="false">$m_title</a>
EOF;
//      <a href="#" data-toggle="popover" data-html="true" data-trigger="manual" data-animation="false" onclick="jQuery('.submenu a').not(jQuery(this)).popover('hide'); jQuery(this).popover('toggle');">$m_title</a>
    } else $link = spinLink("/$m_href", $m_title, $id, '', ''); // the '' args are important

    $menuLinks .= "<li id=\"$id\" class=\"$class\">$link</li>\n";
    $i++;
  }

  if (function_exists('\\CG\\acct') and $mya = r\acct()) {
    $acctName = "$mya->fullName ($mya->mainQid)";
    $co = $mya->co ? ' co' : '';
  } else $acctName = $co = '';

  $breadcrumb = rcredits_breadcrumb(['breadcrumb' => \drupal_get_breadcrumb()]);
  $logo = R_PATH . '/images/' . PROJECT_LOGO . '?' . R_VERSION;
  $promoUrl = r\promoLink();
  
  return <<<EOF
<div class="nav2">
  <div class="nav2-inner">
    <div id="extras" class="">
      <button type="button" class="accounts-toggle photo$co" data-toggle="collapse" data-target="#edit-accounts" aria-expanded="false" aria-controls="edit-accounts">
        <a title="$acctName" data-toggle="popover" data-placement="left"><img src="$base_url/account-thumb"/></a>
      </button>
    </div>
  </div>
</div>

<nav class="navbar navbar-inverse navbar-fixed-top">
  <div class="container">
    <ol class="breadcrumb">
      $breadcrumb
    </ol>
  
    <div class="navbar-header">
      <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#navbar" aria-expanded="false" aria-controls="navbar">
        <span class="sr-only">Toggle navigation</span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
      </button>
      <a class="navbar-brand" href="$promoUrl"><div>
        <img src="$logo" />
        <div id="orgname">Common Good<sup> &reg;</sup></div>
      </div></a>
    </div>
    <div id="navbar" class="collapse navbar-collapse">
      <ul class="nav navbar-nav">
$menuLinks
      </ul>
    </div><!--/.navbar-collapse -->
  </div>
</nav>
EOF;
}

/**
 * Returns HTML for a form element.
 *
 * Each form element is wrapped in a DIV container having the following CSS
 * classes:
 * - form-item: Generic for all form elements.
 * - form-type-#type: The internal element #type.
 * - form-item-#name: The internal form element #name (usually derived from the
 *   $form structure and set via form_builder()).
 * - form-disabled: Only set if the form element is #disabled.
 *
 * In addition to the element itself, the DIV contains a label for the element
 * based on the optional #title_display property, and an optional #description.
 *
 * The optional #title_display property can have these values:
 * - before: The label is output before the element. This is the default.
 *   The label includes the #title and the required marker, if #required.
 * - after: The label is output after the element. For example, this is used
 *   for radio and checkbox #type elements as set in system_element_info().
 *   If the #title is empty but the field is #required, the label will
 *   contain only the required marker.
 * - invisible: Labels are critical for screen readers to enable them to
 *   properly navigate through forms but can be visually distracting. This
 *   property hides the label for everyone except screen readers.
 * - attribute: Set the title attribute on the element to create a tooltip
 *   but output no label element. This is supported only for checkboxes
 *   and radios in form_pre_render_conditional_form_element(). It is used
 *   where a visual label is not needed, such as a table of checkboxes where
 *   the row and column provide the context. The tooltip will include the
 *   title and required marker.
 *
 * If the #title property is not set, then the label and any required marker
 * will not be output, regardless of the #title_display or #required values.
 * This can be useful in cases such as the password_confirm element, which
 * creates children elements that have their own labels and required markers,
 * but the parent element should have neither. Use this carefully because a
 * field without an associated label can cause accessibility challenges.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties of the element.
 *     Properties used: #title, #title_display, #description, #id, #required,
 *     #children, #type, #name.
 *
 * @ingroup themeable
 */
function rcredits_form_element($variables) {
  extract(cgElement($variables, 'name tabled type bare markup help parents children required title title_display id field_prefix field_suffix inline class box'));

  $outerClass = nn($class, []);
  $parentName = isset($parents) ? strtr(nni($parents, 0), [' '=>'-', '_'=>'-', '['=>'-', ']'=>'']) : '';
  $for = nn($name) ?: $parentName;
  if ($for == 'title') $children = "<h3>$children</h3>";
  $children = nn($field_prefix) . $children . nn($field_suffix);

  u\setDft($title_display, 'before');
//  $boxy = in_array($type, ['radio', 'checkbox']) ? ($type . (@$inline ? '-inline' : '')) : '';
  $boxy = in($type, 'radio checkbox') ? $type : '';

  $class = $boxy ? 'option' : 'control-label';
  if ($title_display == 'invisible') $class .= ' sr-only';
  $label = ($boxy ? ($children . nn($title)) : nn($title)) ?: NBSP;
  $label = w\tag('label', $label, compact('for', 'class'));
  if (nn($bare)) return $boxy ? $label : $children;
    
  if (nn($tabled)) {
    if (is_array(nn($help))) $help = nni($help, 1); // ignore hint here (see rcredits_textfield)
    $help = nn($help) ? "\n      <div class=\"help-block\">$help</div>" : '';
    $core = <<<EOF
    $label
    <div class="control-data">
      $children$help
    </div>
EOF;
  } else $core = $boxy ? $label : ($label . $children);

  $id = isset($markup) ? " id=\"edit-$for\"" : '';
  $outerClass[] = "form-group form-item-$for";
  if ($for != $parentName) $outerClass[] = "form-item-$parentName";
  $outerClass[] = $boxy ?: "form-type-$type";
  $required = nn($required) ? ' required="yes"' : '';
  if ($class[0] == 'boxes') $outerClass[] = 'boxes';
  $i = raypos($outerClass, 'mlab-left');
  if ($i !== FALSE) unset($outerClass[$i]); else $outerClass[] = 'mlab-above'; // on mobile, labels default to above
  $outerClass = join(' ', $outerClass);

  return <<<EOF
  <div$id class="$outerClass"$required>
$core
  </div>
EOF;
}

function rcredits_textfield($vs) {
  cgSetAttrs($vs['element'], ray('id name class value size maxlength step min max autocomplete disabled index enctype pattern data-lpignore'));
  extract(cgElement($vs, 'attributes required help class submitButton subtype'));

  if (nn($submitButton)) return rcredits_submit($vs);
  if (nn($required)) $attributes['required'] = 'yes'; // don't add this in unless it is yes
  if (nn($subtype)) $attributes['type'] = $subtype; // num, tel, email, date, etc.
  $class[] = 'form-control input-md';
  if (is_array($ray = nn($help))) {
    if (nni($ray, 0) and !nni($ray, 2)) $ray[2] = $ray[0]; // hover text defaults to same as hint
    foreach (['placeholder', 'desc', 'title'] as $i => $k) {
      if ($k != 'desc' and $v = nni($ray, $i)) $attributes[$k] = $v;
    }
  }    
  $tribs = u\tribs(compact('class') + $attributes);
  return "<input $tribs />";
}

function rcredits_textarea($variables) {
  cgSetAttrs($variables['element'], ray('id name maxlength'));
  extract(cgElement($variables, 'attributes required name help class default_value value'));

  $value = plain(nn($value));
  if (nn($required)) $attributes['required'] = 'yes';
  $class[] = 'form-control input-md';
  if (is_array($ray = nn($help))) {
    if (nni($ray, 0) and !nni($ray, 2)) $ray[2] = $ray[0]; // hover text defaults to same as hint
    foreach (['placeholder', 'desc', 'title'] as $i => $k) {
      if ($k != 'desc' and $v = $ray[$i]) $attributes[$k] = $v;
    }
  }    
  $tribs = u\tribs(compact('class') + (nn($attributes) ?: []));
  return "<textarea $tribs />$value</textarea>";
}

function rcredits_radios($variables) {
  $chn = $variables['element']['#children'];
  extract(cgElement($variables, 'required inline'));
  if (nn($inline)) $chn = preg_replace('/ radio( |")/', " radio-inline$1", $chn);
  if (nn($required)) $chn = str_replace('type="radio"', 'type="radio" required="yes"', $chn);
  return $chn;
}

function rcredits_radio($variables) {
  \element_set_attributes($variables['element'], array('id', 'name', '#return_value' => 'value'));
  extract(cgElement($variables, 'title attributes class return_value value parents'));
  
  if (isset($return_value) and $value == $return_value) $attributes['checked'] = 'checked';
  $type = 'radio';
  $class[] = 'form-radio';
  $tribs = u\tribs(compact(ray('type class')) + $attributes);
  if (test() and nni($attributes, 'checked')) f('t.setRadio', $parents[0], $title);

  return "<input $tribs />";
}

function rcredits_submit($variables) {
  $special = 'title class tickle tabindex disabled field_suffix';
  extract(cgElement($variables, 'bare value id size style parents ' . $special));
  $id2 = 'edit-' . (isset($parents) ? nni($parents, 0) : 'xx'); // even if no id is set (assures mixed case when appropriate
  if (!nn($id) or $id == strtolower($id2)) $id = $id2;
  $extra = cgElement($variables, $special);
  if ($hasSuffix = nni($extra, 'field_suffix')) unset ($extra['field_suffix']);
  $res = spinLink('submit', $value, $id, nn($style), nn($size), $extra);
  if ($hasSuffix) $res = '<div class="presuffix">' . $res . '</div>';
  $variables['element']['#children'] = $res;
  return nn($bare) ? $res : rcredits_form_element($variables);
}

function rcredits_button($variables) {
  extract(cgElement($variables, 'value href button_type class'));
  if ($button_type == 'submit') return rcredits_submit($variables);
  
/**/ if (strpos($href, 'http') === FALSE) die('no http in button'); // $onclick = $href;
  return w\tag(nn($href) ? 'a' : 'button', $value, compact(nn($onclick) ? 'onclick' : 'href', 'class'));
}

function rcredits_checkbox($variables) {
  \element_set_attributes($variables['element'], array('id', 'name', 'value'));
  extract(cgElement($variables, 'title attributes checked class default_value tabindex'));

  $type = 'checkbox';
  if (nn($checked) or nn($default_value)) $attributes['checked'] = 'checked';
  if (nn($class)) $attributes['class'] = $class;
  if (test() and nni($attributes, 'checked')) f('t.setChecked', $title);
  if (isset($tabindex)) $attributes += compact('tabindex');
  $tribs = u\tribs(compact('type') + $attributes);
  return "<input $tribs />";
}

function rcredits_checkboxes($variables) {
  \element_set_attributes($variables['element'], array('id'));
  extract(cgElement($variables, 'choices defaults aliases parents'));
  $for = strtr(nni($parents, 0), [' '=>'-', '_'=>'-', '['=>'-', ']'=>'']);

  // get children first
  $type = 'checkbox';
  $fields = ray('name value checked title');
  $classRay = nn($class) ? compact('class') : [];
  $chn = '';
  foreach ($choices as $k => $v) {
    $name = "{$for}[$k]";
    $value = $k;
    $checked = in($k, $defaults);
///    debug(compact(ray('k v defaults checked')));
    $title = $aliases[$k];
    $fld = w\fld('checkbox', '', '', compact($fields) + $classRay); // \render arg is called by reference
    $chn .= \render($fld);
  }
  extract(cgElement($variables, 'attributes class'));
  $class[] = 'form-checkboxes';
  return w\tag('div', $chn, $attributes + compact('class'));
}

function rcredits_select($variables) {
  cgSetAttrs($variables['element'], ray('id name size'));
  extract(cgElement($variables, 'attributes class required default_value'));
  u\setDft($variables['element']['#value'], nn($default_value)); // otherwise rendered fields can't default
  $class[] = 'form-control';
  if (nn($required)) $attributes['required'] = 'yes'; // don't add this in unless it is yes
  return w\tag('select', \form_select_options($variables['element']), (nn($attributes, []) + compact('class')));
}

function rcredits_fieldset($variables) {
  \element_set_attributes($variables['element'], array('id'));
  extract(cgElement($variables, 'attributes class legend bare help value children'));

  $class[] = 'form-wrapper';
  u\preray(compact('class'), $attributes);
  if (nn($legend)) $legend = w\tag('legend', w\tag('span', $legend, ['class'=>'fieldset-legend']));
  $wrapper = w\tag('div', $children . nn($value), ['class'=>'fieldset-wrapper']);
  $variables['element']['#children'] = w\tag('fieldset', nn($legend) . $wrapper, $attributes);
  return rcredits_form_element($variables);
}

function rcredits_breadcrumb($variables) {
  $cnt = count($breadcrumb = nni($variables, 'breadcrumb'));
  if ($cnt < 2) return '';
  $map = ['Sadmin' => t('Sys Admin'), 'Account' => t('Settings')];
  $head = '<h2 class="sr-only">' . t('You are here') . '</h2>';
  foreach ($breadcrumb as $k => $v) {
    $k = ucwords(str_replace('-', ' ', strtr($k, $map)));
    if ($i = mb_strpos($k, '?')) $k = mb_substr($k, 0, $i);
    $crumbs[] = l($k, BASE_URL . $v);
  }
  $crumbs[$cnt - 1] = $k;
  return $head . '<li>' . join('</li><li>', $crumbs) . '</li>';
}

function rcredits_hidden($variables) {
  \element_set_attributes($variables['element'], array('name', 'value'));
  $type = 'hidden';
  extract(cgElement($variables, 'attributes id'));
  foreach (ray('value id') as $k) if (isset($$k)) $attributes[$k] = $$k;
  $tribs = u\tribs(compact('type') + $attributes);
  return "<input $tribs />\n";
}

/**
 * Return assoc of element parameters, plainly named.
 */
function cgElement($variables, $fields) {
  foreach ($variables['element'] as $k => $v) $e[substr($k, 1)] = $v;
  return just($fields, $e);
}

/**
 * Return HTML for a button-like link with a spinner.
 * @param string $href: where to go.
 */
function spinLink($href, $text, $id = '', $style = 'primary', $size = 'md', $other = []) {
  $dataStyles = ray('zoom-in zoom-out slide-left slide-right'); // slide-up slide-down don't work well on menus
  $dataStyle = $dataStyles[rand(0, count($dataStyles) - 1)];
  list ($tag, $class, $attrs) = ['a', "btn btn-$style btn-$size", 'class href id']; // default tag, class, and attribs
//  if ($id == 'edit-nextStep') $class .= ' center-block';
  if (!$id) unset($id);
  
  if ($href == 'submit') { // submit button
    list ($type, $value, $name, $tag) = ['submit', $text, 'op', 'button']; // name must be "op" for Drupal
    $attrs = 'class type name id value';
  } else { // menu (or list) -type link (not a button)
    if ($href[0] == '/') $href = BASE_URL . $href;
    if (mb_strpos($href, BASE_URL) !== 0 and mb_substr($href, 0, 4) == 'http')  $other += w\away();
    if (!$size) $class = $style;
  }

  if ($class2 = nni($other, 'class')) {$class .= ' ' . (is_array($class2) ? join(' ', $class2) : $class2); unset($other['class']);}
  if (!nni($other, 'target')) $class .= ' ladda-button'; // don't spin if opening a window

  foreach (ray($attrs) as $k) if (isset($$k)) $other[$k] = $$k;
  $attrs2 = u\tribs($other);

  return <<<EOF
  <$tag $attrs2 data-style="$dataStyle">
    <span class="ladda-label">$text</span>
  </$tag>
EOF;
}

function cgSetAttrs(array &$e, array $map) {\element_set_attributes($e, $map + ['#default_value' => 'value']);}
