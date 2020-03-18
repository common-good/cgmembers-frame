<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PostTweaks extends AbstractMigration {

  public function change() {
    $t = $this->table('posts');
    $t->addColumn('radius', 'decimal', ray('precision scale null comment after', 11, 6, TRUE, 'geographic limit of post visibility, in miles', 'emergency'));
    $t->removeColumn('uid');
    $t->Save();
    
    $t = $this->table('people');
    $t->addColumn('uid', 'integer', ray('length null comment after', phx::INT_BIG, TRUE, "poster's associated account ID, if any", 'pid'));
    $t->Save();
    
  }
  
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
