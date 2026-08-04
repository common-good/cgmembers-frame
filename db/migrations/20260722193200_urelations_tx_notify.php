<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class UrelationsTxNotify extends AbstractMigration {
  public function change() {
    $t = $this->table('u_relations');
    $t->addColumn('txNotifyEmail', 'boolean', ray('default null comment', 0, FALSE, 'this relation wants email notifications for the main account\'s money-movement transactions'));
    $t->addColumn('txNotifyText',  'boolean', ray('default null comment', 0, FALSE, 'this relation wants SMS notifications for the main account\'s money-movement transactions'));
    $t->update();
  }
}
