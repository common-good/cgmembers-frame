<?php
use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class SimpleSignup extends AbstractMigration {

  public function change() {
    foreach (['users', 'x_users'] as $table) $this->table($table)
      ->addColumn('steps', 'integer', ['length' => MysqlAdapter::INT_BIG, 'signed' => false, 'null' => false, 'default' => '0', 'comment' => 'boolean account setup steps completed', 'after' => 'jid'])
      ->update();

    $this->table('r_invites')
      ->addColumn('zip', 'string', ['length' => 10, 'default' => 'NULL', 'comment' => 'alleged postal code of recipient', 'after' => 'message'])
      ->update();
      
    $table = $this->table('zip3', ['id' => FALSE, 'primary_key' => 'id', 'comment' => 'meaning of first 3 digits of Zip Codes']);
    
    $table->addColumn('id', 'string', ['length' => 3, 'null' => TRUE, 'comment' => 'first 3 digits of Zip Code']);
    $table->addColumn('region', 'string', ['length' => 255, 'null' => TRUE, 'comment' => 'city or region within a state']);
    $table->addColumn('state', 'string', ['length' => 2, 'null' => TRUE, 'comment' => 'state abbreviation']);
    $table->create();
    $sql = file_get_contents(__DIR__ . '/zip3.sql');
    $this->execute($sql);
  }
}
