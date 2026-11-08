<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class ResetCodes extends AbstractMigration {
  public function change() {
    $t = $this->table('reset_codes', ray('id comment', TRUE, 'one-time password reset codes, owned by the node app (cgpay)'));

    $t->addColumn('uid', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'record ID of the account being reset'));
    $t->addColumn('codeHash', 'string', ray('length null comment', 255, FALSE, 'hash of the code (never the code itself)'));
    $t->addColumn('expires', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'date/time the code stops working'));
    $t->addColumn('tries', 'integer', ray('length null default comment', phx::INT_TINY, FALSE, '0', 'wrong guesses so far'));
    $t->addColumn('channel', 'enum', ray('values comment', ray('email sms'), 'how the code was delivered'));
    $t->addColumn('created', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'date/time the code was requested'));
    $t->addColumn('usedAt', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'date/time the code was used (empty until then)'));

    $t->addIndex(['uid', 'created']);

    $t->create();
  }

}
