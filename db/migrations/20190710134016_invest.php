<?php


use Phinx\Migration\AbstractMigration;

class Invest extends AbstractMigration
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
  public function up() {
    $this->table('r_investments')
      ->changeColumn('return', 'decimal', ['precision' => 7, 'scale'=>6, 'null' => true, 'comment' => 'predicted or actual APR']) 
      ->save();
    $this->execute("ALTER TABLE `r_ratings` CHANGE comments `comments` MEDIUMTEXT NULL COMMENT 'description of investment'");
  }
  
  public function down() {
    $this->table('r_investments')
      ->changeColumn('return', 'decimal', ['precision' => 10, 'scale'=>3, 'null' => true, 'comment' => 'predicted or actual APR']) 
      ->save();
    $this->execute("ALTER TABLE `r_ratings` CHANGE `comment` comments MEDIUMTEXT NULL COMMENT 'description of investment'");
  }  
}
