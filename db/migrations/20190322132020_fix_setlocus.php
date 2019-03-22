<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class FixSetlocus extends AbstractMigration
{
  public function up() {
    $this->table('users')
      ->changeColumn('latitude', 'decimal', ['precision' => 11, 'scale'=>8])
      ->changeColumn('longitude', 'decimal', ['precision' => 11, 'scale'=>8])
      ->save();
  }

  public function down() {}
}
