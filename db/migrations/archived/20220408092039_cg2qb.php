<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
require_once __DIR__ . '/recreate-views.inc';

class Cg2qb extends AbstractMigration {
  public function change() {
    createViews($this, 20220408);
    $this->table('budget_cats')->rename('tx_cats')->update();
    if ($this->isMigratingUp()) {
      $this->doSql("ALTER TABLE `tx_entries_all` CHANGE `cat` `cat` BIGINT(20) NULL DEFAULT NULL COMMENT 'related tx_cats record ID';");
      $this->doSql("ALTER TABLE `tx_requests_all` CHANGE `cat` `cat` BIGINT(20) NULL DEFAULT NULL COMMENT 'related tx_cats record ID';");
    }
  }
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);} // uses $this, so can't be in util.inc
}
