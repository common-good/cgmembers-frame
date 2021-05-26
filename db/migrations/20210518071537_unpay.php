<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Unpay extends AbstractMigration {
  public function change() {
    foreach (ray('r_invoices x_invoices') as $table) {
      $t = $this->table($table);
      $t->addColumn('reversesXid', 'integer', ray('length null comment after', phx::INT_BIG, TRUE, 'xid of the transaction this invoice reverses (if any)', 'data'));
      $t->changeColumn('recursId', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'related record in tx_rules, for recurring charge (or reversed payment)'));
      $t->update();
    }
  }
}
