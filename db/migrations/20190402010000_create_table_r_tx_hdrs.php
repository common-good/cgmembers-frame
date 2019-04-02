<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class CreateTableRTxHdrs extends AbstractMigration
{
  public function up()
  {
    $hdrTable = $this->table('all_tx_hdrs', ['id' => false, 'primary_key' => 'xid', 'comment' => 'Record of all rCredits transactions in the region']);

    $this->addBigInt($hdrTable, 'xid', ['length' => 20, 'identity' => true, 'null' => false, 'comment' => 'the unique transaction ID']);
    $this->addBigInt($hdrTable, 'actorId', ['length' => 20, 'null' => false, 'comment' => "user id of the transaction's initiator"]);
    $this->addBigInt($hdrTable, 'actorAgentId', ['length' => 20, 'null' => true, 'comment' => 'user id of the agent for the initiator (who actually initiated this transaction)']);
    $this->addBigInt($hdrTable, 'flags', ['length' => 20, 'signed' => false, 'null' => false, 'default' => '0', 'comment' => 'boolean characteristics and state flags']);
    $this->addTinyInt($hdrTable, 'channel', ['length' => 4, 'default' => 0, 'comment' => 'through what medium was the transaction entered']);
    $this->addBigInt($hdrTable, 'box', ['length' => 20, 'null' => true, 'comment' => 'on what machine was the transaction entered']);
    $this->addTinyInt($hdrTable, 'goods', ['null' => false, 'comment' => 'kind of thing being dealt in']);
    $hdrTable->addColumn('risk', 'float', ['null' => true, 'comment' => 'suspiciousness rating']);
    $this->addBigInt($hdrTable, 'risks', ['length' => 20, 'signed' => false, 'null' => false, 'default' => '0', 'comment' => 'list of risk factors']);
    $this->addBigInt($hdrTable, 'reversesXid', ['length' => 20, 'null' => true, 'comment' => 'xid of the transaction this one reverses (if any)']);
    $this->addBigInt($hdrTable, 'created', ['length' => 11, 'null' => false, 'default' => '0', 'comment' => 'Unixtime transaction was created']);
    $this->addBigInt($hdrTable, 'deleted', ['length' => 11, 'null' => true, 'default' => null, 'comment' => 'Unixtime transaction was deleted, null if it has not been']);

    $hdrTable->addIndex(['xid'], ['unique' => 'true']);
    $hdrTable->addIndex(['actorId']);
    $hdrTable->addIndex(['created']);
    $hdrTable->addIndex(['reversesXid'], ['unique' => true]);

    /* $hdrTable->addForeignKey('actorId', 'users', 'uid', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */
    /* $hdrTable->addForeignKey('actorAgentId', 'users', 'uid', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */
    /* $hdrTable->addForeignKey('box', 'r_boxes', 'id', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */
    /* $hdrTable->addForeignKey('reversesXid', 'r_tx_hdrs', 'xid', ['delete' => 'RESTRICT', 'update' => 'CASCADE']); */

    $hdrTable->create();

    $this->execute('CREATE VIEW r_tx_hdrs AS SELECT xid, actorId, actorAgentId, flags, channel, box, goods, risk, risks, reversesXid, created FROM all_tx_hdrs WHERE deleted IS NULL');
    $this->execute('CREATE VIEW x_tx_hdrs AS SELECT xid, actorId, actorAgentId, flags, channel, box, goods, risk, risks, reversesXid, created, deleted FROM all_tx_hdrs WHERE deleted IS NOT NULL');
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
    $this->execute('DROP VIEW IF EXISTS x_tx_hdrs');
    $this->execute('DROP VIEW IF EXISTS r_tx_hdrs');
    $this->execute('DROP TABLE IF EXISTS r_tx_hdrs');
    $this->execute('DROP TABLE IF EXISTS all_tx_hdrs');
  }
}
