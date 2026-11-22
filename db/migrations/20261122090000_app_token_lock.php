<?php
use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

/**
 * A store app token can be locked to one company (the app as a cash register): it then works for that company only, until a manager unlocks it for a while.
 * A live-site session opened from a token remembers it, so a cash register's session ends when the register locks again.
 */
class AppTokenLock extends AbstractMigration {
  public function change() {
    $t = $this->table('app_tokens');
    $t->addColumn('lockedTo', 'integer', ray('length null after comment', phx::INT_BIG, TRUE, 'revoked', 'record ID of the company this token is locked to (the app as a cash register), or empty'));
    $t->addColumn('unlockedUntil', 'integer', ray('length null after comment', phx::INT_BIG, TRUE, 'lockedTo', 'date/time a manager\'s unlock of a locked token ends (slides forward on use)'));
    $t->addColumn('unlockedBy', 'integer', ray('length null after comment', phx::INT_BIG, TRUE, 'unlockedUntil', 'record ID of the manager who unlocked it (another manager than the token\'s member reaches only the company)'));
    $t->update();

    $this->table('app_codes')->addColumn('tokenId', 'integer', ray('length null after comment', phx::INT_BIG, TRUE, 'uid', 'record ID of the app token the code came from (app_tokens.id)'))->update();
    $this->table('sessions')->addColumn('appToken', 'integer', ray('length null after comment', phx::INT_BIG, TRUE, 'acct', 'record ID of the app token this session was opened from (app_tokens.id): the session lasts only while the token works, and a cash register\'s only while it is unlocked'))->update();
  }
}
