<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class FixDemoViews extends AbstractMigration {
  public function change() {
    if ($this->isMigratingUp()) {
      createViews($this, 20240116);
    }
  }
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
