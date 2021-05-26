<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PostFormat extends AbstractMigration {

  public function up() {
    $t = $this->table('posts');
    if (!$t->hasColumn('private')) $t->addColumn('private', 'integer', ray('length default comment', phx::INT_TINY, 0, 'show this post only to the administrator'));
    $t->changeColumn('details', 'text', ray('length null comment', phx::TEXT_MEDIUM, TRUE, 'description of item'));
    $t->save();
  }

  public function down() {
// NO (no upside, possible downside) $this->table('posts')->changeColumn('details', 'string', ray('length null comment', 255, TRUE, 'description of item'))->save();
  }
}
