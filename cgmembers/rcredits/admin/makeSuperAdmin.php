<?php
/**
 * @file
 * Make an individual account a superAdmin on a non-production server (anything but main and beta).
 * Run on the server's command line, as the environment's user:
 *   php makeSuperAdmin.php <uid or email> <png-file>
 * Gives the account the member, confirmed, ok and admin flags and every admin permission, makes its
 * superAdmin key (from the dev key), sets a random sign-in password and saves the QR code to scan as a PNG.
 * Replaces signing in as the temporary admin.
 * The password has 12 characters of a 32-character alphabet (60 bits; no look-alike characters, no shift key): about
 * 180 years on average for an attacker who stole the hash and tries a hundred million guesses a second.
 */
use CG as r;
use CG\DB as db;
use CG\QR as qr;

define('DRUPAL_ROOT', __DIR__ . '/../..');
require_once __DIR__ . '/../bootstrap.inc';
drupal_override_server_variables(['url' => 'http://localhost/', 'REQUEST_URI' => '/']);
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
if (!drupal_is_cli()) die("Command line only.\n");
if (isPRODUCTION) die("Never on main or beta.\n");
require_once R_ROOT . '/cg-qr.inc';

list ($ident, $png) = [$argv[1] ?? '', $argv[2] ?? ''];
if (!$ident or !$png) die("usage: php makeSuperAdmin.php <uid or email> <png-file>\n");
if (!($a = r\acct($ident)) or $a->co) die("No such individual account.\n");
if (!is_file(DEV_VPRIVK_FLNM)) die('Missing ' . DEV_VPRIVK_FLNM . " (the dev keys are removed once setup is done).\n");

$a->setBit(['member' => 1, 'confirmed' => 1, 'ok' => 1, 'admin' => 1]);
$pw = $a->makeVKeyE(file_get_contents(DEV_VPRIVK_FLNM), NULL, FALSE);
$id = db\get('id', 'admins', ['uid' => $a->id]);
db\updateOrInsert('admins', ['id' => $id, 'uid' => $a->id, 'can' => (1 << count(ray(A_LIST))) - 1]);

$abc = 'abcdefghijkmnpqrstuvwxyz23456789';
for ($pass = '', $i = 0; $i < 12; $i++) $pass .= $abc[random_int(0, strlen($abc) - 1)];
$a->update(['pass' => $pass]);

$uri = qr\generate($pw, '', t('%PROJECT SuperAdmin'));
file_put_contents($png, base64_decode(substr($uri, strpos($uri, ',') + 1)));
chmod($png, 0600);
echo "$a->fullName is a superAdmin. Sign-in password (shown once, change it if you like): $pass\nScan-QR image: $png (print it, then delete it).\n";
