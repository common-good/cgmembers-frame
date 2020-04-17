<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;
define('CONTRACT_DIR', sys_get_temp_dir() . '/contracts');
define('OLD_CONTRACT_DIR', __DIR__ . '/../../contracts');

class EasyTweaks extends AbstractMigration {
  public function up() {

    if ($this->hasTable('r_contracts')) $this->execute('DROP TABLE r_contracts');
    pr('contract directory: ' . CONTRACT_DIR);

    if (is_dir(OLD_CONTRACT_DIR)) unlink(OLD_CONTRACT_DIR);
    if (!is_dir(CONTRACT_DIR)) mkdir(CONTRACT_DIR, 0755) or die('cannot create contract directory');
  }
  
  public function down() {
    if (is_dir(CONTRACT_DIR)) unlink(CONTRACT_DIR);
    if (!is_dir(OLD_CONTRACT_DIR)) mkdir(OLD_CONTRACT_DIR, 0755) or die('cannot create old contract directory');
  }
}
