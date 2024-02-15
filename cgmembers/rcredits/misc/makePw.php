<?php
const LEN = 51; // chatGPT recommends 50+ for maximum security in the future (> 2/15/2024). Use a multiple of 3 to avoid trailing "="
$strong = FALSE; // "adequately strong" assessment returned by the function

while (!$strong) $res = base64_encode(openssl_random_pseudo_bytes(LEN, $strong));

echo $res;