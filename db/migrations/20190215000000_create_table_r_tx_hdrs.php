<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class CreateTableRTxHdrs extends AbstractMigration
{
  public function up()
  {
    $hdrTable = $this->table('r_tx_hdrs', ['id' => false, 'primary_key' => 'xid', 'comment' => 'Record of all rCredits transactions in the region']);

    $this->addBigInt($hdrTable, 'xid', ['length' => 20, 'identity' => true, 'null' => false, 'comment' => 'the unique transaction ID']);
    $this->addTinyInt($hdrTable, 'type', ['length' => 4, 'null' => false, 'comment' => 'transaction type (transfer, rebate, etc.)']);
    $this->addTinyInt($hdrTable, 'goods', ['length' => 4, 'null' => false, 'comment' => 'is this transfer an exchange for real goods and services?']);
    $this->addBigInt($hdrTable, 'initiator', ['length' => 20, 'null' => false, 'comment' => "user id of the transaction's initiator"]);
    $this->addBigInt($hdrTable, 'initiatorAgent', ['length' => 20, 'null' => true, 'comment' => 'user id of the agent for the initiator (who actually initiated this transaction)']);
    $this->addBigInt($hdrTable, 'flags', ['length' => 20, 'signed' => false, 'null' => false, 'default' => '0', 'comment' => 'boolean characteristics and state flags']);
    $this->addTinyInt($hdrTable, 'channel', ['length' => 4, 'comment' => 'through what medium was the transaction entered']);
    $this->addBigInt($hdrTable, 'box', ['length' => 20, 'null' => true, 'comment' => 'on what machine was the transaction entered']);
    $hdrTable->addColumn('risk', 'float', ['null' => true, 'comment' => 'suspiciousness rating']);
    $this->addBigInt($hdrTable, 'risks', ['length' => 20, 'signed' => false, 'null' => false, 'default' => '0', 'comment' => 'list of risk factors']);
    $this->addBigInt($hdrTable, 'reverses', ['length' => 20, 'null' => true, 'comment' => 'xid of the transaction this one reverses (if any)']);
    $this->addBigInt($hdrTable, 'created', ['length' => 11, 'null' => false, 'default' => '0', 'comment' => 'Unixtime transaction was created']);

    $hdrTable->addIndex(['xid'], ['unique' => 'true']);
    $hdrTable->addIndex(['initiator']);
    $hdrTable->addIndex(['created']);
    $hdrTable->addIndex(['reverses'], ['unique' => true]);

    /* $hdrTable->addForeignKey('initiator', 'users', 'uid', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */
    /* $hdrTable->addForeignKey('initiatorAgent', 'users', 'uid', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */
    /* $hdrTable->addForeignKey('box', 'r_boxes', 'id', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */
    /* $hdrTable->addForeignKey('reverses', 'r_tx_hdrs', 'xid', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */

    $hdrTable->create();
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
    $this->table('r_tx_hdrs')->drop();
  }
}
