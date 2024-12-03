<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Focuses extends AbstractMigration {
  public function change() {
    $t = $this->table('u_focuses', ray('comment', 'how staff members spend their time'));
    $t->addColumn('uid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'staff member\'s account ID'));
    $t->addColumn('project', 'string', ray('length null comment', 255, TRUE, 'what they focused on'));
    $t->addColumn('percent', 'decimal', ray('precision scale null default comment', 6, 3, FALSE, 100, 'what percentage of their time'));
    $t->addIndex(['uid']);
    $t->create();
    if ($this->isMigratingUp()) {}
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
