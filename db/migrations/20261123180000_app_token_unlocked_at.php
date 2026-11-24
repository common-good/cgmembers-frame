<?php
use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

/**
 * A manager's unlock of a cash register slides forward on use, but only up to a few minutes after the unlock: keep when it began.
 */
class AppTokenUnlockedAt extends AbstractMigration {
  public function change() {
    $this->table('app_tokens')->addColumn('unlockedAt', 'integer', ray('length null after comment', phx::INT_BIG, TRUE, 'unlockedUntil', 'date/time of the manager\'s unlock that unlockedUntil belongs to (an unlock never lasts past APP_UNLOCK_MAX_SECS after it)'))->update();
  }
}
