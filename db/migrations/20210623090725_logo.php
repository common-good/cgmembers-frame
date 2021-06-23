<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Logo extends AbstractMigration {
  public function change() {
    $t = $this->table('u_company');
    $t->addColumn('logo', 'text', ray('length null comment', phx::TEXT_REGULAR, TRUE, 'company logo URL'));
    $t->update();
  }
}
