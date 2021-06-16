<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Invoices extends AbstractMigration {
  public function change() {
    $t = $this->table('r_invoices');
    $t->rename('tx_requests_all');
    $t->addColumn('deleted', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'record is deleted'));
    $t->update();

    $this->execute('DROP VIEW IF EXISTS tx_requests');
    $this->execute('CREATE VIEW tx_requests AS SELECT * FROM tx_requests_all WHERE deleted IS NULL');
  }
}
