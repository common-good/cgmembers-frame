<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class PayWithCgLink extends AbstractMigration {

  public function change() {
    $this->table('users')
      ->addColumn('latitude', 'decimal', ['precision' => 11, 'scale'=>9, 'null' => false, 'default' => '0', 'comment' => 'latitude of account\'s physical address', 'after' => 'country']) 
      ->addColumn('longitude', 'decimal', ['precision' => 11, 'scale'=>9, 'null' => false, 'default' => '0', 'comment' => 'longitude of account\'s physical address', 'after' => 'latitude']) 
        ->update();
    }
    $this->table('r_stats')
      ->addColumn('payees', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'median number of payees per active account over the past 30 days', 'after' => 'usdOutCount']) 
      ->addColumn('basket', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'median (positive) amount per transaction over the past 30 days', 'after' => 'payees']) 
      ->update();    
  }

}
