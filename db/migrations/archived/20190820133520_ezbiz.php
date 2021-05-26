<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class Ezbiz extends AbstractMigration {

  public function change() {
    $this->table('r_company')
      ->addColumn('staleNudge', 'integer', ['null' => false, 'default' => '7', 'comment' => 'how many days to wait before reminding customer to pay', 'after' => 'employees']) 
      ->addColumn('contact', 'string', ['length' => 255, 'null' => true, 'default' => NULL, 'comment' => 'whom to contact about this account', 'after' => 'coType']) 
      ->update();   
  }
}
