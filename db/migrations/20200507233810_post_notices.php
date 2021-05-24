<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
const NOTICE_SET = 'offer:d,need:d,tip:w,in:X,out:X,misc:X';

class PostNotices extends AbstractMigration {
  public function change() {
    $fM = '(1<<14)';
    $fW = '(1<<13)';
    $t = $this->table('people');
    $t->addColumn('notices', 'text', ray('length null comment after', phx::TEXT_TINY, TRUE, 'notice preferences', 'created'));
    $t->save();
    $this->execute("UPDATE people SET notices='" . NOTICE_SET . "'");
    $this->execute("UPDATE users SET notices=REPLACE('" . NOTICE_SET . "', 'X', IF(flags&$fM, 'm', IF(flags&$fW, 'w', 'd'))) WHERE uid>512");
  }
}
