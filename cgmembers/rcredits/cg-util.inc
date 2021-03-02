<?php
/**
 * @file
 * rCredits utilities
 * This file may be both on the core VPS and the user interface VPS
 */

namespace CG\Util; // typically abbreviated as "u"
use CG as r; // get rid of this
use CG\DB as db;
use CG\Util as u;
use CG\Web as w;
use CG\Testing as t;

const URL_CHARS = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_~'; // 66 valid URLchars
const HEX_CHARS = '0123456789abcdef';
const MOBILE_REGEX = '/\b(?:a(?:ndroid|vantgo)|b(?:lackberry|olt|o?ost)|cricket|docomo|hiptop|i(?:emobile|p[ao]d)|kitkat|m(?:ini|obi)|palm|(?:i|smart|windows )phone|symbian|up\.(?:browser|link)|tablet(?: browser| pc)|(?:hp-|rim |sony )tablet|w(?:ebos|indows ce|os))/i'; // thanks to Justin DoCanto et al.: https://stackoverflow.com/questions/4117555/simplest-way-to-detect-a-mobile-device-in-php
const PGP_TRASH_LEN = 30; // do not change this, without re-encrypting everything encrypted with its previous value
// NOTE: whether this number is 0 or 200 makes ZERO difference in the strlen of the encrypted value. 30 is plenty


// Some useful Drupal db methods: fetchAllKeyed(m, n), fetchAssoc and fetchAllAssoc(fld), fetchObject and fetchAll(), fetchCol(n), fetchField()

/**
 * array_splice() for associative arrays.
 * If the key is not found, $input is unchanged and return NULL
 */
function splice(&$input, $key, $len = 0, $replacement = NULL) {
  $i = array_search($key, $keys = array_keys($input));
  if ($i === FALSE) return NULL;
  $values = array_values($input);
  $iKeys = array_splice($keys, $i, $len, array_keys(nn($replacement, [])));
  $iValues = array_splice($values, $i, $len, array_values(nn($replacement, [])));
  $input = array_combine($keys, $values);
  return array_combine($iKeys, $iValues);
}

/**
 * Return the amount string, cleaned up (leading $ and any commas or spaces are discarded)
 */
function cleanAmount($amount) {
  $amount = trim($amount);
  if (substr($amount, 0, 1) == '$') $amount = substr($amount, 1);
  $amount = str_replace(R_NONE, '', $amount); // useful for monthly statements
  $amount = preg_replace('/[ \,]/', '', $amount); // ignore spaces and commas
  if (u\nonish($amount)) $amount = 0;
  return $amount;
}

/**
 * Return an error message if the amount is bad.
 * @param string $amount: the questionable amount string (leading $ and any commas or spaces are ignored)
 *   The cleaned-up amount is returned by ref.
 * @param string $restriction: none (the default), >0, !=0, or >=0
 * @param bool $fractionLimit: maximum digits after the decimal (default 2)
 * @param numeric $max: maximum amount ('' if no maximum) (defaults to R_MAX_AMOUNT)
 * @return: FALSE if not a bad amount, else error message
 */
function badAmount(&$amount, $restriction = '', $fractionLimit = 2, $max = NULL) {
  $new = cleanAmount($amount);
  if (!is_numeric($new)) return 'bad amount';

  $amount = nn($new, 0) + 0;
  $dotPos = strpos($amount, '.');
  if ($dotPos !== FALSE and strlen($amount) > $dotPos + 1 + $fractionLimit) return 'too fractional';
  if ($restriction == '>0' and $amount <= 0) return 'nonpositive amount';
  if ($restriction == '>=0' and $amount < 0) return 'negative amount';
  if ($restriction == '!=0' and $amount == 0) return 'zero amount';

  if (is_null($max)) $max = R_MAX_AMOUNT;
  if ($max !== '' and abs($amount) > $max) return t('amount too big', compact('max'));
  return FALSE;
}

/**
 * Say whether a given date of birth is bad. Convert dob to internal format.
 * @param bool $co: <talkin about a company>
 */
function badDob(&$dob, $co) {
  list ($min, $max) = $co ? [-999 * 12, 0] : [-150 * 12, 9]; // 150 years old to just conceived
  list ($min, $max) = [u\plusMonths($min), u\plusMonths($max)];
  $res = u\badDate($dob, $min, $max); // badDate makes dob internal (must be separate from return)
  return $res;
}

/**
 * Say whether the given SSN is the wrong length (since they are random, that's all we can tell).
 */
function badSsn(&$ssn, $dob = '') {
  if (strlen(u\digits($ssn)) == 9) return FALSE;
  return nn($dob) ? 'bad ssn' : 'bad federal id';
}

function ssnName($v, &$firstname = '', &$middlename = '', &$lastname = '') {
  foreach (ray('firstname middlename lastname') as $k) {
    if (!isset($v->$k)) continue;
    if (is_array($v->$k)) $v->$k = join(' ', $v->$k); // concatenate multiple first, middle, or last names
    if ($$k = ucWords(mb_strtolower(str_replace('-', '', $v->$k)))) $res[] = $$k;
  }
  return nn($res) ? join(' ', $res) : '';
}
function ssnAddr($v) {return ucWords(mb_strtolower(nn($v->address) . ', ' . nn($v->city) . ', ' . nn($v->state) . ' ' . nn($v->zip)));}

/**
 * Say whether the given postal code is bad.
 * @param string $z: (MODIFIED) the postal code to test, returned with extraneous punctuation removed
 * @param int $country: country code (defaults to US)
 * @return the error index if the postal code is bad, otherwise FALSE
 */
function badZip(&$z, $country = US_COUNTRY_ID) {
  $z = preg_replace('/[^A-Za-z0-9\\-]/', '', $z);
  $pattern = $country == US_COUNTRY_ID ? '/^\\d{5}(\\-\\d{4})?$/' // United States
  : ($country == CA_COUNTRY_ID ? '/^(?!.*[DFIOQU])[A-VXY]\\d[A-Z] ?\\d[A-Z]\\d$/' // Canada
  : '/*/' ); // allow any postal code for other countries, for now
  return (!preg_match($pattern, $z)) ? 'bad zip' : FALSE;
}

/**
 * Say where the given address contains a PO box.
 */
function poAddress($address) {
  return (strihas(nn($address), ' box ') or strihas(nn($address), 'pob '));
}

/**
 * Format the given amount as currency.
 *
 * @param float $amount
 * $param string $opt (default s$) (note that a space is added to the start of $opt, to make strpos easier):
 *   0 = round to nearest dollar
 *   s = simplify: Omit the cents for whole dollar amounts
 *   $ = include initial dollar sign
 *   r = include a trailing cg
 *   u = include a trailing us
 *   % = percentage (or %0, %1, %2, etc. to show number of digits after decimal)
 * @return
 *   the formatted amount
 */
function fmtAmt($amount, $opt = 's$') {
  $amount0 = $amount = cleanAmount(nn($amount, 0));
  if ($pct = strhas($opt, '%')) $amount *= 100;
  u\EXPECT(compact('amount'), 'float', 'bad num');
  $decimals = u\starts($opt, '%') ? 1 // default to one digit after decimal, for percents
  : ($pct ? trim(strtr($opt, '%s', '  '))
  : (strhas($opt, '0') ? 0 : 2));

  $amount = number_format(floatval($amount), $decimals);
  if ($amount == '-0.00') $amount = strhas($opt, '0') ? '0' : '0.00'; // this happens, for small negative amounts
  if (strhas($opt, '$')) $amount = '$' . $amount;
  if (strhas($opt, 's') and strhas($amount, '.')) $amount = preg_replace($pct ? '/\\.?0+$/' : '/\\.00/', '', $amount);
  if (strhas($opt, 'r')) $amount .= 'cg';
  if (strhas($opt, 'u')) $amount .= 'us';
  if ($pct) $amount .= '%';
  return $amount;
}

/**
 * Say whether the user name is valid.
 * @param string $name: the name to test
 * @param boolean $picky: whether to allow only 2-4 word names (default TRUE)
 */
function validName($name, $individual = TRUE, $picky = TRUE) {
  $name = trim($name);
  $words = substr_count($name, ' ') + 1;
  $pattern = $individual ? "[^\w\-\.,' ]" : '[^\w~!@#\$%\^&\*\(\)\-=\+\\\\;:\'",<\.>/\? ]';
  if (preg_match("`$pattern`iu", $name)) return FALSE; // allows unicode chars and common punctuation
  if (isQid(mb_strtoupper($name))) return FALSE; // never allow a QID as a name
  if ($picky and ($words < 2 or $words > 4)) return FALSE;
  return TRUE;
}

/**
 * Calculate a short_name from the given fullName.
 */
function shortName($fullName, $filler = '') {
  $deletions = ray('Inc. LLC Co. P.C. Corp. LLP');
  foreach ($deletions as $one) $fullName = str_replace(" $one", '', $fullName);
  $fullName = str_replace('&', 'and', $fullName);
  $fullName = asciiOnly($fullName);
  $result = preg_replace('/\W/', $filler, $fullName);
  return mb_strtolower($result === '' ? u\code() : $result);
}

/**
 * Convert to ascii.
 */
function asciiOnly($s) {
  ini_set('mbstring.substitute_character', 'none'); 
  $s = mb_convert_encoding($s, 'UTF-8', 'UTF-8'); // avoid iconv failure (bug in PHP 5.4.3)
  return iconv('UTF-8', 'ASCII//TRANSLIT', $s);
}

/**
 * Say whether the transaction description implies a cash transaction.
 */
function forCash($for) {
  if (is_array($for)) return forCash($for[0]) or forCash($for[1]);
  return ($for == '' or preg_match('`^(' . t('|credits|rCredits|USD|automatic transfer|cash|usd') . ')(\W|$)`i', $for)); // empty or a non-goods word at the beginning
}

/**
 * Return an alphabetic representation of the given non-negative integer.
 * A is the zero digit, B is 1, etc.
 * @param int $n: the integer to represent
 * @param int $len: the length of the string to return
 *   <=0 means length >=-$len
 * @param int $base: the radix (defaults to 26). If base is between 27 and 35, digits 1-9 are used next (no zero)
 */
function n2a($n, $len, $base = 26) {
  $result = '';
  for ($i = 0; ($len > 0 ? ($i < $len) : ($n > 0 or $i < -$len)); $i++) {
    $digit = $n % $base;

    if ($base == 36 or $base == 16) { // special case for hex or alpha+digits (start with digits)
      $result = ($digit < 10 ? $digit : chr(ord("A") + $digit - 10)) . $result;
    } else $result = ($digit < 26 ? chr(ord("A") + $digit)
    : ($digit < 36 ? ($digit + ($base < 36 ? 1 : 0) - 26)
    : chr(ord("a") + $digit)))
    . $result;
    
    $n = (int) ($n / $base);
  }
  return $result;
}

/**
 * Return the numeric equivalent of the given alphabetic string. 
 * In base 27 to 35, zeros are understood as Os (so "1" comes after "Z").
 * @see n2a
 */
function a2n($string, $base = 26) {
  if (26 < $base and $base < 36) $string = str_replace('0', 'O', strtoupper($string));
  $result = 0;
  for ($i = 0; $i < strlen($string); $i++) {
    $n = ord($string[$i]);
    if ($base == 36 or $base == 16) {
      $result = $result * $base + $n - ($n >= ord("A") ? ord("A") - 10 : ord("0"));
    } else $result = $result * $base + $n - 
      ($n >= ord("a") ? ord("a") - 36
       : ($n >= ord("A") ?  ord("A") 
       : ord($base < 36 ? "1" : "0") - 26)
      );
  }
  return $result;
}

/**
 * Return a radix-35 version of the number (and vice versa) for invitation codes.
 * @param int $n: the number
 * @param int $len: length (or minimum length -- see n2a above)
 */
function n2ai($n, $len) {return n2a($n, $len, 35);}
function ai2n($string) {return a2n($string, 35);}

/**
 * Return a radix-62 version of the number (and vice versa) for emailed URL codes.
 */
function n2ae($n, $len) {return n2a($n, $len, 62);}
function ae2n($string) {return a2n($string, 62);}

/**
 * Encode 3 hex digits as 2 legal URL characters: [A-Za-z0-9.-]
 * @param string $s: the three digits to convert
 * u\x32a2(u\a22x3('B7'));
 */
function x32a2($s) {
  $chars = URL_CHARS;
  foreach (str_split($s) as $c) $cs[] = stripos(HEX_CHARS, $c); // get hex values
  return $chars[($cs[0] << 2) + ($cs[1] >> 2)] . $chars[(($cs[1] % 4) << 4) + $cs[2]];
}

/**
 * Decode 2 legal URL characters as 3 hex digits (see x32a2, above).
 * @param string $s: the two characters to convert -- only the first 64 chars in URL_CHARS are allowed,
 */
function a22x3($s) {
  $chars = HEX_CHARS;
  foreach (str_split($s) as $c) $cs[] = strpos(URL_CHARS, $c);
  return $chars[$cs[0] >> 2] . $chars[(($cs[0] % 4) << 2) + ($cs[1] >> 4)] . $chars[$cs[1] % 16];
}

/**
 * Return an HTML attribute string for the given array.
 * @param assoc $ray: list of attribute names and values (value can be an array)
 */
function tribs($ray) {
  foreach ($ray as $k => $v) if (!is_null($v)) {
    $tribs[$k] = "$k=\"" . plain(is_array($v) ? join(' ', $v) : $v) . '"';
  }
  return join(' ', nn($tribs) ?: []);
}

/**
 * URL encode an arbitrary ascii string compactly.
 * Note that this uses non-mb functions, to encode bytes rather than characters.
 */
function urlify($s) {
  if ($s === '') return '';
  $len = strlen($s = bin2hex($s0 = $s));
  for ($i = 0, $result = ''; $i < $len - 2; $i += 3) $result .= x32a2(substr($s, $i, 3));
  if ($taili = $len % 3) $result .= '~' . substr($s, -$taili);
//  if (test()) return $s0;
  return $result;
}

/**
 * URL decode an arbitrary ascii string that was encoded with urlify (see above).
 * @param string $s: the URLified string
 * @return string: the original un-encoded string (FALSE if error)
 */
function deurlify($s) {
  if ($s === '') return '';
  $s0 = $s;
  list ($s, $tail) = explode('~', $s . '~');
  for ($i = 0, $res = ''; $i < strlen($s) - 1; $i += 2) $res .= a22x3(substr($s, $i, 2));
  $res .= $tail;
  return isHex($res) ? hex2bin($res) : FALSE;
}

function isHex($s) {return (preg_match('/^[0-9a-f]+$/i', $s) and !(strlen($s) & 1));}

/**
 * Return the ein in standard format (xx-xxxxxxx)
 */
function fmtEin($ein) {
  $ein = str_replace('-', '', $ein);
  return substr($ein, 0, 2) . '-' . substr($ein,2);
}

/**
 * Format the given phone number as wanted
 * @param string $phone
 * @param string $how: how to format it
 *   '+n' +1dddddddddd (AKA 'internal')
 *   'n'  dddddddddd
 *   '+'  +1 ddd ddd dddd (the international standard with spaces added) (the default)
 *   '-'  ddd-ddd-dddd
 * @return the formatted phone number (FALSE if the $phone is not a phone number or $how is bad)
 */
define('MAX_PHONE_DIGITS', 15);
define('MIN_PHONE_DIGITS', 7);

function fmtPhone($phone, $how = '+') {
  $sep = $how == '-' ? '-' : ' '; // digit group separator
  $foreign = ''; // set to 'n' format if the phone is outside US
  $phone = (preg_replace('~[\(\) \-\./]~', '', $phone)); // ignore typical punctuation
  if (substr($phone, 0, 2) == '+1') {
    $phone = substr($phone, 2);
  } elseif (substr($phone, 0, 1) == '1') {
    $phone = substr($phone, 1);
  } elseif (substr($phone, 0, 1) == '+') $foreign = $phone = substr($phone, 1);

  if (!is_numeric($phone) or (!test() and strlen($phone) < MIN_PHONE_DIGITS) or strlen($phone) > MAX_PHONE_DIGITS) return FALSE;
  if (!$foreign and substr($phone, 0, 1) < 2) return FALSE; // US phone numbers cannot start with 0 or 1
  if (!$foreign and strlen($phone) != 10) return FALSE;

  if ($how == 'n') return $phone;
  if ($how == '+n' or $how == 'internal') return ($foreign ? "+$foreign" : "+1$phone");

  $phone = substr($phone, 0, 3) . $sep . substr($phone, 3, 3) . $sep . substr($phone, 6);
  if ($how == '-') return $phone;
  if ($how == '+' or $how == '') {
    $len = substr($foreign, 0, 1) == '7' ? 1 // get length of country prefix
      : (strpos('. 21 22 23 24 25 26 29 35 37 38 42 51 59 67 68 69 85 87 88 96 97 99 ', ' ' . substr($foreign, 0, 2) . ' ') ? 3 : 2);
    return $foreign ? substr($foreign, 0, $len) . ' ' . substr($foreign, $len) : ($how ? "+1 $phone" : $phone);
  }
  return FALSE;
}

/**
 * Return the given array with its keys prefixed by the given string (and maybe suffixed, too)
 * @param string $prefix: what to prefix each key with
 * @param assoc $ray: the array
 * @param bool $noArray: <make sure no element of the result is an array>
 * @param string $suffix: optional suffix for each key
 */
function prefixKeys($prefix, $ray, $noArray = FALSE, $suffix = '') {
//  EXPECT(compact('prefix', 'ray'), 'string array');
  if ($prefix == '') return $ray;
  $newRay = [];
  foreach ($ray as $k => $v) $newRay[$prefix.$k.$suffix] = ($noArray and is_array($v)) ? pr($v) : $v;
  return $newRay;
}

/**
 * Return the given array with each value prefixed by the given string.
 */
function prefixValues($prefix, $ray) {
  if (!is_array($ray)) $ray = ray($ray);
  foreach ($ray as $k => $v) $ray[$k] = $prefix . $v;
  return $ray;
}

function changeKey($oldKey, $newKey, &$ray) {
  $ray[$newKey] = $ray[$oldKey];
  unset($ray[$oldKey]);
  return $ray;
}

/**
 * Return the difference between two arrays (better than array_diff_assoc)
 */
function diff($a, $b) {
  $result = [];
  foreach ($a as $key => $one) if (!is_null($a[$key])) {
    if ($a[$key] != nni($b, $key) or !isset($b[$key])) $result[] = $key . (isset($b[$key]) ? '' : ': missing in #2');
  }
  foreach ($b as $key => $one) if (!is_null($b[$key]) and !isset($a[$key])) $result[] = $key . ': missing in #1';
  return $result;
}

/**
 * Return random uppercase dictionary word of 3-6 chars
 * The word will not be a valid command or command abbreviation.
 */ /* UNUSED
function nonce($minLen = 1) {
  list ($where, $subs) = array('LENGTH(nonce)>=:minLen', compact('minLen'));
  $offset = db\get('FLOOR(RAND() * COUNT(1))', 'r_nonces', $where, $subs);
  $nonce = db\get('nonce', 'r_nonces', "$where LIMIT $offset, 1", $subs);

  $impossible = join(' ', $GLOBALS['SMS not a nonce']);
  if (strpos(". $impossible ", " $nonce ")) return nonce(); // reserved word? try again
  if (strlen($nonce) > 6) return nonce(); // too long (currently never happens because all are <= 6)
  return $nonce;
}*/

/**
 * Return a memorable random password.
 * @param int $len: number of words in password
 */
function randPass($len = 4) {
  global $nextPass; if (test() and nn($nextPass)) return $nextPass;
  $list = explode("\n", file_get_contents(SHARED_DIR . '/config/words.txt'));
  $listLen = count($list);
  for ($i = 0; $i < $len; $i++) $res[] = str_replace("\r", '', $list[rand(0, $listLen - 1)]); // \r only in Windows
  return join(' ', $res);
}

function ignoreSuffix($s, $suffixes) {
  $suffixes = preg_replace('/\W+/', ' ', $suffixes);
  $pattern = preg_quote(str_replace(' ', ' |', trim($suffixes)), '`');
  return trim(preg_replace("`([^ ])($pattern )`sim", '$1 ', " $s "));
}

/**
 * An HTTP REST requester
 * modified from Wez Furlong's Generic REST Helper (http://wezfurlong.org/blog/2006/nov/http-post-from-php-without-curl/)

Wez writes:
PHP's HTTP wrapper will automatically fill out the Content-Length header based on the length of the $content that you pass in. It will also automatically set the Content-Type to application/x-www-form-urlencoded if you don't specify one in the $extra_headers.
...
Many web services offer a REST-ful interface for consuming their data, using GET requests for information retrieval and POST requests for making changes. Below you'll find a helper function that can very easily be used to consume a REST API.

The $url parameter is the HTTP or HTTPS URL for the web service. $content is an associative array of form parameters to pass to the web service; they will be passed as _GET parameters for GET requests or _POST parameters for POST requests. The $method parameter can be GET or POST (and presumably any other valid HTTP REQUEST method, such as PUT or DELETE, although I haven't tried those and can't say whether they will work as expected). The $format parameter can be "json" or "xml" and will automatically return a decoded json or XML document, respectively.

I've used simplexml here because it is... simple. You could very easily add a "dom" format to return the object using the richer and more complex DOM API instead.

This function uses the ignore_errors context parameter. Without this set (the default is FALSE), PHP will treat 400 and 500 HTTP status codes as a failure to open the stream and won't return you any data. This is usually what you want when using fopen or file_get_contents, but REST services tend to set the HTTP status to indicate the error and will usually send back a payload that describes the error. We turn on ignore_errors so that we treat any returned payload as json or xml.

When using POST with REST, take care: PHP's HTTP redirection handler will drop your POST payload if the endpoint issues a redirect. If you experience problems using POST with the function below, it might be due to redirects. Most of the POST calls I've run into issue redirects if the URL is missing a trailing '/' character. In other words, if you experience problems where it seems like your parameters are not being sent in, try appending a '/' to the end of the URL and try it again.
 */
function post($url, $content = FALSE, $responseFormat = 'raw', $extra_headers = [], $method = 'POST', $request_format = 'url') {
  if (test()) { global $testHit; $testHit = compact('url', 'content'); }
  
  $ignore_errors = TRUE;
  $headers = array('Content-Type' => $request_format == 'json' ? 'application/json' : 'application/x-www-form-urlencoded') + $extra_headers;

  $header = '';
  foreach ($headers as $key => $value) $header .= "$key: $value\n";
  $http = compact(ray('method ignore_errors header'));

  if ($content) {
    $content = http_build_query($content);
    if ($method == 'POST') {
      if ($request_format == 'json') $content = u\jsonize($content);
      $http += compact('content');
    } else $url .= '?' . $content;
  }

  $context = stream_context_create(compact('http'));
  if ($fp = fopen($url, 'rb', FALSE, $context)) {
    /// $meta = stream_get_meta_data($fp); var_dump($meta['wrapper_data']); // for debugging
    $response = stream_get_contents($fp);
  } else $response = FALSE;

  if ($response === FALSE) {
    if (test()) 
    r\tellAdmin("$method $url failed");
    return FALSE;
  }
  
  $result = $responseFormat == 'xml' ? simplexml_load_string($response)
    : ($responseFormat == 'json' ? u\dejsonize(utf8_encode($response))
    : ($response) );
  EXPECT(!is_null($result), "$method $url returned error: $response");
  return $result;
}

function get($url, $content = FALSE, $responseFormat = 'raw', $extra_headers = [], $request_format = 'url') {
  return post($url, $content, $responseFormat, $extra_headers, 'GET', $request_format);
} 

/**
 * POST and redirect (together)
 */
function postnGo($url, $data) {
  $guts = '';
  foreach ($data as $k => $v) {
    foreach (ray('k v') as $x) $$x = htmlentities($$x);
    $guts .= <<<X
      <input type="hidden" name="$k" value="$v" />\n
X;
  }
  $clickHere = t('Click here to continue.');
  
  header_remove('Content-Security-Policy');
/**/  echo <<<X
    <form id="me" action="$url" method="GET">
      $guts
      <noscript><input type="submit" value="$clickHere" /></noscript>
    </form>
    <script>document.getElementById('me').submit();</script>
X;
  return exitJust();
}

function randomInt($len = NULL) {
  $result = '';
  while (strlen($result) < $len) $result .= mt_rand(100000000, 999999999);
  return substr($result, 0, $len);
}

/**
 * Translate constant parameters in a string.
 * For example, SUBS("SELECT * FROM table WHERE zot IN (:R_THIS, :R_THAT)") fills in the constants.
 * Constants of the form "IS_WHATEVER" are a special case. They are taken to mean the corresponding
 *   bit number B_WHATEVER in a field called $flagField. Identifiers of the form x.:IS_WHATEVER are permitted.
 *   NOTE to developer: this could be expanded later to handle a second flag field named {$flagField}2
 *     by changing the expression to "IF(:B_$1<32,$flagField&(1<<:B_$1)<>0,{$flagField}2&(1<<(:B_$1-32))<>0)".
 * @param string $string: the string to fix
 * @param string $flagField: name of the flags field for bit constants
 * @return string: the string with constant names replaced by their values
 */
function SUBS($string, $prefix = ':', $flagField = 'flags') {
  if ($prefix == ':') $string = preg_replace("/([A-za-z]+[0-9]?\.)?{$prefix}IS_([A-Z0-9_]*)/ms", "($1$flagField&(1<<:B_$2)<>0)", $string); // only for queries
//  $string = preg_replace("/\\{$prefix}IS_([A-Z0-9_]*)/ms", "($flagField&(1<<:B_$1))", $string);
  $subs = constantSubs($string, $prefix);
  if ($prefix == ':') foreach ($subs as $k => $v) {
    if (!is_numeric($v) and !strhas($v, ' ') and !strhas($v, '(')) $subs[$k] = "'$v'"; // string literals in SQL (if there are spaces or parens in the string you have to put quotes around it manually, if you want them)
  }
  $subs[':NOW'] = now(); // use current time (hypothetical time when testing)
  return strtr($string, $subs);
}

/**
 * Assert an expectation or check parameters for validity
 * Throw an Exception if the expectation is not met.
 * call by:
 *   EXPECT($assertion, $message, $ret) OR
 *   EXPECT($args, $types, $ret) eg EXPECT(compact(ray('arg1 arg2')), 'bool float');
 * @param boolean $assertion: a statement to assert
 * @param string $message: message to display if assertion is FALSE
 *
 * @param assoc $args: variables, indexed by variable name, for which to check types
 * @param string $types: a corresponding space-delimited array of expected types, 
 *   each of which may in turn be a list of types delimited by '&' or a list of such lists as options, delimited by '|'.
 *
 * @param bool $ret: return the error message rather than throwing it
 * @return string (if $ret is TRUE): an error message or '' (no error)
 * This function name is capitalized so that it stands apart from the actual code.
 * Best practice: don't put any functional code in the EXPECT call (eg don't do EXPECT($i = function($blue)) )
 */
function EXPECT($assertion, $message = '', $ret = FALSE) {
  if (is_array($assertion)) {
    list ($args, $types) = array($assertion, $message); // rename for clarity
///    EXPECT(compact(ray('args types')), 'assoc string'); // can't do this because infinite recursion
///    EXPECT(count($args) != count($types = explode(' ', $types)), 'wrong number of types expected'); // infinite recursion
    $vs = explode(' ', $types);
/**/ if (isDEV and count($args) != count($vs)) die(trace());
    $types = array_combine(array_keys($args), $vs); // not ray() here, $types may have '|'
    foreach ($args as $key => $value) if ($error = expect1($key, $value, $types[$key], $ret)) return $error;
  } elseif (!$assertion) {
    if ($ret) return $message; else throw new \Exception($message);
//    EXPECT(compact(ray('assertion message')), 'bool string&notempty'); // can't do this because infinite recursion
  }
  return ''; // no error
}

function FAIL($message) {throw new \Exception($message);}

function expect1($arg, $value, $type, $ret) {
//  static $ecnt; $ecnt = @$ecnt + 1; u\EXPECT($ecnt < 10000, 'loop');
//  EXPECT(compact(ray('arg type')), 'var string'); // can't do this because infinite recursion

  $msg = "Expected \$$arg ('" . pr($value) . "') to be type '$type'."; // standard err message (keep this)
  if (strpos($type, '|')) {
    foreach (explode('|', $type) as $one) if (expect1($arg, $value, $one, TRUE) == '') return '';
    EXPECT(FALSE, $msg, $ret);
  }
  if (strpos($type, '&')) {
    foreach (explode('&', $type) as $one) if ($error = expect1($arg, $value, $one, $ret)) return $error;
    return ''; // no error
  }
  
  switch ($type) {
    case 'array': return EXPECT(is_array($value), $msg, $ret);
    case 'assoc': 
      if ($msg2 = EXPECT(is_array($value), $msg, $ret)) return $msg2;
      foreach ($value as $key => $one) expect1("$arg key", $key, 'var', $ret);
      return;
    case 'bool': return EXPECT(is_bool($value) or $value == 0 or $value == 1, $msg, $ret);
    case 'email': return EXPECT(\valid_email_address($value), $msg, $ret);
    case 'empty': return EXPECT(empty($value), $msg, $ret);
    case 'float': return EXPECT(myFloat($value), $msg, $ret); // don't use is_float()?
    case 'id': return EXPECT(isId($value), $msg, $ret);
    case 'int': return EXPECT(myInt($value), $msg, $ret); // don't use is_int()
    case 'name': return EXPECT(isName($value), $msg, $ret);
    case 'var': return EXPECT(isVarname($value), $msg, $ret);
    case 'notempty': return EXPECT(!empty($value), $msg, $ret);
    case 'notnull': return EXPECT(!is_null($value), $msg, $ret);
    case 'qid': return EXPECT(isQid($value), $msg, $ret);
    case 'string': return EXPECT(is_string($value), $msg, $ret);
    case 'zid': return EXPECT(isZid($value), $msg, $ret);
  default: return EXPECT(is_object($value) and strcasecmp(get_class($value), "cg\\$type")==0, $msg, $ret);
  }
  throw new \Exception("bad type \"$type\" ($msg)");
}

/** 
 * Log the info in both the log file and database.
 * This makes it easy for tests to see what was done
 *
 * @param string $type: type of log entry (if FALSE, don't log anything)
 * @param mixed $info: the information to log (string or assoc)
 * @param int $adhocChannel: a second channel used from within the primary channel
 * @return the log record id (for exception handler and shutdown function)
 */  
function loga($type, $info = [], $adhocChannel = FALSE) {
  global $mya;
  global $channel;
  
/**/  if (strlen($type) >= 60) die("in loga: $type " . trace()); // don't use EXPECT here

  $ch = nn($channel) ? ray(TX_CHANNELS)[$channel] : t('hack');
//  $useTextLogToo = FALSE; // isDEV;

  $qid = $mya ? $mya->qid . (($mya->cAdmin2 and !$mya->proSe) ? strtolower($mya->agentA->mainQid) : '') : t('nobody');
  $time = date('d H:i:s');

  if (is_array($info)) {
    $map = ["\r" => '', "\n" => '', '    ' => ' '];
    if ($msg = nni($info, 'message')) $info['message'] = strtr(strip_tags(str_replace('>', '>|', $msg)), $map);
    if (nni($info, 'form_id')) {$type .= str_replace('CG\\Web\\form', '', $info['form_id']);}
    if (nni($info, 'toEmail')) $info = just('index subject', $info); // don't show whole email
    foreach (ray('form_id form_build_id form_token orig') as $k) unset($info[$k]);
    $info = u\jsonize(u\noSecrets($info), JSON_UNESCAPED_SLASHES);
  }
/**/ flog($s = "$qid/$time/$ch/$type/$info");
//  if (!test())
//  if ($channel == TX_CRON) mail(R_ADMIN_EMAIL, 'cron log', $s); // temporary until we figure out what's going wrong with flog in cron
}  

/**
 * Return an email a tag.
 */
function emailTag($email = NULL, $subject = '', $body = '', $name = '', $extra = '') {
  u\setDft($email, r\regionField('email'));
  if ($name) $email = urlencode("\"name\"") . ' ' . urlencode("<$email>");
  $res = "mailto:$email";
  if ($subject) $res .= '?subject=' . urlencode($subject);
  if ($body) $res .= '&body=' . urlencode($body);
  return "a href=\"$res\" target=\"_blank\" $extra";
}

function seeAssoc($info) {
  $result = '';
  foreach ($info as $key => $value) $result .= "<b>$key</b>: " . pr($value) . "<br>\n"; // keep
  return "\n<br>$result";
}

/**
 * Look up the given SSN and return xml data.
 */
function ssnLookup($ssn) {
  $xml = tr(R_SSN_REQUEST, '_username _password _ssn', R_SSN_USER, R_SSN_PASS, $ssn);
  if (NOT_PRODUCTION) return u\getFile('ssnresult.txt'); // testing
  return getSSLPage('https://www.nationalpublicdata.com/feeds/FDSFeed.cfm?xml=' . urlencode($xml));
}

/**
 * Curl page getter, because file_get_contents and u\get() fail
 */
function getSSLPage($url) {
  $curl = curl_init();
  curl_setopt($curl, CURLOPT_URL, $url);
  curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
// NO! curl_setopt($ch, CURLOPT_SSLVERSION, 3); 
  curl_setopt($curl, CURLOPT_HEADER, false);
  $res = curl_exec($curl);
  curl_close($curl);
  return $res;
}

/**
 * Parse the bank ID number (BIN) into a US routing number and account number.
 * @param string $bankAccount: the international bank account number
 * @return array [$routing, $account]
 */
function parseBankAccount($bankAccount) {
  $route = substr(nn($bankAccount), 4, 9); // chop of USkk
  $account = substr(nn($bankAccount), 4 + 9); // everything after the routing number
  return [$route, $account];
}

/**
 * Return the first unused id.
 * @param string $key: name of field to search
 * @param string $table: table name
 * @param int $start: first id to use (defaults to 1)
 * @param int $end: last id to use (if any)
 * @param string $where: other criteria
 * @param assoc $subs: substitutions for the other criteria
 * @return: the id (null if none available)
 */
function firstUnusedId($key, $table, $start = 1, $end = 0, $where0 = '1', $subs = []) {
  if (!$end) $end = PHP_INT_MAX - 1;
  if (!db\exists("$table t1", "$where0 AND t1.$key=$start", $subs)) return $start;
  list ($DESC, $le, $offset) = $start > $end ? ['DESC', '>=', -1] : ['', '<=', 1];    
  $where = "$where0 AND $start $le t1.$key AND t1.$key $le $end AND t2.$key IS NULL ORDER BY t1.$key $DESC";
  $beyond = $DESC ? "-1+" : "1+";
  $prev = db\get("t1.$key", "$table t1 LEFT JOIN $table t2 ON t2.$key=(t1.$key + $offset)", $where, $subs);
  return $prev ? ($prev + $offset) : NULL;
}
 
/**
 * Alter an image by resizing, cropping, translation, and/or rotation.
 * @param image $img0: the image to change
 * @param int ($w, $h): new width and height
 * @param int ($x1, $y1): where to begin cropping
 * @param int ($cropW, $cropH): how big an image to end up with
 * @param int $angle: how far to rotate the image (in degrees)
 * @return the modified image
 */
function alterImg($img0, $w, $h, $x1 = 0, $y1 = 0, $cropW = 0, $cropH = 0, $angle = 0) {
  list ($w0, $h0) = [imagesx($img0), imagesy($img0)];
  
  if ($w0 != $w or $h0 != $h) { // resize
    $img1 = imagecreatetruecolor($w, $h); // resize the original image to size of editor
	  imagecopyresampled($img1, $img0, 0, 0, 0, 0, $w, $h, $w0, $h0);
  } else $img1 = $img0;

  if ($angle) { // rotate
    $img1 = imagerotate($img1, -$angle, 0); // rotate the resized image
    $x1 += (imagesx($img1) - $w) / 2; // diff between rotated & original sizes
    $y1 += (imagesy($img1) - $h) / 2;
  }
	
  if ($cropW or $cropH) { // crop
	  $img2 = imagecreatetruecolor($cropW, $cropH); // crop image into selected area
	  imagecolortransparent($img2, imagecolorallocate($img2, 0, 0, 0));
	  imagecopyresampled($img2, $img1, 0, 0, $x1, $y1, $cropW, $cropH, $cropW, $cropH);
  } else $img2 = $img1;
  
  return $img2;
}

define('CRYPT_FLAG', '`!'); // flag start of encrypted data (add a single type letter)

/**
 * encrypt an array for use in URLs
 * @param int $pad: add an arbitrary padding element (to make the key harder to divine)
 */
function cryRay($ray, $pad = 0) {
  if ($pad) $ray['pad'] = u\randomString($pad, 'word'); // complicate (don't encrypt non-printing characters here)
  $res = ry('P', u\jsonize($ray));
  if (test()) t\doCode($res); // must be on separate line (PHP bug)
  return $res;
}

/**
 * decrypt a URL array.
 */
function decryRay($code) {
  if ($code == 'TESTDOCODE') $code = t\doCode(); // test()

  try {$res = u\dery('P', $code);}
  catch (\BadCry $e) {return NULL;}

  try {$res = u\dejsonize($res) ?: unserialize($res);}
  catch (\Exception $e) {return NULL;}
  
  return is_array($res) ? $res : NULL;
}

/**
 * Perform the given type of encryption (or decryption).
 * @param char $type: purpose of the encryption / decryption, as follows (each has its own methods, password, and IV):
 *   R regional admin private key for very secure data (the key is encrypted on the admin's flash drive)
 *   V very secure data (SSNs, etc.)
 *   S standard secure data
 *   P searchable data (last step of encryption is b64)
 *   C cookie
 *   H photo
 *   D device (PGP then b64)
 * @param string $data: the data to be encrypted / decrypted
 * @param bool $de: <decrypt rather than encrypt>
 * @param string $pwInstead: password to use instead of the one in the password file (for asymmetric encryption or decryption)
 * @return the encrypted (or decrypted) data.
 * @throws BadCry
 * TEST with foreach (ray('D S V R C P H') as $k) print "k=$k " . u\decry($k, u\cry($k, 'abcd', FALSE, $k == 'R' ? a(1)->vKeyPw : ''), $k == 'V' ? r\vKey() : ($k == 'R' ? a(1)->vKeyPw : ''));
 */
function cry($type, $data, $de = FALSE, $pwInstead = '') {
	global $cryHows, $mya;
	
	if ($data === '' or $data === NULL) return $data; // no point encrypting/decrypting nothing
  if (is_numeric($data)) $data = strval($data);
	
  if ($type == 'D') return $de ? u\decryPGP(u\b64decode($data)) : u\b64encode(u\cryPGP($data));
  
	if ($de) {
    if (!u\starts($data, CRYPT_FLAG . $type)) throw new \BadCry('encrypted value must begin with crypt flag');
    if ($type == 'V' and !$vKey = r\vKey()) return $data; // if we can't decrypt, return the data as is (for acct::recordChanges)
    $data = substr($data, strlen(CRYPT_FLAG . $type));
  }

	$chow = $cryHows[$type];
	foreach ($chow as $i => $v) if ($chow0 = nni($cryHows, $v)) $chow[$i] = $chow0[$i]; // allow reference to other how
	list ($hows, $pw, $iv) = $chow;
	$pw = u\b64decode($pwInstead ?: $pw);
	u\EXPECT($pw, 'missing pw in cry');
	
	if ($iv == 'NONCE') {
		if ($de) {
			$iv = substr($data, 1, $nonceLen = ord($data[0]));
			$data = substr($data, 1 + $nonceLen);
		} else {
    	$nonceLen = 37; // arbitrary length of nonce (must be >= 32)
			$iv = openssl_random_pseudo_bytes($nonceLen);
			$nonce = chr($nonceLen) . $iv; // since our IVs also serve as an arbitrary length mask, record its length
		}
  } else $iv = u\b64decode($iv);
	
	$hows = $de ? array_reverse(ray($hows)) : ray($hows);
	
	foreach ($hows as $how) {
    switch ($how) {
			case 'xor': $data = strXor($data, $pw . $iv); break; // deXor is the same
			case 'rot': $data = strRotate($data, min(PHP_INT_MAX, hexdec(bin2hex(substr($pw, 0, 4)))) * ($de ? -1 : 1)); break;
			case 'scram': $data = strScramble($data, $pw . $iv, $de); break;
			case 'b64': $data = $de ? u\b64decode($data) : u\b64encode($data); break; // final encrypt step if present
			case 'pgp':
				if ($de) { // decrypt with supplied private key
  			  $success = openssl_private_decrypt($data, $res, u\fmtKey($vKey, 'PRIVATE')) ? $res : FALSE;
///					if (!$success) die(print_r(debug_backtrace(), -1));
				  $data = $success ? substr_replace($res, '', -PGP_TRASH_LEN) : '';
				} else { // encrypt with public key
  				$data .= openssl_random_pseudo_bytes(PGP_TRASH_LEN); // add randomness
    			$success = openssl_public_encrypt($data, $res, u\fmtKey($pw, 'PUBLIC')) ? $res : FALSE;
					$data = $res;
				}
				break;
			default: $data = crypt($data, $how, $pw, $iv, $de); // any standard AES
		}
	}

	return $de ? $data : (CRYPT_FLAG . $type . nn($nonce) . $data);
}

/**
 * Say whether the value is encrypted.
 * Call:
 *   if (u\crypted($type, $s)) ...
 *   if ($type = u\crypted($s)) decry($type, $s); // P, S, or V only
 * @param char $type: encryption type
 * @param string $s: value to test
 * @return <value is encrypted> OR encryption type (FALSE if none)
 */
function crypted($type, $s = NULL) {
	if (func_num_args() == 1) {
    if (strlen($type) < strlen(CRYPT_FLAG) + 1) return FALSE;
	  list ($type, $s) = [$type[strlen(CRYPT_FLAG)], $type];
	  if (!$type or !strhas('PSV', $type)) return FALSE;
		return u\starts($s, CRYPT_FLAG) ? $type : FALSE;
  } else return u\starts($s, CRYPT_FLAG . $type);
}
	
function decryIf($type, $s) {return crypted($type, $s) ? decry($type, $s) : $s;}

function decry($type, $data, $pwInstead = '') {
	return ($type == 'D' or u\starts($data, CRYPT_FLAG)) ? cry($type, $data, TRUE, $pwInstead) : $data;
}

/**
 * Encrypt / Decrypt with our PGP key pair.
 * Test with $cry = u\cryPGP('success'); debug (u\decryPGP($cry));
 * @param string $data: what to encrypt
 * @param bool $priv: <use private key> (otherwise encrypt with public key)
 * @param string $trash: (RETURNED) the randomness added before encryption
 */
function cryPGP($data, $priv = TRUE, &$trash = '') {
  if (!$trash) $trash = openssl_random_pseudo_bytes(PGP_TRASH_LEN); // add randomness
  return cryPGP0('en', $data . $trash, $priv, $res) ? $res : '';
}

/**
 * Decryt with our PGP key pair (for testing at least).
 */
function decryPGP($data, $priv = FALSE) {
  $success = cryPGP0('de', $data, $priv, $res);
	return $success ? substr_replace($res, '', -30) : '';
}

function cryPGP0($way, $data, $priv, &$res = '') {
  $func = 'openssl_' . ($priv ? 'private' : 'public') . "_{$way}crypt";
  $k = u\fmtKey(u\unfmtKey(PGP_KEY), 'PRIVATE');
  return $func($data, $res, $priv ? $k : u\pubKey($k)) ? $res : FALSE;
}

/**
 * Format an RSA-encoded assymmetric key from our config file, adding BEGIN and END lines.
 * @param string $k: the raw key
 * @param string $f: PRIVATE or PUBLIC
 */
function fmtKey($k, $f) {
  $d = '-----';
  if ($f == 'PRIVATE') $f = 'RSA ' . $f; // fails in PHP for private without "RSA" or public with "RSA"
  $k = chunk_split(base64_encode($k), 64);
  return "{$d}BEGIN $f KEY$d\n$k{$d}END $f KEY$d";
}

function unfmtKey($k) {
  $k2 = preg_replace('/(-+[^-]+-+|[\r\n ])/', '', strhas($k, '.') ? file_get_contents($k) : $k);
  return base64_decode($k2) ?: $k2;
}

/**
 * Extract the public key from the private key.
 * @param string $k: a formatted private key (see u\fmtKey)
 * @return: the corresponding public key, formatted
 */
function pubKey($k) {
  return openssl_pkey_get_details(openssl_pkey_get_private($k))['key'];
}

// NEW encryption
function crypt($data, $how, $key, $iv, $decrypt = FALSE) {
  if (empty($data)) return FALSE;
  u\EXPECT(compact(ray('data how key iv')), 'string string string string');

  $ivSize = openssl_cipher_iv_length($how);
	$iv = substr($iv, 0, $ivSize);
  $key = substr($key, 0, 32); // 32 bytes for 256 bits

  $function = $decrypt ? 'openssl_decrypt' : 'openssl_encrypt';
  $data = $function($data, $how, $key, 0, $iv);
  return $data;
} 

/**
 * Initiate CSV download.
 * @param string $filename: pathname of the file that will download itself upon exit()
 */
function csvStart($filename) {
  global $csvFile;
  global $testCSVname; $testCSVname = $filename;
  global $testCSV; $testCSV = [];
  
  if (test()) return;  
  header('Content-Type: text/plain; charset=utf-8');
  header("Content-Disposition: attachment; filename=\"$filename\";");
  header('Content-Type: application/csv; charset=UTF-8'); // tell the browser it's going to be a csv file
  $csvFile = fopen('php://output', 'w');
}

/**
 * Output to the csv file.
 * @param array $line: the line to output
 * Note we use fputs to work around PHP's inability to omit enclosure for fputcsv.
 */
function csv($line, $delimiter = ',') {
  global $csvFile, $testCSV;
///  echo join($delimiter, $line) . "\n"; return; // debug(
  if (test()) {$testCSV[] = $line; return;}
//  fputcsv($csvFile, $line, $delimiter); 
  // No need to surround values with quotes when delim is not a comma (the quotes confuse MailChimp)
  if ($delimiter == ',') fputcsv($csvFile, $line, $delimiter); else fputs($csvFile, join($delimiter, $line) . "\n");
}

/**
 * Return a random ascii string (good for mcrypt key).
 * @param int $len: length of result string
 * @param string $choices: restrict characters to this subset. If empty (the default), use anything but \0
 *   accepted named subsets:
 *     lower, upper, digits (the obvious)
 *     word = upper, lower, and digits
 */
function randomString($len = 16, $choices = '') {
  global $nextCode;
  if (test() and nn($nextCode) and in($choices, ' lower upper word')) return $nextCode;

  $lower = 'abcdefghijklmnopqrstuvwxyz';
  $upper = strtoupper($lower);
  $digits = '0123456789';
  $word = $lower . $upper . $digits;

  if (nn($$choices)) $choices = $$choices;
  $chlen = strlen($choices);

  for ($s = '', $i = 0; $i < $len; $i++) {
    do {$c = openssl_random_pseudo_bytes(1);} while ($choices and ord($c) >= $chlen); // get a random character or index
    $s .= $choices ? $choices[ord($c)] : $c;
  }
  return $s;
}

/**
 * Verify the syntax of the given name. (replacement for \user_validate_name() )
 * Accept any printable character.
 * @param string $name: the field to test (RETURNED) field with minor errors fixed
 * @return: the error message (FALSE if $name is okay)
 */
function badName(&$name) {
//  $name = preg_replace('/[\x00-\x1F\x7F-\xA0\xAD]/u', '', $name);
/*if ($name and !ctype_print($name)) {
    r\tellAdmin('bad char', ray('name trace', bin2hex($name), trace()));
    return 'illegal char';
  } */
  $name = preg_replace('/[\x00-\x1F\x7F\xA0]/u', '', $name);
  $name = trim(preg_replace('/ +/', ' ', $name));
  if (strlen(u\shortName($name)) > USERNAME_MAX_LENGTH) return 'too long';
  return FALSE;
}

define("CTYPE_PRINT_UNICODE_PATTERN", "~^[\pL\pN\s\"\~". preg_quote("!#$%&'()*+,-./:;<=>?@[\]^_`{|}´") ."]+$~u");
function ctype_print_unicode($input) {
    return preg_match(CTYPE_PRINT_UNICODE_PATTERN, $input);
}

/**
 * Say whether the string date is bad.
 * @param string $s: typed date (returned with date converted to internal format, unless error)
 * @param int $min: minimum date (if any)
 * @param int $max: maximum date (if any)
 * @return FALSE if date is okay, else an error index
 */
function badDate(&$s, $min = '', $max = PHP_INT_MAX) {
  if (!$dt = u\s2t(nn($s))) return 'bad date'; // slashes makes it mdy (else dmy assumed)
  if ($min !== '' and ($dt < $min or $dt > $max)) {
    $err = $dt < $min ? 'date too early' : 'date too late';
    if (!is_numeric(@substr($s, -3, 1))) $err .= '|year is 4 digits';
    return $err;
  }
  $s = $dt;
  return FALSE;
}

/**
 * Say whether the phone number is acceptable.
 * @param string $phone: phone number in any format
 * @param int $country: country code
 * @return the error message or FALSE if the phone number is acceptable
 */
function badPhone(&$phone, $country = US_COUNTRY_ID) {
  if (!$phone = u\fmtPhone(trim($phone), '+n')) return 'bad phone';
  if ($country == US_COUNTRY_ID and strlen($phone) != 12) return 'bad phone';
  return FALSE;
}

/**
 * Return a table row with the given data (header or not).
 * @param mixed $row: an array with field headers or assoc with field names and values
 */
function tableRow($row) {
  if ($th = is_numeric(key($row))) $row = array_flip($row);
  $m = $th ? 'th' : 'td';
  $res = '';
  foreach ($row as $k => $v) {
    $res .= tr('<%m class="k-%k">%v</%m>', 'm k _v', $m, $k, $th ? ucwords($k) : $v);
  }
  return "<tr>$res</tr>\n";
}

/**
 * Convert the given array to an HTML div row (for within class "container").
 */
function ray2row($row, $classes = []) {
  if (!$classes and $row and !is_numeric(key($row))) list ($row, $classes) = [array_values($row), array_keys($row)];
  foreach ($row as $i => $v) {
    if ($class = nni($classes, $i)) $class = ' ' . $class;
    $res[] = "<div class=\"cell$class\">$v</div>";
  }
  return '<div class="row">' . join('', nn($res, [])) . "</div>\n";
}

/**
 * Return the latitude and longitude for the given address. Also the postal code at that spot.
 * call with either getLocus(location) or getLocus(addr, city, statecode, zip)
 * @param string $q: either the whole address or just number and street
 * @param string $city
 * @param int $state: state code
 * @param string $zip
 * @return [lat, lon, zip] -- [0, 0, ''] if none found
 */
function getLocus($q, $city = '', $state = '', $zip = '') {
  global $channel;
  
//  if ($channel == TX_WEB) ini_set('default_socket_timeout', 8);
  $none = [0, 0, ''];
  if ("$city$state$zip") {
    $q .= tr(", $city, " . r\realState($state) . " $zip");
  } elseif (empty($q)) return $none;
  
  if (test() or (strhas($q, 'Greenfield, MA') and isDEV)) return f('t.getLocus', $q);

  $params = ray('q key', $q, GEOCODE_KEY);
  
/*
  $params = ray('street city state', $address, $city, r\realState($state)); // US Census
  if (!$matches = (array) @$res->result->addressMatches) return r\tellAdmin(t('setLocus <%a>returned</a> no matches', 'a', w\atag($url))); // US Census
  $coods = @$matches[0]->coordinates;
  $coods = [$coods->y, $coods->x];
  */
  try {
    $res = @file_get_contents($url = GEOCODE_URL . http_build_query($params));
  } catch (\Exception $e) { }
  
  if (!nn($res) or !$res = u\dejsonize($res, FALSE) or !is_object($res)) {
    r\tellAdmin(t('setLocus <%a>failed</a>', '_a', w\atag($url)), compact('res'));
    return $none;
  }

  foreach (nn($res->results, []) as $one) if (nni($one->components, 'country_code') == 'us') {
    $i = $one->confidence; // confidence is actually specificity. Be as specific as possible.
    if (in($one->components->_category, 'building place transportation postcode') 
      and nn($one->components->postcode)
      and !isset($coods[$i])) { // don't overwrite with later same confidence
      $coods[$i] = $one->geometry;
      $zips[$i] = nn($one->components->postcode);
    }
  }

  if (!nn($coods)) return $none;

  $best = max(array_keys($coods));

  return [$coods[$best]->lat, $coods[$best]->lng, $zips[$best]];
}

/**
 * Parse the mailing address into its constituent parts.
 * @param string $addr: the address to parse
 * @param bool $separateNum: return street number and address separately
 * @return [addr, city, state, zip, $country, $num] (one or more will be empty or ER, if error)
 */
function parseAddr($addr, $separateNum = FALSE) {
  if (substr_count($addr, '/') > 1) $addr = str_replace('/', ',', $addr);

  $parts = explode(',', trim($addr));
  $p = count($parts);
  if (!$p) return ['','','',''];
//  if ($p == 1) return is_numeric($addr) ? ['', '', '', $addr] : ['', '', $addr, '']; // zip or st
  if ($p == 1) return ['', '', '', $addr]; // zip
  
  list ($state, $zip, $country) = explode(' ', mb_strtoupper(@trim(array_pop($parts))) . '  ');
  if (is_numeric($state) and !$zip and !$country) {
    $zip = $state;
    $state = array_pop($parts);
  } else {
    if (!$country and !preg_match('/^[A-Z][A-Z]$/', $state)) $state = 'ER';
  }
  $city = trim(array_pop($parts));
  $addr = join(', ', $parts);
  $num = '';
  
  if ($separateNum and strpos($addr, ' ')) {
    $parts = explode(' ', $addr);
    while (is_numeric(($p = array_shift($parts))[0])) $num .= ($num ? ' ' : '') . $p;
    if ($num !== '') $addr = $p . ' ' . join(' ', $parts);
  }
  return [$addr, $city, $state, $zip, $country, $num];
}

function parseOutZip($locus) {
  list ($a, $b, $c, $zip) = parseAddr($locus);
  return $zip;
}

/**
 * Parse the name, returning the first and last names separately. Underlines in last name are treated as spaces.
 * @param string $name: the name to parse
 * @param bool $withMiddles: <separate out the middle name(s)>
 * @return array: [$first, $last] or [$first, $last, $middles]
 */
function parseName($name, $withMiddles = FALSE) {
  $name = trim(preg_replace('/  /', ' ', $name));
  if ($i = mb_strpos($name, ',')) {
    $suffix = mb_substr($name, $i);
    $name = mb_substr($name, 0, $i);
  } elseif (preg_match('/( Jr?)$/', $name, $match)) { // handle common suffixes with no comma
    $suffix = $match[1];
    $name = preg_replace('/( Jr?)$/', '', $name);
  }
  $names = explode(' ', $name);
  $last = str_replace('_', ' ', array_pop($names) . nn($suffix));
  if ($withMiddles) {
    $first = array_shift($names);
    return [$first, $last, join(' ', $names)];
  } else return [join(' ', $names), $last];
}

/**
 * Change something to the proper case, unless it is already right.
 * @param mixed $what: a string or array of strings to fix
 * @param string $proper: the PHP function to use, to fix the case
 * @return: the argument with case fixed, utf8-encoded (ready to add to database)
 */
function normalizeCase($what, $proper = 'ucwords') {
  if (is_array($what)) {
    foreach ($what as $key => $value) $what[$key] = normalizeCase($value, $proper);
    return $what;
  }
  if ($proper = 'ucwords') {
    if ($what != mb_strtolower($what) and $what != mb_strtoupper($what)) return $what; // already mixedcase
    if (mb_substr($what, 0, 4) == 'POB ') return $what; // special case for PO boxes
  }
  return $proper(mb_strtolower($what));
}

define('R_SMALL_WORDS', 'of|a|the|and|an|or|nor|but|is|if|then|else|when|at|from|by|on|off|for|in|out|over|to|into|with');

/**
 * Convert a string to title case.
 */
function titleCase($s) {
  return preg_replace_callback('/\b(' . R_SMALL_WORDS . ')\b/i', function ($m) {return mb_strtolower($m[0]);}, ucwords(mb_strtolower($s)));
}

function gzdecode($data){
    $g=tempnam('./','gz');
    file_put_contents($g,$data);
    ob_start();
    readgzfile($g);
    $d=ob_get_clean();
    u\deleteFile($g);
    return $d;
}

/**
 * Return query SUM fields for each field in the list.
 * @param string $list: space-delimited list of fields
 * @param string $prefix: table (and dot) prefix for the summed field (eg SUM(u.floor) AS floor)
 */
function sumAs($list, $prefix = '') {
  foreach (ray($list) as $one) $result[] = "SUM(IFNULL($prefix$one, 0)) AS $one";
  return join(', ', $result);
}

function fsize($url) {
  if ($size = @filesize($url)) return $size;
  $s = file_get_contents($url);
  return strlen($s);
}

function trimAll(&$ray) {
  foreach ($ray as $key => $value) if (is_string($value)) $ray[$key] = trim($value);
  return $ray;
}

function deb($arg = '') {
  global $counterI;
  $msgCount = count(nn($_SESSION['messages']['status']) ?: []);
  log(($counterI = nn($counterI) + 1) . " ($msgCount messages): " . pr($arg), 'debug');
}

/**
 * Insert the bundle (associative array) before the needle in haystack
 * @param assoc $bundle: array to insert
 * @param assoc $haystack: array into which to insert
 * @param string $needle: key before which to insert
 * @return assoc: the resultant array (FALSE if the needle is not found)
 */
function insertBefore($bundle, $haystack, $needle) {
  if (!$i = array_search($needle, array_keys($haystack))) return FALSE;
  return array_slice($haystack, 0, $i) + $bundle + array_slice($haystack, $i);
}

function formatNum($num, $format) {
  $num = digits($num);
  if ($format == 'ein') $format = 'dd-ddddddd';
  if ($format == 'ss') $format = 'ddd-dd-dddd';
  $oldFormat = '(' . str_replace('d', '\\d', str_replace('-', ')(', $format)) . ')'; // eg (\d\d\d)(\d\d\d)(\d\d\d\d)
  $newFormat = '$1';
  for ($i = 2; $i <= substr_count($format, '-') + 1; $i++) $newFormat .= '-$' . $i;
  return preg_replace("|\\b$oldFormat\\b|", $newFormat, $num);
}  

/**
 * Return a table display of an array.
 * @param assoc $ray: the array
 * @param bool $across: if TRUE (the default) show labels on top, otherwise on the left
 * @param bool $recurse: show array elements as another table
 * @return HTML for a table displaying the labeled values
 */
function rayTable($ray, $across = TRUE, $recurse = FALSE) {
  $heads = $values = $lines = '';
  foreach ($ray as $k => $v) {
    if (is_array($v) and $recurse) $v = rayTable($v, !$across, TRUE);
    $v = plain($v);
    if ($across) {
      $heads .= "<th>$k</th>";
      $values .= "<td>$v</td>";
    } else $lines .= "<tr><th>$k</th><td>$v</td></tr>\n";
  }
  if ($across) $lines = "<tr>$heads</tr>\n<tr>$values</tr>";
  return "<table>\n$lines</table>\n";
}
    
/**
 * Return a random upper/lowercase code.
 * @param bool $upper: use only uppercase
 * @param int $len: how long
 */
function code($upper = FALSE, $len = R_CODE_LEN) {
  global $nextCode;
  if (test() and nn($nextCode)) return $nextCode;
  $res = u\randomString($len, 'word');
  return $upper ? strtoupper($res) : $res;
}

function showCaller($skip = 0) {
  $trace = debug_backtrace();
  for ($i = 0; $i <= $skip; $i++) array_shift($trace); // discard THIS call
  $caller = (object) array_shift($trace); $file = basename(nn($caller->file));
  $precaller = (object) array_shift($trace); $prefile = basename(nn($precaller->file));
  $args = pr(nn($caller->args));
  $preargs = pr(nn($precaller->args));
  return " called $caller->function($args) ($caller->line in $file), from $precaller->function($preargs) ($precaller->line in $file)\n";
}

/**
 * Show what bits are set in the given integer or what integer a string of bit names represents.
 * @param mixed $n: the integer bit array OR space-delimited string of bit names
 * @param string $descs: string array of bit names ('': use bit numbers, default: flag bit names)
 * @return: a list of the bits set n the integer
 */ 
function bits($n, $descs = B_LIST) {
  if ($descs) {
    $descs = ray($descs);
    if (!is_numeric(@key($descs))) $descs = array_keys($descs); // ignore extra info after colon in bit name
  }

  if (is_numeric($n)) { // integer bit array
    $res = '';
    for($i = 0; $i < 32; $i++) if ($n & (1 << $i)) $res .= ' ' . ($descs ? $descs[$i] : $i);
  } else { // string of bit names
    $res = 0;
    $descs = array_flip($descs);
    foreach (ray($n) as $b) $res |= u\bit(nni($descs, $b) ?: u\consta('b', $b));
  }
  return $res;
}

/**
 * Tell the browser to download subsequent output. (end with exit())
 * @param string $filename: what to name the downloaded file
 */
function beginDownload($filename) {
  global $testDownloadFlnm; $testDownloadFlnm = $filename;
  if (test()) return;
  header("Cache-Control: public");
  header("Content-Description: File Transfer");
  header("Content-Disposition: attachment; filename=$filename");
  header("Content-Type: application/octet-stream");
  header("Content-Transfer-Encoding: binary");
}

function purify($s) {
  if (is_array($s)) {
    foreach ($s as $key => $value) $s[$key] = purify($value);
    return $s;
  } else return preg_replace('/( [\x00-\x7F] | [\xC0-\xDF][\x80-\xBF] | [\xE0-\xEF][\x80-\xBF]{2} | [\xF0-\xF7][\x80-\xBF]{3} ) | ./x', '$1', $s);
}

/**
 * Return the obvious WHERE clause for the given fields (fieldname1=:fieldname1 AND ...).
 */
function where($ray) {
  if (!is_numeric(key($ray))) $ray = array_keys($ray);
  foreach ($ray as $k) $ret[] = "`$k`=:$k";
  return join(' AND ', $ret);
}

/**
 * Return the name of the next business day. (for testing, all days are business days)
 * @param int $start: how many days from now to start counting.
 * @return the common speech term for the next business day starting then.
 */
function nextBusDay($start = 0) {
  return (strftime('%u', now() + $start) > 5 and !test()) ? t('Monday')
  : ($start == 0 ? t('today') : ($start == 1 ? t('tomorrow') : strftime('%A', now() + $start)));
}

define('ENGLISH_ONES', t(' One Two Three Four Five Six Seven Eight Nine Ten Eleven Twelve Thirteen Fourteen Fifteen Sixteen Seventeen Eighteen Nineteen'));
define('ENGLISH_TENS', t(' X Twenty Thirty Forty Fifty Sixty Seventy Eighty Ninety'));
define('ENGLISH_LIONS', t(' Thousand Million Billion Trillion Quadrillion Quintillion Sextillion Septillion Octillion Nonillion'));

/**
 * Return the number as an anglicized string.
 * @param int $num: the number to convert.
 * @param int $tri: recursion depth (handle three digits at a time)
 */
function n2english($num, $tri = 0) {
  if ($num < 0) return t('Negative ') . n2english(-$num);
  if ($num == 0) return t('Zero');
 
  $ones = ray(ENGLISH_ONES);
  $tens = ray(ENGLISH_TENS);
  $lions = ray(ENGLISH_LIONS);
  $r = floor($num / 1000); // chunk the number, ...rxyy
  $x = ($num / 100) % 10;
  $y = $num % 100;

  $res = ''; // the result

  if ($x > 0) $res = $ones[$x] . t(' Hundred '); // do hundreds
  if ($y >= 20) { // ones and tens
    $res .= $tens[floor($y / 10)];
    if ($y = $y % 10) $res .= '-' . $ones[$y];
  } elseif ($y > 0) $res .= $ones[$y]; // just ones
  if ($res != '') $res .= ' ' . $lions[$tri]; // add triplet modifier only if there is something left

  return str_replace('  ', ' ', trim($r > 0 ? n2english($r, $tri + 1) . ' ' . $res : $res));
}

/**
 * Return the distance (in miles) between two points on Earth.
 * UNUSED as of 11/23/2019, but may be useful for the new app.
 */
function earthDistance($lat1, $lon1, $lat2, $lon2) {
  $earthRadius = 3959; // miles
  $dLat = deg2rad($lat2 - $lat1);
  $dLon = deg2rad($lon2 - $lon1);

  $a = sin($dLat/2) * sin($dLat/2) + cos(deg2rad($lat1)) * cos(deg2rad($lat2)) * sin($dLon/2) * sin($dLon/2);
  $c = 2 * asin(sqrt($a));
  return $earthRadius * $c;
}

/**
 * Say whether one string abbreviates another.
 * @param string $starts: the possible abbreviation
 * @param string $s: the possibly abbreviated string
 * @param bool $noCase: <ignore case>
 * @param int $starting: earliest location the potential match can start (0 for starting, -mb_strlen($starts) for ending)
 * @return <the first string abbreviates the second>
 */
function abbreviates($starts, $s, $noCase = FALSE, $starting = 0) {
  if (!nn($s) or is_numeric($s)) $s = nn($s) . '';
  u\EXPECT(compact('starts', 's', 'noCase'), 'string string bool');
  if ($noCase) list ($starts, $s) = array(mb_strtolower($starts), mb_strtolower($s));
  return (mb_substr($s, $starting, mb_strlen($starts)) == $starts);
}
function starts($s, $starts, $noCase = FALSE) {return abbreviates($starts, $s, $noCase);} // near synonym
function ends($s, $ends, $noCase = FALSE) {return abbreviates($ends, $s, $noCase, -mb_strlen($ends));}

/**
 * Return the time with some number of months added (or subtracted)
 * @param int $n: how many months to add (may be negative)
 * @param int $time: starting time (defaults to current time)
 * @return int: the resulting time, same day of month if possible, otherwise last day of month.
 * strtotime() should do this, but it actually returns March 2nd for strtotime('-1 month', strtotime('3/30/2014'))
 */
function plusMonths($n, $time = NULL) {
  u\setDft($time, now());
  if ($n > 0) $n = '+' . $n;
  $res = strtotime($n . ' months', $time);
  $day = date('d', $res);
  return $day == date('d', $time) ? $res : strtotime(-$day, $res); // use last day of month if same day fails
}

/**
 * Return the median value in an array
 */
function median($ray) {
  sort($ray);
  if (!$len = count($ray)) return NULL;
  $mid = (int) $len / 2;
  $v = $ray[$mid];
  $v2 = nni($ray, $mid + 1);
  return (($len & 2) and is_numeric($v) and is_numeric($v2)) ? ($v + $v2) / 2 : $v;
}

/**
 * XOR a string of bits with a mask
 */
function strXor($a, $mask) {
	$mlen = strlen($mask);
	u\EXPECT($mlen, 'empty strXor mask');
	$res = '';
	for ($i = 0; $i < strlen($a); $i++) $res .= $a[$i] ^ $mask[$i % $mlen];
	return $res;
}

/**
 * Rotate a string of bits to the left
 */
function strRotate($s, $n) {
	if (!$blen = strlen($s) * 8) return '';
	$n = ($n % $blen) + ($n < 0 ? $blen : 0);
	$chars = phpversion() >= 7 ? intdiv($n,	8) : (int) ($n / 8);
	$s = substr($s, $chars) . substr($s, 0, $chars);
	if (!$bits = $n & 7) return $s; // rotation fell on a character boundary
	
	$s .= substr($s, 0, 1);
	$res = '';
	for ($i = 0; $i < strlen($s) - 1; $i++) {
		$res .= chr((ord($s[$i]) << $bits) + (ord($s[$i + 1]) >> (8 - $bits)));
	}
	return $res;
}

/**
 * Scramble a string by swapping each character with one some distance to its right (0 to 255).
 */
function strScramble($s, $key, $un = FALSE) {
	if (!$slen = strlen($s)) return '';
	$klen = strlen($key);
	u\EXPECT($klen, 'empty scramble key');
	
	for ($i0 = 0; $i0 < $slen; $i0++) {
		$i = $un ? $slen - $i0 - 1 : $i0;
		$j = ord($key[$i % $klen]) % $slen; // all modulus params in this function are >0 (no worries that in PHP -8 % 5 is -3
		list ($s[$i], $s[$j]) = [$s[$j], $s[$i]];
	}
	return $s;
}

function strUnscramble($s, $key) {return strScramble($s, $key, TRUE);}

/**
 * Return the array with secret values hidden.
 */
function noSecrets($ray) {
  foreach ($ray as $k => $v) if (in($k, R_NOLOG)) $ray[$k] = '(?)';
  return $ray;
}

/**
 * Return formatted parsedown text.
 */
function parseUp($s) {
	require_once DRUPAL_ROOT . '/vendor/parsedown/Parsedown.php'; // normally only once per form

	$pd = new \Parsedown();
	$pd->setSafeMode(TRUE);
	return strtr($pd->text($s), ["\\\n" => "<br>", '\\' => "<br>"]);
}

/*
function unSerialFix($s) {
	$s = html_entity_decode($s, ENT_QUOTES, 'UTF-8');
	$s = preg_replace('!s:(\d+):"(.*?)";!e', "'s:'.strlen('$2').':\"$2\";'", $s );
	$s = unserialize($s);
}
*/

/**
 * Check new object constructor info for the proper fields.
 * @param assoc $info: initial information for the object
 * @param string $fields: space-list of allowed field names
 * @param string $reqFields: space-list of required field names
 */
function vetObjectInfo($info, $fields, $reqFields, $objectName) {
  list ($reqKeys, $allowedKeys, $infoKeys) = [ray($reqFields), ray($fields), array_keys($info)];
  list ($missing, $extra) = [array_diff($reqKeys, $infoKeys), array_diff($infoKeys, $allowedKeys)];
  u\EXPECT($missing == [], "missing fields in $objectName:" . pr($missing) . "\npassed: " . pr($infoKeys));
  u\EXPECT($extra == [], "extra fields in $objectName:" . pr($extra));
}

/**
 * Return the value of the constant formed by the prefix and the var.
 */
function consta($prefix, $var) {
  $const = strtoupper(str_replace(' ', '_', $prefix . "_$var"));
  u\EXPECT(defined($const), "constant not defined: $const");
  return constant($const);
}

/**
 * Return the given number with the given number of digits of precision (not like round())
 */
function vague($n, $prec = 1) {
  $n = sprintf('%.' . ($prec - 1) . 'e', $n);
  list ($sig, $log10) = explode('e', $n);
  return number_format($n, ($prec == 1 ? 0 : 1) - $log10);
}
  
/**
 * Return the value of the given bit.
 * @param mixed $bit:
 *   string - a defined bit name (lowercase without the initial B_)
 *   int - return the numeric value of that bit number (power of 2)
 *   ray - return the array of bit numbers as an integer bit array (recursively)
 */
function bit($b) {return is_array($b) ? (bit(array_shift($b)) | ($b ? bit($b) : 0)) : (1 << (is_numeric($b) ? $b : u\consta('b', $b)));}

function getBit($n, $b = NULL) {return is_null($b) ? $n : (bool) (($n ?: 0) & u\bit($b));}
function setBit(&$n, $b, $on = TRUE) {return $n = $on ? (nn($n, 0) | u\bit($b)) : (nn($n, 0) & ~u\bit($b));}

/**
 * Return a date the given number of units after start.
 */
function dateIncr($start, $number, $units) {
  if (in($units, 'once forever')) return NULL;
  if ($units == 'quarter') list ($units, $number) = ['month', $number * 3];
  return strtotime("+$number $units", $start);
}

/**
 * Generates the SQL for determining whether a particular account meets the 
 * criteria for the payer or payee of a rule.
 * @param string $role -- must be 'payer' or 'payee'
 * @param string $uid -- might be an actual uid or a string like :uid
 * @param string $now -- might be an actual timestamp or a string like :NOW
 * @return the sql string
 */
function genRuleSQL($role, $uid, $now = ':NOW', $rulesTnm = 'ru') {
  $sql = <<< EOF
    (CASE $rulesTnm.${role}Type
     WHEN :REF_ANYBODY THEN TRUE
     WHEN :REF_ACCOUNT THEN $role = $uid
     WHEN :REF_ANYCO THEN (SELECT :IS_CO FROM users WHERE uid=$uid)
     WHEN :REF_INDUSTRY THEN EXISTS (
       SELECT 1 FROM r_user_industries ui join ancestors a ON ui.iid=a.base 
       WHERE a.ancestor=$rulesTnm.payee AND ui.uid=$uid
     )
     WHEN :REF_GROUP THEN EXISTS (
       SELECT 1 FROM u_groups g JOIN u_groupies ug ON (ug.grpId = g.id)
       WHERE ug.uid = $uid AND ug.start <= $now AND ug.start <= $rulesTnm.start
       AND IFNULL($now < ug.end, TRUE)
     )
     ELSE FALSE
     END)  
EOF;
  return $sql;
}

/**
 * return (or set) URL parameters, handling tests appropriately
 */
function _GET($ray = NULL) {
  if (!test()) return $_GET;

  global $T_GET;
  if (isset($ray)) {
    t\fixData($ray);
    $T_GET = $ray;
  }
  return $T_GET;
}

// Single line functions that need little or no explanation
function num($a, $else = 0) {return is_numeric($a) ? $a : $else;}
function eq($a, $b, $else = FALSE) {return $a == $b ? $a : $else;}
function neq($a, $b, $else = FALSE) {return $a != $b ? $a : $else;}
function nray($a, $else = FALSE) {return !is_array($a) ? $a : $else;}
function small($n) {return (abs($n) < .001);}
function equy($a, $b) {return small($a - $b);} // sort of equal
function valid_url($url) {return (\valid_url($url) and preg_match('/^[A-Z0-9\-_]+\.[A-Z0-9\-_]+/i', $url) and mb_strpos($url, '@') === FALSE);}
/**/ function prlog($log = PROJECT) {print_r(loga($log));} // keep this
function order($one1st, $one, $two) {return $one1st ? [$one, $two] : [$two, $one];}
function both(&$vars, $k, $one1st, $one, $two) {list ($vars[$k], $vars['other' . ucwords($k)]) = u\order($one1st, $one, $two);}
function wargs($s, $args, $prefix = '') {return strtr($s, prefixKeys($prefix, $args));} // for debugging
function myInt($n) {return (is_numeric($n) and $n == (int) $n);}
function myFloat($n) {return (is_numeric($n) and $n == (float) $n);}
function isId($id) {return (myInt($id) and $id > 0);}
//function isQid($qid) {return preg_match('/(^[A-Z]{6,8}(-[A-Z]{1,8})?$)|(^[A-Z]{3}' . R_MARKS . '[A-Z]{3,}$)|(^![A-Z]{3}(\.?[A-Z]{3,})?$)/', $qid);} // middle choice is deprecated (gone after 2016)
function isQid($qid) {return preg_match('/(^[A-Z]{6,8}(-[A-Z]{1,8})?$)|(^![A-Z]{3}(\.?[A-Z]{3,})?$)/', $qid);}
function isZid($zid) {return (myInt($zid) and $zid !== 0);}
function isNonce($nonce) {return preg_match('/^[A-Z]{3,}$/', $nonce);}
function isVarname($value) {return preg_match('/^[a-z_][a-z_\-0-9]*$/i', $value);}
function isName($value) {return preg_match('/^[ \\x{00c0}-\\x{01ff}a-zA-Z0-9\,\.\'\\-]+$/u', $value);}
///function isName($value) {return preg_match('/^([ \\p{InCJK_Radicals_Supplement}\\p{InKangxi_Radicals}\\p{InIdeographic_Description_Characters}\\p{InCJK_Symbols_and_Punctuation}\\p{InHiragana}\\p{InKatakana}\\p{InBopomofo}\\p{InHangul_Compatibility_Jamo}\\p{InKanbun}\\p{InBopomofo_Extended}\\p{InKatakana_Phonetic_Extensions}\\p{InEnclosed_CJK_Letters_and_Months}\\p{InCJK_Compatibility}\\p{InCJK_Unified_Ideographs_Extension_A}\\p{InYijing_Hexagram_Symbols}\\p{InCJK_Unified_Ideographs}a-zA-Z\'\\-])+$/', $value);}
function isAcct($acct) {return (is_object($acct) and get_class($acct) == 'CG\\Acct');}
function hug($s, $arms = '()') {return mb_substr($arms, 0, 1) . $s . mb_substr($arms, -1, 1);} // deprecated
function s2t($s) {return strtotime(preg_match('/[A-Z]/i', $s) ? $s : str_replace('-', '/', $s));}
function fmtDate($time = NULL, $numeric = FALSE) {return strftime($numeric ? '%m/%d/%Y' : '%d%b%Y', nn($time, now()));}
function fmtTime($time = NULL) {return strtolower(strftime('%I:%M%p', nn($time, now())));}
function lastCall($far = 4) {$back = debug_backtrace(); return array_slice($back, 1, $far);}
function digits(&$n) {return $n = preg_replace('/\D/', '', nn($n));}
function monthDay1($time = NULL) {return strtotime(strftime('1%b%Y', nn($time, now())));}
  function mdt1($time = NULL) {return monthDay1($time);}
function monthDif($dt2, $dt1) {return (date('Y', $dt2) - date('Y', $dt1)) * 12 + (date('m', $dt2) - date('m', $dt1));}
function prefix($prefix, &$var) {$var = $prefix . $var;}
function preray($prefix, &$r0) {u\EXPECT(is_array($prefix) and (is_array($r0) or is_object($r0)), 'badray');$r = $prefix + (array) $r0; $r0 = is_object($r0) ? (object) $r : $r;}
function setDft(&$param, $value) {return $param = isset($param) ? $param : $value;}
function ifSet(&$param, $value) {if (isset($param)) $param = $value;} // opposite of setDft
function log($message, $type = 'state', $info = []) {return u\loga($type, compact('message') + $info);}
function isIPhone() {return preg_match('/iPhone/i', $_SERVER['HTTP_USER_AGENT']);}
function jsonize($obj, $flags = 0) {return json_encode($obj, JSON_UNESCAPED_UNICODE + $flags);}
function dejsonize($s, $retRay = TRUE) {return json_decode($s, $retRay);}
function redash($s) {return str_replace('—', '&mdash;', $s);}
function nonish($s) {return in(mb_strtolower($s), 'n/a nothing na none 0.00'); }
function padto($s, $n) {return str_pad(substr($s, 0, $n), $n);}
function lpad($s, $n) {return str_pad(substr($s, 0, $n), $n, '0', STR_PAD_LEFT);}
function similarEmail($e1, $e2) {return (mb_strtolower(preg_replace('/\+.*@/', '@', $e1)) == mb_strtolower(preg_replace('/\+.*@/', '@', $e1)));}
function rayv($a, $i) {return $a[$i];} // useful when $a is returned from a function
function strip($a) {foreach ($a as $i => $v) $a[$i] = strip_tags($v); return $a;}
function initials($s) {return preg_replace('/[^A-Z]/', '', ucwords(strtr($s, ray('Jr', ''))));}
function roughName($name) {return preg_replace("/[^A-Z '\-]/", '', mb_strtoupper(trim(str_replace('  ', ' ', $name))));}
function urlArgs() {return basename($_SERVER['REQUEST_URI']);}
function uc1($s) {return ucwords(mb_substr($s, 0, 1)) . mb_substr($s, 1);}
function nextAfter($needle, $hay) {return ($i = array_search($needle, $hay)) === FALSE ? NULL : $hay[$i + 1];}
function rotateBits($b, $i) {return ($i = $i % strlen($b)) ? substr($b, $i) . substr($b, 0, $i) : $b;}
function xorBits($b1, $b2) {$res = ''; for ($i = 0; $i < strlen($b1); $i++) $res .= (substr($b1, $i, 1) xor substr($b2, $i, 1)) ? 1 : 0; return $res;}
function a2b($s) {$b = ''; for ($i = 0; $i < strlen($s); $i++) $b .= lpad(decbin(ord($s[$i])), 8); return $b;}
function randomFloat($min = 0, $max = 1) {return $min + mt_rand() / mt_getrandmax() * ($max - $min);}
function hash($s) {return \hash('sha256', $s, false);}
function day2n($day) {return array_search($day, ray('Sun Mon Tue Wed Thu Fri Sat'));} // workaround %u and %w failure
function lf2br($s) {return strtr($s, ["\r" => '', "\n" => '<br>']);}
function sqlSerialValue($s, $n) {return "SUBSTRING_INDEX(SUBSTRING_INDEX($s, '\\\"', $n * 2), '\\\"', -1)";}
function similar($s1, $s2, $chars = 5, $pct0 = .1) {return (similar_text($s1, $s2, $pct) > $chars and $pct > $pct0);}
function unite($a, $b, $m = '') {return ($a and $b) ? tr('%a%m%b', '_a _m _b', $a, $m ?: t(' and '), $b) : ($a ?: $b);}
function honorDesc($honor) {return $honor == 'other' ? ' ' : t(' in %honor of ', compact('honor'));}
function inList($needle, $hay) {return (strhas(" $hay ", " $needle ") or strhas(",$hay,", ",$needle,"));}
function weakPass($pw) {return (mb_strlen($pw) < 8 or !preg_match('/\d/', $pw) or !preg_match('/[A-Z]/', $pw) or !preg_match('/[a-z]/', $pw) or !preg_match('/[^0-9A-Za-z]/', $pw));}
function b64encode($data) {return rtrim(strtr(base64_encode($data), '+/', '-_'), '=');}
function b64decode($data) {return base64_decode(strtr($data, '-_', '+/'));}
function validEmail($email) {return filter_var($email, FILTER_VALIDATE_EMAIL);}
//function bitCount(int $n) {return $n ? (($n % 2) ? 1 : 0) + bitCount($n / 2) : 0;}
function stndrdth($n) {return date('S', mktime(1,1,1,1,( (($n>=10)+($n>=20)+($n==0))*10 + $n%10) ));}
function img2string($img, $quality) {ob_start(); imagejpeg($img, NULL, $quality); return ob_get_clean();}
function deleteFile($flnm) {if (file_exists($flnm)) unlink($flnm);}
function getFile($flnm) {return file_exists($flnm) ? file_get_contents($flnm) : '';}
function prob($p, $yes = TRUE, $no = FALSE) {return (mt_rand() / mt_getrandmax()) < $p ? $yes : $no;}
function routingOk($r) {return (preg_match('/^(|[0-9]{9})$/', $r) and db\exists('r_banks', 'route=:r', compact('r')));}
function bankAcctOk($ba) {return preg_match('/^[0-9]{3,17}$/', $ba);}  // {3,20} ? Dwolla says 3-17
function joins(&$s, $m, $other = '') {return $s = (empty($s) ? '' : "$s$m") . $other;}
function padRay($ray, $n, $fill = '') {for ($i = 0; $i < $n; $i++) $ray[] = $fill; return $ray;}
function isMobile() {return (bool) preg_match(MOBILE_REGEX, $_SERVER['HTTP_USER_AGENT']);}
function deray($ray) {$res = []; foreach (is_array($ray) ? $ray : ray($ray) as $k => $v) $res[] = "$k:$v"; return join(',', $res);}
function joinRays($ray1, $ray2) {foreach ($ray2 as $k) $ray1[] = $k; return $ray1;}
function ry($type, $data) {return substr(u\cry($type, $data), strlen(CRYPT_FLAG));} // encrypt without flag
function dery($type, $code) {return u\decry($type, CRYPT_FLAG . $code);}
function ul($list) {return '<ul><li>' . join("</li>\n<li>", nn($list) ?: []) . "</li></ul>\n";}
function tm($tm) {return strtolower(trim(strftime('%I:%M%p', $tm), '0'));}
function ddddcc($n) {return cleanAmount(number_format(cleanAmount($n), 2));}
