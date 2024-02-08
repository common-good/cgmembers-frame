<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class BadNegAmt extends AbstractMigration {
  public function up() {
    $this->doSql("ALTER TABLE `tx_bads` CHANGE `amount` `amount` DECIMAL(11,2) NOT NULL DEFAULT '0.00' COMMENT 'amount to pay or charge';");
  }
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
