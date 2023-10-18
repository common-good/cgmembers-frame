<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class BigBadField extends AbstractMigration {
  public function change() {
    $t = $this->table('tx_bads');
    $t->changeColumn('deviceId', 'text', ray('length null comment', phx::TEXT_MEDIUM, TRUE, 'ID of the device submitting the transaction'));
    $t->update();
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
