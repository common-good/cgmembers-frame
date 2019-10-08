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
  }
}
