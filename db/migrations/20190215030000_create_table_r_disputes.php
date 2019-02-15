<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

require_once 'cgmembers/rcredits/bootstrap.inc';
require_once 'cgmembers/rcredits/defs.inc';

class CreateTableRDisputes extends AbstractMigration
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
    public function change()
    {
      $disputesTable = $this->table('r_disputes', ['comment' => 'record of disputes transactions']);
      
      $this->addBigInt($disputesTable, 'xid', ['length' => 20, 'null' => false, 'comment' => 'id of the transaction in dispute']);
      $disputesTable->addColumn('reason', 'string', ['length' => 255, 'null' => false, 'comment' => 'reason the transaction is being disputes']);
      $this->addTinyInt($disputesTable, 'status', ['length' => 4, 'null' => false, 'default' => DS_OPEN, 'comment' => 'status of the dispute']);
      $disputesTable->create();
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
