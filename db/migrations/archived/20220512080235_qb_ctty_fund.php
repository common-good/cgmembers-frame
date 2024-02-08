<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class QbCttyFund extends AbstractMigration {
  public function change() {
    $this->table('txs2')
    ->addColumn('qbok', 'integer', ray('null comment after', TRUE, 'date/time this batch was sent to CG\'s QBO', 'bankTxId'))
    ->update();
  }
}
