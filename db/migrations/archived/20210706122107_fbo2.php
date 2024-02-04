<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
require_once __DIR__ . '/recreate-views.inc';

class Fbo2 extends AbstractMigration {
  public function change() {
    createViews($this, 20210705);
    if (!$this->isMigratingUp()) $this->doSql('CREATE VIEW txs_donations AS SELECT * FROM txs_aux;');
  }

  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
