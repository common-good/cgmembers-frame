<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PostBugs extends AbstractMigration {

  public function up() {
    $t = $this->table('people');
    $t->changeColumn('created', 'integer', ray('length null comment', 11, TRUE, 'creation date'));
    $t->save();

    $t = $this->table('posts');
    $t->changeColumn('created', 'integer', ray('length null comment', 11, TRUE, 'start date'));
    $t->changeColumn('end', 'integer', ray('length null comment', 11, TRUE, 'end date'));
    $t->addColumn('confirmed', 'integer', ray('length default comment after', phx::INT_TINY, 0, 'confirmed by email', 'contacts'));
    $t->save();
    
    $t = $this->table('messages');
    $t->changeColumn('created', 'integer', ray('length null comment', 11, TRUE, 'creation date'));
    $t->addColumn('confirmed', 'integer', ray('length default comment after', phx::INT_TINY, 0, 'confirmed by email', 'sender'));
    $t->save();  
    
    $this->execute('UPDATE posts s LEFT JOIN people p USING(pid) SET s.confirmed=1 WHERE p.confirmed=1');
    $this->execute('UPDATE messages s LEFT JOIN people p ON p.pid=s.sender SET s.confirmed=1 WHERE p.confirmed=1');
  }
  
  public function down() {
    $this->table('posts')->removeColumn('confirmed');
    $this->table('messages')->removeColumn('confirmed');
  }

}
