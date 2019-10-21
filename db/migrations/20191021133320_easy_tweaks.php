<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class EasyTweaks extends AbstractMigration {
  public function change() {
    $this->table('r_contracts', ['comment' => 'signing dates of contracts with signup partners'])
      ->addColumn('partner', 'integer', ['length' => MysqlAdapter::INT_BIG, 'null' => FALSE, 'default' => 0, 'comment' => 'account ID of partner organization'])
      ->addColumn('customer', 'string', ['length' => 255, 'null' => true, 'comment' => 'partner identifier for the customer'])
      ->addColumn('created', 'integer', ['length' => MysqlAdapter::INT_REGULAR, 'null' => FALSE, 'default' => 0, 'comment' => 'date/time the customer signed a contract with the partner'])
      ->addIndex(['customer'])
      ->create(); 
  }
}
