<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

require_once 'cgmembers/rcredits/bootstrap.inc';
require_once 'cgmembers/rcredits/defs.inc';

class CreateTableRDisputes extends AbstractMigration
{
  public function up()
  {
    $disputesTable = $this->table('all_disputes', ['comment' => 'record of disputes transactions']);

    $this->addBigInt($disputesTable, 'xid', ['length' => 20, 'null' => false, 'comment' => 'id of the transaction in dispute']);
    $disputesTable->addColumn('reason', 'string', ['length' => 255, 'null' => false, 'comment' => 'reason the transaction is being disputes']);
    $this->addTinyInt($disputesTable, 'status', ['length' => 4, 'null' => false, 'default' => DS_OPEN, 'comment' => 'status of the dispute']);
    $this->addBigInt($disputesTable, 'deleted', ['length' => 20, 'null' => true, 'default' => true, 'comment' => "unix timestamp of when the dispute record was deleted, null if it hasn't been"]);
    $disputesTable->create();

    $this->execute('CREATE VIEW r_disputes AS SELECT id, xid, reason, status FROM all_disputes WHERE deleted IS NULL');
    $this->execute('CREATE VIEW x_disputes AS SELECT id, xid, reason, status, deleted FROM all_disputes WHERE deleted IS NOT NULL');
  }

  public function down() {
    $this->execute('DROP VIEW IF EXISTS x_disputes');
    $this->execute('DROP VIEW IF EXISTS r_disputes');
    $this->table('all_disputes')->drop();
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
