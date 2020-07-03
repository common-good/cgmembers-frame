<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PushEndorse extends AbstractMigration {
  public function change() {
    $t = $this->table('u_shouters');
    $t->addColumn('rating', 'integer', ray('null default comment', FALSE, 0, 'how awesome is the quote, 0=not'));
    $t->save();
  }
}
