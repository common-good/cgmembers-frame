<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PhotoPermission extends AbstractMigration {

  public function change() {
    $t = $this->table('u_shouters');
    $t->addColumn('usePhoto', 'enum', ray('values null comment', [0, 1], FALSE, 'okay to use this member\'s photo in a publicity collage?'));
    $t->save();
  }
}
