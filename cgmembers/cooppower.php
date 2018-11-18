<?php

/**
 * @file
 * Accept data from Coop Power Community Solar Signup
 * to populate Common Good signup page, initiating account setup.
 * Copy this to the web root.
 */

$config = (array) json_decode(utf8_encode(file_get_contents(__DIR__ . "/../config/config.json")));
if (array_key_exists('stage', $config)) {
  define('isDEV', $config['stage'] == 'development');
  define('isSTAGE', $config['stage'] == 'staging');
  define('isPRODUCTION', $config['stage'] == 'production');
  define('isGAME', $config['stage'] == 'game');
  if (! isDEV and ! isSTAGE and ! isPRODUCTION and ! isGAME) {
    die('Invalid value for stage in configuration file.');
  }
} else {
  die('There is no configuration file ../config/config.json');
}

extract($_GET);

$vars = [@$m_street, @$m_city, @$m_state, @$m_zip];
foreach (ray('street city state zip') as $i => $k) ${"m_$k"} = @$vars[$i] ?: $$k; // copy physical address from postal, if blank (never happens if source form works right)
 
$fullName = @$first_name . ' ' . @$last_name;
$qid = @$cg_account;
if (@$referrer) $source = @$source . ': ' . $referrer;
list ($address2, $city2, $state2, $zip2) = [@$street, @$city, @$state, @$zip]; // postal address
list ($address, $city, $state, $zip) = [@$m_street, @$m_city, @$m_state, @$m_zip]; // physical address
foreach (['state', 'state2'] as $k) $$k = strtoupper(@$$k); // convert states to uppercase

list ($partner, $partnerCode, $action) = isDEV
  ? ['NEWAAB', '1495kJHm0h145PHh2345h', 'signup']
  : ['NEWAIL', '5aCnXTQvwRoqKu3YGUvp', 'signup'];
$customer = @$m_number;
$autopay = TRUE;

$fields = ray('fullName email phone address city state zip address2 city2 state2 zip2 partner partnerCode customer source qid autopay');

$guts = '';
foreach ($fields as $k) {
  $v = htmlspecialchars($$k);
  $guts .= <<<EOF
    <input type="hidden" name="$k" value="$v" />\n
EOF;
}

/**/  echo <<<EOF
<html style="height:100%; width:100%; display:table;">
  <body style="display:table-cell; text-align:center; vertical-align:middle;">
  <div style="border:2px solid darkgreen; color:darkblue; font-family:Arial; width:200px; height:100px; margin:auto; padding:10px;">
  Redirecting to CommonGood.earth...<br><br>
  <form id="theform" action="$action" method="POST">
  $guts
  <input type="submit" value="Redirect NOW" />
  </form>

  </div>
  </body>
  <script>document.getElementById('theform').submit();</script>
  </html>
EOF;


exit();

// END

function ray($s) {return explode(' ', $s);}
