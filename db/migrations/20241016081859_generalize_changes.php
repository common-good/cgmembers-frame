<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class GeneralizeChanges extends AbstractMigration {
  public function up() {
    $t = $this->table('r_changes');
    $t->removeIndex(['uid']); 
    $t->addColumn('table', 'string', ray('length null comment after', 255, TRUE, 'table name', 'id'));
    $t->update();

    $t->renameColumn('uid', 'rid'); // what was once just for account ID is now for any table's record ID
    $t->changeColumn('rid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'record ID of record that changed'));
    $t->addIndex(['rid']);
    $t->rename('changes');
    $t->update();

  }
  
  public function down() {
    $t = $this->table('changes');
    $t->removeColumn('table', 'string', ray('length null comment after', 255, TRUE, 'table name', 'id'));
    $t->removeIndex(['rid']);
    $t->update();

    $t->renameColumn('rid', 'uid'); // what was once just for account ID is now for any table's record ID
    $t->addIndex(['uid']);
    $t->rename('r_changes');
    $t->update();
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
