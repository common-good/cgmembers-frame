<?php


use Phinx\Migration\AbstractMigration;
require_once 'cgmembers/rcredits/bootstrap.inc';
require_once 'cgmembers/rcredits/defs.inc';
require_once 'cgmembers/rcredits/cg-util.inc';

use CG\Util as u;

class ReformatTransactions extends AbstractMigration
{
  /**
   * Up method.Change Method.
   *
   * More information on writing migrations is available here:
   * http://docs.phinx.org/en/latest/migrations.html#the-abstractmigration-class
   *
   * Remember to call "create()" or "update()" and NOT "save()" when working
   * with the Table class.
   */
  public function up()
  {
    $undoneBy = [];
    $undoes = [];
    
    $requiredFields = ray('xid serial type goods amount payer payee payerAgent payeeAgent payerFor payeeFor payerReward payeeReward payerTid payeeTid data flags channel box created risk risks');
    
    $flagsMask = (1 << B_OFFLINE | 1 << B_SHORT | 1 << B_RECURS | 1 << B_GIFT | 1 << B_FUNDING | 1 << B_CRUMBS);
    $dataMask = ray('');

    $txsTable = $this->table('r_txs');
    $txHdrsTable = $this->table('r_tx_hdrs');
    $entriesTable = $this->table('r_entries');
    $disputesTbl = $this->table('r_disputes');
    $maxXid = 0;

    $xFees = [];
    $couponValue = [];
    $coupId = [];

    $allChangeKeys = [];
    $allForceValues = [];
    
    $sql = 'select * from r_txs';
    $results = $this->query($sql);
    print("CHANGES ARE DISCARDED\n");
    print("force in data BEING IGNORED\n\n");

    while (true) {
      $result = $results->fetch(PDO::FETCH_ASSOC);
      if (!$result) break;

      extract($result);
      if (is_string($data)) {
        $data = unserialize($data);
        if (!is_array($data)) $data = [];
      }
      $maxXid = max($maxXid, $xid);
      
      $reverses = null;
      $payerEntry = ray('xid entryType amount uid agentUid description acctTid relType related',
                        $xid, ENTRY_PAYER, 0-$amount, $payer, $payerAgent, $payerFor, $payerTid, null, null);
      $payeeEntry = ray('xid entryType amount uid agentUid description acctTid relType related',
                        $xid, ENTRY_PAYEE, $amount, $payee, $payeeAgent, $payeeFor, $payeeTid, null, null);
      $otherEntries = [];

      // xid is untouched

      // serial
      if ($serial != $xid) {  // this is the next part of a group of transactions
        if ($type == TX_XFEE) {
          $feeAmt = arrayGet($xFees, $serial, null);
          if ($feeAmt != null) {
            if ($amount != $feeAmt) {
              print("INCONSISTENCY: $serial wants $feeAmt, fee txn is for $amount\n");
            } else {
              // do nothing -- the fee was already added when processing the original transaction
              unset($xFees[$serial]);
              $xid = 0;  // Signal to ignore this transaction
              $type = TX_TRANSFER;  // avoid error message
            }
          } else {
            print("INCONSISTENCY: $xid is for $amount but original txn, $serial, has no fee\n");
          }
        } elseif ($type == TX_TRANSFER) {
          $cValue = arrayGet($couponValue, $serial, null);
          $cId = arrayGet($coupId, $serial, null);
          if ($cValue != null and $cId != null) { // looks like a coupon
            if (round($cValue, 2) != -$amount) {
              print("INCONSISTENCY: cValue is $cValue, but coupon tx is for $amount on $xid\n");
            } else {
              // do nothing -- the coupon was already processed when processing the original transaction
              unset($couponValue[$serial]);
              unset($coupId[$serial]);
              $xid = 0;  // Signal to ignore this transaction
            }
          } else {
            print("NOT HANDLED: serial is $serial, xid is $xid.\n");
          }
        } else {
          print("NOT HANDLED: serial is $serial, xid is $xid, type is $type.\n");
        }
      }
      
      // type
      if (!array_key_exists($type, [TX_TRANSFER => 1, TX_SIGNUP => 1, TX_GRANT => 1, TX_LOAN => 1])) {
        print("NOT HANDLED: type is $type on " . $result['xid'] . "\n");
      }

      // goods
      if (!array_key_exists($goods, [FOR_GOODS => 1, FOR_USD => 1, FOR_NONGOODS => 1])) {
        print("NOT HANDLED: goods is $goods on $xid\n");
      }

      // data and flags
      // data: changes undoneBy force inv disputed isGift undoneNO undoes xfee coupon coupid

      if (array_key_exists('changes', $data)) {
        $changes = $data['changes'];
        foreach ($changes as $index => $change) {
          if (!array_key_exists(2, $change)) {
            /* print_r("In $xid, changes contains $index => "); */
            /* print_r($change); */
            /* print_r("\n"); */
            /* print_r("BAD CHANGES is $xid"); */
            /* print_r($changes); */
            /* print_r("\n"); */
            /* print_r($change); */
            /* print_r("\n"); */
          } else {
            $allChangeKeys[$change[2]] = 1;
          }
        }
        unset($data['changes']);
      }

      if (array_key_exists('undoneBy', $data)) {
        $reverser = $data['undoneBy'];
        if (arrayGet($undoes, $reverser, null) != null) {
          print("INCONSISTENCY: tx $xid apparently undone by $reverser\n");
        }
        $undoes[$reverser] = $xid;
        $undoneBy[$xid] = $reverser;
        u\setBit($flags, B_UNDONE, false);
        unset($data['undoneBy']);
      }
      
      if (array_key_exists('inv', $data)) {
        $nvid = $data['inv'];
        $result = $this->query("SELECT status FROM r_invoices WHERE nvid='$nvid'");
        if ($result == false) {
          print("ERROR GETTING r_invoices on xid=$xid, (nvid=$nvid), result=$result\n");
        }
        $status = $result->fetch()['status'];
        if ($status != $xid and $status != $reverses) {
          print("INCONSISTENCY: tx $xid refers to invoice $nvid, but that invoice's status is $status, (reverses=$reverses)\n");
        }
        $payeeEntry['relType'] = 'I';
        $payeeEntry['related'] = $nvid;
        unset($data['inv']);
      }
      
      if (array_key_exists('disputed', $data)) {
        $dispute = $data['disputed'];
        if ($dispute) {
          print("NOT HANDLED: dispute $dispute in $xid\n");
        } else {
          $reason = '?';
          if (arrayGet($undoes, $xid, null) != null) { // transaction was reversed
            $status = DS_ACCEPTED;  // dispute has been adjudged correct and offsetting transaction generated
          } else {
            $status = DS_DENIED;
          }
          $disputesTbl->insert(ray('xid, reason, status', $xid, $reason, $status))->save();
        }                       
        unset($data['disputed']);
        u\setBit($flags, B_DISPUTED, false);
      }
      
      if (array_key_exists('isGift', $data)) {
        u\setBit($flags, B_GIFT, true);
        unset($data['isGift']);
      }
      
      if (array_key_exists('undoneNO', $data)) {
        print("IGNORING: undoneNO in $xid\n");
        unset($data['undoneNO']);
      }

      if (u\getBit($flags, B_UNDOES) and !array_key_exists('undoes', $data)) {
        if (array_key_exists($xid, $undoes)) {  // the tx we're reversing knows us
          $reverses = $undoes[$xid];
          $undoneBy[$xid] = $reverses;
          u\setBit($flags, B_UNDOES, false);
        } else {
          print("INCONSISTENCY: undoes flag set, no undoes data, and no entry in undoneBy array\n");
          print_r($undoes);
          print_r("\n");
          print_r($undoneBy);
          print_r("\n");
        }
      }
      
      if (!u\getBit($flags, B_UNDOES) and array_key_exists('undoes', $data)) {
        print("INCONSISTENCY: no undoes flag set but undoes data present\n");
      }
      
      if (array_key_exists('undoes', $data)) {
        $reverses = $data['undoes'];
        if (arrayGet($undoneBy, $reverses, $xid) != $xid) {
          $early = $undoneBy[$reverses];
          print("INCONSISTENCY: tx $reverses is being undone by tx $xid, but was already undone by tx $early\n");
        } else {
          $undoneBy[$reverses] = $xid;
          $undoes[$xid] = $reverses;
        }
        u\setBit($flags, B_UNDOES, false);
        unset($data['undoes']);
      }

      if (array_key_exists('force', $data)) {
        $force = $data['force'];
        $force = (u\getBit($flags, B_OFFLINE) ? 'T' : 'F') . (u\getBit($flags, B_SHORT) ? 'T' : 'F') .
          ($reverses == null ? 'F' : 'T') . $force;
        if (!array_key_exists($force, $allForceValues)) { $allForceValues[$force] = 1; }
        else { $allForceValues[$force] += 1; }
        unset($data['force']);
        u\setBit($flags, B_OFFLINE);
      }
      
      if (array_key_exists('xfee', $data)) {
        $xFeeAmt = $data['xfee'];
        $xFees[$xid] = $xFeeAmt;  // save the data to check consistency
        $otherEntries[] = ray('xid amount uid agentUid description acctTid relType related',
                              $xid, $xFeeAmt, $payee, $payeeAgent, t('transaction fee'), $payeeTid, null, null);
        $otherEntries[] = ray('xid amount uid agentUid description acctTid relType related',
                              $xid, -$xFeeAmt, $payer, $payerAgent, t('transaction fee'), $payerTid, null, null);
        unset($data['xfee']);
      }

      if (array_key_exists('coupon', $data) and array_key_exists('coupid', $data)) {
        $couponValue[$xid] = $data['coupon'];  // save coupon data for later consistency checking
        $coupId[$xid] = $data['coupid'];
        /* $payeeEntry['amount'] += $couponValue[$xid]; */
        $otherEntries[] = ray('xid amount uid agentUid description acctTid relType related',
                              $xid, -$couponValue[$xid], $payee, $payeeAgent, $payeeFor, $payeeTid, 'C', $coupId[$xid]);
        $otherEntries[] = ray('xid amount uid agentUid description acctTid relType related',
                              $xid, $couponValue[$xid], $payer, $payerAgent, $payerFor, $payerTid, 'C', $coupId[$xid]);
        unset($data['coupon']);
        unset($data['coupid']);
      }
      
      // flags: taking disputed offline short undone undoes crumbs roundups roundup recurs gift
      if (u\getBit($flags, B_TAKING)) {
        $actor = $payee;
        $actorAgent = $payeeAgent;
        u\setBit($flags, B_TAKING, false);
      } else {
        $actor = $payer;
        $actorAgent = $payerAgent;
      }

      if (u\getBit($flags, B_DISPUTED)) {
        if ($reverses != null) {  // reverses a disputed transaction
        } else {
          print("INCONSISTENCY: disputed flag set, but no dispute data recorded on $xid\n");
        }
        u\setBit($flags, B_DISPUTED, false);
      }

      // B_OFFLINE is left alone

      // B_SHORT is left alone
      
      if (u\getBit($flags, B_UNDONE)) {  // if there had been undoneBy data we would have turned this off earlier
        print("NOT HANDLED: flag B_UNDONE on $xid\n");
        u\setBit($flags, B_UNDONE, false);
      }
      
      if (u\getBit($flags, B_UNDOES)) {  // if we knew what it undid we would have turned this off earlier
        print("NOT HANDLED: flag B_UNDOES on $xid\n");
        u\setBit($flags, B_UNDOES, false);
      }

      // B_CRUMBS is left alone
      
      if (u\getBit($flags, B_ROUNDUPS)) {
        $xid = 0;  // flag that this transaction should be ignored
        /* print("IGNORING ROUNDUPS on $xid\n"); */
        u\setBit($flags, B_ROUNDUPS, false);
      }

      $roundupDonation = 0;
      if (u\getBit($flags, B_ROUNDUP)) {
        if ($amount > 0 and $payee > 0) {  // communities don't make roundup donations
          $cents = fmod($amount, 1);
          if ($cents > 0) {
            $roundupDonation = round(1 - $cents, 2);
            $payerEntry['amount'] -= $roundupDonation;
            $otherEntries[] = ray('xid entryType amount uid agentUid description acctTid relType related',
                                  $xid, ENTRY_DONATION, $roundupDonation, CG_ROUNDUPS_UID, CG_ROUNDUPS_UID,
                                  t('roundup donation'), '', null, null);
            $otherEntries[] = ray('xid entryType amount uid agentUid description acctTid relType related',
                                  $xid, ENTRY_DONATION, -$roundupDonation, $payer, $payerAgent,
                                  t('roundup donation'), $payerTid, null, null);
            /* print("ROUNDUP DONATION will be $roundupDonation on $xid\n"); */
          }
        }
        u\setBit($flags, B_ROUNDUP, false);
      }

      // B_RECURS is left alone

      // B_GIFT is left alone
      
      // Check for unexpected flags or data
      if (($flags & ~$flagsMask) != 0) {
        $hexFlags = dechex($flags);
        print("NOT HANDLED: flags is $hexFlags on $xid\n");
	print_r($data);
      }

      if ($data != []) {
        print("NOT HANDLED: data is " . print_r($result['data'], true) . " on $xid\n");
      }

      //
      if ($xid != 0) {
        $hdrInfo = compact(ray('xid type goods actor actorAgent flags channel box risk risks reverses created'));
        $txHdrsTable->insert($hdrInfo)->save();
        $entriesTable->insert($payerEntry)->insert($payeeEntry)->save();
        foreach ($otherEntries as $key => $entry) {
          if (array_key_exists('0', $entry)) {
            print_r($entry);
          }
          $entriesTable->insert($entry)->save();
        }
      }
    }

    if ($xFees != []) {
      print_r("Leftover xFees: ");
      print_r($xFees);
      print_r("\n");
    }
    if ($couponValue != []) {
      print_r("Leftover couponValue: ");
      print_r($couponValue);
      print_r("\n");
    }
    if ($coupId != []) {
      print_r("Leftover coupId: ");
      print_r($coupId);
      print_r("\n");
    }

    foreach ($allChangeKeys as $changeKey => $v) {
      print_r("change: $changeKey\n");
    }
    foreach ($allForceValues as $forceKey => $forceCount) {
      print_r("force $forceKey: $forceCount\n");
    }
  
    /*---------------------------------------------------------------*/
    /* Now for r_usd */
    $requiredFields = ray('txid amount payee created completed deposit bankAccount risk risks bankTxId channel');
    
    $sql = 'select * from r_usd';
    $results = $this->query($sql);
    $nextXid = $maxXid;
    
    foreach ($results as $result) {
      $nextXid += 1;
      $result = (array)$result;
      $missingFields = array_diff_key($requiredFields, $result);
      if ($missingFields != []) {
        print("NOT HANDLED: USD MISSING FIELDS " . print_r($missingFields, true) . "\n");
      }
      extract(just($requiredFields, $result));

      if (is_null($channel)) $channel = TX_SYS;
      if (is_null($risks)) $risks = 0;
      
      $hdr = ray('xid type goods actor actorAgent flags channel box risk risks reverses created',
                 $nextXid, TX_BANK, FOR_USD, $payee, $payee, 0, $channel, null, $risk, $risks, null, $completed);

      $bankUid = $amount >= 0 ? CG_INCOMING_BANK_UID : CG_OUTGOING_BANK_UID;
      $e1 = ray('xid amount uid agentUid description acctTid relType related',
                $nextXid, -$amount, $bankUid, $bankUid, ($amount > 0) ? t('from bank') : t('to bank'),
                $bankTxId, null, null);
      
      $e2 = ray('xid amount uid agentUid description acctTid relType related',
                $nextXid, $amount, $payee, $payee, ($amount > 0) ? t('from bank') : t('to bank'), $txid, null, null);


      $txHdrsTable->insert($hdr)->save();
      $entriesTable->insert($e1)->insert($e2)->save();
      $result = $this->execute("UPDATE r_usd SET xid='$nextXid' WHERE txid='$txid'");
      if ($result != 1) {
        print("ERROR UPDATING r_usd on xid=$xid: $result\n");
      }
    }

    /*---------------------------------------------------------------*/
    /* Now for r_usd2 */
    $requiredFields = ray('id type amount completed bankTxId memo');

    $sql = 'select * from r_usd2';
    $results = $this->query($sql);

    foreach ($results as $result) {
      $nextXid += 1;
      $result = (array)$result;
      $missingFields = array_diff_key($requiredFields, $result);
      if ($missingFields != []) {
        print("NOT HANDLED: USD2 MISSING FIELDS " . print_r($missingFields, true) . "\n");
      }
      extract(just($requiredFields, $result));

      // Checking
      switch ($type) {
      case 'S':
        $actor = CG_INCOMING_BANK_UID;
        break;
      case 'T':
        $actor = CG_ADMIN_UID;
        break;
      default:
        print('BAD USD2 TYPE: ' . $result['type'] . "\n");
      }

      $actorAgent = $actor;

      $hdr = ray('xid type goods actor actorAgent flags channel box risk risks reverses created',
                 $nextXid, TX_BANK, FOR_USD, $actor, $actorAgent, 0, TX_SYS, null, null, 0, null, $completed);
      $e1 = ray('xid amount uid agentUid description acctTid relType related',
                $nextXid, -$amount, CG_INCOMING_BANK_UID, CG_INCOMING_BANK_UID, $memo, $bankTxId, null, null);
      $e2 = ray('xid amount uid agentUid description acctTid relType related',
                $nextXid, $amount, CG_INCOMING_BANK_UID, CG_INCOMING_BANK_UID, $memo, $bankTxId, null, null);

      $txHdrsTable->insert($hdr)->save();
      $entriesTable->insert($e1)->insert($e2)->save();
      if ($this->execute("UPDATE r_usd2 SET xid='$xid' WHERE id='$id'") != 1) {
        print("ERROR UPDATING r_usd2 on xid=$nextXid\n");
      }
    }
  }

  public function down() {
    $this->execute('DELETE FROM all_entries');
    $this->execute('DELETE FROM all_tx_hdrs');
    $this->execute('DELETE FROM all_disputes');
    $this->execute('UPDATE r_usd SET xid=DEFAULT');
    $this->execute('UPDATE r_usd2 SET xid=DEFAULT');
  }
}

function arrayGet($arr, $key, $dft) {
  return (is_array($arr) and array_key_exists($key, $arr)) ? $arr[$key] : $dft;
}
