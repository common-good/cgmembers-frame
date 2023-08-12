<?php
/*
     Fundraising Thermometer Generator v1.1
     Sairam Suresh sai1138@yahoo.com / www.entropyfarm.org
    
     NOTE - you must include the full path to the truetype font on your system below 
            if you want text labels to appear on your graph. No TrueType fonts are
            included in this package, you can probably find some on your system or
            else download one off the net.
      

     Inputs: 'unit'    - the ascii value of the currency unit. By default 36 ($)
                         other interesting ones are:
                           163:  British Pound
                           165:  Japanese Yen
                           8355: French Franc
                           8364: Euro

             'max'     - the goal
             'value' - the current amount raised

     Versions:
     1.2 - added a 'burst' image on request, cleaned up the images a little bit.
     1.1 - Internationalized :) added 'unit' at a user's request so other currencies could be used.
     1.0 - intial version

*/
error_reporting(7); // Only report errors

$font = __DIR__ . "/Poppins Medium 500.ttf";

//$unit = ($_GET['unit']) ? $_GET['unit'] : 36; // ascii 36 = $
//$t_unit = ($unit == 'none') ? '' : code2utf($unit);
$unit = ' $';
$maxValue = ($_GET['max']) ? $_GET['max'] : 0;
$value = isset($_GET['value']) ? $_GET['value'] : 0;

const IMG_H = 220;
const BULB_X0 = 24;
const BULB_Y0 = 5;
const BULB_Y9 = 170;
const BULB_H = BULB_Y9 - BULB_Y0;

const TXT_X = 56; // left edge of text
const FONT_SIZE = 16; // font-size in pixels
const MAX_Y = 1.4 * FONT_SIZE;
const MAX_TEXT = 'target: ';
const VAL_TEXT = ' so far';

$imgW = 60 + .5 * FONT_SIZE * max(strlen(MAX_TEXT . '$,' . $maxValue), strlen(VAL_TEXT . '$,' . $value));
$img = imagecreateTrueColor($imgW, IMG_H);

$white = imagecolorallocate ($img, 255, 255, 255);
$black = imagecolorallocate ($img, 0, 0, 0);
$red = imagecolorallocate ($img, 255, 0, 0);
$blue = imagecolorallocate ($img, 0, 0, 255);

imagefill($img, 0, 0, $white);
ImageAlphaBlending($img, true); 

$thermImage = imagecreatefromjpeg("therm.jpg");
$tix = ImageSX($thermImage);
$tiy = ImageSY($thermImage);
ImageCopy($img, $thermImage, 0, 0, 0, 0, $tix, $tiy);
Imagedestroy($thermImage);

/*
  thermbar pic courtesy http://www.rosiehardman.com/
*/
$bar = ImageCreateFromjpeg('thermbar.jpg'); 
$barW = ImageSX($bar); 
$barH = ImageSY($bar); 

// Draw the filled bar
$newH = (is_numeric($maxValue) and $maxValue > 0) ? min($maxValue, round(BULB_H * ($value / $maxValue))) : 0;
$y = BULB_Y9 - $newH;
imagecopyresampled($img, $bar, BULB_X0, $y, 0, 0, $barW, $newH, $barW, $barH); 
Imagedestroy($bar);

//    imagettftext ($img, FONT_SIZE, 0, TXT_X, 355, $black, $font, $unit."0");                 // Write the Zero
if ($y > MAX_Y + FONT_SIZE/2) imagettftext ($img, FONT_SIZE * .85, 0, TXT_X, MAX_Y - 4, $black, $font, MAX_TEXT . $unit . number_format($maxValue)); // the max
if ($y <= MAX_Y + FONT_SIZE/2) {
    imagettftext ($img, FONT_SIZE, 0, TXT_X, MAX_Y, $blue, $font, $unit . number_format($value) . '!!'); // Current > Max
} elseif ($value > 0) {
  imagettftext ($img, FONT_SIZE, 0, TXT_X, $y+FONT_SIZE/2, $blue, $font, $unit. number_format($value) . VAL_TEXT);  // Current < Max
}

if ($value > $maxValue) {
    $burstImg = ImageCreateFromjpeg('burst.jpg');
    $burstW = ImageSX($burstImg);
    $burstH = ImageSY($burstImg);
    ImageCopy($img, $burstImg, 0, 0, 0, 0, $burstW, $burstH);
}

Header("Content-Type: image/jpeg"); 
Imagejpeg($img);
Imagedestroy($img);

function code2utf($num){
 //Returns the utf string corresponding to the unicode value
 //courtesy - romans@void.lv
 if($num<128)return chr($num);
 if($num<2048)return chr(($num>>6)+192).chr(($num&63)+128);
 if($num<65536)return chr(($num>>12)+224).chr((($num>>6)&63)+128).chr(($num&63)+128);
 if($num<2097152)return chr(($num>>18)+240).chr((($num>>12)&63)+128).chr((($num>>6)&63)+128). chr(($num&63)+128);
 return '';
}
