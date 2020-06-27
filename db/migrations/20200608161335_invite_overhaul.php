<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class InviteOverhaul extends AbstractMigration {

  public function change() {
    $t = $this->table('r_invites');
    $t->addColumn('nonudge', 'integer', ray('null default comment', TRUE, NULL, 'date/time this invitee "unsubscribed"'));
    $t->save();

    $t = $this->table('u_shouters');
    $t->addColumn('sawVideo', 'enum', ray('values null comment', [0, 1], FALSE, 'did the member see the video?'));
    $t->save();
  }
}
