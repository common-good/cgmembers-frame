<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class SimplerSignup extends AbstractMigration {
  public function up() {
    $this->execute("ALTER TABLE `u_company` CHANGE `founded` `founded` BIGINT NULL DEFAULT NULL COMMENT 'date the company was founded';");
  }
}
