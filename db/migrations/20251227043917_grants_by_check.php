<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class GrantsByCheck extends AbstractMigration {
  public function change() {
    $t = $this->table('grants');
    $t->addColumn('ckNum', 'string', ray('length null comment after', 255, TRUE, 'grant check number, if any', 'by'));
    $t->addColumn('ckDate', 'integer', ray('length null comment after', phx::INT_BIG, TRUE, 'grant check date, if any', 'ckNum'));
    $t->update();
  }
}
