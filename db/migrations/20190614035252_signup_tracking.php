<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class SignupTracking extends AbstractMigration
{
    /**
     * Change Method.
     *
     * Write your reversible migrations using this method.
     *
     * More information on writing migrations is available here:
     * http://docs.phinx.org/en/latest/migrations.html#the-abstractmigration-class
     *
     * The following commands can be used in this method and Phinx will
     * automatically reverse them when rolling back:
     *
     *    createTable
     *    renameTable
     *    addColumn
     *    renameColumn
     *    addIndex
     *    addForeignKey
     *
     * Remember to call "create()" or "update()" and NOT "save()" when working
     * with the Table class.
     */
    public function change() {
      $table = $this->table('signup', ['id' => FALSE, 'primary_key' => 'preid', 'comment' => 'partial data about incomplete signups']);
      
      $table->addColumn('preid', 'integer', ['length' => MysqlAdapter::INT_BIG, 'null' => FALSE, 'default' => 0, 'comment' => 'presignup cookie value uniquely identifying the person (milliseconds since the epoch, plus 6 digits)']);
      $table->addColumn('source', 'string', ['length' => 255, 'null' => TRUE, 'comment' => 'ad code that led the person to us']);
      $table->addColumn('ip', 'string', ['length' => 39, 'null' => TRUE, 'comment' => 'IP address of potential new member']);
      $table->addColumn('fullName', 'string', ['length' => 255, 'null' => TRUE, 'comment' => 'name on signup page']);
      $table->addColumn('legalName', 'string', ['length' => 255, 'null' => TRUE, 'comment' => 'legalName on signup page']);
      $table->addColumn('email', 'string', ['length' => 255, 'null' => TRUE, 'comment' => 'email on signup page']);
      $table->addColumn('phone', 'string', ['length' => 255, 'null' => TRUE, 'comment' => 'phone number on signup page']);
      $table->addColumn('created', 'integer', ['length' => MysqlAdapter::INT_REGULAR, 'null' => FALSE, 'default' => 0, 'comment' => 'date/time the person arrived at the signup form']);
      $table->create();

      $table->addIndex(['preid'], ['unique' => TRUE]); 

      $this->table('users')
      ->addColumn('preid', 'integer', ['length' => MysqlAdapter::INT_BIG, 'null' => FALSE, 'default' => 0, 'after' => 'lastip', 'comment' => 'signup record ID'])
      ->update();
      
      // added retroactively (also added conditionally in later migration "SimplifyTxs"
      $this->table('x_users')
      ->addColumn('preid', 'integer', ['length' => MysqlAdapter::INT_BIG, 'null' => FALSE, 'default' => 0, 'after' => 'lastip', 'comment' => 'signup record ID'])
      ->update();      
    }
}
