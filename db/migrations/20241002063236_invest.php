<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Invest extends AbstractMigration {
  public function change() {
    $this->doSql("UPDATE tx_cats SET nick='INVEST' WHERE externalId=312;");
  }
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
