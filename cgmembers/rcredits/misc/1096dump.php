<?php

/**
 * Utility to list contents of FIRE-format forms 1099-NEC or 1099-K
 */

if (!$flnm = @$_GET['flnm']) {
  echo <<<X
    <form>
    Filname: <input name="flnm" />
    </form>
X;
  exit();
}

if (!$s = file_get_contents(str_replace('"', '', $flnm))) die('That file does not exist. Go back and try again.');

$lines = explode("\r\n", $s);
$i9 = count($lines) - (strpos($flnm, '1099K') ? 5 : 4);
$flds = explode(', ', 'type:10:1, ein:11:9, qid:20:6, amt:54:12, nm:287:80, addr:367:80, city:447:40, st:487:2, zip:489:5');

$res[] = "ID\tamount\tname\taddress\tcity\tst\tzip";

foreach ($lines as $i => $line) {
  if ($i < 2 or $i > $i9) continue;
  foreach ($flds as $fld => $one) {
    [$fld, $p, $len] = explode(':', $one);
//    echo "fld=$fld p=$p len=$len ";
    $$fld = trim(substr($line, $p, $len));
  }
  foreach (explode(' ', 'nm addr city') as $k) $$k = ucwords(strtolower($$k));
  $type = $type == '1' ? '(co) ' : '';
  $amt = number_format($amt / 100, 2);
  echo "$qid\t$$amt\t$nm, $addr, $city, $st $zip\n";
  $res[] = "$qid\t$amt\t$nm\t$addr\t$city\t$st\t$zip";
}

file_put_contents('E:\\Downloads\\dump.csv', join("\n", $res));

// end