<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

include __DIR__ . '/recreate-views.inc';
include __DIR__ . '/recreate-triggers.inc';

class SimplifyTxs extends AbstractMigration {
  /**
   * Change Method.
   *
   * Write your reversible migrations using this method.
   *
   * More information on writing migrations is available here:
   * http://docs.phinx.org/en/latest/migrations.html#the-abstractmigration-class
   *
   * The following commands can be used in this method and Phinx will
   * automatically reverse them when rolling back:
   *
   *    createTable
   *    renameTable
   *    addColumn
   *    renameColumn
   *    addIndex
   *    addForeignKey
   *
   * Remember to call "create()" or "update()" and NOT "save()" when working
   * with the Table class.
   */
  public function up() { // not change() for anything, because order matters
    $m = $this;
    cgpr('** _____ starting migration SimplifyTxs ____ **');
    extract(BANK_IDS);
    $isBank = "uid IN ($bankIn, $bankOut)";
    $isBankish = "uid IN ($bankIn, $bankOut, $bankCharges)";
    foreach (ray(E_TYPES) as $i => $k) ${$k . 'Type'} = $i;
    $eTypes = array_flip(ray(E_TYPES));

    cgpr('clearing triggers');
    clearTriggers($m);

    cgpr('setting new entryTypes');
    cgpr('..for rebates and usd_fees');
    $m->doSql("UPDATE tx_entries_all SET entryType=IF(relType='D',$rebateType,$usd_feeType) WHERE entryType=0");
    cgpr('..for payees and donations');
    $m->doSql("UPDATE tx_entries_all SET entryType=entryType-1 WHERE entryType>1");

    $sql = 'UPDATE tx_entries_all e JOIN tx_entries_all other USING(xid) SET e.entryType=_that WHERE e.uid<>other.uid AND (_where)';
    $subs = ray('_that _where');
    
    cgpr('..for banking');
    $_that = $bankType;
    $_where = "e.$isBank XOR other.$isBank";
    $m->doSql(strtr($sql, compact($subs)));

    cgpr('..for bank charges');
    $_that = $bank_onlyType;
    $_where = "e.$isBankish AND other.$isBankish";
    $m->doSql(strtr($sql, compact($subs)));
    
    cgpr('creating views');
    createViews($m); // create VIEWS, starting from scratch
    
    cgpr('renumbering entries so pairs are easier to track (the "from" entry id is the negative of the "to" entry id');
    $idCount = $m->fetchRow('SELECT MAX(id) AS max FROM tx_entries_all')[0];
    $base = 2 * $idCount; // make room for the current negative IDs
    $m->doSql("UPDATE tx_entries_all SET id=$base+id");

    $rows = $m->fetchAll('SELECT id AS oldId,entryType,uid,amount,(t.reversesXid IS NOT NULL) AS reverses,xid FROM tx_entries_all e JOIN tx_hdrs_all t USING(xid) ORDER BY xid, entryType, id');
    $id = 1;
    $xNewId = 'zot';
    foreach ($rows as $row) {
      extract($row);
      if ($entryType == $eTypes['bank']) {
        $newId = ($uid == $bankIn or $uid == $bankOut) ? -$id : $id;
      } elseif ($amount + 0 == 0) {
        $newId = ($oldId % 2) ? -$id : $id; // there shouldn't be any zero transactions yet, but there are!
      } else $newId = ($reverses xor $amount < 0) ? -$id : $id;

      if ($newId == $xNewId) die("Duplicate new ID $newId for old entry #" . ($oldId - $base) . ": xid=$xid, entryType=$entryType uid=$uid amount=$amount");
      $m->chg($oldId, $newId);
      
      $xNewId = $newId;
      
      if (($oldId%2) == 0) {
        if (!($id % 1000)) cgpr("Renumbered tx_entries to ID #$id\n");
        $id++;
      }
    }
    
    cgpr('creating triggers');
    createTriggers($m); // create TRIGGERS, starting from scratch (do it last, so nothing gets changed in users table)

    $m->doSql("UPDATE users SET fullName='Community Fund' WHERE uid IN (129, 130)"); // fix roundup and crumb names
    $m->doSql('ALTER TABLE r_areas ENGINE = InnoDB'); // the only table we have that is still ISAM
    $m->doSql('ALTER TABLE tx_entries_all ENGINE = InnoDB'); // this rebuilds the index
    
    // this should have been done in an earlier migration (now is)
    if (!$m->table('x_users')->hasColumn('preid')) {
      $m->table('x_users')
        ->addColumn('preid', 'integer', ['length' => MysqlAdapter::INT_BIG, 'null' => FALSE, 'default' => 0, 'after' => 'lastip', 'comment' => 'signup record ID'])
        ->update();       
    }
    // update `tx_entries_all` SET deleted=1561132677 WHERE xid=122985
    // update `tx_entries_all` set deleted=1561132677 WHERE `xid` = 121730
  }
  
  public function down() {
    $m = $this;
    foreach (ray(E_TYPES) as $i => $k) ${$k . 'Type'} = $i;

    cgpr('changing entry types back to 0 to 3');
    $m->doSql("UPDATE tx_entries_all SET entryType=IF(entryType IN ($rebateType, $usd_feeType), 0, IF(entryType=2, 3, IF(e.id<0, 1, 2)))");

    cgpr('renumbering entries in odd/even pairs starting at 1 again');
    $idCount = $m->fetchRow('SELECT MAX(id) AS max FROM tx_entries_all')[0];
    $base = 2 * $idCount; // make room for the current negative IDs
    $m->doSql("UPDATE tx_entries_all SET id=$base+id WHERE id>0");
    
    for ($id = 1; $id <= $idCount; $id++) {
      $m->chg($base+$id, 2*$id-1);
      $m->chg(-$id, 2*$id);
      if (!($id % 1000)) cgpr("Renumbered tx_entries ID #$id of $idCount\n");
    }
    
    cgpr('creating old views');
    cgpr('..basic');
    createViews($m, 20190622);
    cgpr('..tx_entries_payer');
    $m->doSql('CREATE VIEW tx_entries_payer AS SELECT * FROM tx_entries_all WHERE entryType=1 AND deleted IS NULL');
    cgpr('..tx_entries_payee');
    $m->doSql('CREATE VIEW tx_entries_payee AS SELECT * FROM tx_entries_all WHERE entryType=2 AND deleted IS NULL');

    cgpr('..txs');
    $sql = <<< X
      CREATE VIEW txs AS 
      SELECT t0.*, 
      e1.amount AS amt1, e1.uid AS uid1, e1.agentUid AS agt1, e1.description AS for1, 
        e1.acctTid AS tid1, e1.relType AS relType1, e1.relatedId AS rel1,
      e2.amount AS amt2, e2.uid AS uid2, e2.agentUid AS agt2, e2.description AS for2, 
        e2.acctTid AS tid2, e2.relType AS relType2, e2.relatedId AS rel2
      FROM tx_hdrs t0
      JOIN tx_entries_payer e1 USING (xid)
      JOIN tx_entries_payee e2 USING (xid)
X;
    $m->doSql($sql);

    cgpr('..txs_noreverse');
    $m->doSql('CREATE VIEW txs_noreverse AS SELECT * FROM txs t WHERE reversesXid IS NULL AND NOT EXISTS(SELECT xid FROM tx_hdrs tr WHERE tr.reversesXid = t.xid)');
  }
  
  private function chg($oldId, $newId) {
    $this->execute("UPDATE tx_entries_all SET id=$newId WHERE id=$oldId"); // don't use doSql here (too much output)
//    cgpr("changed $oldId to $newId\n");
  }

  public function doSql($sql) {
//    cgpr("$sql\n");
    $this->execute($sql);
  }
}
