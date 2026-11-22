<?php
use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

/**
 * A store app token can be locked to one company (the app as a cash register): it then works for that company only, until a manager unlocks it for a while.
 */
class AppTokenLock extends AbstractMigration {
  public function change() {
    $t = $this->table('app_tokens');
    $t->addColumn('lockedTo', 'integer', ray('length null after comment', phx::INT_BIG, TRUE, 'revoked', 'record ID of the company this token is locked to (the app as a cash register), or empty'));
    $t->addColumn('unlockedUntil', 'integer', ray('length null after comment', phx::INT_BIG, TRUE, 'lockedTo', 'date/time a manager\'s unlock of a locked token ends (slides forward on use)'));
    $t->addColumn('unlockedBy', 'integer', ray('length null after comment', phx::INT_BIG, TRUE, 'unlockedUntil', 'record ID of the manager who unlocked it (another manager than the token\'s member reaches only the company)'));
    $t->update();
  }
}
