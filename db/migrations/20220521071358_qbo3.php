<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Qbo3 extends AbstractMigration {
  const MIGDT = 20220521;
  public function change() {
    $this->table('x_txs2')
    ->addColumn('qbok', 'integer', ray('null comment after', TRUE, 'date/time this batch was sent to CG\'s QBO', 'bankTxId'))
    ->update();
    if ($this->isMigratingUp()) createViews($this, self::MIGDT); else createViews($this, self::MIGDT - 1);
  }

  public function doSql($sql) {cgpr("$sql\n");$this->execute($sql);}
}
