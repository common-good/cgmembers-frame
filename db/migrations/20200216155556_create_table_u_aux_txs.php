<?php


use Phinx\Migration\AbstractMigration;

class CreateTableUAuxTxs extends AbstractMigration
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
    $table = $this->table('u_auxtxs', ['comment' => 'Occurrences of a rule']);
    $table
      ->addColumn('rule', 'integer', ['comment' => 'Rule of which this is an occurrence'])
      ->addColumn('start', 'date', ['comment' => 'Start of period for which this occurrence applies'])
      ->addColumn('end', 'date', ['null' => true,
                                  'comment' => 'End of period for which this occurrence applies, NULL if it does not end'])
      ->addColumn('code', 'integer', ['null' => true, 'default' => null,
                                      'comment' => 'For gift cards the individual code'])
      ->addForeignKey('rule', 'u_rules', 'id', ['delete' => 'restrict'])
      ->create();
  }
}
