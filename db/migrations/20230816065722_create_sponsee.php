<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class CreateSponsee extends AbstractMigration {
  public function up() {
    $this->doSql("UPDATE tx_cats SET nick='SPONSORED' WHERE id=12100");
  }
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
