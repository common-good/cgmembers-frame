<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PostModerate extends AbstractMigration {

  public function change() {
    $t = $this->table('people');
    $t->addColumn('health', 'string', ray('length default comment', phx::TEXT_TINY, 0, 'summary of COVID19 survey answers'));
    $t->save();
  }

}
