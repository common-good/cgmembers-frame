<?php
use CG\Web as w;
use CG\Util as u;

global $pageTitle;

/**
 * @file
 * Default theme implementation to display the basic html structure of a single Drupal page.
 * called from the stub html.tpl.php in the theme's templates folder.
 *
 * Variables:
 * - $css: An array of CSS files for the current page.
 * - $language: (object) The language the site is being displayed in.
 *   $language->language contains its textual representation.
 *   $language->dir contains the language direction. It will either be 'ltr' or 'rtl'.
 * - $rdf_namespaces: All the RDF namespace prefixes used in the HTML document.
 * - $grddl_profile: A GRDDL profile allowing agents to extract the RDF data.
 * - $head_title: A modified version of the page title, for use in the TITLE
 *   tag.
 * - $head_title_array: (array) An associative array containing the string parts
 *   that were used to generate the $head_title variable, already prepared to be
 *   output as TITLE tag. The key/value pairs may contain one or more of the
 *   following, depending on conditions:
 *   - title: The title of the current page, if any.
 *   - name: The name of the site.
 *   - slogan: The slogan of the site, if any, and if there is no title.
 * - $head: Markup for the HEAD section (including meta tags, keyword tags, and
 *   so on).
 * - $styles: Style tags necessary to import all CSS files for the page.
 * - $scripts: Script tags necessary to load the JavaScript files and settings
 *   for the page.
 * - $page_top: Initial markup from any modules that have altered the
 *   page. This variable should always be output first, before all other dynamic
 *   content.
 * - $page: The rendered page content.
 * - $page_bottom: Final closing markup from any modules that have altered the
 *   page. This variable should always be output last, after all other dynamic
 *   content.
 * - $classes String of classes that can be used to style contextually through
 *   CSS.
 *
 * @see template_preprocess()
 * @see template_preprocess_html()
 * @see template_process()
 */
global $rUrl, $base_url, $pageScripts, $scriptScraps, $mya, $scriptNonce;
$version = isPRODUCTION ? R_VERSION : now();
$styles = preg_replace('~<style.*</style>~ms', '', $styles); // zap all the drupal styles

// handle scripts
if (isDEV and $jsCode = nni($_GET, 'jsCode')) w\jsx('eval', compact('jsCode')); // for testing ajax
if (nn($scriptScraps)) w\js('scraps', 'args', urlencode(u\jsonize($scriptScraps))); // fragments
$s = array_flip(ray(SCRIPTS_TOP)); // standard (included first on every page)
$s += just(array_keys($pageScripts), array_merge(array_flip(ray(SCRIPTS)), $pageScripts)); // select and reorder ad hoc scripts
  u\EXPECT(count($s) >= count(ray(SCRIPTS_TOP)) + count($pageScripts), 'scripts! ' . pr(justNOT(ray(SCRIPTS_TOP . ' ' . SCRIPTS), $pageScripts)) . pr($pageScripts));
$scripts = '';
$tm = now();
foreach ($s as $id => $v) { // having selected the scripts, format for inclusion in page
  $src = "$rUrl/js/$id.js";
  if (!strpos($src, 'x/') or $v) {
    if (u\starts($id, 'goo-')) $src = 'https://www.google.com/' . substr($id, 4); else $src .= "?v=$tm&$v"; // $v is either a small integer (from array_flip) or all script arguments
    $idRay = ray('id', "script-$id");
  } else { $idRay = []; } // no id for 3rd-party scripts
  if (in($id, SCRIPT_MODULES)) $idRay['type'] = 'module';
  $nonceRay = ray('nonce', $styleNonce); // but external nonce fails in Edge as of 12/14/2017
  
  if (strpos($v, ';')) { // temporary for inline
    $scripts .= w\tags('script', $v, $idRay) . "\n";
  } else $scripts .= w\tags('script', '', compact('src') + $idRay + $nonceRay) . "\n";
}

w\sanitizePage($page); // assure no HTML insertion of script, styles, etc.
$page = str_replace('target="_blank"', 'target="_blank" rel="noreferrer noopener"', $page); // flout tabnabbing

if ($mya) $classes = str_replace('not-logged', 'logged', $classes);

$favicon = <<<EOF
  <link rel="apple-touch-icon" sizes="180x180" href="$rUrl/images/favicons/apple-touch-icon.png">
  <link rel="icon" type="image/png" sizes="32x32" href="$rUrl/images/favicons/favicon-32x32.png">
  <link rel="icon" type="image/png" sizes="16x16" href="$rUrl/images/favicons/favicon-16x16.png">
  <link rel="manifest" href="$rUrl/images/favicons/manifest.json?$version">
  <link rel="mask-icon" href="$rUrl/images/favicons/safari-pinned-tab.svg" color="#5bbad5">
  <link rel="shortcut icon" href="$rUrl/images/favicons/favicon.ico">
  <meta name="msapplication-TileColor" content="#da532c">
  <meta name="msapplication-config" content="$rUrl/images/favicons/browserconfig.xml?$version">
  <meta name="theme-color" content="#ffffff">
EOF;

/**/ echo <<<EOF
<!DOCTYPE html>
<html lang="$language->language" dir="$language->dir"
$rdf_namespaces>

<head profile="$grddl_profile">
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <!-- The above 3 meta tags *must* come first in the head -->

  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

$favicon
  <meta name="MobileOptimized" content="width" />
  <meta name="HandheldFriendly" content="true" />
  <meta name="apple-mobile-web-app-capable" content="no"><!-- (not yet) -->
  <meta http-equiv="cleartype" content="on" />
  <title>$pageTitle</title>
  <meta name="description" content="">
  <meta name="author" content="William Spademan -- for Society to Benefit Everyone, Inc.">
  <link rel="stylesheet" href="$rUrl/css/x/bootstrap.min.css?$version" />
  <link rel="stylesheet" href="$rUrl/css/x/jquery-ui.min.css?$version">
  <link rel="stylesheet" href="$rUrl/css/x/ladda-themeless.min.css?$version">
  $styles
  <link rel="stylesheet" href="$rUrl/css/cg.css?$version" type="text/css" />
  <link rel="stylesheet" href="$rUrl/css/rweb.css?$version" type="text/css" />
</head>

<body class="$classes" $attributes>
  $page_top
  $page
$scripts
</body>
</html>
EOF;
