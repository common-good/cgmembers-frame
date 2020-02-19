<?php
/**
 * @file
 * Functions that get run just once a year.
 * Assume that Common Good Finance is the report transmitter.
 */

namespace CG\Special; // typically abbreviated as "y"
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Web as w;

define('EOL', "\r\n"); // must be two characters long (make sure download doesn't change it)
define('STATE_CODE', '25'); // Massachusetts code, for combined Federal/State filing

/**
 * Create 1099-B or 1099-MISC reports for the IRS as a downloadable file to be uploaded to the IRS FIRE site.
 * Log in at fire.irs.gov (or fire.test.irs.gov)
 * @param string $type: A=1099-MISC, B=1099-B, MC=1099-K
 * @param mixed $testMode: TRUE if testing, otherwise FALSE
 * @param array $rows: payee data for 1099-MISC, FALSE for 1099-B
 *    each element of $rows is an assoc: [nm ein co addr qid amount],
 *    where addr is postal address, and qid='' if the payee is not on the server
 * @param assoc $badNames: names reported incorrectly, indexed by uid (1099-B only)
 * @param assoc $badTINs: EINs (no punctuation) reported incorrectly, indexed by uid (1099-B only)
 * @param int $year: reporting year (defaults to most recent completed calendar year) (1099-B only)
 * example for corrections: f('y.do1099bs', FALSE, [23947972 => 'Miss Pelled'], [29274902 => '292039482'], 2015);
 *
 * Example for 1099-MISC (paste in the PHP box on a development machine, to generate):
 * 
    include __DIR__ . '/../cg-yearly.inc';
    $keys = 'nm ein co addr qid amount';
    $rows = [
      ray($keys, 'Creative Thought dba Computer Practitioner', '987654321', 0, 'PO Box 305, Ashfield, MA 01330', 'NEWAKA', '18000'),
      ray($keys, 'Marilyn Distelberg', '987654321', 0, '179 Hillwinds North, Brattleboro, VT 05301', 'NEVABJ', '18000'),
    ];
    r\Special\do1099s('MISC', TRUE, $rows);
 *
 * Get lines on production server with:
     foreach (ray('aka nevabj etc') as $qid) {
       $a = a($qid); $co = $a->co ? 1 : 0;
       echo "ray(\$keys, '$a->legalName', '$a->federalId', '$co', '$a->postalAddr', '$a->mainQid', 'amt'),<" . 'br>';
     }
 */
function do1099s($type, $testMode = FALSE, $rows = FALSE, $badNames = [], $badTINs = [], $year = NULL) {
  global $err1099;
  u\setDft($year, date('Y') - 1);
  $testMode = $testMode ? 'T' : '';
  u\beginDownload("forms1099$type-FY$year.bin$testMode"); // binary file extension: discourage automatic changes during FTP
  putRec(recT($year, $testMode));
  
  $uids = array_keys($badNames + $badTINs);
  
  if ($uids = join(',', $uids ?: [])) { // making :uids a parameter fails 
    $where = "uid IN ($uids)";
  } else $where = 'activated';
  
  $sql = $rows ?: "SELECT uid FROM users WHERE $where ORDER BY uid";

  if ($uids) { // show bad old data first if correcting
    doPayer($type, $year, $sql, $totals, 'G', $badNames, $badTINs);
    $bCount = doPayer($type, $year, $sql, $totals, 'C'); // corrected data
  } else $bCount = doPayer($type, $year, $sql, $totals);
  
  $aCount = $uids ? 2 : 1;
  putRec(recK($bCount * $aCount, $totals));
  putRec(recF($bCount * $aCount, $aCount));
  if (test()) return;
  flush();
  if (@$err1099) return w\go('sadmin/panel'); else exit(); // shouldn't we exit in any case?
}

/**
 * Handle all appropriate payees for the payer (that is the Common Good regional server).
 * @param string $type: A=1099-MISC, B=1099-B, MC=1099-K
 * @param int $year: year to report
 * @param mixed $sql: selection SQL for 1099-B payee records OR array of rows for 1099-MISC
 * @return the number of payee ("B") records.
 */
function doPayer($type, $year, $sql, &$totals, $mod = '', $badNames = [], $badTINs = []) {
  global $seq1099;
  
  if (is_array($rows = $sql) xor $type == TEN99MISC) {
    err1099(t('Rows of data should be passed explicitly for 1099-MISC, otherwise pass an SQL selection query.'));
    return 0;
  }
  putRec(recA($type, $year));
  $bCount = 0; // IRS instructions pub 1220 call this "number of payees", but say it means "number of B records"
  $total = array_fill(0, 16, 0);
  
  if ($type == TEN99MISC) { // 1099-MISC
    foreach ($rows as $row) {
      extract(just('amount', $row));
      $amtRay = array_fill(0, 16, 0);
      $amtRay[6] = $amount;
      putRec(recB($year, $row, $amtRay));
      $total[6] = round($total[6] + $amount, 2);
      $bCount++;
    }
  } else { // 1099-B or 1099-K
    $q = db\q($sql);
    while ($row = $q->fetchAssoc()) {
      extract($row); // uid
      $bCount += do1099($type, $year, $uid, $total, $mod, $badNames, $badTINs);
      $bFactor = ($badNames or $badTINs) ? 2 : 1; // two records for each payee, if correcting
      if ($seq1099 + 0 != $bCount * $bFactor + 2) {
        err1099(t('Record count %rc at odds with payee count %pc.', 'rc pc', $seq1099, $bCount));
        return $bCount;
      }
    }
  }
  
  $totals = '';
  foreach ($total as $tot) $totals .= fmtAmt($tot, 18);
//  $totals = u\lpad(0, 6 * 18) . fmtAmt($total, 18) . u\lpad(0, 9 * 18);
  putRec(recC($bCount, $totals));
  return $bCount;
}

/**
 * Create B records for each income transaction, for each payee.
 * @param string $type: B=1099-B A=1099-MISC MC=1099-K
 * @param int $year: the relevant fiscal year
 * @param int $payee: payee's account ID
 * @param float $total: array of running totals for all B records so far
 * @param string $mod: normally ''. C=show corrected info. G=show incorrect name and/or TIN (as submitted before)
 * @param assoc $badNames: list of incorrectly reported names, indexed by uid
 * @param assoc $badTINs: list of incorrectly reported TINs, indexed by uid
 * @return number of B records created
 */
function do1099($type, $year, $payee, &$total, $mod = '', $badNames = [], $badTINs = []) {
//  global $amts99;
  
  $bCount = 0;
  while ($row = be\get1099($type, $payee, $year)) {
    if ($type == TEN99B) {
      extract(just('created amount for', $row));
      $txCount = '';
      if ($amount <= 0) continue; // IRS says do NOT report non-positive transactions on 1099-B
      if ($mod == 'G') $amount = 0; // IRS says zero out amount when showing the bad old data
      $total[6] = round($total[6] + $amount, 2);
      //    $amts99[] = $amount;
    }
    if ($type == TEN99K) {
      extract(just('gross noCard st txCount', $row));
      if (r\no1099k($gross, $txCount, $st)) continue;
      $created = $for = '';
      $amount = [];
      $amount[] = $gross;
      $amount[] = $noCard;
      $amount[] = 0; // unused
      $amount[] = 0; // withheld
      for ($i = 1; $i <= 12; $i++) $amount[] = $row["amt$i"];
      foreach ($amount as $i => $v) $total[$i] = round($total[$i] + $v, 2);
    }
    
    $bCount++;
    $a = r\acct($payee); $id = $a->id;
    $acctFields = ray('nm qid co addr ein txCount', u\asciiOnly($a->legalNameDpy), $a->mainQid, $a->co, u\asciiOnly($a->postalAddr), $a->federalId, $txCount);
    putRec(recB($type, $year, $acctFields, $amount, $created, $for, $mod, @$badNames[$id], @$badTINs[$id]));
  }
  return $bCount;
}

function recT($year, $testMode) {
  //$region = r\acct(r\serverUid());
  $prior = pad(($year < date('Y') - 1) ? 'P' : '', 1); // P for prior year data, else ' '
  $TCC = '08C20';
  $testing = pad($testMode, 1);
  $foreign = pad(FALSE, 1); // 1 for foreign
  $coName = $transmitterName = pad(strtoupper(CGF_LEGALNAME), 80);
  list ($addr, $city, $st, $zip) = u\parseAddr(strtoupper(CGF_POSTALADDR));
  $coAddr = pad($addr, 40) . pad($city, 40) . $st . pad(nodash($zip), 9);
  $payees = u\lpad(0, 8); // not possible to know the count yet
  $contact = pad(strtoupper(ADMIN_NAME), 40) . pad(u\fmtPhone(R_ADMIN_PHONE,'n'), 15) . pad(R_ADMIN_EMAIL, 50);
  $source = pad('I', 1); // V=vendor I=in-house
  $vendor = sp(40) . sp(40) . sp(40) . sp(2) . sp(9);
  $vendorContact = sp(40) . sp(15);
  $foreignVendor = pad(FALSE, 1);
  
  return 'T' . $year . $prior . CGF_EIN . $TCC . sp(7) . $testing . $foreign . $transmitterName . $coName . $coAddr . sp(15) . $payees . $contact . sp(91) . seq() . sp(10) . $source . $vendor . $vendorContact . sp(35) . $foreignVendor . sp(8) . EOL;
}

/**
 * Create the payer record.
 * @param string $type: B=1099-B A=1099-MISC MC=1099-K
 * @param int $year: year to report
 */
function recA($type, $year) {
  $isMisc = ($type == TEN99MISC);
  $combo = pad(TRUE, 1); // combined Federal/State filing
  $regionTIN = $isMisc ? CGF_EIN : '461821792';
  $nameControl = sp(4);
  $lastEver = pad(FALSE, 1);
  $type = pad($type, 2);
  
  $amountCodes = pad($type == TEN99K ? '12456789ABCDEFG' : '7', 16); // coincidentally the same for 1099-MISC and 1099-B
  
  $foreign = pad(FALSE, 1); // 1 for foreign
  $payerName = pad($isMisc ? CGF_LEGALNAME : r\regionfield('legalName'), 80);
  $isAgent = u\lpad(0, 1); // 1 if second half of payerName is transfer agent, else 0

  list ($addr, $city, $st, $zip) = u\parseAddr($isMisc ? CGF_POSTALADDR : 'c/o Common Good, PO Box 21, Ashfield, MA 013300021');
  $payerAddr = pad($addr, 40) . pad($city, 40) . $st . pad(nodash($zip), 9);
  $payerPhone = pad($isMisc ? u\fmtPhone(CGF_PHONE, 'n') : '4136281723', 15); // was 4133765021 (maybe again someday)
  
  return 'A' . $year . $combo . sp(5) . $regionTIN . $nameControl . $lastEver . $type . $amountCodes . sp(8) . $foreign . $payerName . $isAgent . $payerAddr . $payerPhone . sp(260) . seq() . sp(241) . EOL;
}

/**
 * Return a payee ("B") record for the given year, account, amount, and purpose.
 * @param string $mod: normally ''. C=show corrected info. G=show incorrect name and/or TIN (as submitted before)
 */
function recB($type, $year, $acctFields, $amount, $completed = '', $for = '', $mod = '', $badName = NULL, $badTIN = NULL) {
  extract(just('nm qid co addr ein txCount', $acctFields));
  $who = "$nm ($qid)";
  if (!isset($badName) and !isset($badTIN)) {
    if ($mod == 'G') return err1099(t('Original B Record has no error to be corrected: ') . $who);
  } elseif ($mod != 'G') if (isset($badName) or isset($badTIN)) return err1099(t('Spurious badName (%badName) or badTIN (%badTIN) for payee %who.', compact('badName', 'badTIN', 'who')));
  $corrected = pad($mod, 1);
  $tinType = pad($co ? 1 : 2, 1);
  $payeeTIN = pad(isset($badTIN) ? $badTIN : nodash($ein), 9);
  $accountNum = pad($qid, 20);
  $payerOffice = pad('', 4);
  $foreignPayee = pad('', 1);
  $payeeName = pad(noBreakAt(preg_replace('/[^A-Z &\-]/i', '', u\asciiOnly(isset($badName) ? $badName : $nm)), 40), 2 * 40);
  list ($payeeFirst, $payeeLast) = u\parseName($payeeName);
  $nameControl = pad(preg_replace('/[^A-Z]/i', '', $co ? $payeeName : ($payeeLast . $payeeFirst)), 4);
//  $nameControl = pad(preg_replace('/[^A-Z]/i', '', ($co and $mod != 'G') ? $payeeName : $payeeLast), 4); // temporary for TY2014,2015
  list ($address, $city, $st, $zip) = u\parseAddr(u\asciiOnly($addr));
  if ($st == 'ER' or !$zip or !$city) return err1099(t('Bad postalAddr for %who: %addr', compact(ray('who addr'))));
  $payeeAddr = pad($address, 40) . sp(40) . pad($city, 40) . pad($st, 2) . pad(u\digits($zip), 9);
  if (count($amount) != 16) return err1099(t('There must be 16 amount fields.'));
  $amounts = '';
  foreach ($amount as $amt) $amounts .= fmtAmt($amt, 12);

  if ($type == TEN99B) {
//    $amt = fmtAmt($amount, 12);
//    $amounts = u\lpad(0, 6 * 12) . $amt . u\lpad(0, 9 * 12);
    
    $date = pad($completed ? strftime('%Y%m%d', $completed) : '', 8);
    $desc = pad($for, 39);
    $qty = u\lpad(0, 13);
    $symbol = pad(0, 13);
    $withheld = u\lpad(0, 12) . u\lpad(0, 12);
    $tail = sp(4) . $date . sp(13) . $desc . sp(8) . sp(1) . $qty . $symbol . sp(20) . sp(60) . $withheld . STATE_CODE . EOL;
  }
  if ($type == TEN99K) { // 1099-K
    $twoBadTINs = sp(1); // "2" if IRS griped twice within 3 calendar years that the payee provided a bad name and/or TIN. Otherwise blank.
    $filerType = '1'; // 1=Payment Settlement Entity (PSE)
    $pmtType = '1'; // payment card (vs 2=network) -- NOTE: amount field #2 distinguishes when card is not present
    $txCount = u\lpad($txCount ?: 0, 13);
    $otherPSE = sp(40); // other PSE name and phone
    $mcc = pad(0, 4); // merchant category code (not yet used)
    $special = sp(60);
    $withheld = u\lpad(0, 12) . u\lpad(0, 12);
    $tail = $twoBadTINs . sp(2) . $filerType . $pmtType . $txCount . sp(3) . $otherPSE . $mcc . sp(54) . $special . $withheld . STATE_CODE . EOL;
  }
  
  return strtoupper('B' . $year . $corrected . $nameControl . $tinType . $payeeTIN . $accountNum . $payerOffice . sp(10) . $amounts . $foreignPayee . $payeeName . sp(40) . $payeeAddr .  sp(1) . seq() . sp(36) . $tail);
}

function recC($bCount, $totals) { 
  return 'C' . u\lpad($bCount, 8) . sp(6) . $totals . sp(196) . seq() . sp(241) . EOL;
}
  
function recK($bCount, $totals) {
  $withheld = u\lpad(0, 18) . u\lpad(0, 18);
  
  return 'K' . u\lpad($bCount, 8) . sp(6) . $totals . sp(196) . seq() . sp(199) . $withheld . sp(4) . STATE_CODE . EOL;
}

function recF($bCount, $aCount) {
  return 'F' . u\lpad($aCount, 8) . u\lpad(0, 21) . sp(19) . u\lpad($bCount, 8) . sp(442) . seq() . sp(241) . EOL;
}
    
/**
 * Make sure the string has a whole word at the end of the first chunk (typically for 40/40 names).
 * This is required for payee name.
 * @param string $s: the string to fit in two chunks
 * @param in $chunk: chunk size
 * @return the string, padded out so the second chunk starts on a word boundary (if possible)
 */
function noBreakAt($s, $chunk) {
  if (strlen($s) <= $chunk) return $s;
  $extra = 2 * $chunk - strlen($s);
  for ($i = $chunk; $i >= $chunk - $extra; $i--) {
    if (substr($s, $i, 1) == ' ') {
      return ($i < $chunk - 1) ? substr($s, 0, $i) . sp($chunk - $i) . substr($s, $i + 1) : $s;
    }
  }
  return $s;
}
 
/**
 * Show an error message if the record is the wrong length.
 * @param string $rec: the record to test
 * @return the record
 */ 
function lenCheck($rec) {
  global $seq1099;
  $type = substr($rec, 0, 1);
  $reclen = strlen($rec);
  $ideal = 750;
  if (strlen($rec) != $ideal) return err1099("ERROR: $type record #$seq1099 is $reclen chars -- should be $ideal:\n" . breakup($rec));
  return $rec;
}

function err1099($msg) {
  global $err1099;
  \drupal_set_message($msg);
  $err1099 = TRUE;
/**/ echo  "\n\n********* ERROR *************\n\n"; // mark spot in file
  return '';
}

/**
 * Check the record and write it to the output file.
 * @param string $rec: the record
 */
function putRec($rec) {
  global $err1099; if (@$err1099) return;
  global $testRecs1099; if (test()) $testRecs1099[] = $rec;

/**/  echo lenCheck($rec);
}


/**
 * Return the next record sequence number, formatted.
 */
function seq() {
  global $seq1099;
  return $seq1099 = u\lpad(@$seq1099 ? $seq1099 + 1 : 1, 8);
}

/**
 * Break a line up with "|" every 10 characters, for easier debugging.
 */
function breakup($s, $len = 10) {
  for ($i = 0, $new = ''; $i < strlen($s); $i += $len) {
    $new .= substr($s, $i, $len) . '|';
  }
  return str_replace(' ', '.', $new);
}

function pad($s, $n) {return substr(str_pad($s, $n), 0, $n);}
function sp($n) {return pad('', $n);}
function fmtAmt($n, $cnt) {return u\lpad(preg_replace('/[\,\.]/', '', number_format($n, 2)), $cnt);}
function nodash($n) {return str_replace('-', '', $n);}