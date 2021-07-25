<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Founded extends AbstractMigration {
  public function change() {
    $t = $this->table('u_company');
    $t->addColumn('founded', 'integer', ray('length null comment after', 11, TRUE, 'date the company was founded', 'coFlags'));
    $t->update();
  }
}
