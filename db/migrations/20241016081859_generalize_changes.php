<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class GeneralizeChanges extends AbstractMigration {
  public function change() {
    $t = $this->table('r_changes');
    $t->rename('changes');
    $t->renameColumn('uid', 'rid'); // what was once just for account ID is now for any table's record ID
    $t->changeColumn('rid', 'integer', ray('comment', 'record ID'));
    $t->addColumn('table', 'string', ray('length null comment after', 255, TRUE, 'table name', 'id'));
    $t->update();
    if ($this->isMigratingUp()) {}
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
