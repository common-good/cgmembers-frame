<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class BigQbId extends AbstractMigration {
  public function change() {
    $t = $this->table('tx_cats');
    $t->changeColumn('externalId', 'integer', ray('length null comment after', phx::INT_BIG, TRUE, 'account record ID in external accounting program', 'description'));
    $t->update();
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
