<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Sql extends AbstractMigration {
  public function change() {
    $t = $this->table('cu_tasks');
    $t->addColumn('class', 'string', ray('length null comment after', 255, TRUE, 'category', 'priority'));
    $t->update();
  }
}
