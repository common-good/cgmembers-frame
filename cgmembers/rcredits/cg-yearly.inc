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

const EOL = "\r\n"; // must be two characters long (make sure download doesn't change it)
const STATE_CODE = '25'; // Massachusetts code, for combined Federal/State filing
const AMT_COUNT = 18; // number of amount fields (as of reporting year 2021)
const TCC = '90MD6'; // our TCC identifier assigned by the IRS

/**
 * Create 1099-B, K, NEC, or R reports for the IRS as a downloadable file to be uploaded to the IRS FIRE site.
 * Log in at fire.irs.gov (or fire.test.irs.gov)
 * @param string $type: 1099 type (B, K, NEC, MISC, R, INT, etc)
 * @param mixed $testMode: TRUE if testing, otherwise FALSE
 * @param assoc $extra: miscellaneous parameters
 *   string $ein, $nm, $addr, $phone: payer information (defaults to CG's)
 *   int $year: tax year (defaults to most recently completed calendar year)
 *   assoc $badNames: names reported incorrectly, indexed by uid (1099-B only)
 *   assoc $badTINs: EINs (no punctuation) reported incorrectly, indexed by uid (1099-B only)
 *   array $omit: uids to omit (for example to submit on time when missing 1 or 2 SSNs)
 * @param array $rows: payee data for 1099-NEC, FALSE for 1099-B
 *    each element of $rows is an assoc: [nm ein co addr qid amount],
 *    where addr is postal address, and qid='' if the payee is not on the server
 *
 * Examples:
 * - for 1099-K:
 *     f('y.do1099s', 'K', TRUE, $extra); // TRUE means <test>
 *
 * - for corrections:
       f('y.do1099s', 'K', FALSE, ray('year badNames badTINs', 2015, [23947972 => 'Miss Pelled'], [29274902 => '292039482']));
 *
 * - for 1099-NEC and 1099-R (get amounts from QBO and/or CG):
 *     Get lines for each contractor (or retiring employee for 1099-R) on production server with:
         foreach (ray('aka NEVABJ etc') as $qid) { // uppercase if 5 letters, these can also be numeric account IDs
           $a = a($qid); $co = $a->co ? 1 : 0;
           e cho "ray(\$keys, '$a->legalName', '$a->federalId', '$co', '$a->postalAddr', '$a->mainQid', AMT, 0),<" . 'br>';
         }
     
 *     For 1099-NEC edit just AMT. For 1099-R edit AMT, 0 (amount and date)
 *     Paste in the PHP box on a DEV machine to generate file to upload to fire.irs.gov:
 
       include R_ROOT . '/cg-yearly.inc';
       $extra = NULL; // for CG itself
       $extra = ray('ein nm addr phone', '043125436', 'Creative Thought and Action Inc', 'PO Box 305, Ashfield, MA 01330', '4136281957');
       $keys = 'nm ein co addr qid amount dt';
       $rows = [
         ray($keys, nm, ein, co, addr, acct, amount, dt), // (for CG get these lines from the production server -- see above)
       ];
       r\Special\do1099s('NEC', TRUE, $extra, $rows); // TRUE means <test>
 */
 
function do1099s($type, $testMode = FALSE, $extra = [], $rows = FALSE) {
  global $err1099;
  global $stateCode; $stateCode = $type == 'NEC' ? sp(2) : STATE_CODE; // combined state reporting is appropriate for everything but NEC
  
  extract(just('ein year badNames badTINs omit', $extra ?: [], [NULL, date('Y') - 1, [], [], [] ]));
  $testMode = $testMode ? 'T' : '';

  if (!in($type, array_keys(ray(TEN99TYPES)))) return u\FAIL(t('Bad 1099 type (must be an index of %TEN99TYPES): %type', compact('type')));
  $flnm = (($ein and $ein != CGF_EIN) ? t('guest-') : '') . "Y$year$testMode-forms1099$type.txt";
  u\beginDownload($flnm); // binary file extension: discourage automatic changes during FTP
  putRec(recT($year, $testMode));
  
  $uids = array_keys($badNames + $badTINs);
  
  if ($uids = join(',', $uids ?: [])) { // making :uids a parameter fails 
    $where = "uid IN ($uids)";
  } elseif ($omit) {
    $where = tr('activated AND uid NOT IN (%uids)', 'uids', join(',', $omit));
  } else $where = 'activated';
  
  $sql = $rows ?: "SELECT uid FROM users WHERE $where ORDER BY uid";

  if ($uids) { // show bad old data first if correcting
    doPayer($type, $year, $sql, $totals, 'G', $extra);
    $bCount = doPayer($type, $year, $sql, $totals, 'C', $extra); // corrected data
  } else $bCount = doPayer($type, $year, $sql, $totals, '', $extra);
  
  $aCount = $uids ? 2 : 1;
  if ($stateCode == STATE_CODE) putRec(recK($type, $bCount * $aCount, $totals)); // recK is for state totals, so omit if not reporting to the state
  putRec(recF($bCount * $aCount, $aCount));
  if (test()) return;
  flush();
  if (nn($err1099)) return w\go('sadmin/panel'); else exit(); // shouldn't we exit in any case?
}

/**
 * Handle all appropriate payees for the payer (that is the Common Good regional server).
 * @param string $type: 1099 type
 * @param int $year: year to report
 * @param mixed $sql: selection SQL for 1099-B payee records OR array of rows for 1099-MISC or 1099-NEC or 1099-R
 * @param array $totals: (MODIFIED) totals
 * @param char $mod: G=original data, C=corrected data, ''=first time (not correcting anything)
 * @return the number of payee ("B") records.
 */
function doPayer($type, $year, $sql, &$totals, $mod, $extra) {
  global $seq1099;
  
  extract(just('badNames badTINs', $extra, []));
  
  $manual = in($type, 'MISC NEC R'); // data for these is collected manually
  if (is_array($rows = $sql) xor $manual) {
    err1099(t('Rows of data should be passed explicitly for 1099 type "%type", otherwise pass an SQL selection query.', compact('type')));
    return 0;
  }

  putRec(recA($type, $year, $extra));
  $bCount = 0; // IRS instructions pub 1220 call this "number of payees", but say it means "number of B records"
  $total = array_fill(0, AMT_COUNT, 0);
  
  if ($manual) { // MISC, NEC, or R
    $amtCodes = str_split(ray(TEN99AMTS)[$type]);
    foreach ($rows as $row) {
      extract(just('amount dt', $row, NULL));
      $amtRay = array_fill(0, AMT_COUNT, 0);
      foreach ($amtCodes as $i) {
        $i -= 1; // because arrays start at 0
        $amtRay[$i] = $amount;
        $total[$i] = round($total[$i] + $amount, 2);
      }
      putRec(recB($type, $year, $row, $amtRay, $dt));
      $bCount++;
    }
  } else { // B or K
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
  
  $totals = ''; foreach ($total as $tot) $totals .= fmtAmt($tot, 18);
  putRec(recC($bCount, $totals));
  return $bCount;
}

/**
 * Create B records for each income transaction (1099-B) or each monthly total (1099-K), for each payee.
 * @param string $type: 1099 type
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
    if ($type == 'B') {
      extract(just('created amount for', $row));
      $txCount = '';
      if ($amount <= 0) continue; // IRS says do NOT report non-positive transactions on 1099-B
      if ($mod == 'G') $amount = 0; // IRS says zero out amount when showing the bad old data
      $total[6] = round($total[6] + $amount, 2);
      //    $amts99[] = $amount;
    }
    if ($type == 'K') {
      extract(just('gross noCard st txCount', $row));
      if (r\no1099k($gross)) continue;
      $created = $for = '';
      $amount = []; // initialize for each payee
      $amount[] = $gross;
      $amount[] = $noCard;
      $amount[] = 0; // unused
      $amount[] = 0; // withheld
      for ($i = 1; $i <= 12; $i++) $amount[] = $row["amt$i"]; // amount for each month
      for ($i = count($amount); $i < AMT_COUNT; $i++) $amount[] = 0;
      foreach ($amount as $i => $v) $total[$i] = round($total[$i] + $v, 2);
    }
    
    $bCount++;
    $a = r\acct($payee); $id = $a->id;
    if (!$a->federalId) {
      err1099($a->fullName . t(' does not have a social security number.'));
      return 0; // might be better to just skip it and send admin a message (to get most of the reporting done)
    }
    $acctFields = ray('nm qid co addr ein txCount', u\asciiOnly($a->legalName), $a->mainQid, $a->co, u\asciiOnly($a->postalAddr), $a->federalId, $txCount);
    putRec(recB($type, $year, $acctFields, $amount, $created, $for, $mod, nni($badNames, $id), nni($badTINs, $id)));
  }
  return $bCount;
}

function recT($year, $testMode) {
  //$region = r\acct(r\serverUid());
  $prior = pad(($year < date('Y') - 1) ? 'P' : '', 1); // P for prior year data, else ' '
  $TCC = TCC;
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
 * @param string $type: 1099 type (MISC and NEC are currently for CG itself, B and K are for the region)
 * @param int $year: year to report
 * @param assoc $extra: payer parameters (defaults to CG's) -- ein, nm, addr, phone
 */
function recA($type, $year, $extra = []) {
  global $stateCode;
  
  extract(just('ein nm addr phone', $extra, [CGF_EIN, CGF_LEGALNAME, CGF_POSTALADDR, CGF_PHONE]));

  $forCG = in($type, 'MISC NEC R');
  $combo = pad($stateCode == STATE_CODE, 1); // combined Federal/State filing (for anything but NEC)
  
  $regionTIN = $forCG ? $ein : str_replace('-', '', R_REGION_EIN);
  $nameControl = sp(4);
  $lastEver = pad(FALSE, 1);
  
  $amountCodes = pad(ray(TEN99AMTS)[$type], 16);
  $typeCode = pad(ray(TEN99TYPES)[$type], 2);
  
  $foreign = pad(FALSE, 1); // 1 for foreign
  $payerName = pad($forCG ? $nm : r\regionfield('legalName'), 80);
  $isAgent = u\lpad(0, 1); // 1 if second half of payerName is transfer agent, else 0

  list ($addrLine, $city, $st, $zip) = u\parseAddr($forCG ? $addr : 'c/o Common Good, PO Box 21, Ashfield, MA 013300021');
  $payerAddr = pad($addrLine, 40) . pad($city, 40) . $st . pad(nodash($zip), 9);
  $payerPhone = pad($forCG ? u\fmtPhone($phone, 'n') : CGF_PHONE, 15); // was 4133765021 for the Region (maybe again someday)
  
  return 'A' . $year . $combo . sp(5) . $regionTIN . $nameControl . $lastEver . $typeCode . $amountCodes . sp(8) . $foreign . $payerName . $isAgent . $payerAddr . $payerPhone . sp(260) . seq() . sp(241) . EOL;
}

/**
 * Return a payee ("B") record for the given year, account, amount, and purpose.
 * @param string $mod: normally ''. C=show corrected info. G=show incorrect name and/or TIN (as submitted before)
 */
function recB($type, $year, $acctFields, $amount, $completed = '', $for = '', $mod = '', $badName = '', $badTIN = '') {
  global $stateCode;
  
  extract(just('nm qid co addr ein txCount', $acctFields));
  $who = "$nm ($qid)";
  if (!$badName and !$badTIN) {
    if ($mod == 'G') return err1099(t('Original B Record has no error to be corrected: ') . $who);
  } elseif ($mod != 'G') if ($badName or $badTIN) return err1099(t('Spurious badName (%badName) or badTIN (%badTIN) for payee %who.', compact('badName', 'badTIN', 'who')));
  $corrected = pad($mod, 1);
  $tinType = pad($co ? 1 : 2, 1);
  $payeeTIN = pad($badTIN ?: nodash($ein), 9);
  $accountNum = pad($qid, 20);
  $payerOffice = pad('', 4);
  $foreignPayee = pad('', 1);
  $payeeName = pad(noBreakAt(preg_replace('/[^A-Z &\-]/i', '', u\asciiOnly($badName ?: $nm)), 40), 2 * 40);
  list ($payeeFirst, $payeeLast) = u\parseName($payeeName);
  $nameControl = pad(preg_replace('/[^A-Z]/i', '', $co ? $payeeName : ($payeeLast . $payeeFirst)), 4);
//  $nameControl = pad(preg_replace('/[^A-Z]/i', '', ($co and $mod != 'G') ? $payeeName : $payeeLast), 4); // temporary for TY2014,2015
  list ($address, $city, $st, $zip) = u\parseAddr(u\asciiOnly($addr));
  if ($st == 'ER' or !$zip or !$city) return err1099(t('Bad postalAddr for %who: %addr', compact(ray('who addr'))));
  $payeeAddr = pad($address, 40) . sp(40) . pad($city, 40) . pad($st, 2) . pad(u\digits($zip), 9);

  if (count($amount) != AMT_COUNT) return err1099(t('There must be %amt amount fields. You have only %cnt.', 'amt cnt', AMT_COUNT, count($amount)));
  $amounts = ''; foreach ($amount as $amt) $amounts .= fmtAmt($amt, 12);

  $twoBadTINs = sp(1); // "2" if IRS griped twice within 3 calendar years that the payee provided a bad name and/or TIN. Otherwise blank.
  $withheld = u\lpad(0, 12) . u\lpad(0, 12); // state and local tax withheld (none)
  $date = pad($completed ? fmtDt($completed, 'Ymd') : '', 8);

  if ($type == 'B') { // might be out-of-date as of 2020
//    $amt = fmtAmt($amount, 12);
//    $amounts = u\lpad(0, 6 * 12) . $amt . u\lpad(0, 9 * 12);
        $desc = pad($for, 39);
    $qty = u\lpad(0, 13);
    $symbol = pad(0, 13);
    $tail = sp(4) . $date . sp(13) . $desc . sp(8) . sp(1) . $qty . $symbol . sp(20) . sp(60) . $withheld . $stateCode . EOL;
  } elseif ($type == 'K') {
    $filerType = '1'; // 1=Payment Settlement Entity (PSE)
    $pmtType = '1'; // payment card (vs 2=network) -- NOTE: amount field #2 distinguishes when card is not present
    $txCount = u\lpad($txCount ?: 0, 13);
    $otherPSE = sp(40); // other PSE name and phone
    $mcc = pad(0, 4); // merchant category code (not yet used)
    $special = sp(60);
    $withheld = u\lpad(0, 12) . u\lpad(0, 12);
    $tail = $twoBadTINs . sp(2) . $filerType . $pmtType . $txCount . sp(3) . $otherPSE . $mcc . sp(54) . $special . $withheld . $stateCode . EOL;
  } elseif ($type == 'NEC') {
    $directSales = sp(1);
    $tail = $twoBadTINs . sp(2) . $directSales . sp(175) . $withheld . $stateCode . EOL;
  } elseif ($type == 'R') {
    $distCode = pad(7, 2);
    $totalDist = 1;
    $fatca = sp(1); // FATCA requirement (no): we do not have more than $50k in foreign assets and we are not an individual
    $tail = $twoBadTINs . $distCode . sp(2) . $totalDist . sp(2) . sp(4) . $fatca . $date . sp(98) . sp(60) . $withheld . $stateCode . EOL;
  } else u\FAIL('bad type: ' . $type); // MISC is NYI, but is similar to NEC)
  
  return strtoupper('B' . $year . $corrected . $nameControl . $tinType . $payeeTIN . $accountNum . $payerOffice . sp(10) . $amounts . sp(16) . $foreignPayee . $payeeName .  $payeeAddr .  sp(1) . seq() . sp(36) . $tail);
}

function recC($bCount, $totals) { 
  return 'C' . u\lpad($bCount, 8) . sp(6) . $totals . sp(160) . seq() . sp(241) . EOL;
}
  
function recK($type, $bCount, $totals) {
  global $stateCode;

  $withheld = u\lpad(0, 18) . u\lpad(0, 18);
  return 'K' . u\lpad($bCount, 8) . sp(6) . $totals . sp(160) . seq() . sp(199) . $withheld . sp(4) . $stateCode . EOL; // include state (not sp(2)) even for NEC, even though not accepted in MA (IRS rejects it otherwise)
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
  $recType = substr($rec, 0, 1);
  $reclen = strlen($rec);
  $ideal = 750;
  if (strlen($rec) != $ideal) return err1099("ERROR: $recType record #$seq1099 is $reclen chars -- should be $ideal:\n" . breakup($rec));
  return $rec;
}

function err1099($msg) {
  global $err1099;
  \drupal_set_message($msg);
  $err1099 = TRUE;
/**/ echo  "\n\n********* ERROR *************\n\n"; // mark spot in file
/**/ echo $msg;
  return '';
}

/**
 * Check the record and write it to the output file.
 * @param string $rec: the record
 */
function putRec($rec) {
  global $err1099; if (nn($err1099)) return;
  global $testRecs; 
  
  $res = lenCheck($rec);
  if (test()) return $testRecs[] = $res;
/**/  echo $res;
}


/**
 * Return the next record sequence number, formatted.
 */
function seq() {
  global $seq1099;
  return $seq1099 = u\lpad(nn($seq1099) ? $seq1099 + 1 : 1, 8);
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
