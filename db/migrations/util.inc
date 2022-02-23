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

function cgpr($s) {
/**/ echo $s = print_r($s, 1) . "\n";
  file_put_contents(__DIR__ . '/migrate.log', $s, FILE_APPEND);
}

function allSoFar($w) {return '(' . join(') OR (', $w) . ')';}

function makeTxsView(&$z, $_view, $_where1 = '', $_where2 = '') {
  cgpr("making $_view");
  $m = $z['m'];
  foreach ([1, 2] as $i) {
    if (${"_where$i"}) $z["w$i"][] = '(' . ${"_where$i"} . ')'; else ${"_where$i"} = allSoFar($z["w$i"]);
  }
  $subs = compact(ray('_view _where1 _where2'));
  $m->doSql(strtr($z['sql'], $subs));
}

/**
 * Move bits around in a bit array (integer) and/or delete and/or add bits.
 * @param phinx $ph: the phinx object
 * @param string $table: name of the table that has the bit array field
 * @param string $field: name of the bit array field
 * @param string $old: the old bit list (space-delimited array)
 * @param string $new: the new bit list
 */
function fixFlags($ph, $table, $field, $old0, $new0) {
  $bmask = '((1<<%b)-1)'; // mask all bits lower than this new position
  $pmask = '((1<<%p)-1)'; // mask all bits lower than this old position
  $delBit = "(((%field>>1)&~$pmask)|(%field&$pmask))"; // outer parens required
  $insBit = "((%field&~$bmask)<<1)|(%field&$bmask)";
  $moveBit = str_replace('%field', $delBit, $insBit) . '|(((%field>>%p)&1)<<%b)';
  $sql0 = "UPDATE $table SET $field=";
  
  $new = ray($new0);
  $old = ray($old0);
  
  for ($p = count($old)-1; $p >= 0; $p--) if (array_search($old[$p], $new) === FALSE) {
    $ph->execute($sql0 . strtr($delBit, ray('%field %p', $field, $p)));
    unset($old[$p]);
  }
  $old = array_values($old); // renumber, closing the gaps
  
  foreach ($new as $b => $flag) {
    $p = array_search($flag, $old);
    if ($p === $b) continue; // bit is already in the right place
    $ph->execute($sql0 . strtr(isset($p) ? $moveBit : $insBit, ray('%field %b %p', $field, $b, $p)));
    $out = isset($p) ? array_splice($old, $p, 1) : $flag;
    array_splice($old, $b, 0, $out);
  }
    
}
