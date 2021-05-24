<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PostTips extends AbstractMigration {

  public function up() {
    $this->table('posts')->changeColumn('type', 'enum', ray('values null comment', ray('need offer tip'), TRUE, 'item type'))->update();
    
    $this->execute('UPDATE posts SET cat=21 WHERE cat=3'); // change clothing to stuff
    $this->execute('UPDATE posts SET cat=17 WHERE cat=12'); // change social service advice to info
    $this->execute('UPDATE posts SET cat=20 WHERE cat=15'); // change communication to other
    $this->execute('UPDATE posts SET cat=20 WHERE cat=8'); // change adult care to other
    $this->execute('DELETE FROM post_cats WHERE id IN (3, 8, 12, 15)');
    $this->execute("UPDATE post_cats SET cat='health' WHERE cat='healthcare'");
    $this->execute("UPDATE post_cats SET cat='information/skills' WHERE cat='teaching/learning'");
    $this->execute("UPDATE post_cats SET cat='muscle/labor' WHERE cat='muscle'");
    $this->execute("UPDATE post_cats SET cat='finance/money' WHERE cat='finance'");
    $this->execute("UPDATE post_cats SET cat='travel/rides', sort=1930 WHERE cat='rides'");
    $this->execute("UPDATE post_cats SET sort=1910 WHERE cat='cleaning'");
  }

}
