<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class DescLen extends AbstractMigration {
  public function change() {
    if ($this->isMigratingUp()) {
      $this->execute("ALTER TABLE `tx_entries_all` CHANGE `description` `description` MEDIUMTEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'NULL' COMMENT 'description for this entry';");
    }
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
