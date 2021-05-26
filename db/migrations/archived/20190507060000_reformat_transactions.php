<?php


use Phinx\Migration\AbstractMigration;
require_once __DIR__ . '/util.inc';

class ReformatTransactions extends AbstractMigration {
  /* **************************************************************************************************
   * The following lines are taken from cgmembers/rcredits/defs.inc as of the time this migration
   * was created.
   */
  const CG_ADMIN_UID = 1;  // uid of administrator account
  const PLACEHOLDER_1_UID = 2;  // uid of first placeholder
  const PLACEHOLDER_2_UID = 3;  // uid of second placeholder
  const CG_ROUNDUPS_UID = 129;  // Donations start at 128 (128 used for general donations)
  const CG_CRUMBS_UID = 130;  //
  const CG_SERVICE_CHARGES_UID = 192;
  const CG_INCOMING_BANK_UID = 256;  // Bank accounts start at 256
  const CG_OUTGOING_BANK_UID = 257;  //

  // Transaction flags (for flags field in transaction records)
  const B_TAKING = 0; // payee initiated the transaction
  const B_DISPUTED = 1; // non-originator disputes the transaction -- signaled by existence of tx_disputes (NOT LEGAL FOR TRANSACTIONS, BUT STILL USED BY INVOICES
  const B_OFFLINE = 2; // transaction was taken offline (or was forced?)
  const B_SHORT = 3; // transaction was taken (offline) despite credit shortfall
  const OLD_B_UNDONE = 4; // undone by another transaction -- signaled by existence of reversing transaction
  const OLD_B_UNDOES = 5; // undoes another transaction -- signaled by reversesXid not null
  const B_CRUMBS = 6; // monthly donation of percentage of receipts
  const OLD_B_ROUNDUPS = 7; // monthly donation of rounded up cents -- no longer occurs
  const B_ROUNDUP = 8; // payer donated the change to the community fund -- not used here, 
  const B_RECURS = 9; // recurring transaction
  const B_GIFT = 10; // grant or gift (of any type)
  const B_LOAN = 11; // community loan (UNUSED?)
  const B_INVESTMENT = 12; // community investment (UNUSED?) or investment club investment
  const B_STAKE = 13; // member buying or selling stake in investment club
  const B_FINE = 14; // community fine (UNUSED?)
  const B_NOASK = 15; // transaction was taken with ID checking OFF
  const B_FUNDING = 16; // invoice has already instigated an appropriate bank transfer request
  const B_FOOD = 17; // contribution to food fund
  const TX_FLAGS = 'offline short crumbs recurs gift loan investment stake fine noask funding food';

// Transaction channels (roughly in order of simplicity and generality of messages) (CGF)
  const TX_SYS = 0;
  const TX_SMS = 1; 
  const TX_WEB = 2;
  const TX_POS = 3; // smart phone or other computer-like device
  const TX_TONE = 4; // touch tone phone
  const TX_CRON = 5;
  const TX_LINK = 6; // user clicked a no-signin link
  const TX_AJAX = 7;
  const TX_FOREIGN = 8; // user clicked a "Pay with CG" button or request an app to charge them

  // Transaction types
  const TX_TRANSFER = 0; // normal fund transfer (usually for actual goods and services) -- not creating rC
  const OLD_TX_XFEE = 10;
  const TX_BANK = 99; // used only internally, to mark bank transfers

  const DS_OPEN = 1;  // dispute not resolved
  const DS_ACCEPTED = 2;  // dispute accepted and transaction reversed
  const DS_DENIED = 3;  // dispute denied


  const ENTRY_OTHER = 0;
  const ENTRY_PAYER = 1;  // the uid in this entry is for the payer
  const ENTRY_PAYEE = 2;  // the uid in this entry is for the payee
  const ENTRY_DONATION = 3;  // this is a transaction-related donation (e.g., roundup donations)
  // the precise nature can be determined from the uid

  const FOR_GOODS = 0; // index into R_WHYS for goods
  const FOR_USD = 1; // index into R_WHYS for USD exchange
  const FOR_NONGOODS = 2; // index into R_WHYS for loan, etc.

  /* This ends the section of lines taken from cgmembers/rcredits/defs.inc 
   * *************************************************************************************************/


  /**
   * Up method.Change Method.
   *
   * More information on writing migrations is available here:
   * http://docs.phinx.org/en/latest/migrations.html#the-abstractmigration-class
   *
   * Remember to call "create()" or "update()" and NOT "save()" when working
   * with the Table class.
   */
  public function up() {
    $undoneBy = [];
    $undoes = [];
    
    $flagsMask = (1 << self::B_OFFLINE | 1 << self::B_SHORT | 1 << self::B_RECURS | 1 << self::B_GIFT | 1 << self::B_FUNDING | 1 << self::B_CRUMBS | 1 << self::B_STAKE);

    $txsTable = $this->table('r_txs');
    $txHdrsTable = $this->table('tx_hdrs');
    $entriesTable = $this->table('tx_entries');
    $disputesTbl = $this->table('tx_disputes');
    $coupatedTbl = $this->table('r_coupated');
    $maxXid = 0;

    $xFees = [];
    $couponValue = [];
    $coupId = [];

    $allChangeKeys = [];
    $allForceValues = [];
    
    $results = $this->query('select * from r_txs');
    print("CHANGES ARE DISCARDED\n");
    print("force in data BEING IGNORED\n\n");

    while (true) {
      $oldTx = $results->fetch(PDO::FETCH_ASSOC);
      if (empty($oldTx)) break;

      extract($oldTx);
      $dataString = $data;
      if (is_string($data)) {
        $data = unserialize($data);
        if (!is_array($data)) $data = [];
      }
      $maxXid = max($maxXid, $xid);
      
      $reversesXid = null;
      $payerEntry = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                        $xid, self::ENTRY_PAYER, 0-$amount, $payer, $payerAgent, $payerFor, $payerTid, null, null);
      $payeeEntry = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                        $xid, self::ENTRY_PAYEE, $amount, $payee, $payeeAgent, $payeeFor, $payeeTid, null, null);
      $otherEntries = [];

      // xid is untouched

      // serial
      if ($serial != $xid) {  // this is the next part of a group of transactions
        if ($type == self::OLD_TX_XFEE) {
          $feeAmt = arrayGet($xFees, $serial, null);
          if ($feeAmt != null) {
            if ($amount != $feeAmt) {
              print("INCONSISTENCY: $serial wants $feeAmt, fee txn is for $amount\n");
            } else {
              // do nothing -- the fee was already added when processing the original transaction
              unset($xFees[$serial]);
              $xid = 0;  // Signal to ignore this transaction
              $type = self::TX_TRANSFER;  // avoid error message
            }
          } else {
            print("INCONSISTENCY: $xid is for $amount but original txn, $serial, has no fee\n");
          }
        } elseif ($type == self::TX_TRANSFER) {
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
      
      /* // type */
      /* if (!array_key_exists($type, [self::OLD_TX_TRANSFER => 1, TX_GRANT => 1, self::TX_LOAN => 1])) { */
      /*   print("NOT HANDLED: type is $type on " . $oldTx['xid'] . "\n"); */
      /* } */

      // goods
      if (!array_key_exists($goods, [self::FOR_GOODS => 1, self::FOR_USD => 1, self::FOR_NONGOODS => 1])) {
        print("NOT HANDLED: goods is $goods on $xid\n");
      }

      /* if ($type == self::OLD_TX_TRANSFER) { */
      /*   if ($goods == OLD_FOR_GOODS) $newType = self::TX_GOODS; */
      /*   elseif ($goods == OLD_FOR_USD) $newType = self::TX_USD; */
      /*   elseif ($goods == OLD_FOR_NONGOODS) $newType = self::TX_SERVICES; */
      /*   else die('should never get here'); */
      /* } else { */
      /*   $newType = $type; */
      /* } */
      
      // data and flags
      // data: changes undoneBy force inv disputed isGift undoneNO undoes xfee coupon coupid

      if (array_key_exists('changes', $data)) {
        $changes = $data['changes'];
        foreach ($changes as $index => $change) {
          if (!array_key_exists(2, $change)) {
            if (!preg_match('/[0-9]{10}( [A-Z]{6})?/', $index)) {
              print_r("In $xid, payer is $payer, payee is $payee, changes contains '$index' => ");
              print_r($change);
              print_r("\n");
              /* print_r("BAD CHANGES in $xid:"); */
              /* print_r($change); */
              /* print_r("\n"); */
            }
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
        setBit($flags, self::OLD_B_UNDONE, false);
        unset($data['undoneBy']);
        $payerEntry['description'] = preg_replace('/ \(?reverse(s|d by) #[0-9]*\)?/', '', $payerEntry['description']); 
        $payeeEntry['description'] = preg_replace('/ \(?reverse(s|d by) #[0-9]*\)?/', '', $payeeEntry['description']); 
      }

      if (array_key_exists('inv', $data)) {
        $nvid = $data['inv'];
        $result = $this->query("SELECT status FROM r_invoices WHERE nvid='$nvid'");
        if ($result == false) {
          print("ERROR GETTING r_invoices on xid=$xid, (nvid=$nvid), result=$result\n");
        }
        $status = $result->fetch()['status'];
        if ($status != $xid and $status != $reversesXid) {
          print("INCONSISTENCY: tx $xid refers to invoice $nvid, but that invoice's status is $status, (reversesXid=$reversesXid)\n");
        }
        $payeeEntry['relType'] = 'I';
        $payeeEntry['relatedId'] = $nvid;
        unset($data['inv']);
      }
      
      if (array_key_exists('disputed', $data)) {
        $dispute = $data['disputed'];
        if ($dispute) {
          print("NOT HANDLED: dispute $dispute in $xid\n");
        } else {
          $reason = '?';
          if (arrayGet($undoes, $xid, null) != null) { // transaction was reversed
            $status = self::DS_ACCEPTED;  // dispute has been adjudged correct and offsetting transaction generated
          } else {
            $status = self::DS_DENIED;
          }
          $disputesTbl->insert(ray('xid uid agentUid reason status', $xid, $payer, $payer, $reason, $status))->save();
        }                       
        unset($data['disputed']);
        setBit($flags, self::B_DISPUTED, false);
      }
      
      if (array_key_exists('isGift', $data)) {
        setBit($flags, self::B_GIFT, true);
        unset($data['isGift']);
      }
      
      if (array_key_exists('undoneNO', $data)) {
        print("IGNORING: undoneNO in $xid\n");
        unset($data['undoneNO']);
      }

      if (getBit($flags, self::OLD_B_UNDOES)) {
        if (!array_key_exists('undoes', $data)) {
          if (array_key_exists($xid, $undoes)) {  // the tx we're reversing knows us
            $reversesXid = $undoes[$xid];
            $undoneBy[$xid] = $reversesXid;
          } else {
            print("INCONSISTENCY: undoes flag set, no undoes data, and no entry in undoneBy array\n");
            print_r($undoes);
            print_r("\n");
            print_r($undoneBy);
            print_r("\n");
            $reversesXid = null;
          }
        }
        $payerEntry['description'] = preg_replace('/ \(?reverse(s|d by) #[0-9]*\)?/', '', $payerEntry['description']); 
        $payeeEntry['description'] = preg_replace('/ \(?reverse(s|d by) #[0-9]*\)?/', '', $payeeEntry['description']); 
        setBit($flags, self::OLD_B_UNDOES, false);
        unset($data['undoes']);
      }

      
      if (!getBit($flags, self::OLD_B_UNDOES) and array_key_exists('undoes', $data)) {
        print("INCONSISTENCY: no undoes flag set but undoes data present\n");
      }
      
      if (array_key_exists('undoes', $data)) {
        $reversesXid = $data['undoes'];
        if (arrayGet($undoneBy, $reversesXid, $xid) != $xid) {
          $early = $undoneBy[$reversesXid];
          print("INCONSISTENCY: tx $reversesXid is being undone by tx $xid, but was already undone by tx $early\n");
        } else {
          $undoneBy[$reversesXid] = $xid;
          $undoes[$xid] = $reversesXid;
        }
        setBit($flags, self::OLD_B_UNDOES, false);
        unset($data['undoes']);
        $payerEntry['description'] = preg_replace('/ \(?reverse(s|d by) #[0-9]*\)?/', '', $payerEntry['description']); 
        $payeeEntry['description'] = preg_replace('/ \(?reverse(s|d by) #[0-9]*\)?/', '', $payeeEntry['description']); 
      }

      if (array_key_exists('force', $data)) {
        $force = $data['force'];
        $force = (getBit($flags, self::B_OFFLINE) ? 'T' : 'F') . (getBit($flags, self::B_SHORT) ? 'T' : 'F') .
          ($reversesXid == null ? 'F' : 'T') . $force;
        if (!array_key_exists($force, $allForceValues)) { $allForceValues[$force] = 1; }
        else { $allForceValues[$force] += 1; }
        unset($data['force']);
        setBit($flags, self::B_OFFLINE);
      }

      if (array_key_exists('xfee', $data)) {
        $xFeeAmt = $data['xfee'];
        $xFees[$xid] = $xFeeAmt;  // save the data to check consistency
        $otherEntries[] = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                              $xid, self::ENTRY_OTHER, $xFeeAmt, $payee, $payeeAgent, 'transaction fee', $payeeTid, null, null);
        $otherEntries[] = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                              $xid, self::ENTRY_OTHER, -$xFeeAmt, $payer, $payerAgent, 'transaction fee', $payerTid, null, null);
        unset($data['xfee']);
      }

      if (array_key_exists('coupon', $data) and array_key_exists('coupid', $data)) {
        $couponValue[$xid] = $data['coupon'];  // save coupon data for later consistency checking
        $coupId[$xid] = $data['coupid'];
        $cid = $data['coupid'];
        $coupated = $this->query("select * from r_coupated where uid=$payer and coupid=$cid");
        if (empty($coupated)) {
          print("Coupated record missing for user $uid, coupon $cid\n");
          $coupatedId = $coupated['id'];
        } else {
          $coupatedId = null;
        }
        /* $payeeEntry['amount'] += $couponValue[$xid]; */
        $otherEntries[] = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                              $xid, self::ENTRY_OTHER, -$couponValue[$xid], $payee, $payeeAgent, 'discount rebate', $payeeTid, 'D', $coupatedId);
        $otherEntries[] = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                              $xid, self::ENTRY_OTHER, $couponValue[$xid], $payer, $payerAgent, 'discount rebate', $payerTid, 'D', $coupatedId);
        unset($data['coupon']);
        unset($data['coupid']);
      }

      // flags: taking disputed offline short undone undoes crumbs roundups roundup recurs gift
      if (getBit($flags, self::B_TAKING)) {
        $actorId = $payee;
        $actorAgentId = $payeeAgent;
        setBit($flags, self::B_TAKING, false);
      } else {
        $actorId = $payer;
        $actorAgentId = $payerAgent;
      }

      if (getBit($flags, self::B_DISPUTED)) {
        if ($reversesXid != null) {  // reverses a disputed transaction
        } else {
          print("INCONSISTENCY: disputed flag set, but no dispute data recorded on $xid\n");
        }
        setBit($flags, self::B_DISPUTED, false);
      }

      // B_OFFLINE is left alone

      // B_SHORT is left alone
      
      if (getBit($flags, self::OLD_B_UNDONE)) {  // if there had been undoneBy data we would have turned this off earlier
        print("NOT HANDLED: flag OLD_B_UNDONE on $xid\n");
        setBit($flags, self::OLD_B_UNDONE, false);
      }
      
      if (getBit($flags, self::OLD_B_UNDOES)) {  // if we knew what it undid we would have turned this off earlier
        print("NOT HANDLED: flag OLD_B_UNDOES on $xid\n");
        setBit($flags, self::OLD_B_UNDOES, false);
      }

      // B_CRUMBS is left alone
      
      if (getBit($flags, self::OLD_B_ROUNDUPS)) {
        $xid = 0;  // flag that this transaction should be ignored
        /* print("IGNORING ROUNDUPS on $xid\n"); */
        setBit($flags, self::OLD_B_ROUNDUPS, false);
      }

      $roundupDonation = 0;
      if (getBit($flags, self::B_ROUNDUP)) {
        if ($amount > 0 and $payee > 0) {  // communities don't make roundup donations
          $cents = fmod($amount, 1);
          if ($cents > 0) {
            $roundupDonation = round(1 - $cents, 2);
            /* $payerEntry['amount'] -= $roundupDonation; */
            $otherEntries[] = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                                  $xid, self::ENTRY_DONATION, $roundupDonation, self::CG_ROUNDUPS_UID, self::CG_ROUNDUPS_UID,
                                  'roundup donation', '', null, null);
            $otherEntries[] = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                                  $xid, self::ENTRY_DONATION, -$roundupDonation, $payer, $payerAgent,
                                  'roundup donation', $payerTid, null, null);
            /* print("ROUNDUP DONATION will be $roundupDonation on $xid\n"); */
          }
        }
        setBit($flags, self::B_ROUNDUP, false);
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
        print("NOT HANDLED: data is " . print_r($oldTx['data'], true) . " on $xid\n");
      }

      //
      if ($xid != 0) {
        $hdrInfo = just('xid goods risk risks created', $oldTx) +
          ray('actorId actorAgentId flags channel boxId reversesXid',
              $actorId, $actorAgentId, $flags, $channel, $oldTx['box'], null);
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
    
    $oldTxs = $this->query('select * from r_usd');
    $nextXid = $maxXid;
    
    foreach ($oldTxs as $result) {
      $oldTx = (array)$result;
      $missingFields = array_diff_key($requiredFields, $oldTx);
      if ($missingFields != []) {
        print("NOT HANDLED: USD MISSING FIELDS " . print_r($missingFields, true) . "\n");
      }
      extract(just($requiredFields, $oldTx));
      if (empty($completed)) continue;  // no transaction associated with incomplete US$ transfer
      
      if (is_null($channel)) $channel = self::TX_SYS;
      if (is_null($risks)) $risks = 0;
      
      $nextXid += 1;
      
      $hdr = ray('xid actorId actorAgentId flags channel boxId goods risk risks reversesXid created',
                 $nextXid, $payee, $payee, 0, $channel, null, self::FOR_USD, $risk, $risks, null, $completed);
      $bankUid = $amount >= 0 ? self::CG_INCOMING_BANK_UID : self::CG_OUTGOING_BANK_UID;
      $e1 = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                $nextXid, self::ENTRY_PAYER, -$amount, $bankUid, $bankUid, ($amount > 0) ? 'from bank' : 'to bank',
                null, null, null);
      
      $e2 = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                $nextXid, self::ENTRY_PAYEE, $amount, $payee, $payee, ($amount > 0) ? 'from bank' : 'to bank', null, null, null);

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
      $result = (array)$result;
      $missingFields = array_diff_key($requiredFields, $result);
      if ($missingFields != []) {
        print("NOT HANDLED: USD2 MISSING FIELDS " . print_r($missingFields, true) . "\n");
      }
      extract(just($requiredFields, $result));

      if (empty($completed)) continue;  // no transaction associated with incomplete US$ transfer
      
      // Checking
      switch ($type) {
      case 'S':
        $actorId = self::CG_INCOMING_BANK_UID;
        $payee = self::CG_SERVICE_CHARGES_UID;
        break;
      case 'T':
        $actorId = self::CG_ADMIN_UID;
        $payee = self::CG_INCOMING_BANK_UID;
        break;
      default:
        print('BAD USD2 TYPE: ' . $result['type'] . "\n");
      }

      $actorAgentId = $actorId;

      $nextXid += 1;

      $hdr = ray('xid actorId actorAgentId flags channel boxId goods risk risks reversesXid created',
                 $nextXid, $actorId, $actorAgentId, 0, self::TX_SYS, null, self::FOR_USD, null, 0, null, $completed);
      $e1 = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                $nextXid, self::ENTRY_PAYER, -$amount, self::CG_INCOMING_BANK_UID, self::CG_INCOMING_BANK_UID, $memo, null, null, null);
      $e2 = ray('xid entryType amount uid agentUid description acctTid relType relatedId',
                $nextXid, self::ENTRY_PAYEE, $amount, self::CG_SERVICE_CHARGES_UID, self::CG_INCOMING_BANK_UID, $memo, null, null, null);

      $txHdrsTable->insert($hdr)->save();
      $entriesTable->insert($e1)->insert($e2)->save();
      if ($this->execute("UPDATE r_usd2 SET xid='$nextXid' WHERE id='$id'") != 1) {
        print("ERROR UPDATING r_usd2 on xid=$nextXid\n");
      }
    }
  }

  public function down() {
    $this->execute('DELETE FROM tx_entries_all');
    $this->execute('DELETE FROM tx_hdrs_all');
    $this->execute('DELETE FROM tx_disputes_all');
    $this->execute('UPDATE r_usd SET xid=DEFAULT');
    $this->execute('UPDATE r_usd2 SET xid=DEFAULT');
  }
}
