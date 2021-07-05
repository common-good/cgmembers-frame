<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
require_once __DIR__ . '/recreate-views.inc';

class Txs2b extends AbstractMigration {
  public function change() {createViews($this, 20210705);} // just to fix staging server
  
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
