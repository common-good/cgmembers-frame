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
      ->addColumn('grpId', 'integer', ['null' => false])
      ->addColumn('isMember', 'boolean', ['default' => false, 'comment' => 'User is a member of the group'])
      ->addColumn('canAdd', 'boolean', ['default' => false, 'comment' => 'User can add other users to the group'])
      ->addColumn('canRemove', 'boolean', ['default' => false, 'comment' => 'User can remove other users from the group'])
      ->addColumn('start', 'biginteger', ['null' => false])
      ->addColumn('end', 'biginteger', ['default' => null, 'null' => true])
      ->addIndex(['uid', 'start'])
      ->addIndex(['grpId', 'start'])
      /* ->addForeignKey('uid', 'users', 'uid', ['delete' => 'RESTRICT']) */
      /* ->addForeignKey('grpId', 'u_groups', 'id', ['delete' => 'RESTRICT']) */
      ->create();
  }
}
