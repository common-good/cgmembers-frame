<?php
namespace CG\Web;
use CG as r;
use CG\Util as u;
use CG\DB as db;

define('F_FILEHDR', 'recType:R1, priority:R2, destNum:R10, originNum:R10, datetime:R10, fileIdModifier:R1, recSize:R3, blocking:R2, format:R1, destName:L23, originName:L23, ref:L8'); // header format
define('F_BATCHHDR', 'recType:R1, class:R3, originName:L16, data:L20, originNum:R10, secCode:L3, purpose:L10, when:L6, entryDate:R6, settle:L3, status:R1, destNum:R8, batchNum:R7'); // batch header format
define('F_PPD', 'recType:R1, txCode:R2, routing:R9, account:L17, amount:R10, id:L15, name:L22, data:L2, addendaFlag:R1, destNum:R8, count:R7'); // PPD (Prearranged Payment and Deposit) entry detail format
define('F_BATCHFTR', 'recType:R1, class:R3, count:R6, hash:R10, debits:R12, credits:R12, originNum:R10, auth:L19, reserve:L6, destNum:R8, batchNum:R7'); // end of batch (control record)
define('F_FILEFTR', 'recType:R1, batches:R6, blocks:R6, entries:R8, hash:R10, debits:R12, credits:R12, reserve:L39'); // end of file (control record)
define('DEST_NUM', '011500120'); // usually a routing number for the bank providing ACH service

/**
 * Create an ACH request file (debits only, for now).
 */
function achify($where = 'deposit=0') {
	$CGCODE = 'ASOBENE1'; // company code assigned by Citizens Bank for upload filename
  $reserve = ''; // leave reserved fields empty
  $count = $hash = $credits = $debits = 0;

  list ($recType, $priority, $recSize, $blocking, $format) = [1, 1, 94, 10, 1];
  $fileIdModifier = 'A'; // distinguishes files on the same date and between the same participants (A-Z, 0-9); never send two files on the same day, so this can be static
  list ($destNum, $destName) = [' ' . DEST_NUM, 'Citizens Bank'];
  $datetime = strftime('%y%m%d%H%M');
  list ($originNum, $originName, $ref) = ['1' . CGF_EIN, PROJECT, ''];
  $recs[] = achRecord(compact(array_keys($ray = ray(F_FILEHDR))), $ray);
  
  $destNum = substr(DEST_NUM, 0, 8); // BATCHHDR, PPD, and BATCHFTR omit last digit (check digit) of routing number
  list ($recType, $class, $secCode, $settle, $status) = [5, 200, 'PPD', '', 1]; // 200=mixed (maybe), 225=debits, 220=credits
  $entryDate = strftime('%y%m%d', strtotime('tomorrow'));
  $batchNum = 1; // batch (company) number within file
  list ($data, $purpose, $when) = ['', t('CG Credits'), strftime('%b%d')];
  $recs[] = achRecord(compact(array_keys($ray = ray(F_BATCHHDR))), $ray);

  $q = db\q("SELECT txid,payee,amount,deposit FROM r_usd WHERE $where ORDER BY txid");
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($payee);
    $bankAccountType = 'checking'; // we handle only checking accounts at this time
    if ($a->mainQid == 'NEWAFX') $bankAccountType = 'savings';
    $count++;
    list ($recType, $id, $name, $addendaFlag) = [6, $a->mainQid, $a->legalName, 0];
    list ($routing, $account) = u\parseBankAccount($a->bankAccount);
		
		$amount *= 100;
    if ($amount <= 0) $credits -= $amount; else $debits += $amount;
    $txCode = $amount == 0 ? 23 
    : (@$bankAccountType == 'checking' ? ($amount < 0 ? 22 : 27) : ($amount < 0 ? 32 : 37));
		$amount = abs($amount);

    $recs[] = achRecord(compact(array_keys($ray = ray(F_PPD))), $ray);

    $hash += substr($routing, 0, 8);
    if (!$deposit) db\update('r_usd', ray('deposit txid', strtotime('today'), $txid), 'txid');
  }
  
  if (!$count) return 0;
  
  list ($recType, $auth) = [8, ''];
  $recs[] = achRecord(compact(array_keys($ray = ray(F_BATCHFTR))), $ray);

  list ($recType, $batches, $entries) = [9, 1, $count];
  $blocks = ceil((2 + $count + 2) / 10); // how many blocks of 10 lines, rounded up (2 hdrs, "count" entries, 2 ftrs)
  $recs[] = achRecord(compact(array_keys($ray = ray(F_FILEFTR))), $ray);

/**/  foreach ($recs as $line) if (strlen($line) != $recSize) debug('bad ach line len: ' . strlen($line) . " - $line");

  $datetime = strftime('%Y%m%d%H%M'); // use full year (unlike ach record format)
///    if (isDEV) debug($recs);
  header('Content-type: text/plain');
  header("Content-Disposition: attachment; filename=$CGCODE.ach-$datetime.txt");
/**/  echo join("\n", $recs);
  exit();
}

/**
 * Return an ACH record for the given data and format.
 * @param assoc $values: field values, in order
 * @param assoc $jlens: field justifications (R or L) and lengths, in order
 * @return the field values, justified / filled / truncated appropriately, then concatenated.
 */
function achRecord($values, $jlens) {
  $res = '';
  foreach ($jlens as $k => $jlen) {
    list ($left, $len) = [$jlen[0] == 'L', substr($jlen, 1)];
    $v = $values[$k];
    $res .= $left ? pad($v, $len) : padnum($v, $len);
  }
  return $res;
}

function pad($s, $len) {return substr(str_pad($s, $len), 0, $len);}
function padnum($n, $len) {return u\lpad(str_replace(',', '', number_format((int) $n, 0)), $len);}
