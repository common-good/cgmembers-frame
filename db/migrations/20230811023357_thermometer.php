<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Thermometer extends AbstractMigration {
  public function change() {
    $t = $this->table('u_company');
    $t->addColumn('target', 'decimal', ray('precision scale null default comment after', 11, 2, FALSE, '0', 'fundraising target amount', 'logo'));
    $t->addColumn('targetStart', 'integer', ray('length null comment after', 11, TRUE, 'starting date of fundraising project', 'target'));
    $t->update();
  }
}

