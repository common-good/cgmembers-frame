<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class CcGifts extends AbstractMigration {
  public function change() {
    $t = $this->table('tx_timed');
    $t->addColumn('flags', 'integer', ray('length null default comment after', phx::INT_BIG, false, 0, 'transaction flag bits', 'amtMax'));
    $t->update();
    if ($this->isMigratingUp()) {
      $this->execute('UPDATE tx_timed tm SET flags=(1<<1024) WHERE tm.to IN (26742000000002,26742000001195,26742000000672,26742000000262,26742000000265,26742000000508,26742000000331)'); // set gift flag for recurring payments to these companies
    }
  }
}
