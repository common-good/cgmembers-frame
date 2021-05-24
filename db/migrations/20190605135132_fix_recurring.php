<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class FixRecurring extends AbstractMigration
{
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
      $this->execute("SET GLOBAL sql_mode=(SELECT REPLACE(@@sql_mode,'ONLY_FULL_GROUP_BY',''))");
      $this->table('r_recurs')
      ->addColumn('purpose', 'string', ['length' => 255, 'null' => true, 'comment' => 'purpose of the recurring payment', 'after' => 'amount'])
      ->update();

      $this->table('tx_hdrs_all')
      ->addColumn('recursId', 'integer', ['length' => MysqlAdapter::INT_BIG, 'default' => 0, 'null' => FALSE, 'comment' => 'related record in recurs table', 'after' => 'risks'])
      ->update();
      
      $this->table('r_invoices')
      ->addColumn('recursId', 'integer', ['length' => MysqlAdapter::INT_BIG, 'default' => 0, 'null' => FALSE, 'comment' => 'related record in recurs table', 'after' => 'data'])
      ->update();  

      $this->execute('DROP VIEW tx_hdrs');
      $this->execute('CREATE VIEW tx_hdrs AS SELECT * FROM tx_hdrs_all WHERE deleted IS NULL');

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
      $this->execute($sql);

      $this->execute('CREATE VIEW txs_noreverse AS SELECT * FROM txs t WHERE reversesXid IS NULL AND NOT EXISTS(SELECT xid FROM tx_hdrs tr WHERE tr.reversesXid = t.xid)');

      $sql = <<< X
        UPDATE txs t
        JOIN (
          SELECT MAX(id) AS id, payer, payee, amount, created FROM r_recurs
          GROUP BY payer, payee, amount, created
        ) r ON r.payer=t.uid1 AND r.payee=t.uid2 AND r.amount=t.amt2 AND r.created<=t.created+60 AND t.flags&(1<<9)
        SET t.recursId=r.id
X;
      $this->execute($sql);
      
      $sql = 'UPDATE txs SET for1=MID(for1, 1, IF(LOCATE(CHAR(40), for1) > 2, LOCATE(CHAR(40), for1) - 1, 9999)) WHERE recursId';
      $this->execute($sql); // can't SET more than one underlying table at a time
      $this->execute(str_replace('for1', 'for2', $sql));

      $sql = <<< X
        UPDATE r_recurs r LEFT JOIN (
          SELECT for2 AS `purpose`, recursId FROM txs ORDER BY xid
        )  t ON t.recursId=r.id
        SET r.`purpose`=IF(r.payee=26742000000002, 'donation', t.`purpose`)
X;
      $this->execute($sql);
    }
    
    public function down() {
      $this->execute('DROP VIEW txs_noreverse');
      $this->execute('DROP VIEW txs');
      $this->execute('ALTER TABLE r_invoices DROP COLUMN recursId');
      $this->execute('ALTER TABLE tx_hdrs_all DROP COLUMN recursId');
      $this->execute('ALTER TABLE r_recurs DROP COLUMN `purpose`');
      $this->execute('DROP VIEW tx_hdrs');
      $this->execute('CREATE VIEW tx_hdrs AS SELECT * FROM tx_hdrs_all WHERE deleted IS NULL');
    }
}
