<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class GiftThanksX extends AbstractMigration {
  public function change() {
    $t = $this->table('x_company');
    $t->addColumn('giftThanks', 'text', ray('length null after comment', phx::TEXT_LONG, TRUE, 'staleNudge', 'extra email text for donation thanks'));
    $t->update();
    if ($this->isMigratingUp()) {}
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
