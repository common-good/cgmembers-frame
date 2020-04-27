<?php

/**
 * @file
 * Handles integration of PHP templates with the Drupal theme system.
 * (renamed from themes/phptemplate/phptemplate.engine cg 2018-04-02)
 */

/**
 * Implements hook_init().
 */
function phptemplate_init($template) {
  $file = dirname($template->filename) . '/template.inc'; // cg changed from .php to .inc 2018-03-30
  include_once DRUPAL_ROOT . '/' . $file;
}

/**
 * Implements hook_theme().
 */
function phptemplate_theme($existing, $type, $theme, $path) {
  $templates = drupal_find_theme_functions($existing, array($theme));
  $templates += drupal_find_theme_templates($existing, '.tpl', $path); // cg changed .tpl.php to .tpl 2018-03-30
  return $templates;
}
