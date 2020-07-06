<?php
namespace CG\Web;
use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Web as w;
use CG\Testing as t;

/**
 * @file
 * rWeb subroutine file
 * Tools for rWeb forms.
 */

define('R_NONE', '<span class="none">--</span>'); // how to show an empty field let the tests see null values
define('R_AGREE_0', 1); // number of the first point in the rCredits Agreement
define('R_AGREE_9', 8); // number of the last point in the rCredits Agreement
define('REQ', '*'); // marks required fields (use this as start of field's label)
define('BARE', '-'); // marks bare (unwrapped) fields -- no div wrapper around them

require_once __DIR__ . '/../cg-util.inc';
require_once __DIR__ . '/../cg-strings.inc';

global $txDays; $txDays = array(1 => 'Day', 7 => '7 days', 30 => '30 days', 90 => '90 days', 180 => '6 months', 365 => '12 months', -1 => 'Year to date', -2 => 'Ever');

$GLOBALS['tStrings'] += [ // these have to be here, not in cg-strings.inc, because of calls to atag
//  'new savings balance' => t('<p>IMPORTANT! Your %RCREDITS balance has been split to include a Credit Reserve containing your incentive rewards. Read <%a>last week\'s %PROJECT newsletter</a> for details.</p>', '_a', atag('http://us9.campaign-archive1.com/?u=b1d89f860859e1d9b906fa0b0&id=f1f54edd80&e=')),
  'increase min' => t('<%a>Click here</a> to increase your target balance!', '_a', atag('/settings/preferences')),
  'no such company' => t('There is no such company.'),
  'insufficient relation' => t('You cannot open an account unless you have a close relationship to it. <%a>Invite someone</a> instead.', '_a', atag('/community/invite')),
  'bad login' => t('That account/password combination does not exist. Try again or <%a>ask for a new password</a>.', '_a', atag('/settings/password')),
  'change min first' => t('That request would leave you with less than your target balance. <%a>Lower your target balance</a> first.','_a', atag('/settings/fund')),
  'must opt' => t('You must not create a limited account for this person without giving them the opportunity to open a real %PROJECT Account. <%a>Invite them</a>!', '_a', atag('/community/invite')),
  'no local proxy' => t('There are no other members nearby. <%a>Invite someone</a>!', '_a', atag('/community/invite')),
  'short cash help' => t('<%a>Click here</a> for more information about cashing out incentive rewards.', '_a', atag('/help/cashing-out')),
];
$GLOBALS['tStrings']['flooded'] = $GLOBALS['tStrings']['bad login'];

/**
 * Go to the specified page
 * Accommodate testing, if that's what's happening.
 * Drupal urlencodes the parameters (= and &), so the target page will have to sort that out.
 * @param string $page: page to go to
 * @param string $msg: message or index of message to output, if any
 * @param string $field: error field, if any ('err' is typical, triggering error message rather than status message)
 */
function go($page, $msg = '', $field = '') {
  global $base_url;
  global $lastGo, $xlastGo; list ($xlastGo, $lastGo) = [nn($lastGo), $page]; // for testing

  $msg = strip_tags($msg);
  if (substr($page, 0, 1) == '/') $page = substr($page, 1); // supplying the slash is preferred, to agree with w\atag()

  t\output("Going to page $page (page0=$lastGo msg=$msg field=$field)");
  if ($msg) say($msg, $field);
 
  if (test()) {
    $_POST = []; // make sure it doesn't act like we submitted the target page
    return strhas($page, 'http://') ? '' : f('t.pageForm', $page);
  }

  if (!u\starts($page, 'http')) $page = "$base_url/$page";

  \drupal_session_commit();
  header("Location: $page", TRUE, 302); 
  exit();
}

/**
 * Redirect to a new page AND open a new tab with a different URL, with focus on the first new page.
 */
function go2($url1, $url2) {
  global $base_url;
  global $lastGo, $xlastGo; list ($xlastGo, $lastGo) = [nn($lastGo), $url1]; // for testing

  foreach (ray('url1 url2') as $k) {
    if (substr($$k, 0, 1) == '/') $$k = substr($$k, 1); // supplying the slash is preferred, to agree with w\atag()
		if (!u\starts($$k, 'http')) $$k = "$base_url/" . $$k;
	}
	
	header_remove('Content-Security-Policy'); // allow the script
/**/	echo <<< X
<html><script>
  window.open('$url2', '_blank');
  location.href = '$url1';
</script></html>
X;
  exit();
}

/**
 * Sometimes when redirecting offsite, we just hang. This works.
 * @param string $url: URL to go to
 */
function goFar($url, $params = '') {
  $mark = mb_strpos($url, '?') ? '&' : '?';
  $url = urlencode("$url$mark$params");
  return w\go(BASE_URL . '/rcredits/relay.php?goto=' . $url);
}

/**
 * Process an uploaded file, returning the filename (or returning FALSE and setting an error message).
 * If the file upload is optional, the caller can ignore the error message.
 * @param string $err: (RETURNED) error message, if any
 * @param string $fld: field name of file input (defaults to "file")
 * @return filename of the temporary uploaded file (FALSE if failure)
 */
function uploadFile(&$err = '', $fld = 'file') {
  if (!nni($_FILES['files']['name'], $fld)) return !$err = t('upload required');
  $file = new \stdClass();
  foreach ($_FILES['files'] as $k => $v) $file->$k = $v[$fld];
  if (function_exists('mime_content_type') and $filetype = mime_content_type($file->tmp_name)) {
    if (!in($filetype, 'text/csv text/plain')) return !$err = t('bad file type', compact('filetype'));
  }
  if ($file->size > MAX_UPLOAD_SIZE) return !$err = t('That file is too big.');
	if (!$file->size) return !$err = t('That file is empty.');
	return $file->tmp_name;
}

function htmlItem($name, $id, $value, $class, $other = []) {return $value;}

function htmlText($name, $id, $value, $class, $other = []) {
  extract(just('tribs invisible id prefix suffix inline class box disabled', $other));

  $class = [nn($class)];
  $class[] = 'form-control input-md';
//  if ($extra = rcAuto(@$autocomplete_path, $id)) $class[] = 'form-autocomplete';

  $tribs = u\tribs(compact('type', 'class') + (nn($tribs) ?: []));
  return "<input $tribs />";
}
function htmlPassword($name, $id, $value, $class, $other = []) {
  return htmlText($name, $id, $value, $class, ['type' => 'password'] + $other);
}
function htmlHidden($name, $id, $value) {
  return htmlText($name, $id, $value, '', ['type' => 'hidden']);
}

function htmlSubmit($name, $id, $value, $class, $other = []) {
  extract(just('tribs size style', $other));
  return <<<EOF
  <button type="submit" id="$id" name="op" class="btn btn-$style btn-$size ladda-button" data-style="expand-right">
    <span class="ladda-label">$value</span>
  </button>
EOF;
}

function htmlButton($name, $id, $value, $class, $other = []) {
  extract(just('tribs href class', $other, NULL));

  if ($href and !strhas($href, 'http')) $href = BASE_URL . '/' . $href;
//  u\EXPECT(!$href or strhas($href, 'http'), 'no http in html button'); // $onclick = $href;
  if ($href) list ($tag, $role) = ['a', 'button']; else $tag = 'button';
  foreach (ray('href class role') as $k) if (isset($$k)) $tribs[$k] = $$k;
  return w\tag($tag, $value, $tribs);
}

/**
 * Create HTML for a form or page element (field).
 * @param string $type: what type of field
 * @param string $label: how to label the field
 *    if label begins with '*' the field is required
 *    if the first or second character of label is '-', don't wrap the field in a div wrapper
 * @param array $desc: how to describe the field: [hint, help text, hover text]
 *    hover text defaults to same as hint
 * @param mixed $other: assoc of additional settings
 * @return the HTML
 */
//  if (!is_array($other)) $other = ['markup' => $other]; // move this to item
// * @param mixed $choices: array or assoc of choices // move this to htmlSelect
//    if (!isset($x_default)) $x_default = $choices ? key($choices) : ''; // Drupal's select fails without this

function NEWfld($type, $label, $desc = '', $other = []) {
///  debug(compact(ray('type label desc other')));
  $label0 = substr($label, 0, 1); // not [0]
  if ($label0 == REQ) list ($required, $label) = ['yes', substr($label, 1)];
  if ($bare = ($label0 == BARE)) $label = substr($label, 1); // bare? (no div wrapper)

  list ($placeholder, $help, $title) = u\padRay(is_array($desc) ? $desc : ['', $desc], 3);
  if (!$title) $title = $placeholder; // hover text defaults to same as hint
  foreach (['placeholder', 'title'] as $k) if ($$k) $tribs[$k] = $$k;

  $info = []; foreach (ray('type label required bare help tribs other') as $k) if (isset($$k)) $info[$k] = $$k;
  return $info; // see rend()
}

// only barely begun
function rendForm($form) {
  $guts = rend($form);
  return str_replace('frm-', 'r-', w\tagN('form', $guts, $form['#attributes']));
} 

function NEWcgform($form, $onSubmit = '', $labeled = TRUE) {
  $form['#attributes']['class'][] = $labeled ? 'form-horizontal' : 'form-vertical';
  foreach ($form as $k => $v) if (substr($k, 0, 1) != '#') {
//    $form[$k]['#tabled'] = TRUE;
//    if (@$form[$k]['#type'] == 'fieldset') $form[$k] = cgform($form[$k], $onSubmit, $level + 1);
    $all[$k] = $v;
    unset($form[$k]);
  }
  $form['all'] = oldItem(rend($all));
  return $form;
}

/**
 * Modify a page in standard ways before passing it through to Drupal core for processing.
 */
function cgform($form, $labeled = TRUE) {
  $form['#attributes']['class'][] = $labeled ? 'form-horizontal' : 'form-vertical';
 
  foreach ($form as $k => $v) if (substr($k, 0, 1) != '#') {
    if (empty($v)) {unset($form[$k]); continue;} // NULL is ignored
    
    if (!is_array($v)) { // these common fields can be specified simply
      $form[$k] = $v = in($k, ITEM_FLDS) ? item($v)
      : ($k == 'submit' ? submit($v)
      : ($k == 'noEnter' ? submit('', '', '', disabled()) // prevent Enter from submitting form (must precede submits)
      : ($k == 'frm' ? hidFld($v)
      : u\FAIL("field $k is not an array: " . pr($v)) )));
    }
    
    $type = nni($v, '#type');
    if ($type == 'hidden') f('t.POST', [$k => nni($v, '#value')]); // fake post for tests
    if ($type == 'fieldset') $form[$k] = cgform($v, $labeled);
    if ($labeled and is_array($form[$k])) $form[$k]['#tabled'] = TRUE; // remember that form is laid out in a table (labels in left column)
  }

  $uri = $_SERVER['REQUEST_URI'];
  if ($i = strpos($uri, '/er=')) { // coming back after an error (or called from a test)
    $vs = urlq2ray(u\deurlify(substr($uri, $i + 4)));
    foreach ($vs as $k => $v) {
      $form[$k]['#value'] = $v;
    }
  }

  return $form;
}

/**
 * Construct a return URL with original parameter values.
 * Call from validation function as: return er($sta, $msg, $fld)
 * @param assoc $sta: the form's status parameter
 * @param string $msg: what to tell the user
 * @param string $fld: field to focus on, if any
 */
function er($sta, $msg, $erFld = '') {
  $input = justNOT('form_build_id form_id opid submit', $sta['input']);

  $er = u\urlify(http_build_query($input + compact('erFld')));
  $uri = $_SERVER['REQUEST_URI'];
  if ($args = $sta['build_info']['args'][0]) $uri = str_replace($args, '', $uri);
  return go(str_replace('//', '/', "/$uri/er=$er"), $msg, 'err'); // sometimes has one "/" sometimes two
}

/**
 * Drupal render the given field, with the given name.
 * @param string $name: field name
 * @param assoc $field: a Drupal field description array
 * @return Drupal rendering, with repairs.
 */
function rendA($name, $field) {
//    $rent .= preg_replace('/name="(.*)"/', "name=\"$k\"", rent($field));
  return preg_replace('/<(input|textarea|select|button) /', "<$1 id=\"edit-$name\" name=\"$name\" ", rent($field));
}

// NYI
function rend($fields) {
  $html = '';
  foreach ($fields as $name => $ray) {
    extract(just('type bare other', $ray, NULL)); // see fld()
    extract(just('id value class', $other, NULL));
    u\setDft($id, "edit-$name");
    $func = 'CG\\Web\\html' . ucfirst($type);
    $fldHtml = $func($name, $id, $value, $class, $other);
    $html .= $bare ? $fldHtml : wrap($name, $fldHtml, $ray);
  }
  return $html;
}

function wrap($name, $html, $ray) {
  extract(just('type label required bare help other', $ray)); // see fld()
  extract(just('invisible id prefix suffix inline class box disabled', $other, ''));
  $for = strtr($name, ['_'=>'-', '['=>'-', ']'=>'']);
  if ($for == 'title') $html = "<h3>$html</h3>";
  $html = $prefix . $html . $suffix;

  $boxy = in($type, 'radio checkbox') ? $type : '';

  $class = [$class];
  $class[] = $boxy ? 'option' : 'control-label col-sm-offset-1 col-xs-2';
  if ($invisible) $class[] = 'sr-only';
  $label = w\tag('label', $boxy ? $html . $label : $label, compact('for', 'class'));
    
  if (!$bare) {
    $help = $help ? "\n      <div class=\"help-block\">$help</div>" : '';
    $lineup = $type == 'item' ? ' lineup' : '';
    $core = <<<EOF
    $label
    <div class="control-data col-xs-10 col-sm-6$lineup">
      $html$help
    </div>
EOF;
  } else $core = $boxy ? $label : $label . $html;

  $id = $lineup ? " id=\"edit-$for\"" : '';
  $outerClass = "form-group wrap-$for";
  $outerClass .= ' ' . ($boxy ?: "type-$type");
  if ($box) $outerClass .= ' box1';
  if ($disabled) $outerClass .= ' disabled';
  
  return <<<EOF
  <div$id class="$outerClass">
$core
  </div>
EOF;
}
  
/**
 * Say whether the member has permission to run the named query (all queries are for members only).
 * @param string $qName: name of the query -- if first character is a digit, restrict to admins/companies
 *   0=super 1=admin 2=cAdmin2 B_CO(6)=company
 * @param string $sql: the query, optionally followed by subtext, after a tilda (~)
 *                     (RETURNED) the query modified to apply to the current community, if appropriate
 * @param string $subtext: (RETURNED) the explanatory text, if any
 * @return FALSE if not permitted, else the query name without any leading digit
 */
function queryPermitted($qName, &$sql = '', &$subtext = '') {
  global $mya; if (!$mya) return FALSE; // queries are for signed-in members only

  $cttyName = $mya->community ? $mya->cttyA->fullName : t('All Communities');
  list ($sql, $text2) = explode('~', "$sql~");
  $map[':myid'] = $mya->id;
  $map['=:ctty'] = $mya->community ? "=$mya->community" : '<>0'; // alternative is for superAdmin and signed out
  $map[':ctty IN'] = $mya->community ? "$mya->community IN" : '1 NOT IN';
  $sql = strtr($sql0 = $sql, $map);
  $subtext = mb_strpos($sql0, ':ctty') ? "<h4>$cttyName</h4><br>$text2" : $text2;
  
  if (!preg_match('/^([0-9])(.*)/', $qName, $m)) {
    return $qName; // no restrictions
  } else {
    list ($zot, $d, $qName) = $m;
    if (mb_strpos($sql0, ':myid')) $subtext = "<div><h4>for $mya->fullName</h4>$text2</div>";
    return $d >= ($mya->superAdmin ? 0 : ($mya->admin ? 1 : ($mya->cAdmin2 ? 2 : ($mya->co ? B_CO : 9))))
    ? $qName
    : FALSE;
  }
}
  
/**
 * Return a table of formatted records, given an array of rows.
 */
define('ALIGN_RIGHT', 'proxiedFor trust');
define('ALIGN_CENTER', '');
define('FMT_INT', 'trust count');
//define('FMT_2D', 'negativeBalanceTotal positiveBalanceTotal');
 
function showRecords($ray, $formatFunc = 'CG\\Web\\formatField', $flagsField = B_LIST) {
///    debug("showRecords(" . pr($ray) . ", $formatFunc, $flagsField)");
  if (!$ray or empty($ray) or count($ray)==0) return 'data set is empty';
  $first = (array)reset($ray);
  $lines[] = '<tr><th>' . join('</th><th>', array_keys($first)) . "</th></tr>\n";
  foreach ($ray as $row) {
    $line = '';
    foreach ($row as $k => $v) {
      $v = $formatFunc($k, plain($v), $flagsField);
//      if ($k == 'for2' and $v == $row->for1) $v = 'same'; // admin uses this for conciseness
      $align = in($k, ALIGN_RIGHT) ? 'R' : (in($k, ALIGN_CENTER) ? 'C' : 'L');
      foreach (ray('balance credit trust count total $ pct dollars') as $w) if (strihas($k, $w)) {
        $align = 'R';
        if ($v !== '' and is_numeric($v)) $v = in($w, 'pct') ? u\fmtAmt($v, '%') : number_format($v, in($w, FMT_INT) ? 0 : 2);
      }
        
//      if (in_array($k, ray(FMT_INT))) $v = round($v);
//      if (in_array($k, ray(FMT_2D)) or strhas($k, '$')) if ($v != '') $v = number_format($v, 2);
      $line .= "  <td class=\"align$align\">$v</td>\n";
    }
    $lines[] = $line;
  }
  return "<table>\n<tr>" . join("</tr>\n<tr>\n", $lines) . "</tr>\n</table>";
}

/**
 * Return the given field, formatted for viewing.
 * Unlike a\formatField, the third argument ($flagsField) is ignored.
 */
function formatField($k, $v) {
  $v = $v . ''; // convert to string
//  if ($v and $k == 'channel') $v = u\rayv(ray(TX_CHANNELS), $v);
//  if ($v and in_array($k, ray(R_DATE_FLDS))) $v = '<div style="line-height:65%;">' . strftime('%d%b', $v) . '<span style="font-size:50%;">\'' . strftime('%y', $v) . '<br>' . strftime(isDEV ? '%I:%M %p' : '%l:%M %P', $v) . '</span></div>';

  if (u\crypted('P', $v)) $v = u\decry('P', $v); // decrypt phone and email
  if ($k == 'phone') $v = u\fmtPhone($v);
  if ($k == 'account') $v = $v ? r\qid($v) : '';
  return $v;
}

/**
 * Create a Drupal field array.
 * Calls (typically from a helper function like numFld or selectFld):
 *   fld(type, args)
 *   fld(type, label, help, value)
 *   fld(type, label, help, other, options)
 * @param string $type: field type (text num pass area tel date email range radios select submit item hidden file checkbox)
 * @param assoc $args: all other arguments besides type
 * @param assoc $label: textual prompt to precede the field
 *   optionally preceded by REQ and/or BARE (single characters) in that order
 *   REQ means the field is required, BARE means don't wrap the field in a div (overrideable by wrap => TRUE in $other)
 * @param mixed $help: help string to go below the field or [placeholder, help] or [placeholder, help, hover text]
 * @param assoc $x: additional attributes
 * @param assoc $options: options for select, radio buttons, or checkboxes
 * @return the field array
 */
function fld($type, $label = '', $help = '', $x = [], $options = []) {
  if (is_array($label)) list ($label, $help, $x, $options) = $label + ['', '', [], []];
  if (!is_array($x)) $x = [$type == 'item' ? 'markup' : 'value' => $x];

  if ($type == 'int') u\setDft($x['step'], 1);
  $type = strtr($type, ray('text:textfield, num:number, int:number, pass:password, area:textarea, phone:tel'));
  if (in($subtype = $type, 'number tel date email range password')) {
    $dft = nni($x, 'default_value', NULL);
    if ($type == 'tel') {
      if ($dft) $x['default_value'] = u\fmtPhone($dft);
    } elseif ($type == 'number') {
      u\setDft($x['step'], .01);
      u\setDft($x['min'], 0);
    } elseif ($type == 'date') {
      if ($dft) $x['default_value'] = date('Y-m-d', $dft);
    }
    $x += compact('subtype');
    $type = 'textfield';
  }

  if ($type == 'textfield') u\setDft($x['maxlength'], 4096);
  if ($type == 'select') u\setDft($x['default_value'], $options ? key($options) : ''); // Drupal's select fails without a default
  
  if ($label and $label[0] == REQ) list ($x['required'], $label) = ['yes', substr($label, 1)];
  if ($bare = ($label and $label[0] == BARE)) $label = substr($label, 1); // bare? (no div wrapper)
  if (nni($x, 'wrap')) $bare = FALSE;
  u\setDft($x['title'], $label); 
  $x['title_display'] = 'before';
  
  $field = u\prefixKeys('#', $x + compact(ray('type bare help options')));

  return $field;
}

function textFld() {return fld('text', func_get_args());}
function passFld() {return fld('pass', func_get_args());}
function areaFld() {return fld('area', func_get_args());}
function numFld() {return fld('num', func_get_args());}
function intFld() {return fld('int', func_get_args());}
function phoneFld() {return fld('phone', func_get_args());}
function dateFld() {return fld('date', func_get_args());}
function emailFld() {return fld('email', func_get_args());}
function rangeFld() {return fld('range', func_get_args());}
function radiosFld() {return fld('radios', func_get_args());}
function selectFld() {return fld('select', func_get_args());}

// Plan: move all theme functions here. Output each page as a single item(). Each field type function should have a nogroup option (start label with -)
function submit($title = '', $style = '', $size = '', $other = []) {
  if (!$style) $style = 'primary';
  if (!$size) $size = 'md';
  $other += [
    'input' => TRUE,
    'name' => 'op',
    'button_type' => 'submit',
    'executes_submit_callback' => TRUE,
    'limit_validation_errors' => FALSE,
    'process' => array('ajax_process_form'),
    'theme_wrappers' => array('button'),
  ];
  if (substr($title, 0, 1) == BARE) list ($bare, $title) = [BARE, substr($title, 1)];
  return fld('submit', nn($bare), '', $other + ['value' => $title ?: t('Save')] + compact('style', 'size'));
}
function submi($title = '', $style = '', $size = 'xs', $other = []) {
  return submit(BARE . $title, $style, $size, $other);
}

/**
 * Return a button field array.
 * @param string $value: what the button says
 * @param string $href: where the button goes (or does)
 * @param string $title: hover text
 * @param string $style: Bootstrap style
 * @param string $size: Bootstrap size
 * @param assoc $extra: other attributes (id, class, etc)
 */
function buttonFld($value, $href = '', $title = NULL, $style = 'success', $size = 'xs', $extra = []) {
  $class = trim(nni($extra, 'class') . " btn btn-$style btn-$size");
  return NEWfld('button', BARE, ['', '', $title], compact(ray('href value title class')) + $extra);
}
function button($value, $href = '', $title = NULL, $style = 'success', $size = 'xs', $extra = []) {
  return rend([buttonFld($value, $href, $title, $style, $size, $extra)]);
}
function btnSubmit($value, $submit = NULL) {
  $class = 'submit' . ($submit ?: t('Submit'));
  return button($value, '', NULL, 'success', 'xs', compact('class'));
}
function btnShowDiv($divId, $title, $style='success', $size = 'xs') {
  return "<a href=\"#$divId\" class=\"btn btn-$size btn-$style\" data-toggle=\"collapse\">$title</a>";
}

function NEWitem($value, $title = '', $desc = '', $other = []) {return fld('item', $title, $desc, compact('value') + $other);}
function item($markup, $title = '', $desc = '', $other = []) {return fld('item', $title, $desc, compact('markup') + $other);}
function hidFld($value = '', $other = []) {return fld('hidden', '', '', compact('value') + $other);}
function NEWtextFld($label = '', $desc = '', $other = []) {return fld('text', $label, $desc, $other);}

function fileFld($label = '', $desc = '', $extra = []) {return fld('file', $label, $desc, $extra + ray('enctype', 'multipart/form-data'));}
function dataFld($id, $data) {return item(w\tags('div', "<!--$data-->", compact('id')));}
function boxFld($name, $label = '', $desc = '', $dft = NULL, $other = []) {
  $fld = fld('checkbox', $desc, '', compact('name') + dft(nn($dft)) + $other);
  return $label ? item(\render($fld), $label, '', ['box' => TRUE]) : $fld;
}
function togFld($name, $label = '', $desc = '', $dft = NULL, $other = []) {
  js('x/bootstrap-toggle.min');
  css('x/bootstrap-toggle.min.css');
  extract(just($ks = 'on off onstyle offstyle size', $other));
  foreach (ray($ks, t('Yes'), t('No'), 'success', 'default', 'mini') as $k => $v) u\setDft($$k, $v);
  
  $other['attributes'] = ray("data-toggle:togglel,data-on:$on,data-off:$off,data-onstyle:$onstyle,data-offstyle:$offstyle,data-size:$size");
  return boxFld($name, $label, $desc, $dft, $other);
}

/**
 *
 * @see also boxesFld()
 */ 
function boxFlds($group, $label, $question, $options, $other = []) {
  $question = $question ? "<p>$question</p>\n" : '';
  $list = '';
  foreach ($options as $k => $v) {
    $list .= <<<EOF
<div class="row">
<div><input type="checkbox" id="edit-$group-$k" name="{$group}[$k]" /></div>
<div>$v</div>
</div>
EOF;
  }
  return item($question . $list, $label, '', ['class' => ['boxes']] + $other);
}

function fieldSet($id, $flds, $label = '', $desc = '', $other = []) {return fld('fieldset', $label, $desc, compact('id') + $other) + $flds;}

/**
 * Return a form field for donating fractional dollars.
 */
function roundupFld($a) {
  return w\boolFld(t('Round Up!'), t('Round all your payments up to a whole dollar amount and donate the cents toward building the %PROJECT Economy in your area.'), $a ? $a->roundup : FALSE);
}

/**
 * Return a boolean (Yes / No) field.
 * @param string $title: field label
 * @param string $help: helpful description of field
 * @param mixed $dft: the default value (TRUE or FALSE) or an assoc of attributes
 * @param array $labels: labels for the two choices (defaults to [No, Yes])
 */
function boolFld($title = '', $help = '', $dft = NULL, $labels = NULL, $extra = []) {
  $class = ['yesno'];
  $inline = TRUE;
  $dft = is_null($dft) ? [] : (is_array($dft) ? $dft : dft(nn($dft, 0) ?: 0));
  return radiosFld($title, $help, $extra + $dft + compact('class', 'inline'), $labels ?: array(t('No'), t('Yes')));
}

/*
 * Create a rendered checkboxes field
 * @param string $title:
 * @param string $desc:
 * @param array $defaults: simple array of indexes of already-checked boxes
 * @param assoc $choices: name for each checkbox (indexed)
 * @param assoc $aliases: name to display for each checkbox (defaults to choice name)
 * @see also boxFlds()
 */
function boxesFld($title, $desc, $defaults, $choices, $aliases = NULL) {
  u\setDft($aliases, $choices);
  if (!$defaults) $defaults = [];
  return fld('checkboxes', $title, $desc, compact('choices', 'defaults', 'aliases'), []);
}

/**
 * Return a set of checkboxes representing bits in a bit array.
 * @param string $label: short prompt for the set of boxes
 * @param int $bitRay: the bit array (current value)
 * @param string $prefix: prefix for defined constants for this bit array field (empty means use $bits index)
 * @param string $bits: array (space or comma-delimited) of names of bits to include (without prefixes)
 * @param assoc $aliases: map of bit names to their display aliases (which may include formatting)
 * @param string $desc: helpful description to show below the field
 */
function bitsFld($label, $bitRay, $prefix, $bits, $aliases = [], $desc = '') {
  if (!is_array($bits)) $bits = ray($bits);
  if (!is_numeric(key($bits))) $bits = array_keys($bits); // ignore values (eg weights for risk arrays)
  $defaults = [];
  foreach ($bits as $bit => $bitName) {
    if ($prefix) $bit = u\consta($prefix, $bitName);
    $bitNames[$bit] = $bitName;
    $bitAliases[$bit] = $aliases ? strtr($bitName, $aliases) : $bitName;
    if ($bitRay and u\getBit($bitRay, $bit)) $defaults[] = $bit;
  }
  return boxesFld($label, $desc, $defaults, $bitNames, $bitAliases);
}

/**
 * Return a signature field for the Common Good Agreement or for backing.
 */
function signedBy() {
  global $mya;
  $behalf = $mya->co ? t('(on behalf of %co) ', 'co', $mya->fullName) : '';
    
  $signerA = ($mya->cAdmin or ($mya->co and $mya->proSe))
  ? (($mya->co and $mya->helper) ? $mya->helperA : $mya) 
  : $mya->agentA;
  return textFld(t('Signed:'), $behalf . t('Type your full legal name here ') . "($signerA->legalName)", required($mya->cAdmin ? $signerA->legalNameDpy : ''));
}

/**
 * Complain if the signature parameter is bad.
 */
function badSignature(&$sta) {
  global $mya;
  $legalName = r\agent()->legalNameDpy;
  if (!$mya->cAdmin and strcasecmp(trim($sta['input']['signedBy']), $legalName) != 0) return say('bad signature', compact('legalName'), 'signedBy');
}  
  
/**
 * Return onSubmit script for a typeahead member account field (and include needed script files).
 * @param string $field: the field name
 * @param string $question: what to ask, for confirmation or disambiguation (with params %amount and %name)
 * @param assoc $extra: less common parameters:
 *   amount: UNUSED amount substitution for question (defaults to form.amount.value)
 *   allowNonmember: <allow a non-member email address>
 *   coOnly: <restrict results to companies>
 */
function whoFldSubmit($field, $question, $extra = []) {
  extract(just('amount allowNonmember coOnly', $extra, ''));
  js('x/typeahead.bundle.min');
  jsx('suggest-who', compact(ray('field question amount allowNonmember coOnly')));
}

/**
 * Retrieve the specified form, called with the args, and return it rendered.
 * Sets: $formArray and $formSta, for testing.
 * Sets: $pageTitle
 */
function showForm($function, $arg1 = '', $arg2 = '', $arg3 = '', $arg4 = '') {
  global $formArray, $formSta, $rUrl;
  global $mya;

  if (!(nni($formSta, 'confirm')) and !in($function, R_NOLOG_FORMS)) {
    $formName = str_replace(base_path(), '', $_SERVER['REQUEST_URI']);
    $logit = TRUE;
    foreach(ray('sadmin/ account- /admin /wp-admin /wp-login /xmlrpc') as $i => $k) if (strhas($formName, $k)) {
      if ($i >= 2) return hack(t('blog attack'));
      $logit = FALSE;
    }
    
    if ($logit) {
      if ($logs = $_POST) {
        u\loga('in', $formName . substr(pr(u\noSecrets($logs)), 5)); // without "Array"
      } else {u\loga('form', [$formName]);}
    }
  }

  $args = 'arg1' . ($arg2 !== '' ? ' arg2' : '') . ($arg3 !== '' ? ' arg3' : '') . ($arg4 !== '' ? ' arg4' : '');
  $formName = substr($function, 0, 1) == '/' ? substr($function, 1) : "CG\\Web\\form$function";
  $specialForm = in($function, 'Empty footer accounts Login');
  $adminForm = in($function, 'Admin MemberList MemberInfo');
  $args = func_get_args(); // this and next few lines are from \drupal_get_form()
  array_shift($args); // Remove $function from the arguments.
  $formSta['build_info'] = compact('args');
  $formSta['cache'] = TRUE;
//    $formSta['method'] = 'post';
  if (!r\up() and !$adminForm and (!$specialForm or $function == 'Help')) {
    if ($mya and $mya->superAdmin) {
      say(t('The system is DOWN.'));
      //if (!$mya->cttyUp) say(t('The community is DOWN.'));
    } else $formName = 'CG\\Web\\formSystemDown';
  }
  
  $form = \drupal_build_form($formName, $formSta);

  $req = basename($_SERVER['REQUEST_URI']);
  if (!$mya and in($req, 'login logout')) return w\go(PROMO_URL); // nothing else works
  if ($req == 'user' and $mya) return w\go('summary'); // summary not ''
  foreach ($form as $k => $v) if (is_array($v) and nni($v, '#type') == 'submit') {$submit1 = nni($v, '#id') ?: "edit-$k"; break;}
  $form['opid'] = hidFld(nn($submit1) ?: 'none', ['name' => 'opid']); // default to first, in case submitted programmatically

  if (!nni($form, 'confirm')) $form += which(); // add the choices popup form, if any
  if (!$specialForm) {
    \drupal_set_breadcrumb(makeBread());
    setPageTitle($form);
  }
  $formArray = $form2 = $form; 
  return \render($form2);
}

/**
 * Construct and return the breadcrumb list.
 */ 
function makeBread() {
  $tree = explode('/', $_SERVER['REQUEST_URI']);
  $bread = [];
  foreach ($tree as $k) if (preg_match('/^[a-z\-]+(\?|$)/', $k)) $bread[ucwords($k)] = $kk = isset($kk) ? "$kk/$k" : '';
  return $bread;
}

/**
 * Set the page title, usually based on the form title.
 */
function setPageTitle($form) {
  global $pageTitle;
  $pageTitle = ($pageTitle = nni($form, 'title'))
  ? strip_tags($pageTitle['#markup'])
  : ucwords(str_replace('/', ' | ', $_SERVER['REQUEST_URI']));
  $pageTitle = PROJECT . ' | ' . $pageTitle;
}

define('FORBID_TAGS', '/.{0,40}<(script|style|frame|object|embed|applet|audio|video).{0,40}/i'); // forbidden html tags
define('FORBID_ATTRS', '/(.{0,40}\\s(href|src|style|on[a-z]+)\\s*=.{0,40})/i'); // forbidden attributes

/**
 * Sanitize the page before displaying it. 
 * Actually, since the page should NEVER have illegal stuff, we don't display a failed page, just detect it.
 * @param string $page: the body of the page, not including our selected scripts
 */
function sanitizePage($page) {
  // preg_quote needed in case URL has a ~ in it
  $except = join('|', 
    ['<img src="' . preg_quote(BASE_URL, '~'), 
     '<img src="https://cg4.us/', 
     '<iframe src="(' . str_replace(' ', '|', preg_quote(FRAME_SOURCES, '~')) . ')',
     'href="(mailto:|http|#|/|"|sadmin)',
     '(href|src|style)=(&quot;|&amp;)', // our tests output stuff like this
    ]);

  $page = preg_replace("~$except~", '', $page);

  if ($tags = preg_match(FORBID_TAGS, $page, $matches) or preg_match(FORBID_ATTRS, $page, $matches)) {
    $msg = t('Page has a forbidden HTML %thing: %what', 'thing what', $tags ? t('tag') : t('attribute'), pr($matches));
    if (isPRODUCTION) w\hack($msg);
    
  	if (isDEV) {
      header_remove('Content-Security-Policy'); // let us show styles
      $page = str_replace('pre>', 'WASPRE>', $page);
      $m = htmlentities($matches[0]);
      $msg .= '<br><br>' . str_replace($matches[0], "--><code style=\"background-color:yellow;\">$m</code>", $page);
    }
/**/die($msg);
  }
}

/**
 * Output a photo upload success or error message and exit.
 * @param string $msg: if error, what to say. if success, url and type of file uploaded.
 * @param string $status: 'success' or 'error' (the default)
 */
function photoRet($msg, $status = 'error') {
  $res = compact('status') + ray($status == 'success' ? 'url' : 'message', $msg);
  u\loga('photoRet', $res);
/**/	echo json_encode($res, JSON_UNESCAPED_SLASHES);
  exit();
}

/**
 * Provide the crucial fields and scripts for choosing or changing a password.
 * @param bool $reset: <changing, rather than choosing>
 * @param string $req: REQ for field is required, else ''
 */
function pickPassword($reset = TRUE, $req = '') {
  global $mya;
  $pass1 = textFld($req . ($reset ? t('New password:') : t('Choose a Password:')), '', clas('password-field') + autofill('off'));
  $pass2 = textFld($req . ($reset ? t('Confirm new password:') : t('Confirm password:')), '', clas('password-confirm') + autofill('off'));
  $strong = hidFld(0, ['id' => 'edit-strong']);
  focusOn('pass1');
  
  $password = [
    'strengthTitle' => 'Strength:',
    'makeLonger' => 'Make it longer',
    'weak' => 'Weak (not strong enough)',
    'fair' => 'Fair (but not strong enough)',
    'good' => 'Good (but not strong enough)',
    'strong' => 'Strong (but not strong enough)',
    'verystrong' => 'Very strong (strong enough)',
    'mighty' => 'Mighty (strong enough)',
    'paranoid' => 'Paranoid? (plenty strong)',
    'hasWeaknesses' => 'To make your password stronger:',
    'tooShort' => 'Make it at least 6 characters',
    'addLowerCase' => 'Add lowercase letters',
    'addUpperCase' => 'Add uppercase letters',
    'addNumbers' => 'Add numbers',
    'addPunctuation' => 'Add punctuation',
    'sameAsUsername' => 'Make it different from your username',
    'confirmSuccess' => 'yes',
    'confirmFailure' => 'no',
    'confirmTitle' => 'Passwords match:',
    'username' => $mya ? strtolower($mya->mainQid) : '',
  ];

  $settings = json_encode(compact('password'));
  js('drupal'); // for password strength meter
  js('user');
  $settings = item("<!--//--><![CDATA[//><!--\njQuery.extend(Drupal.settings, {$settings});\n//--><!]]>");

  return compact(ray('pass1 pass2 strong settings'));
}
  
/**
 * Return a unique persistant identifier for the current machine (set randomly, if none yet).
 * @return the box ID string
 */
function box() {
  global $box;
  if ($box = nni($_COOKIE, 'box')) return $box;
  return r\setCook('box', $box = u\code());
}

/**
 * Return the box record id for the given machine code and account record ID.
 * @param int $code: machine identifier
 */
function boxUser($code, $uid) {
  if ($id = nni($_COOKIE, $cookid = 'boxnum' . $uid) and !test()) return $id; // we only get this if last set today
  if ($id = db\get('id', 'r_boxes', 'code=:code AND uid=:uid', compact(ray('code uid')))) {
    db\update('r_boxes', ray('id access', $id, now()), 'id');
  } else $id = r\makeDevice($uid, $code, TX_WEB);
  return r\setCook($cookid, $id, now() + SESSION_LIFE);
}

/**
 * Return rows from an uploaded file.
 */
function uploadrows($filename) {
  file_put_contents($filename, $str = strtr($get = file_get_contents($filename), ["\r\n" => "\n", "\r" => "\n"]));
  $fp = fopen($filename, "r");  
  while ($row = fgetcsv($fp)) $rows[] = $row; // get all rows, so we can look ahead
  fclose($fp);
  u\deleteFile($filename);
  return $rows;
}

/**
 * See whether someone is trying too many logins.
 * @param int $uid: account record ID
 * @param assoc $sta: (MODIFIED) the form status, updated with relevant flood info
 * @param
 * @return mixed: FALSE only if we're not being flooded
 */
define('MAX_PW_FAILS', 10);
define('MAX_IP_FAILS', 50);

function flooding($uid, &$sta, &$err) {
  if (test()) return !$uid;
  $trigger = 'flood_control_triggered';
  $window = HOUR_SECS; 
  if (!\flood_is_allowed('failed_login_attempt_ip', MAX_IP_FAILS, $window)) {
    $sta[$trigger] = 'ip';
    $err = t('There have been too many failed sign-in attempts from your IP Address. Try again later.');
    r\tellAdmin($err, ['ip' => $_SERVER['REMOTE_ADDR']]);
    return TRUE;
  }
  
  $err = t('bad login');
  if (!$uid) return TRUE;

  $uidOnly = FALSE; 
  $window = 6 * HOUR_SECS;
  $sta['flood_control_user_identifier'] = $id = $uidOnly ? $uid : $uid . '-' . ip_address();
  if (!\flood_is_allowed('failed_login_attempt_user', MAX_PW_FAILS, $window, $id)) {
    $sta[$trigger] = 'user';
    $err = t('There have been too many failed sign-in attempts to your account (%MAX_PW_FAILS). Try again later.');
    return TRUE;
  }

  return FALSE;
}

function noFlood($sta) {\flood_clear_event('failed_login_attempt_user', nni($sta, 'flood_control_user_identifier'));}

function updateFlood($sta, $success) {
  if ($success) return noFlood($sta); // Clear failures (don't block a user who might log in and out more than once in an hour)
  
  \flood_register_event('failed_login_attempt_ip', 3600); // Always register an IP-based failed login event.
  if ($id = nni($sta, 'flood_control_user_identifier')) \flood_register_event('failed_login_attempt_user', 21600, $id); // Register a per-user failed login event.

  if ($trigger = nni($sta, 'flood_control_triggered')) {
    $msg = $trigger == 'user' ? t('Sorry, there have been more than :MAX_PW_FAILS failed sign-in attempts for this account. It is temporarily blocked.') : t('Sorry, too many failed login attempts from your IP address. This IP address is temporarily blocked.');
    w\say($msg . t(' Try again later or <%a>request a new password</a>.', '_a', atag('/user/password')), 'name');
  } else return w\say(t('bad login', '_a', atag('/settings/password')), 'name');
}

/**
 * Return all the session variables
 * @param string $sid: the session ID
 */
function sessionVars($sid) {
  if (!$both = db\get('uid,session', 'sessions', 'ssid=:sid', compact('sid'))) return [];
  extract($both);
  $res['uid'] = $uid;
  
  $vars = explode(SVAR_HEADER, $session);
  array_shift($vars); // first is empty of course
  foreach ($vars as $one) {
    list ($k, $v) = explode('|', $one);
    $res[$k] = unserialize($v);
  }
  return $res;
}

/**
 * Set or retrieve a session variable.
 * Calls:
 *   svar($name, $value);
 *   $value = svar($name);
 * @param string $name: name of variable
 * @return: current value of the named session variable
 */
function svar($name) {
  $name = SVAR_HEADER . $name;
  $args = func_get_args();
  if (count($args) > 1) $_SESSION[$name] = $args[1];
  return isset($_SESSION) ? nni($_SESSION, $name) : ''; // current value
}

/**
 * Offer choices for user to choose from
 * 1. If called with all arguments, the arguments are stored in svar('pop') and the form is reloaded
 * 2. If called as which('info'), this function returns $info from svar('pop')
 * 3. If called with NO arguments (from showForm()), this function returns a fieldset to insert as a popup layover div into
 * the form awaiting the choice. Clicking on a choice fills in the missing value and submits the form.
 * For #3, we expect svar('pop') to contain: $formName, $choices, $resultField, and $question
 *   $formName: the form name
 *   $choices: an array of choices
 * @param array $choices: associative array of things to choose from
 * @param string $resultField: name of field that gets the choice
 * @param array $info: field values to be stored, then reinstated when the page is refreshed to show the choices
 * @param string $question: the question to ask
 */
function which($choices = '', $resultField = '', $info = '', $question = 'Which one?') {
  $formName = current_path();
  
  if (is_array($choices)) { // called with all args (#1)
    svar('pop', compact(ray('formName choices resultField info question')));
    w\go($formName); // reload page
  }

  if (!is_array($pop = svar('pop'))) return [];
  extract(svar('pop'), EXTR_PREFIX_ALL, 'pop');
  if ($formName != $pop_formName) return [];
  if ($choices == 'info') return $pop_info; // called as which('info') (#2)
  if ($choices) return [];
  
  svar('pop', NULL); // called with no args (#3)
  $tribs['size'] = 12;
  $choice = selectFld('', '', $tribs, $pop_choices);
  $choice = render($choice);
  $legend = t('Which one?');
  $popup = item(w\modal('which', $legend, $pop_question, $choice, t('Cancel')));
  jsx('which', 'field', $pop_resultField);
  return compact('popup');
}

/**
 * Create a modal dialog
 * @param string $id: the dialog id
 * @param string $legend: header
 * @param string $prompt: what to say under the header
 * @param string $body: what to put in the body of the dialog
 * @param string $closer: what to show in the upper right corner, to close the dialog (default empty)
 * call with <button type="button" class="btn btn-info btn-lg" data-toggle="modal" data-target="#$id">Go</button>
 * @return HTML for the modal dialog
 */
function modal($id, $legend, $prompt, $body, $closer = '') {
  if ($closer == 'X') $closer = '&times;'; // a better X (normally with class "close" alone)
  if ($prompt !== '') $prompt = "<p>$prompt</p>";
  return <<<EOF
<div id="$id" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="pull-right btn btn-xs btn-default" data-dismiss="modal">$closer</button>
        <h4 class="modal-title">$legend</h4>
        $prompt
      </div>
      <div class="modal-body">
        $body
      </div>
      <!--div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div-->
    </div>

  </div>
</div>
EOF;
}

/**
 * Return a submenu page (list) for a given menu or named menu section.
 * @param string $title: header for the menu
 * @param mixed $menu: menu section name or assoc of menu items
 * @return the menu as a Drupal form (not yet rendered)
 */
function subMenu($title, $menu) {
  $title = item($title, '', '', clas('submenu'));
  $list = item(subMenuHtml($menu), BARE);
  return compact('title', 'list');
}

function subMenuHtml($menu) {
  global $base_url;

  if (!is_array($menu)) $menu = \rweb_menu($menu);
  $list = '';
  foreach ($menu as $k => $v) {
    list ($type, $head, $args, $perms, $func, $detail, $icon) = u\padRay($v, 7);
    if ($type == 'call' or !w\webAccess($perms)) continue;
    if (nn($detail)) $detail = "<p class=\"list-group-item-text\">$detail</p>";
    $text = <<<EOF
    <h4 class="list-group-item-heading">
      <span class="glyphicon glyphicon-$icon"></span> &nbsp;
      $head
    </h4>
    $detail
EOF;
//    $onclick = "jQuery(this).find('.glyphicon').css('color', 'darkblue');"; // glyph looks funny when ladda-spinning
//    $list .= spinLink("/$k", $text, '', 'list-group-item', '', compact('onclick'));
    $list .= spinLink("/$k", $text, '', 'list-group-item', '');
  }
  return $list;
}

function keep_values(&$form) {
  if (!$info = which('info')) return $form;
  foreach ($form as $key => $value) {
    if (isset($info[$key])) {
      $form[$key]['#default_value'] = $info[$key];
    } elseif (substr($key, 0, 1) != '#') foreach ($form[$key] as $key2 => $value2) { // fieldset
      if (isset($info[$key2])) $form[$key][$key2]['#default_value'] = $info[$key2];
    }
  }
  return $form; // for convenience we both modify the form and return it
}

function memberStep($step, $done, $text, $links = '', $newWindow = FALSE) {
  u\EXPECT('done', 'bool');
  $target = $newWindow ? ' target="_blank"' : '';
  if ($links) {
    if (!is_array($links)) $links = array($links);
    foreach ($links as $href) {
      if (!strhas($href, 'http')) $href = BASE_URL . "/$href";
      $link = "<a href=\"$href\"$target>";
      $linkx = '</a>';
      $text = preg_replace('/<a>/', $link, $text, 1);
    }
  } else $link = $linkx = '';
  return <<<EOF
<tr>
  <td class="done done-$step state$done">$link<div>&nbsp;</div>$linkx</td>
  <td class="step step$step">$link<div>$step</div>$linkx</td>
  <td class="text"><div>$text</div></td>
</tr>
EOF;
}

// NOT USED
function divButton($href, $title) {
  if (!strhas($href, 'http')) $href = BASE_URL . "/$href";
  return w\popHelp('', $title, $href);
  /*
  return  <<<EOF
<a href="$href" title="$title"><div></div></a>
EOF;
*/
}

// NOT USED
function imageButton($href, $title, $src) {
  global $rUrl;
  if (!strhas($href, 'http')) $href = BASE_URL . "/$href";
  if (!strhas($src, 'http')) $src = "$rUrl/images/icons/$src";
  return w\popHelp("<img src=\"$src\" />", $title, $href);
/*  return  <<<EOF
<a href="$href" title="$title"><img src="$src" border="0" /></a>
EOF; */
}

/**
 * Return a form element consisting of a button to redirect to a relative or absolute URL.
 * @param string $label: button label
 * @param string $goto: where to go when the button is pressed
 * @param string $title: what to say when hovering over the button
 * @return: the button html
 */
function goButton($label, $goto, $title) {
  $gotoHead = '';
  $name = str_replace('/', '_', $goto);
  return array($name => fld('item', '', '', button($label, "$gotoHead/$goto", $title)));
}

/**
 * Go to the button's destination if the permission requirement is met.
 * @param mixed $perms: single permission number or array of permission numbers
 * Remaining params are as for goButton().
 */
function goButtonIf($perms, $label, $goto, $title) {
  global $mya;
  if (!is_array($perms)) $perms = array($perms);
  foreach ($perms as $perm) if (!$mya->can($perm)) return [];
  return goButton($label, $goto, $title);
}

/**
 * Create relation for company relationship specified in registration form (if any)  
 * @return TRUE if there is a formal relationship
 */
function suCompanyRelation($other, $data) {
  extract(just('company companyPhone owner employee contractor', $data, NULL));
  if (($owner or $employee) and $main = findCompany($company, $companyPhone)) { // company account exists
    return r\acct($main)->newRelation(compact(ray('other employee owner'))); // no permissions/draw (for security)
  } else return FALSE;
}

/**
 * Set up relationship of this new account with the account that created it (if any).
 */
function suCreatorRelation($helper, $args, $myid) {
  extract($params = just('owner employee flow', $args));
  if (!nn($owner) and !nn($employee) and !nn($flow)) return; // no useful relation (for example by true invitation)

  $draw = ($flow & 2) ? 1 : 0;
  r\acct($myid)->newRelation(ray('other permission employee owner draw', $helper, r\perm(B_MANAGE), $employee, $owner, $draw));
  if ($flow & 1) r\acct($helper)->newRelation(ray('other draw', $myid, TRUE));
//  } else r\tellAdmin("bad params in formSignup: $error -- ", $params);
} 

/**
 * Check email format and duplication and complain if appropriate.
 * @param string $email: the email address to check (also RETURNED with improvements)
 * @param bool $dupsOk: <allow duplication of another account's email address>
 * @param acct $a: current account if any
 * @return: TRUE if email is okay to use, else FALSE
 */
function emailOkay(&$email, $dupsOk, $a = FALSE, $suppressErr = FALSE) {
  $email = mb_strtolower($email);
  if (!u\validEmail($email)) return $suppressErr ? FALSE : say('bad email', compact('email'), 'email');
  if (!$dupsOk and emailDup($email, $a ? $a->id : 0, $suppressErr)) return FALSE;
  return TRUE;
}

/**
 * Say whether the email is a duplicate and complain if it is.
 * NOTE: Companies with duplicate email addresses are okay, so don't call this for companies.
 * @param string $email: the email address to test
 * @param int $uid: the account it is for (0 if the account is not created yet)
 * @return <email is a dup>
 */
function emailDup($email, $uid, $suppressErr = FALSE) {
  global $base_url;
  $email = mb_strtolower($email);
  $crypt = u\cry('P', $email);
  if (!$who = db\get('fullName', 'users', 'email=:crypt and uid<>:uid AND NOT :IS_CO', compact('crypt', 'uid'))) return FALSE;
  if (!$suppressErr) {
    $_a = atag("/settings/password/$email");
    $emailTagged = str_replace('@', '+whatever@', $email);
    $dupMsg = 'duplicate email';
    if (mb_stripos($email, '@gmail.com')) $dupMsg .= '|email plus';
    if (!$uid) $forgot = '|forgot password';
    say($dupMsg . nn($forgot), compact(ray('emailTagged who _a')), 'email');
  }
  return TRUE;
}

/**
 * Tell the staff about a new member.
 */
function tellStaffNewMember($params) {
  extract($params);
  $phone = u\fmtPhone(nn($phone));
  $state = nn($state) ? r\realState($state) : 'state unknown';
  $acctType = isset($acctype) ? ray(CO_TYPES)[$acctType] : NULL;
  $helper = nn($helper) ? r\acct($helper)->fullName : 'unknown';
  $copts = u\jsonEncode(nn($copts));
  $ats = (isset($ats) and is_array($ats)) ? join(',', array_values($ats)) : '';
  $params = compact(ray('phone state acctType helper copts ats')) + $params;
  unset($params['code']); // don't need to see this
  $message = '<h2>NEW SIGNUP</h2>';
  r\tellCAdmin($message, $params);
}

/**
 * Return a record ID for the given company or company phone.
 * @return record ID (FALSE if not found)
 */
function findCompany($company, $companyPhone) {
  if (!$company and !$companyPhone) return FALSE;
  $companyShortName = u\shortName($company);
  $companyUid = db\get('uid', 'users', ':IS_CO AND phone=:companyPhone', compact('companyPhone'));
  if (!$companyUid) $companyUid = db\get('uid', 'users', ':IS_CO AND name=:companyShortName', compact('companyShortName'));
  if (!$companyUid and mb_strlen($companyShortName) > 8 ) {
    $companyShortName .= '%';
    $companyUid = db\get('uid', 'users', ':IS_CO AND name LIKE :companyShortName', compact('companyShortName'));
  }
  return $companyUid;
}

/**
 * Validate and return tenure data
 * @param int $years: number of years at present address
 * @param int $months: number of additional months at present address
 * @param string $err: (RETURNED) error message, if any
 * @return the number of months the member lived at their current address as of their joining date (NULL if error)
 */
function tenure($years, $months, &$err = '') {
  global $mya;
  if (!$years and !$months) {$err = t('You must say how long you\'ve been there.'); return NULL;}
  foreach (ray('years months') as $k) {
    if (empty($$k)) $$k = 0;
    if ($err = u\badAmount($$k, '>=0')) {$err = t('How-long: ') . tr($err); return NULL;}
  }
  $tenure = $years * 12 + $months ;
  if ($mya and $mya->activated) $tenure -= (now() - $mya->activated) / MONTH_SECS; // tenure is at time of joining
  return $tenure;
} 

/**
 * Return form fields for name and email.
 * @param int $co: 1=company, 0 (or '') = personal, -1=community
 * @param string $fullName: default full name
 * @param string $email: default email address
 * @param bool $hideEmail: <don't ask for email address>
 * @param int $myid: current account record ID, if any
 * @return the relevant form fields
 */
function nameAndEmailFlds($co, $fullName = '', $email = '', $hideEmail = FALSE, $myid = 0) {
  if ($co) {
    list ($what, $who) = $co < 0 ? [t('Community'), t('The community\'s')] : [t('Company'), t('The company\'s')];
  } else {
    $_proxyInfo = w\lnk('/help/proxies', t('more about proxies'), w\away());
    $_proxyInfo = "[$_proxyInfo]";
  //  $_proxyInfo = w\btn('/help/proxies', t('more about proxies'), 'default', 'xs', w\away());
    $personalMsg = $myid ? '' : t(' Your name and city will be visible to other members in your region who may wish to select you as their <b>proxy</b> in your community\'s democratic process. %proxyInfo', compact('_proxyInfo'));
    list ($what, $who) = [t('Full name'), t('Your')];
  }

  $fullName = textFld("$what:", ["$who " . t('full name'), $co ? t('dba desc') : (t('usename desc') . $personalMsg)], required(nn($fullName)));
  $legalName = textFld(t('Legal name:'), ["$who " . t('full legal name (if different)'), $who . t('legalname desc')]);
  if ($email and !emailOkay($email, FALSE, $myid ? r\acct($myid) : '')) $hideEmail = FALSE; // make sure they can fix it
  if (mb_strpos($email, '@' . EMAILX) and !test()) $hideEmail = FALSE; // handle placeholder for making joint accounts
  $email = $hideEmail ? hidFld(nn($email)) : emailFld(t('Email:'), [t('Email'), t('<b>Type carefully.</b> All emails from the system will be sent to this address. It will not be made public, but will be viewable by accounts you transact with (you can choose greater privacy by clicking "Advanced" on the Preferences step).')], required(nn($email)));
  jsx('legal-name');
  return compact(ray('fullName legalName email'));
}

/**
 * Return the part of the form that is (mostly) common to formContact and formSignup
 * @param assoc $defaults: initial field values (defaults to current values) - see list at "$fields ="...
 *   plus {partner, partnerCode, customer} for signup from partner site
 */
function contactFields($defaults = []) {
  global $mya;

  $step0 = !$mya->stepIsDone('contact');
  $partnerExtras = ' partner customer partnerCode'; // add-on fields passed through only for partner signup (start with space) (even partnerCode is necessary, in case form is redrawn on error)
  $dbFlds = 'phone address city state zip tenure owns postalAddr source';
  $fields = 'phone physical address city state zip tenure owns sameAddr2 address2 city2 state2 zip2 postalAddr hidState hidState2 other source';
  foreach ($defaults as $k => $v) if (is_null($v)) unset($defaults[$k]); // needed for joint account setup
  extract($defaults + (array) $mya->account($dbFlds));

  if (nn($postalAddr)) list ($address2, $city2, $state2, $zip2) = u\parseAddr(nn($postalAddr));
  if (!nn($state2)) $state2 = 0; // otherwise default fails in countrys.js

  foreach (['state', 'state2'] as $k) {
    if (nn($$k)) $$k = db\get('id', 'r_states', ":$k IN (id, name, abbreviation) and country_id=:R_COUNTRY_ID", compact($k));
  }
  js('states'); // required for setPostalAddr, whether or not the fields are shown

  if ($step0 and nn($address) and nn($city) and nn($state) and !u\badZip($zip) 
    and !u\poAddress($address) and u\fmtPhone($phone)
    and nn($address2) and nn($city2) and nn($state2) and !u\badZip($zip2)) { // already got a bunch of data

    $postalAddr = ''; // get it from the separate fields
    $hidState = $state + 0;
    $hidState2 = $state2 ?: $hidState;
    if (nn($partner) and nn($partnerCode) and nn($customer) // customer signup 
      and ($partnerA = r\acct($partner) and $partnerA->emailCode == $partnerCode)) { // using valid data from partner org
      $fields .= $partnerExtras;
      $hideNot = 'tenure owns'; // DO ask about these
    } else $hideNot = u\badAmount($tenure, '>=0') ? 'tenure' : ''; // otherwise this is a joint Account setup or error, so DON'T ask about tenure and owns (unless tenure error)

    $fieldsToHide = justNOT($hideNot, ray($fields));
    foreach ($fieldsToHide as $k) $$k = hidFld(nn($$k));
    
  } else {
    $fieldsToHide = [];
    $state = nn($state) + 0;
    $hidState = hidFld($state + 0); // remember this when Drupal forgets it on error
    $hidState2 = hidFld(nn($state2) + 0);

  //  js('http://ziplookup.googlecode.com/git/public/ziplookup/zipLookup.min.js', 'file', 'header');
//    js('zipLookup.js', 'file', 'footer');
    jsx('addr');
    $phone = phoneFld(REQ . t('Phone:'), [t('Your phone number')], dft($phone));
//    $faxetc = @$faxetc ? textFld(t('Fax etc:'), [t('Other contact info'),t('List other ways to contact you: fax, mobile, IM, etc. This field is NOT public.')], dft(@$faxetc)) : NULL;
//    $country = selectFld(t('Country:'), t(''), dft(@$country)); // can't use required() here
    $physical = item('', t('Physical Address'));
    $address = textFld(t('Street Address:'), [t('Physical address'), t('Where is your building located (physical address)?')], required(nn($address)));
    $city = textFld(t('City:'), [t('City where you live')], required(nn($city)));
    $state = stateFld(nn($state)); // can't use required() here
    $zip = zipFld(nn($zip));
    
    $sameAddr2 = ($step0 ? nn($state2) : !(nn($mya) and $mya->cAdmin)) // show box on signup (until mailing addr filled) & to admin
    ? item('', t('Mailing Address')) 
    : w\boxFld('sameAddr', t('Mailing Address'), t('same as above'), 0);
    
    $postalAddr = hidFld();
    $address2 = textFld(t('Street Address:'), [t('Postal street address')], required(nn($address2)));
    $city2 = textFld(t('City:'), [t('Postal city')], required(nn($city2)));
    $state2 = stateFld(nn($state2));
    $zip2 = textFld(t('Postal Code:'), [t('Postal code'), ''], required(nn($zip2)));
    
    $other = $step0 ? item('', t('Other Information')) : NULL;
    if ((!nn($mya) or !$mya->co) and !nn($source)) {
      $source = $step0 ? sourceFld() : NULL;
    } else $source = hidFld(nn($source));
  }

  if (!in('tenure', $fieldsToHide) and $step0) {
    $years = intFld(BARE, [t('Years')], suffix(t('&nbsp years + ')));
    $months = intFld(BARE, [t('Months')], dft(nn($tenure)) + suffix(t('&nbsp months')));
    $tenure = fieldSet('tenure2', compact('years', 'months'), t('How long:'), t('How long have you been at your current address (roughly)'));
  } else {$tenure = NULL;}

  $owns = in('owns', $fieldsToHide) ? NULL : boolFld('', '', $step0 ? nn($owns) : $mya->owns, [t('Rent'), t('Own')]); // default fails

  return compact(ray($fields));
}  

function stateFld($dft) {return selectFld(REQ . t('State:'), t(''), dft($dft), ['' => '--'] + db\keyed('id,name', 'r_states', 'country_id=:R_COUNTRY_ID'));}
function sourceFld($dft = NULL) {return textFld(t('Referred By:'), [t('How you heard about %PROJECT'), t('Where did you hear about %PROJECT? (name of company or person, web address, publication name, radio station name, or whatever)')], dft(nn($dft)));}
function zipFld($zip = NULL) {return textFld(t('Postal Code:'), [t('Physical location postal code'), t('Postal code for your <em>physical location</em>')], required(nn($zip)));}
function sellingFld($selling = NULL) {return areaFld(t('Selling:'), [t('Products'),t('Enter one or more <b>VERY SHORT</b> transaction descriptions (for example "food") for when you make a sale &mdash; one line for each. Put the most common one first. This will appear in the description of the transaction for both you and the customer (for example "$20 for <i>food</i> on %dt")', 'dt', u\fmtDate(now(), TRUE))], required($selling));}
function routingFld($req = '') {return intFld(REQ . t('Routing:'), t('Type carefully your bank\'s routing number. (check to be sure it\'s correct!)'), vrange(10000000, 999999999));}
function crumbsFld($dft, $req = FALSE) {return numFld($req . t('Crumbs:'), [t('Contribution Percentage'), t('Percentage to contribute on each payment you receive')], dft(($dft * 100) ?: '') + step(.1) + suffix(' %'));}

/**
 * Standardize the "selling" field input.
 * @param string $selling: (MODIFIED) EOL-delimited list of short descriptions of what the company sells
 * @return and error message or FALSE if there is no error
 */
function zBadSelling(&$selling) {  
  $selling = explode("\n", $selling);
  foreach ($selling as $one) {
    $one = trim($one); // don't allow empty or reserved values
    if (strlen($one) > SELLING_MAXLEN) return t('Each short description of something you are selling is limited to %SELLING_MAXLEN characters.');
    if (!u\nonish($one) and !in($one, ['', S_REFUND, S_USD_OUT, t('cash')]) and !r\usdin($one)) $new[] = mb_strtolower($one);
  }
  if (!nn($new)) return t('missing field', 'field', 'selling');
  $selling = join("\n", $new);
  return FALSE;
}

/**
 * Interpret cookie to get source of new member and record it in the signup table.
 * @param string $source2: where the partner or cookie says the new member came to us from, if known
 */
function getPreId($source2 = NULL) {
  if (!$preid = nni($_COOKIE, 'preid')) {
    $preid = now() . u\randomString(9, 'digits');
    setcookie('preid', $preid, now() + 100 * YEAR_SECS, '/', $_SERVER['HTTP_HOST']);
  } else $preid = substr(preg_replace('/\\D*/', '', $preid), 0, 20); // ignore all but the digits

  if ($oldPreid = db\get('preid', 'signup', compact('preid'))) {
    $preid = $oldPreid;
  } else {db\insert('signup', ray('preid source ip', $preid, substr($source2, 0, 255), $_SERVER['REMOTE_ADDR']), 'preid');}  
  return $preid;
}
  
/**
 * Return a list of company categories.
 */
function coCats() {
  $sql = <<<EOF
    SELECT iid, IF(parent IS NULL, UCASE(industry), CONCAT('--', industry)) AS cat, parent 
    FROM r_industries ORDER BY IFNULL(parent, 0), iid
EOF;
  return db\q($sql)->fetchAllKeyed();
}
  
/**
 * Return the fields used to verify an individual's SSN or company's EIN (also asked for in signup).
 * @param bool $personal: is the account personal (not a company)
 * @param char $req: REQ if required, else ''
 */
function ssnFields($personal = TRUE, $req = '') {
  if ($personal) {
    $ssReason = $req ? t('Required by the IRS if you accept more than $20,000 a year in payments through %PROJECT ($600 a year in MA and VT).') : '';
    $federalId = passFld($req . t('ssTitle'), [t('ssDesc'), $ssReason], autofill('off'));
    $dob = dateFld($req . t('Birth Date:'), [t('Your birth date'), t('Your date of birth (mm/dd/yyyy) &mdash; must match the date on your ID.')], autofill('off'));
  } else {
    $federalId = passFld($req . t('einTitle'), [t('einDesc')], autofill('off'));
    $dob = dateFld($req . t('Founded:'), [t('Founding date'), t('When did this company begin, approximately (mm/dd/yyyy)?')]);
  }
  
  return compact(ray('federalId dob'));
}

function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}

function adminField($label) {
  global $mya;
  return ($mya->cAdmin and $mya->federalId) ? item(R_ON_FILE, $label) : NULL;
}

function onFile($field, $sta) {
  return (nn($sta['complete form'][$field]['#value']) == R_ON_FILE and nni($sta['input'], $field) == R_ON_FILE);
}

function dropdown($name, $value, $options, $id) {
  $name = "$name-$id";
  $zot = array('zot' => selectFld('', '', compact('value', 'name'), $options));
  return \render($zot);
}

function amountField($name, $value, $id) {
  $name = "$name-$id";
  $class = 'amount'; $class = compact('class');
  $zot = array('zot' => textFld('', '', compact(ray('value name class'))));
  return \render($zot);
}

function toggle($field, $value, $id, $highlight = 'never') {
  $name = $id = "$field-$id";
  $zot = w\togFld($id, '', '', $value);
  return \render($zot);
}
  
/**
 * Return the recipient of a recent invitation OR mark an invitation USED
 * Call by:
 *   $email = invitation($code, '', $inviter, $iCode, $err) returned with $inviter=inviter uid
 *   invitation($code, $invitee) marks the invitation USED by recording the invitee's account record ID
 * @param string $code: the invitation code
 * @param int $invitee: account record ID of the invited person
 * @param int $inviter: (RETURNED, if invitee='') account record ID of the inviter (returned even if error)
 * @param int $iCode: (RETURNED) the invitation sequence number
 * @param int $err: (RETURNED) error message, if any
 * @return:
 *   (1) encrypted email, if invited specifically with our form
 *   (2) TRUE if invited with an emailable link or marking invitation USED
 *   (3) FALSE if no such invitation or marking invitation USED failed
 */
function invitation($code, $invitee = '', &$inviter = '', &$iCode = -1, &$err = '') {
  if (!$invitee and $a = r\iCardAcct($code, $iCode)) {
    if ($iCode >= IBY_ICARD and db\exists('users', 'helper=:id AND iCode=:iCode', ray('id iCode', $a->id, $iCode))) return !$err = 'used invite';
    $inviter = $a->id;
    return TRUE;
  } else {
    $fields = 'id,email,inviter,invitee,invited';
    if (!$result = db\get($fields, 'r_invites', 'code=:code', compact('code'))) return !$err = 'bad invite';
    $id = $result['id']; // don't extract yet
    if (nn($invitee)) return db\update('r_invites', compact('id', 'invitee'), 'id'); // mark it used
    
    $iCode = IBY_FORM;
    extract($result); // looking for email and inviter, so get details
    if ($invitee) return !$err = 'used invite';
    $lastweek = strtotime((R_INVITE_DAYS + 2) . ' days ago'); // +2 for leniance
//    if ($invited < $lastweek) return !$err = 'expired invite';
    if ($invited < $lastweek) {
      $iCode = IBY_LATE; // convert to "unconfirmed" email invitation
      $inviterName = r\acct($inviter)->fullName;
      w\say('expired invite converted', compact('inviterName'));
    }
    return u\decry('P', $email); // $inviter is returned implicitly
  }
}

/**
 * Check if form was called with a QID (typically from an email link). Return the appropriate account.
 * @param assoc $sta: the form's status parameter, possibly with an eLink parameter (for verify and submit)
 *   (RETURNED) with $sta['eLink'] equal to just the qid and ecode part of eLink, plus the allow param
 * @param string $eLink: parameters from URL
 *   (RETURNED) same as $sta['eLink']
 * @param string $allow: what not-signed-in accounts to allow: individual (the default), 'acct', or 'any'
 * @return the account (defaults to current account), FALSE if error
 */
function eLinkAcct(&$sta, &$eLink = '', $allow = 'individual') {
  if (!$eLink) extract(just('eLink', $sta)); // verifying or submitting; get stored eLink args
  extract(just('qid ecode allow', $eLink));
  $a = r\acct($qid = nn($qid)); // current or specified account
  if ($allow != 'any') {
    if (!$a) return !w\go('', 'requires signin');
    if ($allow == 'individual' and $a->co) return !w\go('', t('That page is for individual accounts only.'));
    if ($qid and nn($ecode) != $a->emailCode) return !w\hack('bad ecode');
  }
  $eLink = $sta['eLink'] = "allow=$allow&qid=$qid&ecode=" . nn($ecode); // remember it (cttys don't use ecode)
  return $a;
}

function hack($message, $info = []) {
  r\tellAdmin(t('HACK attempt: ') . $message, $info + $_SERVER);
/**/	if (isDEV) debug("$message: " . pr($info));
  if (test()) return []; // to simulate a form
  return r\signedIn() ? signout() : w\go('');
}

function fundRpt($now, $then, $type = '') {
  //$up = $then ? number_format(abs($now / $then - 1) * 100, 1) : '100';
  $RUS = strhas($type, 'r') ? '@R' : '';
  $change = u\fmtAmt(abs($now - $then));
  $way = $now >= $then ? t('up') : t('down');
//  $s = u\fmtAmt($now) . t("$RUS &mdash; @way @change$RUS from a month ago", compact('way', 'change'));
  $s = u\fmtAmt($now) . t($RUS, compact('way', 'change'));
  return strhas($type, '$') ? $s : str_replace('$', '', $s);
}

function txsRpt($amt, $count, $accts, $type) {
  $type = $type == 'p' ? t('personal') : ($type == 'b' ? t('company') : '');
//  list ($totaling, $account, $month, $per, $avg) = array(t('totaling'), t('account'), t('mo'), t('per'), t('avg'));
  list ($acct, $month) = array(t('acct'), t('mo'));
  list ($amt, $perAcct) = array(u\fmtAmt($amt), u\fmtAmt($amt / ($accts ?: 1)));
  return "$count ($amt) / $month = $perAcct / $acct";
}

/**
 * Present simple and advanced versions of date range selection, for display with optional download.
 * @param assoc $args: parameters passed upon form submission, perhaps including date range selection
 * @param string $url: page to submit to (gets returned, prefixed with base_url and with parameters appended)
 * @param array $choices: choices for selection of states to show
 * @param string $settings: comma-delimited string array of indexes into choices for state settings
 * @param bool $downloadable: <show option to download data>
 * @return assoc [dateRange, url, starting, ending]
 *   dateRange: formatted simple/advanced paired date-selection fieldsets, with submission buttons and states
 *   url: page to submit to
 *   starting: (int) starting date
 *   ending: (int) ending date
 *   states: (array) options to display
 */
function dateRange($args, $url, $choices = [], $settings = '', $downloadable = TRUE) {
  global $txDays;
  extract(just('period starting ending states', $args));
  u\setDft($states, $settings);

  if ($choices) {
    if ($states) $stateDfts[] = ray($states);
    $states2 = boxesFld(t('Show:'), '', nn($stateDfts), $choices);
  } else $states2 = NULL;
  $showingAdv = ($states != $settings or (nn($ending) and $ending != strtotime('tomorrow') - 1));
  jsx('advanced-dates', compact('showingAdv')); // showing advanced selection
  u\setDft($period, strpos($url, 'invoice') ? TX_DEFAULT_INV_PERIOD : TX_DEFAULT_PERIOD);
  list ($starting, $ending) = rangeDates(nn($period), nn($starting), nn($ending));
  $url = BASE_URL . "/$url/period=$period&starting=$starting&ending=$ending&states=$states";

  $showAdvanced2 = '<a id="showAdvanced">advanced</a>';
  $submitPeriod = submi(t('Show')); // label has to be different from submitDates (see below)
  $downloadPeriod = $downloadable ? submi(t('Download')) : NULL;
///  $actions = render($submitPeriod) . render(@$downloadPeriod) . render($showAdvanced);
  $actions = fieldSet('actionsSimple', compact('submitPeriod', 'downloadPeriod'), '', '', suffix($showAdvanced2));
///  $period = selectFld(t('For the past:'), '', dft($period), $txDays);
//  $onchange = "$('#edit-starting, #edit-ending').val('');";
//  $onchange = "var id='#edit-submitPeriod'; if (!$(id).is(':visible')) id='#edit-downloadPeriod'; $(id).click();";
//  $period = selectFld(t('For the past:'), '', dft($period) + w\onchange($onchange), $txDays);
  $period = selectFld(t('For the past:'), '', dft($period), $txDays);
//  $period = selectFld(t('For the past:'), '', dft($period), $txDays);
  $simple = fieldSet('simple', compact('period', 'actions'), BARE);
  
  $res = compact(ray('url starting ending states')); // the easy parts of the result
  $submitDates = submi(t('show')); // label has to be different (Drupal bug -- else can't tell which is clicked)
//  $showSimple2 = '<a id="showSimple" href="javascript:void(0);" onclick="jQuery(\'#advanced\').hide(); jQuery(\'#simple\').show();">simpler</a>';
  $showSimple2 = '<a id="showSimple">simpler</a>';
  $downloadDates = $downloadable ? submi(t('download')) : NULL; // must be lowercase
  $actions = fieldSet('actionsAdvanced', compact('submitDates', 'downloadDates'), '', '', suffix($showSimple2));
  $starting = dateFld(t('Starting:'), '', dft($starting));
  $ending = dateFld(t('Ending:'), '', dft($ending));
  $downloadMsg = $downloadable ? item(t('<b class="loud">TIP</b>: Tell your browser to allow popups for this site.')) : NULL;
  $advanced = fieldSet('advanced', compact(ray('starting ending states2 actions')), BARE);
  $dateRange = compact(ray('simple advanced downloadMsg'));
  return compact('dateRange') + $res;
}

/**
 * Validate the chosen date range.
 * @param bool $limit: <limit the range to 180 days or less>
 */
function dateRangeValidate(&$sta, $limit = TRUE) {
  extract(just('period starting ending download', $sta['input'], ''));
  $op = op($sta);
  $downloadOp = ($op == 'submitPeriod' or $op == 'downloadPeriod');
  list ($starting, $ending, $period) = $downloadOp ? rangeDates($period) : rangeDates('', $starting, $ending);
  if ($limit and $ending - $starting > TX_MAX_DAYS * DAY_SECS and !$downloadOp) return w\say('too many days', 'err');
  u\preray(compact(ray('starting ending period')), $sta['input']);
}

/**
 * Initiate a download, if appropriate.
 */
function downloadCheck($page) {
  global $base_url;
  $nm = "$page-downloadQuery";
  if ($query = w\svar($nm)) jsx('download', 'url', "$base_url/$page/$query");
  w\svar($nm, '');
}

/**
 * Handle the results of the date range dialog.
 * @param string $page: what page we're coming from and going to
 * @param assoc $sta: form status array, for finding out whether user pressed Download and building redirect url
 * @param assoc $sta: (MODIFIED) with $sta['redirect'] set unless downloading
 */
function dateRangeRedirect($page, &$sta) {
//  if (confirming_s($sta)) return;

  $sta['input']['states'] = join(',', nni($sta['input'], 'states', []));
  $query = http_build_query(just('states period starting ending', $sta['input']));
  if (u\starts(op($sta), 'download')) {
    w\svar("$page-downloadQuery", $query);
    say('downloading');
  } else return w\go("$page/$query");
//  } else $sta['redirect'] = "$page/$query";
}

/**
 * Return usable starting and ending dates (inclusive).
 * @param int $period: index into TX_DAYS -- for calculating starting and ending if none explicitly specified
 * @param int $starting: explicit starting date (optional)
 * @param int $ending: explicit ending date (optional)
 * @return array($starting, $ending, $period), where $starting is adjusted to start of day and $ending to end of day
 */
function rangeDates($period, $starting = '', $ending = '') {
  global $txDays;
  if ($starting and !is_numeric($starting) and $err = u\badDate($starting)) say($err, 'starting');
  if ($starting > now()) say('beyond today', 'starting');
  if ($ending and !is_numeric($ending) and $err = u\badDate($ending)) say($err, 'ending');
  if (!$ending or $ending > now()) $ending = now();

  $period = (int) $starting ? ($ending - $starting) / DAY_SECS : ($period ?: TX_DEFAULT_PERIOD);
  $ago = $period == '1' ? '1 day' : (nni($txDays, $period) ?: ($period . ' days'));
  if (!$starting) $starting = strpos($ago, 'month') ?
    u\plusMonths(-preg_replace('/ ?months?/', '', $ago))
  : strtotime($period == '-1' ? '1jan' : ($period == '-2' ? '1/1/2013' : "$ago ago"));
  return array(strtotime('today', $starting), strtotime('tomorrow', $ending) - 1, $period);
}  

/**
 * Figure out who the user means, offering choices to disambiguate
 * If the intended person or company does not exist, 
 * @param string $who: what the user typed
 * @param string $field_name: name of the field the user typed in
 * @param array $info: field values to be stored, then reinstated when the page is refreshed to show the choices
 * @param string $selfMsg: index to error message to give if user self-refers
 * @param bool $paying: <we're preparing to pay this person> (in which case the person has to have an active account)
 * @return:
 *   NULL if there is an error
 *   acct of the identified person
 *   no return (refreshes page) if ambiguous
 */
function whois($who, $field_name, $info, $selfMsg = 'no self-trading', $paying = FALSE) {
  global $mya;
  if (u\isAcct($result = be\identify($who, $mya, $paying, $selfMsg))) return $result;
  list ($msg, $args, $choices) = $result;
//  foreach (ray('form_build_id form_token form_id op') as $one) unset($info[$one]); // just for neatness
  if (u\abbreviates('who=', $last = basename($return = \current_path()))) $return = dirname($return);
  $info['return'] = plain(str_replace('/', R_URL_SLASH, $return)); // even urlencoded slashes confuse Drupal
  foreach ($choices as $uid => $fullName) $choices[$uid] = r\qid($uid) . ': ' . $fullName;
  if (empty($choices)) return say($msg, $args, $field_name); else which($choices, $field_name, $info, t($msg, $args));
}
  
/**
 * Output the No-signin result message and terminate.
 * @param string $msg: the message or message index
 * @param string $err: 'err' or ''
 */
function doSay($msg = '', $err = '') {
  if (isDEV) file_put_contents(f('t.dosayFilename', $err ? 'error' : 'status'), $msg); // no way to know if actually testing
  return w\go('empty/say=1', $msg, $err);
}

/**
 * Add an item to the menu structure (called when reinstalling or rebuilding menus).
 * @param string $path: URL path to this item (without the domain)
 * @param string $title: human-readable label for operation
 * @param int $type: menu item type (see hook_menu)
 * @param string $function: what function to call when item is activated (with or without namespace qualifiers)
 * @param string $args: space-delimited list of arguments to pass to the function
 * @param string $perms: space-delimited list of permission bit names (lowercase without B_ )
 * @return the menu item array as expected by Drupal for hook_menu
 */
function menu($path, $title, $type, $function, $args, $perms = '') {
  global $menuWeight; $menuWeight += !nn($menuWeight) ?: 1;

  foreach ($args = ray($args) as $k => $v) if (is_numeric($v)) $args[$k] = (int) $v;
  
  $adminPerms = (strihas($perms, 'admin'));
  $admin = (u\starts($path, 'sadmin') or $perms == 'dev' or $adminPerms);
  $smart = ($path == 'pos');

  $formName = ($args and !is_numeric($args[0])) ? $args[0] : $function;
  $alt = $smart ? 'rsmart/rsmart.inc' : ($admin ? 'admin/admin-forms.inc' : 'rweb/rweb.inc');
  $file = strtolower("../forms/$formName.inc");
  if (!file_exists(R_ROOT . "/rweb/$file")) $file = "../$alt";
  
  $item = array(
    'title' => $title,
    'type' => $type,
    'page callback' => ($smart ? 'CG\\Smart\\' : 'CG\\Web\\') . $function,
    'page arguments' => $args,
    'access callback' => $perms ? 'CG\\Web\\webAccess' : TRUE,
    'access arguments' => $perms ? array($perms) : NULL,
    'weight' => $menuWeight - 1,
    'menu_name' => 'main-menu', 
    'module' => $smart ? 'rsmart' : 'rweb',
    'file' => $file,
    'options' => ray('class', $title),
  );

  return $item;
}

/**
 * Return a help message for the page.
 * @param string $args: parameters to pass to rawHelp
 * @param assoc $args: (RETURNED) returned parameters (passed from rawHelp back to caller)
 */
function helpText($page, $tag = '', &$args = '') {
  include_once __DIR__ . '/rweb-help.inc';
  $page = strtolower($page);
  $help = $page == 'charge' ? strtr(rawHelp('pay', $tag, $args), ray('pay', 'charge')) : rawHelp($page, $tag, $args);
  if ($help) return u\SUBS($help, '%');
  return t('There is no written help available for this page. Give it a try! If you are still puzzled, please feel free to give us a call at %regPhone. Or send your question by <%a>email</a>.', 'regPhone _a', u\fmtPhone(r\regionfield('phone')), w\emailtag(NULL, t('help with %PROJECT Members Site, %page page', 'page', $page)));
}

/**
 * Complain about an input error if a required field is blank
 * @param array $fields: associative array of field names, with or without a prefix
 * @param string $prefix: UNUSED? optional field name prefix (add if missing, else remove from message)
 */
function blank_field($fields, $prefix = '') {
  foreach ($fields as $key => $value) {
    $prefixed = u\abbreviates($prefix, $key);
    $field = strtoupper($prefixed ? substr($key, strlen($prefix)) : $key);
    $actual_name = $prefixed ? $key : ($prefix . $key);
    if (trim($value) == '') {say('required field', compact('field'), $actual_name); return TRUE;}
  }
  return FALSE;
}

/**
 * Display a drupal message (error or not)
 * Possible syntaxes:
 *   say(NULL) [do nothing in this case]
 *   say('index', array(optional args), 'optional error field')
 *   say('index', 'error field')
 *   say(array(index, args), 'optional error field')
 *
 * Note: the optional error field prevents form submission only if the field exists (so use "ERR" only when form submission is not an issue)
 *
 * @return FALSE (transfer() depends on this)
 */
function say($index = NULL, $args = [], $errField = '') {
  global $mya, $fieldErr; // used mostly in tests (and in formTx)

  if (!$index) return;
///  if (is_array($index) and !isset($index[0])) debug($index);
  if (is_array($index)) list ($index, $args, $errField) = array($index[0], $index[1], $args); // error returned from a function
  if (!is_array($args)) list ($args, $errField) = array($errField, $args); // allow either order, for easy 2-param calls
  $hasTable = strhas($index, '<table');
  $message = $hasTable ? $index : tr($index, $args); 
  $fieldErr = ($errField and $errField != 'ERR');
  if ($fieldErr) {
    \form_set_error($errField, $message);
//    jsx('focus-on', 'field', $errField);
  } else \drupal_set_message($message, nn($errField) == 'ERR' ? 'error' : 'status', FALSE);
  $adminReport = ($mya and $mya->admin2 and mb_strlen($message) > 200);
  if (!$hasTable and !$adminReport) u\loga($fieldErr ? 'sayerr' : 'say', compact('message'));
  return FALSE;
}

/**
 * Return a data series value, with formatting.
 * @param string $f: the formatted value
 * @param numeric $add: an amount to "stack" under the returned value (but not the formatted value)
 * @return an assoc [v, f], where v is the value and f is what to display when hovering over it
 */
function vf($f, $add = 0) {
  $v = strtr($f, ['$' => '', ',' => '', '%' => '']);
  if (strpos($f, '%')) $v *= .01;
  $v += $add; // assume add is a percentage, if v is
  return compact('v', 'f');
}
 
define('DATA_POINTS', 18); // how many x-axis data points to show for each granularity (day, week, etc.)

/**
 * Return an iframe containing a chart
 * @param string $chart: the chart name
 * @param int $ctty: community account ID
 * @param bool $selectable: <show the account and community selectors> (default FALSE)
 */
function chartFrame($chart, $selectable = FALSE, $ctty = NULL) {
  global $base_url, $mya;
  u\setDft($ctty, $mya ? $mya->community : 0);
  $region = r\acct('!' . R_SERVER_ID)->fullName;
  list ($w, $h) = $selectable ? [600, 600]: [480, 365];
//  $url = isDEV ? "$base_url/rcredits/misc/chart.php?site=dev" : ("https://cg4.us/chart.php?site=" . str_replace('https://', '', $base_url));
  $url = (isDEV ? "$base_url/rcredits/misc" : "https://cg4.us") . "/chart.php?site=$base_url";
  return w\tag('iframe', '', ray('src width height', "$url&region=$region&ctty=0&chart=$chart&selectable=$selectable&t=" . now(), $w, $h)); // src must be first
}

function firstGraphDt($ctty) {  
  $dt = db\min('created', 'r_stats', 'ctty=:ctty', compact('ctty'));
  if ($ctty and $cttyA = r\acct($ctty)) $dt = max($dt, $cttyA->created);
  return max(R_LAUNCH, strtotime('today', $dt));
}

/**
 * Display charts representing the condition of the %PROJECT system in the given community.
 * @param int $ctty: account record ID of community to show graphs for (defaults to all communities)
 * @param string $period: grain of displayed data (days, weeks, months, or years)
 * @param bool $download: <download month data instead of showing graph>
 * @return: an assoc of chart data for the specified period
 * FOR DEV: http://localhost/cgmembers-frame/cgmembers/rcredits/misc/chart.php?ctty=0&chart=growth&site=dev
 */
function showCharts($ctty = 0, $period = 'months', $download = FALSE) {
  global $base_url;

  list ($in, $out, $net, $personal, $companies) = array(t('in'), t('out'), t('net'), t('personal'), t('companies'));
  $txTypes = ray('p2p p2b b2b b2p', t('Person-to-person'), t('Person-to-business (mostly by %PROJECT card)'), t('Business-to-business (paying suppliers)'), t('Business-to-person (including payroll)'));
  $today = today();

  $charts = ray('success growth funds velocity banking volume', 'success-metrics', 'growth', 'dollar-pool', 'circulation-velocity', 'bank-transfers', 'transaction-volume');
  foreach ($charts as $k => $zot) u\setDft(${$k . 'Data'}, []);
  $downData[] = ray('Companies Members Active Joining P2P P2B B2P B2B PctActive MemsPerCo TxsPerActive PayeesPerActive Basket Gifts Invites');
  
  for ($ago = DATA_POINTS; $ago >= 0; $ago--) {
    $dt = strtotime($period == 'quarters' ? (-3*$ago) . ' months' : "-$ago $period", $today);
    if ($dt < R_LAUNCH) continue;
    $stats = r\stats($ctty, $dt);
    extract(just('ctty created', $stats)); // careful with this (don't overwrite local vars)
    $stats30 = r\stats($ctty, strtotime('-30 days', $created));
    extract(w\fundStats($stats, FALSE, $stats30)); // format, originals in {var}0, difs in d_{var}
    $gifts0 =  max(0, ($patronage0 + $d_roundups0 + $d_crumbs0)) / 5; // normalize (/)
    $payees0 *= 50; // normalize (*)
    $success0 = $aAccts0 * $payees0 * $basket0 / 8000; // normalize (/)
    if (test()) $success0 *= 8000; // invisible when normalized, so don't
    
    if ($download) {
      $pctActive = $aAccts0 / (($bAccts0 + $pAccts0) ?: 1);
      $memsPerCo = $bAccts0 ? $pAccts0 / $bAccts0 : 0;
      $txsPerActive = $aAccts0 ? $txsCount0 / $aAccts0 : 0;
      $downData[] = [$bAccts0, $pAccts0, $aAccts0, $newbs0, $d_p2p0, $d_p2b0, $d_b2p0, $d_b2b0, $pctActive, $memsPerCo, $txsPerActive, $payees0, $basket0, $gifts0, $d_invites0];
    } else {
  //    $ray = ray('v f', $pAccts0 + $bAccts0, $bAccts);
  //    $growthData[] = [$created, vf($bAccts, $pAccts0), $pAccts0, $newbs0, $aAccts0, $conx0, $conxLocal0];  
      $successData[] = [$created, $success0, $aAccts0, $gifts0, $payees0, $basket0, $d_invites0];
      $growthData[] = [$created, vf($bAccts, $pAccts0), $pAccts0, $newbs0, $aAccts0];  
      $fundsData[] = [$created, vf($balsPos), vf($usd), vf(u\fmtAmt($balsPos0 - $d_txs0)), vf($topN), vf($botN), vf($floors), vf($balsNeg)]; // $bals === $usd, so not shown
      $velocityData[] = [$created, vf($cgRate, $localRate0), vf($localRate), vf($usdVolRate)];
      $bankingData[] = [$created, vf($d_usdIn), vf($d_usdOut), vf($d_cgIn), vf($d_cgOut)];

      $x2x = [];
      foreach ($txTypes as $k => $desc) {
        $v = max(1, ${"d_{$k}0"}); // to make the logarithmic scale look okay
        $cnt = ${"d_{$k}Count"};
        $cnt .= empty($cnt + 0) ? '' : (' @ ' . u\fmtAmt($v / $cnt) . ' avg');
        $f = ${"d_{$k}"} . " ($cnt) $desc"; // eg "$123 (7 @ $6.25 avg)"
        $x2x[] = compact('v', 'f');
      }
      array_unshift($x2x, $created);
      $volumeData[] = $x2x;
    }    
    // add a pie chart for p2p,... cash purchase (one for volume, one for count?)
    // also pie for circling/explicit savings/biz required nut/other (runny)
    // pie for admin showing channel? member activity (0 txs/mo, 1-5, etc.), where's the money, etc.

//    $dt = strtotime("+$delta days", $dtX = $dt);
//    if ($dt > $today and $dtX < $today) $dt = $today; // always show the latest
  }

  if ($download) return $downData;

  $success = number_format($success0, 2);
  $accts = "$pAccts members + $bAccts companies";
  $funds = $usd;
  $velocity = ($bals0 ? u\fmtAmt($txsRate0, '%') : 0) . t(' per mo.');
  $usd = $d_usd . t(' (net)');

  $avg = u\fmtAmt($d_txsCount0 ? $d_txs0 / $d_txsCount0 : 0);
  $txs = "$d_txsCount @ $avg avg";
  $topPct = $aAccts0 >= 100 ? '%' : '';

  $vs = compact(ray('success accts funds velocity usd txs topPct'));

  foreach($charts as $k => $help) {
    $one = "<div id=\"onechart\" class=\"onechart ct-chart ct-golden-section\"></div>";
//    if (count($charts) > 1) $one = "<a href=\"$base_url/help/$help/qid=$ctty\">$one</a>";
    $charts[$k] = "<div>$one</div>";
    $dataNames[] = $k . 'Data';
  }

  return compact('vs', $dataNames);
}


/**
 * Return an array of formatted amounts for the given statistics.
 * Used in graphs and table of community funds
 */
function fundStats($stats, $round = FALSE, $stats30 = NULL) {
  extract($stats);
//  if ($stats30) extract($stats30, EXTR_PREFIX_ALL, 'o');
//  list ($r0, $usd0) = array($r, $usd);
  if ($ctty) { // a community
    $cttyA = r\acct($ctty);
    $res['cttyName'] = $ctty == r\serverUid() ? t('Seedpack (Outliers)') : trim(strtr($cttyA->fullName, [t('Region')=>'', PROJECT=>'']));
  } else $res['cttyName'] = t('TOTAL');

  $intStatFlds = ray('pAccts bAccts newbs aAccts conx conxLocal'); // excludes fields ending in 'Count'
//  $intStatFlds = [ 'pAccts', 'bAccts', 'newbs', 'aAccts', 'conx', 'conxLocal' ]; // excludes fields ending in 'Count'

  // calculate monthly velocity with respect to total credit in circulation (balsPos not bals)
  foreach ($rateFlds = ray('cg usdVol txs local') as $z) {
    $stats["{$z}Rate"] = ($stats[$z] - $stats30[$z]) / (($balsPos + 0) ?: 1);
    $stats30["{$z}Rate"] = 0; // just to make d_ not die
  }
  foreach (array_keys($stats) as $k) {
    if (!in($k, 'id ctty created')) {
      if ($stats30) $stats["d_$k"] = $stats[$k] - $stats30[$k];
      $fmt = ($round or strpos($k, 'Count') or in_array($k, $intStatFlds)) ? '0' : 's$';
      if (strpos($k, 'Rate')) $fmt = '%';
//      if (strpos($k, 'Per')) $fmt = '';
      foreach ($stats30 ? [$k, "d_$k"] : [$k] as $fnm) {
        $res[$fnm . '0'] = $stats[$fnm] + 0; // not $$fnm (because that wouldn't catch rates)
        $res[$fnm] = u\fmtAmt($stats[$fnm], $fmt);
      }
    }
  }

  return $res;
}

/**
 * Return a form showing month choices.
 * @param string $title: page title
 * @param string $things: "statements" or "notices"
 * @param string $subtext: explanatory text
 * @param int $end: how many months from now to end (starts with account creation month)
 * @param string $target: link target ('' or _blank)
 */
function monthChoices($title, $things, $subtext = '', $end = -1, $target = '_blank') {
  global $mya, $base_url;
  $start = u\monthDay1($mya->created); // start of account's first month
  $thisMo = u\monthDay1(); // start of this month
  $end = $end ? u\plusMonths($end, $thisMo) : $thisMo;
  $endYear = date('Y', $end);
  $startYear = date('Y', $start);

  for ($year = $endYear; $year >= $startYear; $year--) {  
//    for ($month = $start; $month <= $; $month = u\plusMonths(-1, $month)) {
    $list[$year] = '';
    for ($month = strtotime('1/1/' . $year); 
         $month <= $end and date('Y', $month) == $year; 
         $month = u\plusMonths(1, $month)) {
      $mon = strftime('%b', $month);
      $list[$year] .= $month < $start ? '.<br>' : "<a href=\"$base_url/history/$things/$mon$year\" target=\"$target\">$mon</a><br>\n";
    }
  }
  
  if ($end >= $start) {
    $headers = $data = '';
    foreach ($list as $year => $v) {
      $headers .= "<th>$year</th>";
      $data .= "<td>$v</td>";
    }
    $list = "<table id=\"$things\">\n<tr>$headers</tr>\n<tr>$data</tr>\n</table>";
  } else $list = t('There are no %things available yet for this account.', 'things', mb_strtolower($title));
  
  $title = item($title);
  $subtext = nn($subtext) ? item($subtext) : NULL;
  $list = item($list);
  return compact(ray('title subtext list'));
}

/**
 * Create a menu entry (see rweb.module)
 */
function userMenuItem($router_path, $link_title) {
  $menu_name = 'user-menu';
  $link_path = $router_path;
  if (db\exists('{menu_links}', 'menu_name=:menu_name AND link_path=:link_path', compact('menu_name', 'link_path'))) return; // done
  $module = 'menu';
  $customized = TRUE;
  $depth = 1;
  $mlid = 0;
  $item = compact(ray('mlid menu_name router_path link_path link_title module customized depth'));
  menu_link_save($item);
}

/**
 * Consolidate or extract hidden fields.
 */
function hidFlds($ray) {return is_array($ray) ? hidFld(plain(serialize($ray))) : unserialize(unplain($ray));}

function focusOn($focusFieldName) {
  u\EXPECT(!empty($focusFieldName), 'focus on null');
  $focusFieldName = str_replace('_', '-', $focusFieldName);
  jsx('focus-on', 'field', $focusFieldName);
}

/**
 * Return TRUE if the current account has all the required permissions.
 */
function webAccess($perms = NULL) {
  global $mya;

  foreach (ray($perms) as $perm) {
    if ($not = (substr($perm, 0, 1) == '-')) $perm = substr($perm, 1);

    $pass =
      $perm == ANY ? TRUE // needed for submenus (see w\subMenu())
    : ($perm == 'dev' ? isDEV
    : ($perm == 'signedIn' ? (bool) $mya
    : ($perm == 'iclub' ? ($mya and $mya->iclub)
    : ($perm == 'superadmin' ? ($mya and $mya->superAdmin)
    : ($perm == 'superadminORdev' ? (($mya and $mya->superAdmin) or isDEV)
    : ($perm == 'prose' ? ($mya and $mya->proSe)
    : ($perm == 'investor' ? ($mya and ($mya->iclub or db\exists('r_stakes', 'uid=:id', ['id'=>$mya->id])))
    : ($perm == 'depends' ? ($mya and $mya->depends and $mya->proSe)
    : ($mya and $mya->can(u\consta('b', $perm))) ))))))));
    if ($not ? $pass : !$pass) return FALSE;
  }
  return TRUE;
}

function css($sheet, $weight = -99) {
  global $rUrl;
  $options = ['group' => CSS_THEME, 'preprocess' => FALSE, 'external' => TRUE] + weight($weight);
  $version = isPRODUCTION ? R_VERSION : now();
  \drupal_add_css("$rUrl/css/$sheet?$version", $options);
}

/**
 * Verify dollar amount choice (with "other amount" field).
 * @param assoc $sta: (MODIFIED) form status array, from form's verify function
 * @param numeric min: minimum amount
 * @return <success>
 */
function checkAmtChoice(&$sta, $min = 0) { 
  extract(just('amtChoice amount period', $sta['input']));
  if ($amtChoice != -1) $amount = $amtChoice;
  if (isset($period) and !$period and $amount) return say('missing field', array('field' => 'when'), 'period');
  if ($err = u\badAmount($amount, '>=0')) return say($err, 'amount');
  if ($amount < $min) return say(t('The amount must be at least $%min.', compact('min')), 'amount');
  $sta['input'] = compact(ray('amount')) + $sta['input'];
  return TRUE;
}

/**
 * Return standard fields for the bottom of any form used during account setup.
 * 
 */
function setupFoot($label = '', $extra = [], $noSkip = FALSE) {
  global $mya, $base_url;
  if ($mya and $remains = $mya->stepCount()) {
    if ($remains > 1) { // more than just this step
      $remains--; // don't count current step
      $skip = $noSkip ? '' : skipButton();
      $res['nextStep'] = submit($label ?: (t('Next ') . '&raquo;'), '', '', w\suffix($skip));
      $steps = $remains == 1 ? t('step') : t('steps');
      $res['progress'] = item(t('Just <b>%n</b> more %steps to go', 'n steps', $remains, $steps));
    } else $res['nextStep'] = submit(t('Finish'));
  } else $res['submit'] = submit($label, '', '', $extra);
  return $res;
}

/**
 * Return the text with a help popover and optional link.
 * @param string $text: text to hover over
 * @param string $help: help verbage
 * @param string $href: where to go on click (defaults to nowhere)
 * @param string $class: html class(es)
 */
function popHelp($text, $help, $href = '', $class = '') {
  $trigger = $href == '#' ? 'hover' : 'hover'; // click doesn't work :(
  list ($href, $tag) = ($href and $href != '#') ? [" href=\"$href\"", 'a'] : ['', 'span'];
  if ($class) $class = " class=\"$class\"";
  $text = ucfirst($text);
  return "<$tag$href$class type=\"button\" data-toggle=\"popover\" data-placement=\"bottom\" data-trigger=\"$trigger\" data-content=\"$help\" title=\"$text\">$text</$tag>";
}

/**
 * Check the amount, fix it in the input array, return any error.
 */
function amtErr($field, &$sta, $restriction = '', $fractionLimit = 2) {
  return u\badAmount($sta['input'][$field], $restriction, $fractionLimit);
}

function tags($tag, $content = '', $tribs = []) {
	$tribs = $tribs ? ' ' . u\tribs($tribs) : '';
	return "<$tag$tribs>$content</$tag>";
}

/**
 * Return an HTML tag with optional attributes, surrounding optional HTML.
 * @param string $tag: the tag
 * @param string $html: (optional) inner HTML
 * @param assoc $tribs: (optional) keyed-array of attributes
 * @return the resultant HTML
 */
function tag($tag, $html = '', $tribs = []) {
  $tribs = $tribs ? ' ' . u\tribs($tribs) : '';
  return "<$tag$tribs>$html</$tag>";
}
function tagN($tag, $html = '', $tribs = []) {return str_replace('#', '', tag($tag, $html, $tribs));}

/**
 * Return an a tag for a non-spinning button.
 * @param string $href: URL to go to on click ('' if the click is handled with JS)
 * @param string $style: button style (default, primary, success, error, warning)
 * @param strng $size: xs sm md lg
 * @param assoc/string $extras: array of attributes or id string
 */
function atagB($href, $style = 'primary', $size = 'xs', $extras = []) {
  if (is_string($extras)) $extras = ['id' => $extras];
  $class = nni($extras, 'class');
  $extras['class'] =  ($class ? "$class " : '') . "btn btn-$size btn-$style";
  return atag($href, $extras + ['role' => 'button']);
}

/**
 * Return an a tag.
 */
function atag($href, $extras = []) {
  if (is_string($extras)) $extras = ['id' => $extras];
  if (nni($extras, 'role') != 'button') $extras += notab();
  if (substr($href, 0, 1) == '/') $href = BASE_URL . $href;
// NO  if ($href[0] == '$') $href = 'javascript:' . substr($href, 1);
  if ($href and !u\starts($href, BASE_URL)) {
    if (substr($href, 0, 6) == 'mailto' or substr($href, 0, 4) == 'http') $extras += w\away();
  }
  return 'a ' . u\tribs(($href ? compact('href') : []) + $extras);
}
function emailtag($email = NULL, $subject = '', $body = '') {
  u\setDft($email, r\regionfield('email'));
  foreach (['subject', 'body'] as $k) if ($$k) $$k = "$k=" . urlencode($$k) . '&';
  return "a href=\"mailto:$email?$subject$body\" target=\"_blank\"";
}
function btn($href, $text, $style = '', $size = '', $extras = []) {
  return '<' . atagB($href, $style ?: 'primary', $size ?: 'xs', $extras) . ">$text</a>";
}
function lnk($href, $text, $extras = []) {return '<' . atag($href, $extras) . ">$text</a>";}
function doLnk($class, $text, $data = NULL, $extras = []) {return w\lnk('', $text, compact('class', 'data') + $extras);}
function doBtn($class, $text, $data = '') {return w\btn('', $text, 'primary', 'xs', compact('class', 'data'));}

/**
 * Skip a setup step and save it for last. Has no effect if step is already done.
 * If defaulting to the current step, redirect to the next step.
 * @param string $substitute: step to do instead (later)
 * @param string $justDid: step just completed
 */
function skipStep($substitute = NULL, $justDid = '') {
  global $mya;

  if (!$mya or !$stepsLeft = $mya->stepsLeft()) return; // nothing to do
  $step = $stepsLeft[0];
  if (in($step, S_OPTIONAL)) return w\go($mya->nextStepUrl($step, $msg), '');
    
  unset($stepsLeft[0]);
  if (!in($step, S_OPTIONAL)) $stepsLeft[] = $substitute ?: $step; // put this step at the end (except optional steps that can be done later or never)
  $mya->update('stepsLeft', array_values($stepsLeft)); // renumber and update
  $nextStep = $mya->nextStepUrl($justDid, $msg);
  if ($justDid and $msg) w\say($msg);
  return w\go($nextStep);
}

/**
 * Return account and password fields for sign-in.
 */
function signinFlds($acct, $acctDesc = '', $passDesc = '') {
  $acct = textFld(t('Account ID:'), [t('Your account ID'), $acctDesc], required($acct) + w\autofill('section-rSignin rAccount'));
  $pass = passFld(t('Password:'), [t('Your password'), $passDesc], required() + w\autofill('section-rSignin rPass'));
  return [$acct, $pass];
}

/**
 * Format info about the new member's company.
 * @param assoc $info: company info from when the member signed up
 * @param string $company: (RETURNED) the company name
 */
function signupCoDisplay($info, &$company = '') {
  if (!$info) return '';
  extract(just('owner employee company', $info, ''));
  return "$owner $employee: $company";
}

/**
 * Return a table as specified.
 * @param ray $rows: set of row data
 * @param ray $heads: column headers
 * @param string $colClasses: (optional) class of each column
 * @param ray $rowClasses: (optional) class of each row
 * @return HTML of the full table
 */
function makeTable($rows, $heads, $colClasses = [], $rowClasses = [], $tableTribs = 'id=adhoc') {
  u\EXPECT(!$colClasses or count($colClasses) == count($heads), 'wrong aligns count');
  if ($rows) u\EXPECT(count($rows[0]) == count($heads), 'wrong heads count');
  
  $heads = makeRow('th', $heads, $colClasses);
  if ($rows) {
    foreach ($rows as $i => $row) $lines[] = makeRow('td', $row, $colClasses, nni($rowClasses, $i));
    $after = '';
  } else $after = t('There are no such records.');
  
  $rows = join('', nn($lines, [])); // this works even if $rows is []

  return <<< X
<table $tableTribs>
$heads
$rows
</table>
$after
X;
}

function makeRow($tag, $row, $colClasses = [], $rowClass = '') {
  foreach ($row as $i => $k) {
    if ($class = nni($colClasses, $i)) $class = " class=\"$class\"";
    $line[$i] = "<$tag$class>$k</$tag>";
  }
  if ($rowClass) $rowClass = " class=\"$rowClass\"";
  return "<tr$rowClass>\n" . join("\n", $line) . '</tr>';
}

/**
 * Get text from submitted submit button.
 */
function opText($sta) {
  $op = strtolower('edit-' . op($sta));
  foreach ($sta['buttons'] as $k => $one) if (nni($one, '#id') == $op) return nni($one, '#value');
  return '';
}

/**
 * If signing up, go to the next step. Otherwise just display the message, if any.
 * @param string $step: current step
 * @param string $msg: message to display, if any
 * @param assoc $info: info to report to admin, if any
 */
function goNextStep($step, $msg = NULL, $info = []) {
  global $mya;
  $url = $mya ? $mya->nextStepUrl($step, $msg, $info) : 'signin';
  return w\go($url, $msg);
}

function backButton() {
  jsx('back-button'); 
  return '<button class="btn btn-back btn-xs btn-primary" tabindex="-1"><span>&laquo;</span>&nbsp;Go Back</button>';
}

// one-line functions that need no explanation

function disabled($value = NULL, $disabled = TRUE) {return compact('disabled') + (isset($value) ? compact('value') : []);}
function dft($v) {return ray('default_value', nn($v, NULL));}
function weight($weight) {return compact('weight');}
function clas($class) {return ['class' => [$class]];}
function suffix($field_suffix = '') {return compact('field_suffix');}
function prefix($prefix) {return compact('prefix');}
function maxlen($maxlength) {return compact('maxlength');}
function vmin($min) {return compact('min');}
function vmax($max) {return compact('max');}
function vrange($min, $max) {return compact('min', 'max');}
function step($step) {return compact('step');}
function autofill($autocomplete = 'on') {return compact('autocomplete');}
function required($dft = NULL) {return ['required' => TRUE] + (isset($dft) ? dft($dft) : []);}
function op($sta) {global $testOp; return test() ? $testOp : substr(nni($sta['input'], 'opid'), 5);} // without edit-
function verifyBy($byVoice) {return $byVoice ? t('Voice') : t('SMS');}
function sayFieldErr($err, $field) {return say(strtoupper($field) . ': ' . t($err), $field);}
function err($msg) {return say($msg, 'err');}
function rent($element) {return \render($element);} // Drupal makes render hard, requires pass by reference
function away() {return ['target' => '_blank'];}
function js($script, $args = '') {global $pageScripts; $pageScripts[$script] = http_build_query(rayy(func_get_args(), 1));} // see html.tpl
function jsx($script, $args = '') {global $scriptScraps; $scriptScraps[$script] = http_build_query(rayy(func_get_args(), 1));} // see html.tpl
function softErr($msg) {w\say($msg, 'err'); return w\go('empty');}
function clearHeaders() {if (!headers_sent()) header_remove();}
function skipButton() {return button(t('Skip for now'), '/settings/skip', t('Leave this step for last'), 'default', 'xs', ['class' => 'skip']);}
function notab() {return tabindex(-1);}
function tabindex($n) {return ['tabindex' => $n];}