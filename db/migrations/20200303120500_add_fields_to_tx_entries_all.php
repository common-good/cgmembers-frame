<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

include_once __DIR__ . '/recreate-views.inc';

class AddFieldsToTxEntriesAll extends AbstractMigration
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
    $table = $this->table('tx_entries_all');
    $table
      ->addColumn('rule', 'integer', ['null' => true, 'default' => null,
                                      'comment' => 'Auxiliary transaction to which this entry is related.'])
      /* ->addForeignKey('rule', 'tx_rules', 'id', ['delete' => 'restrict']) */
      ->update();
    createViews($this);
  }

  public function doSql($sql) {
//    pr("$sql\n");
    $this->execute($sql);
  }
}
