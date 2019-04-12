<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class AlterUsdTables extends AbstractMigration
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
    $usdTable = $this->table('r_usd');
    $usdTable->addColumn('xid', 'integer', ['length' => MysqlAdapter::INT_BIG, 'default' => 0, 'comment' => 'id of related tx_hdrs record']);
    $usdTable->update();

    $usd2Table = $this->table('r_usd2');
    $usd2Table->addColumn('xid', 'integer', ['length' => MysqlAdapter::INT_BIG, 'default' => 0, 'comment' => 'id of related tx_hdrs record']);
    $usd2Table->update();

    $xusdTable = $this->table('x_usd');
    $xusdTable->addColumn('xid', 'integer', ['length' => MysqlAdapter::INT_BIG, 'default' => 0, 'comment' => 'id of related tx_hdrs record']);
    $xusdTable->update();

    /* $xusd2Table = $this->table('x_usd2'); */
    /* $xusd2Table->addColumn('xid', 'integer', ['length' => MysqlAdapter::INT_BIG, 'default' => 0, 'comment' => 'id of related tx_hdrs record']); */
    /* $xusd2Table->update(); */
  }
}
