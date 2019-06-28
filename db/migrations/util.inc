<?php

/**
 * Utility functions for migrations
 * These must continue to work for all past migrations.
 */

function arrayGet($arr, $key, $dft) {
  return (is_array($arr) and array_key_exists($key, $arr)) ? $arr[$key] : $dft;
}

function ray($s, $value1 = NULL) {
  if (func_num_args() > 1) { // multi-argument call -- return an associative array
    $args = func_get_args();
    $keys = ray(array_shift($args));
    return array_combine($keys, $args);
  }
    
  // this func is called too often for the overhead u\EXPECT(compact('s'), 'string');
  $pattern = strpos($s, "\n") !== FALSE ? '/\R/' 
    : (strpos($s, '|') !== FALSE ? ' *\| *' 
       : (strpos($s, ',') !== FALSE ? '\, *' 
          : (strpos($s, ';') !== FALSE ? '; *' 
             : (strpos($s, ' ') !== FALSE ? '  *' // strangely ' +' fails but '  *' works
                : FALSE))));

  $simple = $pattern ? mb_split($pattern, $s) : array($s);
  if (!strpos($pattern, ',') or !strpos($s, ':')) return $simple; // no subargs

  $ray = [];
  foreach ($simple as $one) {
    list ($k, $v) = explode(':', $one . ':');
    $ray[$k] = $v;
  }
  return $ray;
}

function bit($b) {return is_array($b) ? (bit(array_shift($b)) | ($b ? bit($b) : 0)) : (1 << $b);}

function getBit($n, $b = NULL) {return is_null($b) ? $n : (bool) ($n & bit($b));}
function setBit(&$n, $b, $on = TRUE) {return $n = $on ? ($n | bit($b)) : ($n & ~bit($b));}

/**
 * Return just the specified key/value pairs from the given array, trimmed.
 * Typical use: extract(just('key2 key4 key 7', $info));
 * If a key did not exist in the original array, it will not be in the returned array either.
 * @param string/array $these: flat array or space-delimited array of field names
 * @param array/object/string $info: possible key/value pairs to return (passed as array, object, or UrlArgString)
 * @param bool $ordered: <order the returned keys as in $these>
 * @return array of the results [NOT trimmed, because values might be encrypted!]
 */
function just($these, $info, $ordered = FALSE) {
  $ray = is_string($info) ? urlq2ray($info) : (array) $info;
  if (!is_array($these)) $these = ray($these);
  if ($ordered) {
    foreach ($these as $k) if (isset($info[$k])) $res[$k] = $info[$k];
    return $res;
  } else return array_intersect_key($ray, array_flip($these));
}

function pr($s) {
  echo "\n$s\n";
  file_put_contents(__DIR__ . '/migrate.log', print_r($s, 1) . "\n", FILE_APPEND);
}
