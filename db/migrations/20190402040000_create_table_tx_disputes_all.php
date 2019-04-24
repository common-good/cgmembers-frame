<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

define('DS_OPEN', 1);

class CreateTableTxDisputesAll extends AbstractMigration
{
  public function up()
  {
    $disputesTable = $this->table('tx_disputes_all', ['comment' => 'record of dispute of transaction']);

    $this->addBigInt($disputesTable, 'xid', ['length' => 20, 'null' => false, 'comment' => 'id of the transaction in dispute']);
    $this->addBigInt($disputesTable, 'uid', ['length' => 20, 'null' => false, 'comment' => 'id of the user who disputes the transaction']);
    $disputesTable->addColumn('reason', 'string', ['length' => 255, 'null' => false, 'comment' => 'reason the transaction is being disputes']);
    $this->addTinyInt($disputesTable, 'status', ['length' => 4, 'null' => false, 'default' => DS_OPEN, 'comment' => 'status of the dispute']);
    $this->addBigInt($disputesTable, 'deleted', ['length' => 20, 'null' => true, 'default' => true, 'comment' => "unix timestamp of when the dispute record was deleted, null if it hasn't been"]);
    $disputesTable->create();

    $this->execute('CREATE VIEW tx_disputes AS SELECT id, xid, uid, reason, status FROM tx_disputes_all WHERE deleted IS NULL');
    $this->execute('CREATE VIEW tx_disputes_deleted AS SELECT id, xid, uid, reason, status, deleted FROM tx_disputes_all WHERE deleted IS NOT NULL');
  }

  public function down() {
    $this->execute('DROP VIEW IF EXISTS tx_disputes');
    $this->execute('DROP VIEW IF EXISTS tx_disputes_deleted');
    $this->table('tx_disputes_all')->drop();
  }
  
  private function addTinyInt($table, $name, $options = []) {
    $options['length'] = MysqlAdapter::INT_TINY;
    $table->addColumn($name, 'integer', $options);
    return $table;
  }

  private function addBigInt($table, $name, $options = []) {
    $options['length'] = MysqlAdapter::INT_BIG;
    $table->addColumn($name, 'integer', $options);
    return $table;
  }
}
