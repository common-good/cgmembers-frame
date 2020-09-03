<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
const NOTICE_DFTS = 'offer:d,need:d,tip:w,request:w,auto:d,othertx:i,other:d';

class PostNotices extends AbstractMigration {

  public function change() {
    $t = $this->table('people');
    $t->addColumn('notices', 'text', ray('length null comment after', phx::TEXT_TINY, TRUE, 'notice preferences', 'created'));
    $t->save();
    $this->execute("UPDATE people SET notices='" . NOTICE_DFTS . "'");
    $this->execute("UPDATE users SET notices='" . NOTICE_DFTS . "'");
  }
}
