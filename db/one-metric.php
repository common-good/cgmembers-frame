<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class OneMetric extends AbstractMigration {

  public function change() {
    $this->table('r_stats')
      ->addColumn('patronage', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'current intended recurring donations per month', 'after' => 'basket']) 
      ->addColumn('roundups', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'average roundups per month in the recent past', 'after' => 'patronage']) 
      ->addColumn('crumbs', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'average  crumbs per month in the recent past', 'after' => 'roundups']) 
      ->addColumn('invites', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'total invitations to date', 'after' => 'crumbs']) 
      ->update()

      ->changeColumn('payees', 'comment' => 'median number of payees per active account in the recent past')
      ->changeColumn('basket', 'comment' => 'median (positive) amount per transaction in the recent past')
      ->save();    

/* (already done on production server)
      $this->table('x_users')
      ->addColumn('latitude', 'decimal', ['precision' => 11, 'scale'=>8, 'null' => false, 'default' => '0', 'comment' => 'latitude of account\'s physical address', 'after' => 'country']) 
      ->addColumn('longitude', 'decimal', ['precision' => 11, 'scale'=>8, 'null' => false, 'default' => '0', 'comment' => 'longitude of account\'s physical address', 'after' => 'latitude']) 
      ->update();  
  }
*/
}
