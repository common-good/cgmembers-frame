<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class CatTest extends AbstractMigration {
  public function change() {
    if ($this->isMigratingUp()) {
      foreach (ray('txs2 x_txs2') as $tnm) {
        $this->execute("ALTER TABLE $tnm CHANGE `bankTxId` `bankTxId` BIGINT(20) NULL DEFAULT NULL COMMENT 'bank transaction ID';");
        $this->execute("ALTER TABLE $tnm DROP `qbok`;");
      }
    }
  }
}
