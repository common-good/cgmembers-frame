<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PostGeosearch extends AbstractMigration {

  public function change() {
    $t = $this->table('post_cats');
    $t->insert(ray('cat sort', 'stuff', 1950));
    $t->save();

    $t = $this->table('people');
    $t->addColumn('street', 'string', ray('length null comment after', 255, TRUE, 'street address without street number or apt number', 'fullName'));
    $t->save();
  }
}
