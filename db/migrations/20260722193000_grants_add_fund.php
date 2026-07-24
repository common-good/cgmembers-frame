<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class GrantsAddFund extends AbstractMigration {
  public function change() {
    $t = $this->table('grants');
    $t->addColumn('fund', 'string', ray('length null comment after', 255, TRUE, 'specific Fund within grant-making organization (if any)', 'pid'));
    $t->update();
  }
}
