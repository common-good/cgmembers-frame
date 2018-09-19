<?php
define('GHERKIN_EOL', '\\'); // record delimiter in multiline arguments
global $sceneTest; // current scenario test object
global $testOnly; // step functions can create the asserted reality, except when called as "Then" (or "And")

function Gherkin($statement, $type) {global $sceneTest; return $sceneTest->gherkin($statement, $type);}
function Given($statement) {return Gherkin($statement, __FUNCTION__);}
function When_($statement) {return Gherkin($statement, __FUNCTION__);}
function Then_($statement) {return Gherkin($statement, __FUNCTION__);}
function And__($statement) {return Gherkin($statement, __FUNCTION__);}

// the body of the gherkin function in each test file
// (indirectly called by Given(), When(), etc.
function gherkinGuts($statement, $type) {
  global $sceneTest, $testOnly, $skipToStep;
  if($type == 'Given' or $type == 'When_') $testOnly = FALSE;
  if($type == 'Then_') $testOnly = TRUE;

  $argPatterns = '"(.*?)"|([\-\+]?[0-9]+(?:[\.\,\-][0-9]+)*)';
  $function = lcfirst(preg_replace("/%[A-Z][A-Z0-9_]*|$argPatterns|[^A-Z]/ims", '', ucwords($statement)));

  $statement = strtr(getConstants($statement), $sceneTest->subs); // getConstants first, in case random args have "@"
//  $statement = preg_replace('/%\((.*?)\) /e', '\1', $statement); // evaluate %(expression) (after most subs)
  $statement = preg_replace_callback('/%\((.*?)\) /', function($m) {return eval("return $m[1];");}, $statement); // evaluate %(expression) (after most subs)
///  print_r($statement);
  $statement = cleanMultilineArg($statement);
///  print_r($statement);

  preg_match_all("/$argPatterns/ms", $statement, $matches);
  $args = otherFixes(multilineCheck($matches[0])); // phpbug: $matches[1] has null for numeric args (the check removes quotes)
  $count = count($args);
  if ($count > 8) die("Too many args ($count) in statement: $statement");
  return $function(@$args[0], @$args[1], @$args[2], @$args[3], @$args[4], @$args[5], @$args[6], @$args[7]);
}

/**
 * Remove quotes around standard % string subs in multiline args (necessary?) and add a quote at the end.
 */
function cleanMultilineArg($statement) {
  if (preg_match('/(ARRAY|ASSOC)\\\\/', $statement, $match, PREG_OFFSET_CAPTURE)) {
    list ($type, $i) = $match[1];
    return substr($statement, 0, $i) . $type . str_replace('"', '', substr($statement, $i + strlen($type))) . '"'; 
  } else return $statement;
}

/**
 * Random String Generator
 *
 * int $len: length of string to generate (0 = random 1->50)
 * string $type: ?=any printable 9=digits A=letters
 * return semi-random string with no single or double quotes in it (but maybe spaces)
 */
function randomString($len = 0, $type = '?'){
  if (!$len) $len = mt_rand(1, 50);

 	$symbol = '-_~=+;!@#%^&*(){}[]<>.?\' '; // no double quotes, commas, or vertical bars (messes up args)
  $upper = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
  $lower = 'abcdefghijklmnopqrstuvwxwz';
  $digits = '0123456789';
  
  $chars = $upper . $lower . $digits . $symbol;
  if ($type == '9') $chars = $digits;
  if ($type == 'A') $chars = $upper . $lower;
  
  for($s = ''; $len > 0; $len--) $s .= $chars{mt_rand(0, strlen($chars)-1)};
  $s = str_replace('=>', '->', $s); // don't let it look like a sub-argument
//  $s0 = preg_replace('/[%@][A-Z]/e', 'strtolower("$0")', $s); // percent and ampersand occasionally look like substitution parameters
  $s = preg_replace_callback('/[%@][A-Z]/', function($m) {return strtolower($m[0]);}, $s); // percent and ampersand occasionally look like substitution parameters
  return($s); //  return str_shuffle($s); ?
}

function randomPhone() {return '+1' . mt_rand(2, 9) . randomString(9, '9');}

/**
 * The Usual Subtitutions
 *
 * Set some common parameters that will remain constant throughout the Scenario
 * These may or may not get used in any particular Scenario, but it is convenient to have them always available.
 */
function usualSubs() {
  global $picturePath;
  $date_format = '%d-%b-%Y';

  $subs = $randoms = array();
  for ($i = 3; $i > 0; $i--) $randoms[] = "%whatever$i";
  for ($i = 3; $i > 0; $i--) $randoms[] = "%random$i";
  $randoms[] = '%whatever';
  $randoms[] = '%random';
  for ($i = 5; $i > 0; $i--) $randoms[] = "%number$i"; // phone numbers

  foreach ($randoms as $key) {
    while(in_array($r = (substr($key, 0, 7) == '%number' ? randomPhone() : ('"' . randomString() . '"')), $subs));
    $subs[$key] = $r;
  }
  foreach (array(20, 32) as $i) $subs["%whatever$i"] = '"' . randomString($i) . '"';

  for ($i = 20; $i > 0; $i--) { // set up high numbers first, eg to avoid missing the "5" in %today-15
    foreach (array('-', '+') as $sign) {
      $subs["%today$sign{$i}d"] = strtotime("$sign$i days");
      $subs["%today$sign{$i}w"] = strtotime("$sign$i weeks");
      $subs["%today$sign{$i}m"] = plusMonths("$sign$i");
      if ($sign == '-')
      $subs["%today$sign{$i}y"] = strtotime("$sign$i years");
      $subs["%yesterday$sign{$i}m"] = plusMonths("$sign$i", strtotime('-1 day'));
      $subs["%tomorrow$sign{$i}m"] = plusMonths("$sign$i", strtotime('+1 day'));
    }
  }
  $subs['%today'] = time(); // must be after loop
  $subs['%yesterday'] = strtotime('-1 day');
  $subs['%tomorrow'] = strtotime('+1 day');
 
  if (function_exists('extraSubs')) extraSubs($subs); // defined in .steps -- a chance to add or replace the usual subs

  return $subs;
}

/**
 * Multi-line check
 *
 * If the final argument is a string representing a Gherkin multi-line definition of records,
 * then change it to the associative array represented by that string.
 * @param $args: list of arguments to pass to the step function
 * @return $args, possibly with the final argument replaced by an array
 */
function multilineCheck($args) {
  global $sceneTest;
  for($i = 0; $i < count($args); $i++) $args[$i] = squeeze($args[$i], '"');
  $last = end($args);
  if (!preg_match('/^(ARRAY|ASSOC)/', $last, $match)) return $args;
  $assoc = ($match[1] == 'ASSOC');
  $data = explode(GHERKIN_EOL, preg_replace('/ *\| */m', '|', $last));
  array_shift($data); // discard the matrix arg identifier
//  $keys = explode('|', squeeze(array_shift($data), '|'));
  foreach ($data as $line) {
    if (function_exists('multiline_tweak')) multiline_tweak($line);
    $values = explode('|', squeeze($line, '|'));
    $lineCols = count($values);
    $keyCols = @$ray ? count($ray[0]) : $lineCols;
    if ($lineCols != $keyCols) {
/**/  die("bad multiline field count in $sceneTest->sceneName:\nline ($lineCols) = " . print_r($values, 1) . "\nfirst ($keyCols) = " . print_r($ray[0], 1));
    }
    //$result[] = array_combine($keys, $values);
    $ray[] = $values;
  }
  if ($assoc and count($ray) > 1) { // interpret the array as an associative array: first line is the keys
    $keys = array_shift($ray);
    foreach ($ray as $values) $result[] = array_combine($keys, $values);
  } else $result = $ray;
  
  $args[count($args) - 1] = $result;
  return $args;
}

/**
 * Squeeze a string
 *
 * If the first and last char of $string is $char, shrink the string by one char at both ends.
 */
function squeeze($string, $char) {
  $first = substr($string, 0, 1);
  $last = substr($string, -1);
  return ($first == $char and $last == $char)? substr($string, 1, strlen($string) - 2) : $string;
}

/**
 * Make a string's first character lowercase
 *
 * @param string $str
 * @return string the resulting string.
 */
if(!function_exists('lcfirst')) {
  function lcfirst($str) {
    $str[0] = strtolower($str[0]);
    return (string)$str;
  }
}

/**
 * Translate constant parameters in a string.
 * @param string $string: the string to fix
 * @return string: the string with constant names (preceded by %) replaced by their values
 * Constants must be uppercase and underscores (for example, if A_TIGER is defined as 1, %A_TIGER gets replaced with "1")
 */
function getConstants($string) {
  preg_match_all("/%([A-Z_]+)/ms", $string, $matches);
  $map = array();
  foreach ($matches[1] as $one) $map["%$one"] = constant($one);
  return strtr($string, $map);
}

/**
 * 
 */
function otherFixes($args) {
  foreach ($args as $key => $one) {
    if (!is_array($one)) {
      $one = str_replace("''", '"', $one); // lastly, interpret double apostrophes as double quotes
      // NO! if (is_numeric($without = str_replace(',', '', $one))) $one = $without; // remove commas from numbers
      if (strpos($one, '=>')) { // arg is an array, parse it
        $new = array();
        foreach (explode(',', $one) as $subvalue) {
          if (strpos($subvalue, '=>') !== FALSE) {
            list ($k, $value) = explode('=>', $subvalue);
            $new[$k] = $value;
          } else $new[] = "ERROR (bad subvalue syntax: \"$subvalue\")";
        }
        $args[$key] = $new;
      } else $args[$key] = $one;
    } else $args[$key] = otherFixes($one);
  }
  return $args;
}

/**
 * Return the time with some number of months added (or subtracted)
 * @param int $months: how many months to add (may be negative)
 * @param int $time: starting time (defaults to current time)
 * @return int: the resulting time, same day of month if possible, otherwise last day of month.
 * strtotime() should do this, but it actually returns March 2nd for strtotime('-1 month', strtotime('3/30/2014'))
 */
function plusMonths($months, $time = '') {
  if ($time === '') $time = time();
  if ($months > 0) $months = '+' . $months;
  $res = strtotime($months . 'months', $time);
  $day = date('d', $res);
  return $day == date('d', $time) ? $res : strtotime(-$day, $res); // use last day of month if same day fails
}
