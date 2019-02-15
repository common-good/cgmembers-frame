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
    
    $flagsMask = (1<<B_OFFLINE | 1<<B_SHORT | 1<<B_RECURS | 1<<B_GIFT | 1<<B_FUNDING);
    $dataMask = ray('');

    $txsTable = $this->table('r_txs');
    $txHdrsTable = $this->table('r_tx_hdrs');
    $entriesTable = $this->table('r_entries');
    $disputesTbl = $this->table('r_disputes');
    $maxXid = 0;
    
    $sql = 'select * from r_txs';
    $results = $this->query($sql);

    print("CHANGES ARE DISCARDED\n");
    print("force in data BEING IGNORED\n\n");

    foreach ($results as $result) {
      $result = (array)$result;

      $xid = $result['xid'];
      $missingFields = array_diff_key($requiredFields, $result);
      if ($missingFields != []) {
        print("MISSING FIELD(S) $missingFields IN R_TXS $xid\n");
      }
      extract(just($requiredFields, $result));
      $data = unserialize($data) ?: [];
      $maxXid = max($maxXid, $xid);

      /* $hdrInfo = ray('xid type goods initiator initiatorAgent flags channel box risk risks reverses created', ); */
      /* $entryInfo = ray('xid amount uid agentUid description acctTid relType related', ); */
      /* $disputeInfo = ray('xid reason status', ); */

      // xid doesn't change
      // type doesn't change if it's in the correct list
      // goods doesn't change if it's valid

      // initiator[Agent] -- payee[Agent] or payer[Agent] -- turn off TAKING flag
      if (u\getBit($flags, B_TAKING)) {
        $initiator = $payee;
        $initiatorAgent = $payeeAgent;
        u\setBit($flags, B_TAKING, false);
      } else {
        $initiator = $payer;
        $initiatorAgent = $payerAgent;
      }
      
      // channel doesn't change
      // box doesn't change
      // risk doesn't change
      // risks doesn't change
      
      // created doesn't change

      // reverses
      // flags

      
      // forces in data
      if (is_array($data) and array_key_exists('force', $data)) {
        unset($data['force']);
      }

      // inv in data
      if (arrayGet($data, 'inv', '') != '') {
        $invoiceLinkFound = true;
        $invoiceLink = $data['inv'];
        unset($data['inv']);
      } else {
        $invoiceLinkFound = false;
      }
      
      // isGift in data
      if ((arrayGet($data, 'isGift', null) != null) and
          (!u\getBit($result['flags'], B_GIFT)) and
	  ($payee < 0)) { // repair flag   
	u\setBit($result['flags'], B_GIFT);
	unset($data['isGift']);
      }
      if (arrayGet($data, 'isGift', null) != null) {
        if (u\getBit($result['flags'], B_GIFT)) {
	  // it's cool
        } else {
          print("INCONSISTENCY: data contains isGift, payee=$payee, xid=$xid\n");
	  print_r($result);
        }
      }

      // Investments
      if (u\getBit($flags, B_INVESTMENT)) {
        print("INVESTMENT FLAG ON on $xid, record is " . print_r($result) . " -- IGNORING FLAG\n");
        u\setBit($flags, B_INVESTMENT, false);
      }

      // Throw away changes
      if ($data and array_key_exists('changes', $data)) {
        unset($data['changes']);
        /* print("CHANGES BEING DISCARDED on xid $xid\n"); */
      }

      if (array_key_exists('undoneNO', $data) or array_key_exists('unNO', $data)) {
        unset($data['undoneNO']);
        unset($data['unNO']);
      }
        
      // Disputed?
      if (u\getBit($flags, B_DISPUTED) or (array_key_exists('disputed', $data))) {
        u\setBit($flags, B_DISPUTED, false);
	if (array_key_exists('disputed', $data)) {
	  if ($data['disputed']) {  // still being disputed?
	    print("DISPUTED TRANSACTION: apparently still in dispute, xid=$xid\n");
	    print_r($result);
	    print_r(unserialize($result['data']));
	  } else {  //dispute resolved?
	    if (arrayGet($data, 'undoneBy', false)) { // dispute resolved by undo
	      $disputeInfo = ray('xid reason status', $xid, '', DS_ACCEPTED);
	      $disputesTbl->insert($disputeInfo)->save();
	    } else {  // dispute resolved by rejection
	      $disputeInfo = ray('xid reason status', $xid, '', DS_DENIED);
	      $disputesTbl->insert($disputeInfo)->save();
	      // print("DISPUTED TRANSACTION: apparently rejected, xid=$xid\n");
	      // print_r($result);
	      // print_r(unserialize($result['data']));
	    }
	    unset($data['disputed']);
	  }
	} else {  // dispute resolved by rejection
 	  $disputeInfo = ray('xid reason status', $xid, '', DS_DENIED);
 	  $disputesTbl->insert($disputeInfo)->save();
	   // print("DISPUTED TRANSACTION: flag set, data not set, xid=$xid\n");
	   // print_r($result);
	   // print_r(unserialize($result['data']));
	}
      }

      // reverses
      $reverses = null;
      if (arrayGet($data, 'undoes', null) != null) { // this tx reverses another tx
        $reverses = $data['undoes'];
        unset($data['undoes']);
        u\setBit($flags, B_UNDOES, false);  // just in case
        if (
        $undoneBy[$reverses] = $xid;
      }

      if (arrayGet($data, 'undoneBy', null) != null) { // apparently we're being undone
        u\setBit($flags, B_UNDONE, false);
        
      if (u\getBit($flags, B_UNDONE) or
          array_key_exists('undo', $data) or array_key_exists('undone', $data) or
          array_key_exists('undoes', $data) or array_key_exists('undoneBy', $data)) {
        if (u\getBit($flags, B_UNDONE) and (array_key_exists('undo', $data)
                                            or array_key_exists('undoneBy', $data))) {
          $undoneBy[$xid] = $data['undoneBy'];
          $undoes[$data['undoneBy']] = $xid;
          u\setBit($flags, B_UNDONE, false);
          unset($data['undo']);
          unset($data['undoneBy']);
        }
        if (u\getBit($flags, B_UNDOES) and (array_key_exists($xid, $undoes))) {
          u\setBit($flags, B_UNDOES, false);
          $reverses = $undoes[$xid];
          unset($undoes[$xid]);
        }
        if (u\getBit($flags, B_UNDOES) and strpos($payerFor, 'reverses')) {
          u\setBit($flags, B_UNDOES, false);
        }
      }
      if (u\getBit($flags, B_UNDONE) or u\getBit($flags, B_UNDOES) or
          array_key_exists('undo', $data) or array_key_exists('undone', $data) or array_key_exists('undoes', $data)
          or array_key_exists('undoneBy', $data)) {
        print("Record $xid participates in undoing\n");
        print_r(just('flags data xid', $result));
      }

      // How about B_ROUNDUP?
      if (u\getBit($flags, B_ROUNDUP)) {
        if ($amount > 0) {
	  $amount ?????????????????
}


      // check type, goods, flags, and data
      if (!array_key_exists($type, [TX_TRANSFER, TX_SIGNUP, TX_GRANT, TX_LOAN])) {
        print("NOT HANDLED: type is $type on $xid\n");
      }
      if (!array_key_exists($goods, [FOR_GOODS, FOR_USD, FOR_NONGOODS, FOR_SHARE])) {
        print("NOT HANDLED: goods is $goods on $xid\n");
      }
      if (($flags & ~$flagsMask) != 0) {
        print("NOT HANDLED: flags is $flags on $xid\n");
	print_r($data);
      }
      if ($data and $data != []) {
        print("NOT HANDLED: data is " . print_r($result['data'], true) . " on $xid\n");
      }
      
      // build r_tx_hdrs record
      $hdr = ray('xid type goods initiator initiatorAgent flags channel box risk risks reverses created',
                 $xid, $type, $goods, $initiator, $initiatorAgent, $flags, $channel, $box, $risk, $risks, $reverses, $created);
      
      /* if (u\getBit($flags, B_ROUNDUPS)) { */
      /*   debug("NOT HANDLED YET: ROUNDUPS on $xid"); */
      /* } */
    
      /* if (u\getBit($flags, B_ROUNDUP)) { */
      /*   debug("NOT HANDLED YET: ROUNDUP on $xid"); */
      /* } */
    
      /* if (u\getBit($flags, B_RECURS)) { */
      /*   debug("NOT HANDLED YET: RECURS on $xid"); */
      /* } */

      /* if (u\getBit($flags, B_LOAN)) { */
      /*   debug("UNEXPECTED LOAN on $xid"); */
      /* } */
    
      /* if (u\getBit($flags, B_INVESTMENT)) { */
      /*   debug("UNEXPECTED INVESTMENT on $xid"); */
      /* } */
    
      /* if (u\getBit($flags, B_STAKE)) { */
      /*   debug("UNEXPECTED STAKE on $xid"); */
      /* } */
    
      /* if (u\getBit($flags, B_FINE)) { */
      /*   debug("UNEXPECTED FINE on $xid"); */
      /* } */
    
      /* if (u\getBit($flags, B_NOASK)) { */
      /*   debug("UNEXPECTED NOASK on $xid"); */
      /* } */

      //
      $payerEntry = [];
      $payerEntry = just('xid', $result);
      $payerEntry['amount'] = 0-$amount;
      $payerEntry['uid'] = $payer;
      $payerEntry['agentUid'] = $payerAgent;
      $payerEntry['description'] = $payerFor;
      $payerEntry['acctTid'] = $payerTid;
      if ($invoiceLinkFound) {
        $payerEntry['relType'] = 'I';
        $payerEntry['related'] = $invoiceLink;
      } else {
        $payerEntry['relType'] = null;
        $payerEntry['related'] = null;
      }

      $payeeEntry = [];
      $payeeEntry = just('xid', $result);
      $payeeEntry['amount'] = $amount;
      $payeeEntry['uid'] = $payee;
      $payeeEntry['agentUid'] = $payeeAgent;
      $payeeEntry['description'] = $payeeFor;
      $payeeEntry['acctTid'] = $payeeTid;
      if ($invoiceLinkFound) {
        $payerEntry['relType'] = 'I';
        $payerEntry['related'] = $invoiceLink;
      } else {
        $payerEntry['relType'] = null;
        $payerEntry['related'] = null;
      }

      $txHdrsTable->insert($hdr)->save();
      $entriesTable->insert($payerEntry)->insert($payeeEntry)->save();
    }

    print("Leftover undone bys:\n");
    foreach ($undoneBy as $k => $v) {
      print("$k was undone by $v\n");
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
      
      $xid = $nextXid;
      $type = TX_BANK;
      $goods = FOR_USD;
      $initiator = $payee; // Assume that the payee initiated the transaction, because we don't really know.
      $initiatorAgent = $payee; // And that they did it themselves...
      $flags = 0;
      $box = null;
      $reverses = null;
      $created = $completed;
      
      $hdr = compact('xid type goods initiator intiatorAgent flags channel box risk risks reverses created');

      $e1 = compact('xid amount uid agentUid description acctTid relType related',
                    $xid, -$amount, CG_BANK_UID, CG_BANK_UID, ($amount > 0) ? 'from bank' : 'to bank', $bankTxId, null, null);

      $e2 = compact('xid amount uid agentUid description acctTid relType related',
                    $xid, $amount, $payee, $payee, ($amount > 0) ? 'from bank' : 'to bank', $txid, null, null);


      $txHdrsTable->insert($hdr)->save();
      $entriesTable->insert($e1)->insert($e2)->save();
      if ($this->execute("UPDATE r_usd SET xid='$xid' WHERE txid='$txid'") != 1) {
        print("ERROR UPDATING r_usd on xid=$xid\n");
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
        $result['initiator'] = CG_BANK_UID;
        break;
      case 'T':
        $result['initiator'] = CG_ADMIN_UID;
        break;
      default:
        print('BAD USD2 TYPE: ' . $result['type'] . "\n");
      }

      $xid = $nextXid;
      $initiator = ($result['type'] == 'S') ? CG_BANK_UID : CG_ADMIN_UID;
      $initiatorAgent = $initiator;

      $hdr = compact('xid type goods initiator intiatorAgent flags channel box risk risks reverses created',
                     $xid, TX_BANK, FOR_USD, $initiator, $initiatorAgent, 0, null, null, null, null, $completed);
      $e1 = compact('xid amount uid agentUid description acctTid relType related',
                    $xid, -$amount, CG_BANK_UID, CG_BANK_UID, $memo, $bankTxId, null, null);
      $e2 = compact('xid amount uid agentUid description acctTid relType related',
                    $xid, $amount, CG_BANK_UID, CG_BANK_UID, $memo, $bankTxId, null, null);

      $txHdrsTable->insert($hdr)->save();
      $entriesTable->insert($e1)->insert($e2)->save();
      if ($this->execute("UPDATE r_usd2 SET xid='$xid' WHERE id='$id'") != 1) {
        print("ERROR UPDATING r_usd2 on xid=$xid\n");
      }
    }

  }

  public function down() {
    $this->execute('DELETE FROM r_entries');
    $this->execute('DELETE FROM r_tx_hdrs');
    $this->execute('UPDATE r_usd SET xid=DEFAULT');
    $this->execute('UPDATE r_usd2 SET xid=DEFAULT');
  }
}

function arrayGet($arr, $key, $dft) {
  return (is_array($arr) and array_key_exists($key, $arr)) ? $arr[$key] : $dft;
}
