<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PostExpand extends AbstractMigration {
  public function change() {
    $t = $this->table('posts');
    $t->addColumn('service', 'integer', ray('length null comment after', phx::INT_TINY, TRUE, '0=goods 1=service', 'cat'));
    $t->save();

    $this->execute("UPDATE `post_cats` SET `cat` = 'info/training' WHERE `post_cats`.`id` = 17");
    $this->execute("DELETE FROM `post_cats` WHERE `post_cats`.`id` = 21");
    $this->execute("UPDATE posts SET service=IF(cat=21, 0, 1)");
    $this->execute("UPDATE posts SET exchange=2 WHERE exchange=1");
  }
}
