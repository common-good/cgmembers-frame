<?php
use Phinx\Migration\AbstractMigration;

require_once __DIR__ . '/recreate-views.inc';

class PeopleAccountsViews extends AbstractMigration {
  public function up() {createViews($this, 20261113);}
  public function down() {createViews($this, 20261113);}
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
