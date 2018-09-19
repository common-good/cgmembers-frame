<?php
/**
 * @file
 * Tell admin about any Security Policy violations
 * (save this file in the web root)
 */

$host = $_SERVER['HTTP_HOST'];
if ($host == 'localhost') list ($dev, $host) = [TRUE, $host . '/cgMembers'];
$agt = $_SERVER['HTTP_USER_AGENT'];
$ip = $_SERVER['REMOTE_ADDR'];
$scheme = @$dev ? 'http' : 'https';
$isSafari = (stripos($agt, 'Chrome') === FALSE and stripos($agt, 'Safari') !== FALSE);
$isFirefox = (stripos($agt, 'Chrome') === FALSE and stripos($agt, 'Firefox') !== FALSE);

if (!$json = file_get_contents('php://input')) throw new Exception('Bad Request');
if ($isSafari) {
  if (strpos($json, 'spin.min.js')) exit(); // ignore spinner errors on Apple
  if (strpos($agt, 'Safari537') and strpos($json, 'EFFECTIVE-DIRECTIVE: style-src')) exit();
}
if ($isFirefox) { // ignore scripts added by Firefox itself (?)
  $frags = 'function (NAVIGATOR, OBJECT)~const V8_STACK~opacity: 1~moz-extension';
  foreach (explode('~', $frags) as $k) if (strpos($json, $k)) exit();
}

$message = "The user agent $agt from IP $ip reported the following content security policy (CSP) violation on $host:\n\n";
						
$csp = json_decode($json, true);
if (is_null($csp)) throw new Exception('Bad JSON Violation');

$sid = @$_GET['sid'];
$ajax = "$scheme://$host/ajax?op=whoami&sid=$sid&data=fullName,qid";
$info = (array) json_decode(file_get_contents($ajax));
$report = $csp['csp-report'] + ['current user' => @$info['whoami']]; // + compact('ajax');

foreach ($report as $k => $v) {
  $v = str_replace(';', ";\n        ", $v);
  $k = strtoupper($k);
  $message .= "    $k: $v\n\n";
}

if (@$dev) {file_put_contents('_CSP.txt', $message); exit();}

#
# Send the report

$sender  = "info@$host";
$to = 'wspademan@gmail.com';
$subject = "CSP Report for $host";
$hdrs = "From: $sender";
mail($to, $subject, $message, $hdrs);
