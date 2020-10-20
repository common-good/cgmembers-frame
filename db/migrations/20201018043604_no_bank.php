<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class NoBank extends AbstractMigration {
  public function change() { // keep "Bank" off the menu
    $this->execute('TRUNCATE menu_links;'); // force clean rebuild
    $this->execute("DELETE FROM menu_router WHERE access_callback NOT IN (1, 'user_access');");
  }
}
