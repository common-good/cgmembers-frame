<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Please extends AbstractMigration {
  public function change() {
    $t = $this->table('u_shouters', ray('id primary_key comment', FALSE, 'uid', 'people who have signed a public statement of support'));
    $t->addColumn('uid', 'biginteger', ray('null comment', FALSE, 'record ID in users table'));
    $t->addColumn('org', 'string', ray('length null comment', 255, TRUE, 'signer\'s organization, if any'));
    $t->addColumn('title', 'string', ray('length null comment', 255, TRUE, 'signer\'s title'));
    $t->addColumn('website', 'string', ray('length null comment', 255, TRUE, 'organization website, if any'));
    $t->addColumn('quote', 'text', ray('length null comment', phx::TEXT_LONG, TRUE, 'what benefit this signer sees for the community'));
    $t->addColumn('created', 'integer', ray('length null comment', 11, TRUE, 'creation date'));
    $t->addIndex(['uid'], ['unique' => TRUE]);
    $t->create();
  }
}
