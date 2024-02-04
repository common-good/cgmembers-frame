<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Cap extends AbstractMigration {
  public function change() {
    $t = $this->table('cu_tasks');
    $t->addColumn('cap', 'integer', ray('length null comment after', phx::INT_BIG, TRUE, 'maximum time to complete this task', 'estimate'));
    $t->update();
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
