<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class MoveCat extends AbstractMigration {
  public function change() {
    $t = $this->table('tx_hdrs_all');
    if ($this->isMigratingUp()) {
      $t->removeColumn('cat');
    } else {
      $t->addColumn('cat', 'integer', ray('length null comment after', phx::INT_BIG, TRUE, 'related budget_cats record ID', 'goods'));
      $t->addIndex(['cat']);
    }
    $t->update();

    $t = $this->table('tx_entries_all');
    $t->addColumn('cat', 'integer', ray('length null comment after', phx::INT_BIG, TRUE, 'related budget_cats record ID', 'description'));
    if ($this->isMigratingUp()) {
      $t->removeColumn('acctTid');
    } else {
      $t->addColumn('acctTid', 'string', ray('null comment', TRUE, 'transaction ID for the account to which this entry applies'));
    }
    $t->update();
    
    createViews($this, 20210706);
  }
  
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
