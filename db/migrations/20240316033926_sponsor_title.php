<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class SponsorTitle extends AbstractMigration {
  public function change() {
    $t = $this->table('u_company');
    $t->addColumn('contactTitle', 'text', ray('length null comment after', phx::TEXT_TINY, TRUE, 'contact person\'s position in the organization', 'contact')); // or BLOB_TINY/_REGULAR/_MEDIUM/_LONG
    $t->update();
  }
}
