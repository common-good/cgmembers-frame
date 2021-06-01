<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class RenewBacking extends AbstractMigration {
  public function change() {
    foreach(ray('users x_users') as $table) {
      $t = $this->table($table);
      $t->addColumn('backingNext', 'decimal', ray('precision scale null comment after', 11, 2, TRUE, 'lower backing amount for the next year', 'backingDate'));
      $t->save();
    }
    
    $t = $this->table('u_track', ray('id primary_key comment', FALSE, 'id', 'contact information for non-members'));
    $t->addColumn('id', 'integer', ray('identity length null comment', TRUE, phx::INT_BIG, FALSE, 'record ID'));
    $t->addColumn('uid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'related account record ID'));
    $t->addColumn('type', 'string', ray('length null comment', 255, TRUE, 'what type of email or email address (for invite)'));
    $t->addColumn('sent', 'string', ray('length null comment', 11, TRUE, 'latest date sent'));
    $t->addColumn('seen', 'string', ray('length null comment', 11, TRUE, 'latest date opened'));
    $t->addIndex(['uid']);
    $t->addIndex(['type']);
    $t->create();    
  }
}
