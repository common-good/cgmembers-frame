<?php

use Phinx\Migration\AbstractMigration;

class FixDisputes extends AbstractMigration {

  public function change() {
    /*
    $cols = $this->table('tx_disputes_all')->getColumns();
    print_r($cols); die('here');

    $this->table('tx_disputes_all')
      ->changeColumn('return', 'decimal', ['precision' => 10, 'scale'=>3, 'null' => true, 'comment' => 'predicted or actual APR'])
    $this->addBigInt($disputesTable, 'xid', ['length' => 20, 'null' => false, 'comment' => 'id of the transaction in dispute']);
    $this->addBigInt($disputesTable, 'uid', ['length' => 20, 'null' => false, 'comment' => 'id of the user who disputes the transaction']);
    $this->addBigInt($disputesTable, 'agentUid', ['length' => 20, 'null' => false, 'comment' => 'id of the user who actually acted for the nominal user']);
    $disputesTable->addColumn('reason', 'string', ['length' => 255, 'null' => false, 'comment' => 'reason the transaction is being disputes']);      
    */
  }
}
