<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class OFAC extends AbstractMigration {
  public function change() {
    $t = $this->table('ofac', ray('id comment', FALSE, 'list of known foreign criminals'));
    $t->addColumn('nm', 'string', ray('length null comment', 255, TRUE, 'the person\'s name'));
    $t->addColumn('co', 'integer', ray('length null default comment', phx::INT_TINY, FALSE, '0', 'is this a company?'));
    $t->addIndex(['nm']);
    $t->addIndex(['co']);
    $t->create();
    if ($this->isMigratingUp()) {}
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
