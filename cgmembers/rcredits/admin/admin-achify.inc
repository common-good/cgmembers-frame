<?php
namespace CG\Web;
use CG as r;
use CG\Util as u;
use CG\DB as db;

// formats for records in a the NACHA file
const F_FILEHDR = 'recType:R1, priority:R2, sp:L1, destNum:R9, originPrefix:L1, originNum:R9, datetime:R10, fileIdModifier:R1, recSize:R3, blocking:R2, format:R1, destName:L23, originName:L23, ref:L8';
const F_BATCHHDR = 'recType:R1, class:R3, originName:L16, data:L20, originPrefix:L1, originNum:R9, secCode:L3, purpose:L10, when:L6, entryDate:R6, settle:L3, status:R1, destNum:R8, batchNum:R7';
const F_PPD = 'recType:R1, txCode:R2, routing:R9, account:L17, amount:R10, id:L15, name:L22, data:L2, addendaFlag:R1, destNum:R8, count:R7'; // Prearranged Payment and Deposit (entry detail)
const F_BATCHFTR = 'recType:R1, class:R3, count:R6, hash:R10, debits:R12, credits:R12, originPrefix:L1, originNum:R9, auth:L19, reserve:L6, destNum:R8, batchNum:R7';
const F_FILEFTR = 'recType:R1, batches:R6, blocks:R6, entries:R8, hash:R10, debits:R12, credits:R12, reserve:L39';

const BANK_DATA = [ // our bank's name (<=23chars), routing number, prefix ("_" means blank), same-day cutoff time, maximum ACH total
  'BSL'      => 'Brattleboro S&L, 211672476, _, 3pm, 40000',
  'Citizens' => 'Citizens Bank, 011500120, 1, 11:59pm, 9999999',
];
const BANK = 'BSL'; // which bank we're using (index into BANK_DATA)

/**
 * Create an ACH request file (debits only, for now).
 */
function achify($where = 'deposit=0', $mark = FALSE, $bank = BANK) {
  global $mya;
  
  if (!$bank) $bank = BANK; // links give us '' instead of omitted
  list ($destName, $destNum, $originPrefix, $cutoff, $maxAch) = explode(', ', BANK_DATA[$bank]); // not ray() because of colons in the data
  $sp = ' '; // filler for routing number in header
  $originPrefix = str_replace('_', ' ', $originPrefix); // _ was for visibility
  $reserve = ''; // leave reserved fields empty
  $count = $hash = $credits = $debits = 0;

  list ($recType, $priority, $recSize, $blocking, $format) = [1, 1, 94, 10, 1];
  $fileIdModifier = 'A'; // distinguishes files on the same date and between the same participants (A-Z, 0-9); never send two files on the same day, so this can be static
  $datetime = strftime('%y%m%d%H%M');
  list ($originNum, $originName, $ref) = [CGF_EIN, PROJECT, ''];
  
  $recs[] = achRecord(compact(array_keys($ray = ray(F_FILEHDR))), $ray);
  
  $destNum = substr($destNum, 0, 8); // omit last digit (check digit) of routing number from here on (for BATCHHDR, PPD, and BATCHFTR)
  list ($recType, $class, $secCode, $settle, $status) = [5, 200, 'PPD', '', 1]; // 200=mixed (maybe), 225=debits, 220=credits
  $entryDate = strftime('%y%m%d', u\nextBizDay(now() < strtotime($cutoff) ? today() : tomorrow()));
  
  $batchNum = 1; // batch (company) number within file
  list ($data, $purpose, $when) = ['', t('CG Credit'), strftime('%b%d')];

  $recs[] = achRecord(compact(array_keys($ray = ray(F_BATCHHDR))), $ray);

  $sql = <<< X
    SELECT txid,payee,amount,deposit,bankAccount,isSavings,pid FROM txs2 
    WHERE ($where) AND ABS(amount)<$maxAch ORDER BY amount>0, ABS(amount) DESC, txid
X;
  $q = db\q($sql);
  $tot = 0;

  $DBTX = \db_transaction();
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($tot + abs($amount) > $maxAch) continue;
    $tot += abs($amount);
    $count++;
    
    if ($payee == UID_OUTER) {
      $name = db\get('fullName', 'people', compact('pid'));
      $id = $pid;
    } else { // member account transfer
      $a = r\acct($payee);
      $name = u\asciiOnly($a->legalName);
      $id = $a->mainQid;
    }

    $name = u\asciiOnly($name);
    list ($recType, $addendaFlag) = [6, 0];
    if (!$bankAccount = u\decry('V', $bankAccount)) u\FAIL('bad bank account in txs2: ' . pr($row));
    list ($routing, $account) = u\parseBankAccount($bankAccount);
    if (isDEV and !$routing) $routing = $account = '211672476';
    
    $amount *= 100;
    if ($amount <= 0) $credits -= $amount; else $debits += $amount;
    $txCode = $amount == 0 ? 23 
    : ($isSavings ? ($amount < 0 ? 32 : 37) : ($amount < 0 ? 22 : 27));
    $amount = abs($amount);

    $recs[] = achRecord(compact(array_keys($ray = ray(F_PPD))), $ray);

    $hash += substr($routing, 0, 8);
    if (!$deposit and $mark and $mya->superAdmin) db\update('txs2_bank', ray('deposit txid', today(), $txid), 'txid');
  }
  unset($DBTX);
  
  if (!$count) return 0;
  
  list ($recType, $auth) = [8, ''];
  $recs[] = achRecord(compact(array_keys($ray = ray(F_BATCHFTR))), $ray);

  list ($recType, $batches, $entries) = [9, 1, $count];
  $blocks = ceil((2 + $count + 2) / 10); // how many blocks of 10 lines, rounded up (2 hdrs, "count" entries, 2 ftrs)
  $recs[] = achRecord(compact(array_keys($ray = ray(F_FILEFTR))), $ray);

  foreach ($recs as $line) if (strlen($line) != $recSize) u\FAIL('bad ach line len: ' . strlen($line) . " - $line");

///    if (isDEV) debug($recs);
  header('Content-type: text/plain');
  header(tr('Content-Disposition: attachment; filename=%bank-20%datetime.ach', compact(ray('bank datetime')))); // use full year (unlike in NACHA records)
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
    $res .= $left 
    ? substr(str_pad($v, $len), 0, $len)
    : u\lpad(strval((int) round($v)), $len);
  }
  return $res;
}
