<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Notices extends AbstractMigration {
  public function change() {
    $t = $this->table('r_notices');
    $t->addColumn('type', 'string', ray('length null comment after', 255, TRUE, 'type of notice', 'created'));
    $t->save();
    
  }
}
