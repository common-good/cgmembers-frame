<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PgpDeviceId extends AbstractMigration {
  public function change() {
    $t = $this->table('r_boxes');
    $t->addColumn('version', 'integer', ray('length null comment after', phx::INT_MEDIUM, TRUE, 'latest software version on the device', 'nonce'));
    $t->save();
  }
}
