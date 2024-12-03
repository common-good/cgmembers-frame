<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
require_once __DIR__ . '/recreate-views.inc';

class TxsDeleted extends AbstractMigration {
  public function up() {
    createViews($this, 20240811);
  }

  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
