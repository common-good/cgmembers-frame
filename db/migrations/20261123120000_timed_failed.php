<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class TimedFailed extends AbstractMigration {
  public function change() {
    $t = $this->table('tx_timed');
    $t->addColumn('failed', 'integer', ray('length null after comment', phx::INT_BIG, TRUE, 'stripeId', 'date/time a card charge failed (the recurrence is paused until the donor updates the card); NULL if not paused'));
    $t->update();
  }
}
