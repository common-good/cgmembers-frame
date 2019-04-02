<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class FixSetlocus extends AbstractMigration
{
  public function up() {
    $this->table('users')
      ->changeColumn('latitude', 'decimal', ['precision' => 11, 'scale'=>8, 'null' => false, 'default' => '0', 'comment' => 'latitude of account\'s physical address', 'after' => 'country']) 
      ->changeColumn('longitude', 'decimal', ['precision' => 11, 'scale'=>8, 'null' => false, 'default' => '0', 'comment' => 'longitude of account\'s physical address', 'after' => 'latitude']) 
      ->save();
  }

  public function down() {
    $this->table('users')
      ->changeColumn('latitude', 'decimal', ['precision' => 11, 'scale'=>9, 'null' => false, 'default' => '0', 'comment' => 'latitude of account\'s physical address', 'after' => 'country']) 
      ->changeColumn('longitude', 'decimal', ['precision' => 11, 'scale'=>9, 'null' => false, 'default' => '0', 'comment' => 'longitude of account\'s physical address', 'after' => 'latitude']) 
      ->save();
  }
}
