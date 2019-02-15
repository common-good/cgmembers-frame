<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class CreateTableREntries extends AbstractMigration
{
  public function up()
  {
    $entryTable = $this->table('r_entries', ['comment' => 'Record of a transaction line entry']);  // primary key is id
    
    $this->addBigInt($entryTable, 'xid', ['length' => 20, 'null' => false, 'comment' => 'the ID of the transaction to which this entry belongs']);
    /* $this->addTinyInt($entryTable, 'entryType', ['length' => 4, 'null' => 'false', 'comment' => 'entry type']); */
    $entryTable->addColumn('amount', 'decimal', ['precision' => 11, 'scale' => 2, 'signed' => true, 'null' => false, 'default' => '0.00', 'comment' => 'amount, may be negative']);
    $this->addBigInt($entryTable, 'uid', ['length' => 20, 'null' => true, 'comment' => 'user id of the account to which this entry applies (null indicates external source, e.g., for money transferred in from a bank']);
    $this->addBigInt($entryTable, 'agentUid', ['length' => 20, 'null' => true, 'comment' => "user id of account's agent (who approved this transaction for this account)"]);
    $entryTable->addColumn('description', 'string', ['length' => 255, 'default' => 'NULL', 'comment' => 'description for this entry']);
    $entryTable->addColumn('acctTid', 'integer', ['length' => 11, 'null' => true, 'comment' => 'transaction ID for the account to which this entry applies']);
    $entryTable->addColumn('relType', 'string', ['length' => 1, 'null' => true, 'comment' => "type of related record, 'D' for coupated, 'I' for invoice"]);
    $this->addBigInt($entryTable, 'related', ['length' => 20, 'null' => true, 'comment' => "id of related record"]);
    
    $entryTable->addIndex(['xid']);
    $entryTable->addIndex(['uid']);

    /* $entryTable->addForeignKey('xid', 'r_tx_hdrs', 'xid', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */
    /* $entryTable->addForeignKey('uid', 'users', 'uid', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */
    /* $entryTable->addForeignKey('agentUid', 'users', 'uid', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */

    $entryTable->create();
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

  public function down() {
    $this->table('r_entries')->drop();
  }
}
