<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Nicks extends AbstractMigration {
  public function change() {
    if ($this->isMigratingUp()) {
      $this->doSql("UPDATE tx_cats SET nick='CG-GRANT-PERSON' WHERE id=5050");
      $this->doSql("UPDATE tx_cats SET nick='CG-GRANT-ORG' WHERE id=5100");
      $this->doSql("UPDATE tx_cats SET nick='CG-GRANT-FOREIGN' WHERE id=5150");
      $this->doSql("UPDATE tx_cats SET nick='GRANT-PERSON' WHERE id=8900");
      $this->doSql("UPDATE tx_cats SET nick='GRANT-ORG' WHERE id=8905");
      $this->doSql("UPDATE tx_cats SET nick='GRANT-FOREIGN' WHERE id=8910");
    }
  }
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
