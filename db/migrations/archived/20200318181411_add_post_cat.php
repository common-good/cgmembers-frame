<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class AddPostCat extends AbstractMigration {

  public function up() {
    $this->execute("DELETE FROM post_cats where cat='other'");
    $t = $this->table('post_cats');
    $t->insert(['cat' =>'finance']);
    $t->insert(['cat' =>'other']);
    $t->addColumn('sort', 'integer', ray('length default comment', phx::INT_MEDIUM, 0, 'sorting order'));
    $t->save();
    $this->execute('UPDATE post_cats SET sort=id*100');
  }
}
