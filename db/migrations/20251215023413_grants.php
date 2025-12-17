<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Grants extends AbstractMigration {
  public function change() {
    $t = $this->table('grants', ray('id comment', TRUE, 'record of grants expected by fiscally sponsored partners'));

    $t->addColumn('uid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'record ID of the sponsee account'));
    $t->addColumn('pid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'people record ID for the grantor'));
    $t->addColumn('xid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'transaction ID crediting the sponsee account'));
    $t->addColumn('amount', 'decimal', ray('precision scale null default comment', 11, 2, FALSE, '0', 'amount of expected grant'));
    $t->addColumn('by', 'enum', ray('values comment', ray('check ach wire'), 'how the funds are expected to be sent'));
    $t->addColumn('documented', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'date the grant agreement was sent to Common Good'));
    $t->addColumn('received', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'date the funds were received'));
    $t->addColumn('created', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'date the sponsee notified us'));

    foreach (ray('uid pid xid') as $k) $t->addIndex([$k]);

    $t->create();
  }
  
}
