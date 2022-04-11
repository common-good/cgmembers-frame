<?php
use CG\Util as u;
use CG\Cron as cr;

/**
 * @file
 * Handles incoming requests to fire off regularly-scheduled tasks (cron jobs).
 * Run as a cron job:
 *
 */
define('DRUPAL_ROOT', __DIR__ . '/../..');
require_once __DIR__ . '/../bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL); // boot before including rcron.inc
require_once R_ROOT . '/rcron/rcron.inc';

if (variable_get('cron_key', 'drupal') != nni($_GET, 'cron_key')) {
  u\log(t('Cron could not run because an invalid key was used.'));
  \drupal_access_denied();
} elseif (variable_get('maintenance_mode', 0)) {
  u\log(t('Cron could not run because the site is in maintenance mode.'));
  \drupal_access_denied();
} else cr\run();
