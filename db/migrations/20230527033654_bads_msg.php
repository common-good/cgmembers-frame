<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class BadsMsg extends AbstractMigration {
  public function change() {
    $t = $this->table('tx_bads');
    $t->addColumn('problem', 'text', ray('length null after comment', phx::TEXT_MEDIUM, TRUE, 'offline', 'problem description'));
    $t->update();
  }
    
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
