<?php


use Phinx\Migration\AbstractMigration;

class InitializeBalances extends AbstractMigration
{
  public function up() {
    $this->execute('UPDATE users u SET balance='
                   . 'IFNULL((SELECT SUM(amount) FROM tx_entries e WHERE e.uid IN (u.uid, u.jid)), 0)');
  }
    
  public function down() {
  }      
}
