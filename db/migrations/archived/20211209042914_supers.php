<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Supers extends AbstractMigration {
  public function change() {
    $t = $this->table('admins', ray('comment', 'permissions for each admin'));
    $t->addColumn('uid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'related account record ID'));
    $t->addColumn('vKeyE', 'blob', ray('length null comment', phx::BLOB_LONG, TRUE, 'very secret private key (vKey) encrypted with a password specific to this account'));
    $t->addColumn('can', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'bit array of permissions for this admin'));
    $t->create();
  }
}
