<?php

/* 
 * Mirror a Gherkin array (toggle vertical/horizontal)
 */
 
if (!$s = @$_GET['s']) {
  echo <<< X
    <p>Input Gherkin array, including headers (vertical or horizontal format).</p>
    <form>
      <textarea name="s"></textarea>
      <input type="submit">
    </form>
X;
  exit();
}

$ray = explode("\n", trim($s));

$line1 = trim($ray[0]);
$c9 = substr($line1, -1);
$c89 = substr($line1, -2);
[$oldTagLen, $tag] = $c89 == '**' ? [2, '*']
  : ($c89 == '//' ? [2, '']
  : ($c9 == '*' ? [1, '**'] : [0, '//']) );
if ($oldTagLen) $ray[0] = $line1 = substr($line1, 0, strlen($line1) - oldTagLen);

$hcnt = substr_count($line1, '|') - 1;
$vcnt = count($ray);
if (!$ray[$vcnt - 1]) { unset($ray[$vcnt - 1]); $vcnt--; }

foreach ($ray as $i => $line) {
  $line = trim($line);
  if (substr($line, 0, 1) != '|' or substr($line, -1) != '|') die("missing | on line $i");
  $lineRay = explode('|', substr($line, 1, strlen($line) - 2));
  if (count($lineRay) != $hcnt) die("bad field count on line $i: $hcnt vs. " . count($lineRay));
  foreach ($lineRay as $j => $fld) {
    $res[$j][$i] = $fld = trim($fld);
    $maxLen[$i] = max(@$maxLen[$i], strlen($fld));
  }
}

header('Content-Type: text/plain');
foreach ($res as $i => $lineRay) {
  echo '  |';
  foreach ($lineRay as $j => $fld) {
    $fld = str_pad($fld, $maxLen[$j], ' ');
    echo " $fld |";
  }
  echo ($i == 0) ? "$tag\n" : "\n";
}

exit();