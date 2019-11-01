<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;
define('CONTRACT_DIR', __DIR__ . '/../../contracts');

class EasyTweaks extends AbstractMigration {
  public function up() {

    if ($this->hasTable('r_contracts')) $this->execute('DROP TABLE r_contracts');
    pr('contract directory: ' . CONTRACT_DIR);
    if (!is_dir(CONTRACT_DIR)) mkdir(CONTRACT_DIR, 0755) or die('cannot create contract directory');
  }
  
  public function down() {
    if (is_dir(CONTRACT_DIR)) unlink(CONTRACT_DIR);
  }
}
