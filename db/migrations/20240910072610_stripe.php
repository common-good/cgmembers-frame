<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Stripe extends AbstractMigration {
  public function change() {
    $t = $this->table('people');
    $t->addColumn('stripeCid', 'string', ray('length null comment', 255, TRUE, 'customerId from Stripe'));
    $t->update();
    
    $t = $this->table('tx_timed');
    $t->addColumn('stripeId', 'string', ray('length null comment', 255, TRUE, 'setupIntentId from Stripe'));
    $t->update();
    
    if ($this->isMigratingUp()) {}
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
