<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PostTouchups2 extends AbstractMigration {

  public function change() {
    $t = $this->table('posts');
    $t->removeColumn('confirmed');
    $t->save();
    
    $t = $this->table('messages');
    $t->removeColumn('confirmed');
    $t->save();    
  }

}