<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class ShortSignup extends AbstractMigration {
  public function change() {
    $this->table('signup')->rename('legacy_signup');
  }
}
