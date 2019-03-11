<?php


use Phinx\Migration\AbstractMigration;

class PayWithCgLink extends AbstractMigration
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
    $this->table('users')
      ->addColumn('latitude', 'decimal', ['precision' => 11, 'scale'=>9, 'null' => false, 'default' => '0', 'comment' => 'latitude of account\'s physical address', 'after' => 'country']) 
      ->addColumn('longitude', 'decimal', ['precision' => 11, 'scale'=>9, 'null' => false, 'default' => '0', 'comment' => 'longitude of account\'s physical address', 'after' => 'latitude']) 
      ->update();
    $this->table('r_stats')
      ->addColumn('payees', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'median number of payees per active account over the past 30 days', 'after' => 'usdOutCount']) 
      ->addColumn('basket', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'median (positive) amount per transaction over the past 30 days', 'after' => 'payees']) 
      ->update();    
    
  }
}
