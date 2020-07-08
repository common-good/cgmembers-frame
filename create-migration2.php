<?php
/**
 * @file
 * Create a standard migration with the name phinx chose.
 */
 
const DIR = 'db/migrations/';

$class = $argv[1];
$flnm = gmdate('Ymdhis') . strtolower(preg_replace('/[A-Z]/', '_$0', $class)) . '.php';
$s = file_get_contents(DIR . '_model.php');
$s = str_replace('Model', $class, $s);
file_put_contents(DIR . $flnm, $s);
echo "Created migration: $flnm ($class)";