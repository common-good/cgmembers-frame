<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Sweep extends AbstractMigration {
  public function change() {
    $this->execute("ALTER TABLE `tx_templates` CHANGE `amount` `amount` DECIMAL(11,2) UNSIGNED NULL DEFAULT '0.00' COMMENT 'Fixed amount to transfer (NULL for sweep)';");
  }
}
