<?php
namespace CG\Txs; // typically abbreviated as "x"
use CG\Backend as be;
use CG\Util as u;
use CG as r;
use CG\DB as db;
use CG\Txs as x;
use CG\Web as w;

/**
 * @file
 * Auxiliary functions for listing transactions
 */

define('DT_MAX', 100000000000); // bigger than any date
define('TX_TID_MAX', 100000000); // zero-filler
 
define('TX_OPT_KEYS' , 'agents channels boxes exchanges pexchanges'); // these are all placeholders now
define('TX_URL_OPTS', 'states ' . TX_OPT_KEYS);
define('TX_OPT_AGENTS', 1);
define('TX_OPT_CHANNELS', 2);
define('TX_OPT_BOXES', 3);
define('TX_OPT_EXCHANGES', 4);
define('TX_OPT_PEXCHANGES', 5);

function setupDownload($starting, $ending, $fileId = PROJECT_ID) {
  global $mya;
  $dt1 = strftime('%Y%m%d', $starting);
  $dt2 = strftime('%Y%m%d', $ending);
  u\csvStart($fileId . "$dt1-$dt2.csv");
}

function txColoredState($disputed) {
  $states = array('&#10004;', 'disputed');
  $colors = array('ok', 'disputed');
  $state = $disputed ? 1 : 0;
  $color = $colors[$state];
  $state = $states[$state];
  return "<span class=\"state-$color\">$state</span>";
}

/**
 * Sum the amounts in each transaction category, based on description.
 * @param array $cats: (MODIFIED) counts and amount totals, indexed by category description
 * @param string $purpose: description of transaction
 * @param numeric $amount: transaction amount
 * @param bool $dups: (RETURNED) set TRUE if this category has at least two transactions in it
 */
function addCat(&$cats, $purpose, $amount, &$dups) {
  if (mb_strpos($purpose, '(') === 0 and ($i = mb_strpos($purpose, ')')) and $i < mb_strlen($purpose) - 1) {
    $purpose = trim(mb_substr($purpose, $i + 1)); // remove reversal info, if any
  }
  if ($m = strpbrk($purpose, ':-;(/[{~#*') and $i = mb_strpos($purpose, $m) and $i < mb_strlen($purpose) - 1) {
    $cat = trim(mb_substr($purpose, 0, $i));
  } else $cat = $purpose;
  
  if ($one = nni($cats, $cat)) $dups = TRUE; else $one = array(0, 0);
  list ($oldCount, $oldAmount) = $one;
  $cats[$cat] = array($oldCount + 1, $oldAmount + $amount);
}

/**
 * Adjust the categories list before displaying.
 */
function finishCats(&$cats) {
  $otherInCount = $otherIn = $otherOutCount = $otherOut = 0;
  foreach ($cats as $cat => $one) if ($one[0] == 1 and !u\starts($cat, INFLATION_DESC)) { // show inflation in summary
    if ($one[1] < 0) {
      $otherOutCount++; $otherOut += $one[1];
    } else {$otherInCount++; $otherIn += $one[1];}
    
    unset($cats[$cat]);
  }
  
  ksort($cats);
  if ($otherInCount) $cats[t('other funds IN')] = array($otherInCount, $otherIn);
  if ($otherOutCount) $cats[t('other funds OUT')] = array($otherOutCount, $otherOut);
}
