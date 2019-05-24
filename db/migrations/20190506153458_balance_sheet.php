<?php
use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class BalanceSheet extends AbstractMigration {
  /**
   * Change Method.
   *
   * More information on writing migrations is available here:
   * http://docs.phinx.org/en/latest/migrations.html#the-abstractmigration-class
   *
   * Remember to call "create()" or "update()" and NOT "save()" when working
   * with the Table class.
   */
  public function change() {
    $this->table('r_investments')
      ->addColumn('reserve', 'decimal', ['precision' => 10, 'scale'=>3, 'null' => false, 'default' => '0', 'comment' => 'fraction to hold in reserve for possible loss', 'after' => 'soundness']) 
      ->update();
  }
}
