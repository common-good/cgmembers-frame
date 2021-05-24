<?php

require_once(R_ROOT . "/rcredits.module");

use \CG\Util as u;
use \CG\Web as w;

include_once R_ROOT . '/cg-util.inc';

/**
 * @file
 * Functions for error handling.
 */

/**
 * Maps PHP error constants to watchdog severity levels.
 *
 * The error constants are documented at
 * http://php.net/manual/errorfunc.constants.php
 *
 * @ingroup logging_severity_levels
 */
function drupal_error_levels() {
  return [
    E_ERROR => ['Error', WATCHDOG_ERROR],
    E_WARNING => ['Warning', WATCHDOG_WARNING],
    E_PARSE => ['Parse error', WATCHDOG_ERROR],
    E_NOTICE => ['Notice', WATCHDOG_NOTICE],
    E_CORE_ERROR => ['Core error', WATCHDOG_ERROR],
    E_CORE_WARNING => ['Core warning', WATCHDOG_WARNING],
    E_COMPILE_ERROR => ['Compile error', WATCHDOG_ERROR],
    E_COMPILE_WARNING => ['Compile warning', WATCHDOG_WARNING],
    E_USER_ERROR => ['User error', WATCHDOG_ERROR],
    E_USER_WARNING => ['User warning', WATCHDOG_WARNING],
    E_USER_NOTICE => ['User notice', WATCHDOG_NOTICE],
    E_DEPRECATED => ['Deprecated function', WATCHDOG_DEPRECATED],
    E_USER_DEPRECATED => ['User deprecated function', WATCHDOG_USER_DEPRECATED],
    E_STRICT => ['Strict warning', WATCHDOG_DEBUG],
    E_RECOVERABLE_ERROR => ['Recoverable fatal error', WATCHDOG_ERROR],
  ];
}

/**
 * Handle unhandled exceptions like we handle errors.
 */
function handleException($e) {return handleError(E_EXCEPTION, $e->getMessage(), $e->getFile(), $e->getLine(), NULL, $e->getTrace());}

/**
 * Handle all errors, including warnings.
 * On production, all errors generate an email to admin, user sees a friendly apology on fatal error, otherwise nothing
 * On dev, all errors die with stack trace, except from PHP window where the error is displayed with just the caller
 * Treat recoverable errors as fatal.
 * @param $level: The level of the error raised.
 * @param $msg: The error message.
 * @param $file: The filename that the error was raised in.
 * @param $line: The line number the error was raised at.
 * @param $context: An array that points to the active symbol table at the point the error occurred.
 * @param $trace: Stack trace (only when called from handleException)
 */
function handleError($level, $msg, $file, $line, $context, $trace = NULL) {
  global $channel;
  list($severity_msg, $severity) = nni(drupal_error_levels(), $level, ['unknown error', WATCHDOG_ERROR]);

  if ($level == E_WARNING and substr($msg, 0, 6) == 'fopen(') return; // work around flaw in fopen
  if ($severity == WATCHDOG_DEBUG) return;
///  debug($context); echo pr($context); // handy when debugging

  $nasty = ($severity < WATCHDOG_WARNING);
  $title = $nasty ? t('CRASH!') : t('WARNING');
  $file = basename($file);
  $msg = t('<b>%title: </b>%msg<br> - line %line in %file' . "<br>\n", compact(ray('title msg line file')));
  u\loga($title, $msg);
  $devPhpPage = (!isPRODUCTION and $channel == TX_WEB and basename($_SERVER['REQUEST_URI']) == 'php');

  if (empty($trace)) $trace = debug_backtrace() ?: [];
  $traceMsg = [];
  foreach ($trace as $one) {
    if (!is_array($one) or count($traceMsg) > 20) break; // don't overflow memory
    if ($devPhpPage and count($traceMsg) > 2) break; // keep it short for PHP page on dev
    extract(just('line file function args', $one, ''), EXTR_PREFIX_ALL, 'x'); // sometimes just function and args

    $x_file = basename($x_file);
    $x_function = basename($x_function);
    $args = preg_replace('/Array\s+\(/sm', 'Array', pr($x_args));
    $args = substr($args, 6); // discard the gratuitous "Array" at the start
    $args = preg_replace('/$\s*\)$\s$/sm', '', $args);
    $args = '    ' . str_replace("\n", "<br>\n    ", $args);
    $args = str_replace('  ', ' &nbsp;', $args);
    $traceMsg[] = "- $x_line in $x_file, calling $x_function() with:<br>\n$args";
  }

  if (!function_exists('filter_xss_admin')) require_once DRUPAL_ROOT . '/includes/common.inc';
  $traceMsg = \filter_xss_admin($msg . join("<br>\n", $traceMsg));
  
  if (isPRODUCTION) {
    $msg = strtr($msg . $traceMsg, ['&nbsp;' => '', '<br>' => '', '&gt;' => '>']);
    mail(R_ADMIN_EMAIL, t('%PROJECT ' . $title), $msg);
    if (!$nasty) return;
    if ($channel == TX_WEB) return w\softErr(tr('crash msg'));
/**/die();
  } else {
    if ($devPhpPage) {
      w\say($nasty ? $traceMsg : $msg, 'err');
      return w\go('sadmin/php');
    }
/**/echo $traceMsg;
/**/die();
  }
      
}

/**
 * Decodes an exception and retrieves the correct caller.
 *
 * @param $exception
 *   The exception object that was thrown.
 *
 * @return
 *   An error in the format expected by _drupal_log_error().
 */
function _drupal_decode_exception($exception) {
  $message = $exception->getMessage();

  $backtrace = $exception->getTrace();
  // Add the line throwing the exception to the backtrace.
  array_unshift($backtrace, array('line' => $exception->getLine(), 'file' => $exception->getFile()));

  // For PDOException errors, we try to return the initial caller,
  // skipping internal functions of the database layer.
  if ($exception instanceof PDOException) {
    // The first element in the stack is the call, the second element gives us the caller.
    // We skip calls that occurred in one of the classes of the database layer
    // or in one of its global functions.
    $db_functions = array('db_query',  'db_query_range');
    while (!empty($backtrace[1]) && ($caller = $backtrace[1]) &&
        ((isset($caller['class']) && (strpos($caller['class'], 'Query') !== FALSE || strpos($caller['class'], 'Database') !== FALSE || strpos($caller['class'], 'PDO') !== FALSE)) ||
        in_array($caller['function'], $db_functions))) {
      // We remove that call.
      array_shift($backtrace);
    }
    if (isset($exception->query_string, $exception->args)) {
/**/  $message .= ": " . $exception->query_string . "; " . print_r($exception->args, TRUE);
    }
  }
  $caller = _drupal_get_last_caller($backtrace);

  return array(
    '%type' => get_class($exception),
    // The standard PHP exception handler considers that the exception message
    // is plain-text. We mimick this behavior here.
    '!message' => check_plain($message),
    '%function' => $caller['function'],
    '%file' => $caller['file'],
    '%line' => $caller['line'],
    'severity_level' => WATCHDOG_ERROR,
  );
}

/**
 * Renders an exception error message without further exceptions.
 *
 * @param $exception
 *   The exception object that was thrown.
 * @return
 *   An error message.
 */
function _drupal_render_exception_safe($exception) {
  return check_plain(strtr('%type: !message in %function (line %line of %file).', _drupal_decode_exception($exception)));
}

/**
 * Determines whether an error should be displayed.
 *
 * When in maintenance mode or when error_level is ERROR_REPORTING_DISPLAY_ALL,
 * all errors should be displayed. For ERROR_REPORTING_DISPLAY_SOME, $error
 * will be examined to determine if it should be displayed.
 *
 * @param $error
 *   Optional error to examine for ERROR_REPORTING_DISPLAY_SOME.
 *
 * @return
 *   TRUE if an error should be displayed.
 */
function error_displayable($error = NULL) {
  $error_level = variable_get('error_level', ERROR_REPORTING_DISPLAY_ALL);
  $updating = (defined('MAINTENANCE_MODE') && MAINTENANCE_MODE == 'update');
  $all_errors_displayed = ($error_level == ERROR_REPORTING_DISPLAY_ALL);
  $error_needs_display = ($error_level == ERROR_REPORTING_DISPLAY_SOME &&
    isset($error) && $error['%type'] != 'Notice' && $error['%type'] != 'Strict warning');

  return ($updating || $all_errors_displayed || $error_needs_display);
}

/**
 * Logs a PHP error or exception and displays an error page in fatal cases.
 *
 * @param $error
 *   An array with the following keys: %type, !message, %function, %file, %line
 *   and severity_level. All the parameters are plain-text, with the exception
 *   of !message, which needs to be a safe HTML string.
 * @param $fatal
 *   TRUE if the error is fatal.
 */
function _drupal_log_error($error, $fatal = FALSE) {
  // Initialize a maintenance theme if the bootstrap was not complete.
  // Do it early because drupal_set_message() triggers a drupal_theme_initialize().
  if ($fatal && (drupal_get_bootstrap_phase() != DRUPAL_BOOTSTRAP_FULL)) {
    unset($GLOBALS['theme']);
    if (!defined('MAINTENANCE_MODE')) {
      define('MAINTENANCE_MODE', 'error');
    }
    drupal_maintenance_theme();
  }

  // When running inside the testing framework, we relay the errors
  // to the tested site by the way of HTTP headers.
  $test_info = &$GLOBALS['drupal_test_info'];
  if (!empty($test_info['in_child_site']) && !headers_sent() && (!defined('SIMPLETEST_COLLECT_ERRORS') || SIMPLETEST_COLLECT_ERRORS)) {
    // $number does not use drupal_static as it should not be reset
    // as it uniquely identifies each PHP error.
    static $number = 0;
    $assertion = array(
      $error['!message'],
      $error['%type'],
      array(
        'function' => $error['%function'],
        'file' => $error['%file'],
        'line' => $error['%line'],
      ),
    );
    header('X-Drupal-Assertion-' . $number . ': ' . rawurlencode(serialize($assertion)));
    $number++;
  }

  watchdog('php', '%type: !message in %function (line %line of %file).', $error, $error['severity_level']);

  if ($fatal) {
    drupal_add_http_header('Status', '500 Service unavailable (with message)');
  }

  if (drupal_is_cli()) {
    if ($fatal) {
      // When called from CLI, simply output a plain text message.
      print html_entity_decode(strip_tags(t('%type: !message in %function (line %line of %file).', $error))). "\n";
      exit;
    }
  }

  if (isset($_SERVER['HTTP_X_REQUESTED_WITH']) && $_SERVER['HTTP_X_REQUESTED_WITH'] == 'XMLHttpRequest') {
    if ($fatal) {
      if (error_displayable($error)) {
        // When called from JavaScript, simply output the error message.
        print t('%type: !message in %function (line %line of %file).', $error);
      }
      exit;
    }
  }
  else {
    // Display the message if the current error reporting level allows this type
    // of message to be displayed, and unconditionnaly in update.php.
    if (error_displayable($error)) {
      $class = 'error';

      // If error type is 'User notice' then treat it as debug information
      // instead of an error message, see dd().
      if ($error['%type'] == 'User notice') {
        $error['%type'] = 'Debug';
        $class = 'status';
      }

      drupal_set_message(format_string('%type: !message in %function (line %line of %file).', $error), $class); // CGF
    }

    if ($fatal) {
      drupal_set_title(t('Error'));
      // We fallback to a maintenance page at this point, because the page generation
      // itself can generate errors.
/**/  print_r(debug_backtrace());
      print theme('maintenance_page', array('content' => t('The website encountered an unexpected error. Please try again later.')));
      exit;
    }
  }
}

/**
 * Gets the last caller from a backtrace.
 *
 * @param $backtrace
 *   A standard PHP backtrace.
 *
 * @return
 *   An associative array with keys 'file', 'line' and 'function'.
 */
function _drupal_get_last_caller($backtrace) {
  // Errors that occur inside PHP internal functions do not generate
  // information about file and line. Ignore black listed functions.
  $blacklist = array('debug', '_drupal_error_handler', '_drupal_exception_handler');
  while (($backtrace && !isset($backtrace[0]['line'])) ||
         (isset($backtrace[1]['function']) && in_array($backtrace[1]['function'], $blacklist))) {
    array_shift($backtrace);
  }

  // The first trace is the call itself.
  // It gives us the line and the file of the last call.
  $call = $backtrace[0];

  // The second call give us the function where the call originated.
  if (isset($backtrace[1])) {
    if (isset($backtrace[1]['class'])) {
      $call['function'] = $backtrace[1]['class'] . $backtrace[1]['type'] . $backtrace[1]['function'] . '()';
    }
    else {
      $call['function'] = $backtrace[1]['function'] . '()';
    }
  }
  else {
    $call['function'] = 'main()';
  }
  return $call;
}
