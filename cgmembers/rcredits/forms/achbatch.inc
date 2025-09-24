<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

// formats for records in a the NACHA file
const F_FILEHDR = 'recType:R1, priority:R2, sp:L1, destNum:R9, originPrefix:L1, originNum:R9, datetime:R10, fileIdModifier:R1, recSize:R3, blocking:R2, format:R1, destName:L23, originName:L23, ref:L8';
const F_BATCHHDR = 'recType:R1, class:R3, originName:L16, data:L20, companyId:R10, secCode:L3, purpose:L10, when:L6, entryDate:R6, settle:L3, status:R1, destNum:R8, batchNum:R7';
const F_PPD = 'recType:R1, txCode:R2, routing:R9, account:L17, amount:R10, id:L15, name:L22, data:L2, addendaFlag:R1, destNum:R8, count:R7'; // Prearranged Payment and Deposit (entry detail)
const F_BATCHFTR = 'recType:R1, class:R3, count:R6, hash:R10, debits:R12, credits:R12, companyId:R10, auth:L19, reserve:L6, destNum:R8, batchNum:R7';
const F_FILEFTR = 'recType:R1, batches:R6, blocks:R6, entries:R8, hash:R10, debits:R12, credits:R12, reserve:L39';

/**
 * Create an ACH Batch file for all relevant transfer requests.
 * @params string $args: URL argument list including:
 *   date: 0 for pending transfers (the default), unixtime for a redo
 *   way: in or out (defaults to neither, meaning both) OR an amount to transfer from or (if <0) TO our "offshore" bank account
 *   mark: 1 means set deposit date to NOW, 0 (or omitted) means don't change anything
 *   bank: index to BANK_DATA array; for example, BSL or CFB (defaults to the default bank)
 */
function achBatch($args) {
  extract(just('date way mark bank balanced', $args, ''));

  u\setDft($date, 0);
  if (is_numeric($way)) {
    $offshore = $way;
    $way = ' AND FALSE';
    $date = now() + DAY_SECS; // make sure no deposit date qualifies
  } elseif ($way) $way = ' AND amount' . ($way == 'in' ? '>=0' : '<=0');

  $date = (int) $date; // prevent SQL injection
  achify("deposit=$date AND created<=:NOW" . $way, $mark, $bank ?: BANK, $balanced, nn($offshore)); // <=:NOW lets us verify with $0 transfers before a scheduled real one

  if (test()) return;
}

/**
 * Create an ACH request file.
 */
function achify($where = 'deposit=0', $mark = FALSE, $bank = BANK, $balanced = NULL, $offshore = '') {
  global $hash, $credits, $debits;
  global $testRecs;
  
  $b = r\bankData($bank);
  list ($destName, $destNum, $coName, $coIdPrefix, $originPrefix, $cutoff, $maxAchIn, $maxAchOut, $maxAchTotal) = [$b->name, $b->routing, $b->coName, $b->coIdPrefix, $b->achPrefix, $b->cutoff, $b->maxDailyAchIn, $b->maxDailyAchOut, $b->maxDailyAchTotal];
  $balanced = isset($balanced) ? $balanced : $b->balanced; // allow user to change from default
  $sp = ' '; // filler for routing number in header
  $originPrefix = str_replace('_', ' ', $originPrefix); // _ was for visibility (blank for browser upload, "1" for direct-from-program upload)
  $reserve = ''; // leave reserved fields empty
  $count = $hash = $credits = $debits = 0;

  list ($recType, $priority, $recSize, $blocking, $format) = [1, 1, 94, 10, 1];
  $fileIdModifier = 'A'; // distinguishes files on the same date and between the same participants (A-Z, 0-9); never send two files on the same day, so this can be static
  $datetime = fmtDt(now(), 'yyMMddHHmm');
  list ($originNum, $originName, $ref) = [CGF_EIN, $coName, ''];
  $companyId = $coIdPrefix . CGF_EIN;
  
  $recs[] = achRecord(compact(array_keys($ray = ray(F_FILEHDR))), $ray);
  
  $destNum = substr($destNum, 0, 8); // omit last digit (check digit) of routing number from here on (for BATCHHDR, PPD, and BATCHFTR)
  list ($recType, $class, $secCode, $settle, $status) = [5, 200, 'PPD', '', 1]; // 200=mixed (maybe), 225=debits, 220=credits

  $entryDate = fmtDt($entryDate0 = u\nextBizDay(u\nextBizTm() < strtotime($cutoff) ? today() : u\nextBizDay()), 'yyMMdd');
  
  $batchNum = 1; // batch (company) number within file
  list ($data, $purpose, $when) = ['', t('CG Credit'), fmtDt(now(), 'MMMdd')];

  $recs[] = achRecord(compact(array_keys($ray = ray(F_BATCHHDR))), $ray);

  // Collect pending transfers, outgoing first, largest first
  $sql = <<< X
    SELECT xid,payee,amount,deposit,bankAccount,isSavings,pid FROM txs2 
    WHERE ($where) AND ABS(amount)<=$maxAchTotal AND amount BETWEEN -$maxAchOut AND $maxAchIn
    AND bankAccount IS NOT NULL ORDER BY amount>0, ABS(amount) DESC, xid
X;
  $q = db\q($sql); // omit "outer" payments by check/wire/CC (anything but ACH, for which we have the bankAccount)
  $tot = 0;
  $depositDt = $mark ? now() : NULL; // a single unique time for each batch

  $DBTX = \db_transaction();
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($tot + abs($amount) > $maxAchTotal) continue;
    if ($amount < 0 ? ($credits - $amount > $maxAchOut) : ($debits + $amount > $maxAchIn)) continue;
    $tot += abs($amount);
    $count++;
    $recs[] = ppd($row, $destNum, $count, $depositDt);
  }

  unset($DBTX);
 
/* 
  if ($offshore) { // add a transfer to or from our "offshore" interest-bearing account
    $count++;
    $b2 = r\bankData(BANK_OFFSHORE);
    $txCode = $offshore < 0 ? 22 : 27; // NO: $offshore < 0 ? 32 : 37 (StacyLee says Money Market accounts are treated as checking)
    $amount = $offshore * 100;
    if ($amount <= 0) $credits -= $amount; else $debits += $amount;
    $amount = abs($amount);
    [$routing, $account, $id, $name] = [$b2->routing, $b2->acctIn, BANK_OFFSHORE, u\asciiOnly(PROJECT . ' @ ' . BANK_OFFSHORE)];
    $recs[] = achRecord(compact(array_keys($ray = ray(F_PPD))), $ray);
    $hash += substr($routing, 0, 8);
  }
*/
  if (!$count) return 0;
  
  $offset = $credits - $debits;
  if ($balanced and $offset != 0) { // no need for an offset entry if file is already balanced
    list ($recType, $data, $addendaFlag) = [6, '', 0];
    list ($txCode, $routing, $account, $amount, $id, $name) // offset record parameters
      = [$offset < 0 ? 22 : 27, $b->routing, $b->acctIn, abs($offset), t('CTTYFUND OFFSET'), $b->coName];
    $tot += $amount;
    $count++;
    if ($offset <= 0) $credits -= $offset; else $debits += $offset;
    $hash += substr($routing, 0, 8);
    $recs[] = achRecord(compact(array_keys($ray = ray(F_PPD))), $ray);
  }
  
  
  list ($recType, $auth) = [8, ''];
  $recs[] = achRecord(compact(array_keys($ray = ray(F_BATCHFTR))), $ray);

  list ($recType, $batches, $entries, $lines) = [9, 1, $count, 2 + $count + 2];
  $blocks = ceil($lines / 10); // how many blocks of 10 lines, rounded up (2 hdrs, "count" entries, 2 ftrs)
  $recs[] = achRecord(compact(array_keys($ray = ray(F_FILEFTR))), $ray);

  if ($balanced) for ($i = $lines; $i < $blocks * 10; $i++) $recs[] = str_repeat('9', $recSize); // fill

  foreach ($recs as $line) if (strlen($line) != $recSize) u\FAIL('bad ach line len: ' . strlen($line) . " - $line");

  $id = $entryDate0 . substr(r\bankData($bank)->acctIn, -4); // set bankTxId to effective date (10 digits), bank account number (last 4 digits)  
  if ($depositDt) db\q('UPDATE txs2 SET bankTxId=:id WHERE deposit=:dt', ray('id dt', $id, $depositDt));

  u\beginDownload(tr('%bank-20%datetime.ach', compact(ray('bank datetime')))); // use full year (unlike in NACHA records)

  if (test()) return $testRecs = $recs;
/**/  echo join("\n", $recs);
  exit();
}

/**
 * Process one transaction
 * @param assoc $row: the txs2 bank transfer request record
 * @param int $destNum: bank identifier as header for transaction number
 * @param int $count: batch file line count
 * $param bool $depositDt: date to mark the transfer request complete (or NULL)
 */
function ppd($row, $destNum, $count, $depositDt) {
  global $mya;
  global $hash, $credits, $debits;
  extract($row);

  list ($recType, $data, $addendaFlag) = [6, '', 0];

  if ($pid) {
    $name = db\get('fullName', 'people', compact('pid'));
    $id = $pid;
  } else { // member account transfer
    $a = r\acct($payee);
    $name = u\asciiOnly($a->achName);
    $id = $a->mainQid;
  }
  $id .= '-' . $xid;

  $name = u\asciiOnly($name);
  if (!$bankAccount = u\decry('V', $bankAccount)) {
    if (!$pid and $bankAccount = $a->bankAccount) {
      db\update('txs2', ray('xid bankAccount', $xid, u\cry('V', $bankAccount)), 'xid'); 
    } else u\FAIL('bad bank account in txs2: ' . pr($row));
  }
  list ($routing, $account) = u\parseBankAccount($bankAccount);
  if (isDEV and !$routing) $routing = $account = '211672476';
  
  $amount *= 100;
  if ($amount <= 0) $credits -= $amount; else $debits += $amount;
  $txCode = $amount == 0 ? 23 
  : ($isSavings ? ($amount < 0 ? 32 : 37) : ($amount < 0 ? 22 : 27));
  $amount = abs($amount);

  $hash += substr($routing, 0, 8);
  if (!$deposit and $depositDt and $mya->admAch) db\update('txs2', ray('deposit xid', $depositDt, $xid), 'xid');

  return achRecord(compact(array_keys($ray = ray(F_PPD))), $ray);
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
    : u\lpad(is_numeric($v) ? strval((int) round($v)) : $v, $len);
  }
  return $res;
}
