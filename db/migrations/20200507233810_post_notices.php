<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PostNotices extends AbstractMigration {

  public function change() {
    $t = $this-table('people');
    $t->addColumn('notices', 'string', ray('length null comment', phx::TEXT_MEDIUM, TRUE, 'post notice preferences'));
    $t->save();
  }
}
