<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
const B_LIST = 'member confirmed ok carded ided refill co draws roundup nonudge nosearch depends bankOk u13 u14 contactable partner paper secret underage debt reinvest savings cashoutW cashoutM iclubq u26 u27 u28 u29 admin';

class Admins extends AbstractMigration {
  public function change() {
    if ($this->isMigratingUp()) {
      $allFlags = 0;
      foreach (ray(B_LIST) as $bit => $bitName) if (!preg_match('/^u\\d+$/', $bitName)) $allFlags |= (1 << $bit);
      $this->execute("UPDATE users SET flags=(flags&$allFlags)"); // turn off unused flags
    }
  }
}
