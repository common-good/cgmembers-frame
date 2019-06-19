<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class FoodPercents extends AbstractMigration
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
    public function change() {
      $this->table('x_invoices')
      ->addColumn('recursId', 'integer', ['length' => MysqlAdapter::INT_BIG, 'default' => 0, 'null' => FALSE, 'comment' => 'related record in recurs table', 'after' => 'data'])
      ->update(); 
      foreach (['r_photos', 'x_photos'] as $table) {
        $this->table($table)
        ->addColumn('thumb', 'blob', ['length' => MysqlAdapter::BLOB_REGULAR, 'default' => NULL, 'null' => TRUE, 'comment' => 'small version of photo', 'after' => 'photo'])
        ->update(); 
      }
    }
}

