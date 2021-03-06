<?php


use Phinx\Migration\AbstractMigration;

class OneMetric extends AbstractMigration
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
  public function up() {
    $this->table('r_stats')
      ->addColumn('patronage', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'current intended recurring donations per month', 'after' => 'basket']) 
      ->addColumn('roundups', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'average roundups per month in the recent past', 'after' => 'patronage']) 
      ->addColumn('crumbs', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'average crumbs per month in the recent past', 'after' => 'roundups']) 
      ->addColumn('invites', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'total invitations to date', 'after' => 'crumbs']) 

      ->changeColumn('payees', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'median number of payees per active account in the recent past', 'after' => 'usdOutCount']) 
      ->changeColumn('basket', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'median (positive) amount per transaction in the recent past', 'after' => 'payees']) 
      ->save();
    
    //!!!!!!!!!!!! The following fields should have been added in 20190311170655_pay_with_cg_link.php, but were
    //!!!!!!!!!!!! missed when that migration was run on production, so that migration has been modified, and we're
    //!!!!!!!!!!!! adding them here conditionally just to be sure.
    $x_users = $this->table('x_users');
    if (!$x_users->hasColumn('latitude')) { // use latitude as a marker
      $x_users
        ->addColumn('latitude', 'decimal', ['precision' => 11, 'scale'=>8, 'null' => false, 'default' => '0', 'comment' => "latitude of account's physical address", 'after' => 'country']) 
        ->addColumn('longitude', 'decimal', ['precision' => 11, 'scale'=>8, 'null' => false, 'default' => '0', 'comment' => "longitude of account's physical address", 'after' => 'latitude']) 
        ->save();
    }
  }

  public function down() {
    $this->table('r_stats')
      ->removeColumn('patronage', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'current intended recurring donations per month', 'after' => 'basket']) 
      ->removeColumn('roundups', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'average roundups per month in the recent past', 'after' => 'patronage']) 
      ->removeColumn('crumbs', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'average  crumbs per month in the recent past', 'after' => 'roundups']) 
      ->removeColumn('invites', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'total invitations to date', 'after' => 'crumbs']) 
      ->save();

    $this->table('r_stats')
      ->changeColumn('payees', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'median number of payees per active account over the past 30 days', 'after' => 'usdOutCount']) 
      ->changeColumn('basket', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'median (positive) amount per transaction over the past 30 days', 'after' => 'payees']) 
      ->save();    
  }
}
