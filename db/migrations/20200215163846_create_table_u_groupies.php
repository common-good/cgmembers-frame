<?php


use Phinx\Migration\AbstractMigration;

class CreateTableUGroupies extends AbstractMigration
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
  public function change()
  {
    $table = $this->table('u_groupies');
    $table->addColumn('uid', 'biginteger', ['null' => false])
      ->addColumn('grp_id', 'integer', ['null' => false])
      ->addColumn('is_member', 'boolean', ['default' => false, 'comment' => 'User is a member of the group'])
      ->addColumn('can_add', 'boolean', ['default' => false, 'comment' => 'User can add other users to the group'])
      ->addColumn('can_remove', 'boolean', ['default' => false, 'comment' => 'User can remove other users from the group'])
      ->addColumn('start', 'timestamp', ['default' => 'CURRENT_TIMESTAMP', 'null' => false])
      ->addColumn('end', 'timestamp', ['default' => null, 'null' => true])
      ->addIndex(['uid', 'start'])
      ->addIndex(['grp_id', 'start'])
      ->addForeignKey('uid', 'users', 'uid', ['delete' => 'RESTRICT'])
      ->addForeignKey('grp_id', 'u_groups', 'id', ['delete' => 'RESTRICT'])
      ->create();
  }
}
