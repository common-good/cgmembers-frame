<?php


use Phinx\Migration\AbstractMigration;

class PostSplitFixes extends AbstractMigration
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
    public function up()
    {
      $this->execute('CREATE VIEW tx_entries_payer AS SELECT * FROM tx_entries_all WHERE entryType=1 AND deleted IS NULL');
      $this->execute('CREATE VIEW tx_entries_payee AS SELECT * FROM tx_entries_all WHERE entryType=2 AND deleted IS NULL');
    }

    public function down()
    {
      $this->execute('DROP VIEW IF EXISTS tx_entries_payer');
      $this->execute('DROP VIEW IF EXISTS tx_entries_payee');
    }
}
