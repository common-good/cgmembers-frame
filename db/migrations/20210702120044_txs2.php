<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
require_once __DIR__ . '/recreate-views.inc';

class Txs2 extends AbstractMigration {
  public function change() {
    foreach (ray('r_usd x_usd') as $tnm) {
      $t = $this->table($tnm);
      $t->rename($tnm == 'r_usd' ? 'txs2' : 'x_txs2');
      $t->addColumn('pid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'related people record ID'));
      $t->addIndex(['pid']);
      $t->update();
    }

    $t = $this->table('tx_hdrs_all');
    $t->changeColumn('recursId', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'related record ID in tx_timed, for recurring or delayed transaction'));
    $t->addColumn('cat', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'related budget_cats record ID'));
    $t->addIndex(['cat']);
    $t->addIndex(['recursId']);
    $t->update();

    $t = $this->table('tx_requests_all');
    $t->changeColumn('recursId', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'related record ID in tx_timed, for recurring or delayed charge (or reversed payment)'));
    $t->update();
    
    $t = $this->table('tx_templates');
    $t->rename('tx_timed');
    $t->update();
    
    createViews($this);
  }
  
  public function doSql($sql) {
    cgpr("$sql\n");
    $this->execute($sql);
  }
}
