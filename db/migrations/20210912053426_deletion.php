<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Deletion extends AbstractMigration {
  public function change() {
    $this->table('r_photos')->rename('u_photo');
    $this->table('x_photos')->rename('x_photo');
    $this->table('u_shouters')->rename('u_shout');
    $this->table('r_relations')->rename('u_relations');
    $this->table('x_invoices')->rename('legacy_x_invoices');
    $this->table('x_txs')->rename('legacy_x_txs');
    
    foreach (ray('shout company') as $k) {
      if ($this->isMigratingUp()) {
        $this->execute("CREATE TABLE x_$k LIKE u_$k"); // make backups for u_company and u_shout
        $this->execute("ALTER TABLE x_$k ADD COLUMN `deleted` bigint(20) DEFAULT NULL COMMENT 'Unixtime record was deleted' FIRST");
      } else $this->execute("DROP TABLE x_$k");
    }
  }
}
