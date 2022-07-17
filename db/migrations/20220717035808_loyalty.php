<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Loyalty extends AbstractMigration {
  public function up() {
    createViews($this, 20220717);
    $this->execute("ALTER TABLE `tx_requests_all` CHANGE `amount` `amount` DECIMAL(11,2) NULL DEFAULT NULL COMMENT 'amount to charge (negative for store credit)'");
  }
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
