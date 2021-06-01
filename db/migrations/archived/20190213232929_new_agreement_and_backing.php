<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class NewAgreementAndBacking extends AbstractMigration
{
  /**
   * Up Method.
   *
   * More information on writing migrations is available here:
   * http://docs.phinx.org/en/latest/migrations.html#the-abstractmigration-class
   *
   * Remember to call "create()" or "update()" and NOT "save()" when working
   * with the Table class.
   */
  public function up()
  {
    foreach (['users', 'x_users'] as $tblName) {
      $this->table($tblName)
        ->removeColumn('changesX')
        ->removeColumn('share')
        ->removeColumn('rebate')
        ->addColumn('backing', 'decimal', ['precision' => 11, 'scale'=>2, 'null' => false, 'default' => '0', 'comment' => 'amount account-holder chose to back', 'after' => 'crumbs']) 
        ->addColumn('backingDate', 'integer', ['length' => MysqlAdapter::INT_BIG, 'null' => false, 'default' => '0', 'comment' => 'date account-holder started backing', 'after' => 'backing'])
        ->addColumn('food', 'decimal', ['precision' => 6, 'scale' => 3, 'null' => false, 'default' => '0', 'comment' => 'percentage of each food purchase to donate to the food fund', 'after' => 'backingDate'])
        ->update();
    }

    foreach (['r_txs', 'x_txs'] as $tblName) {
      $this->table($tblName)
        ->removeColumn('payerReward')
        ->removeColumn('payeeReward')
        ->update();
    }
  }

    public function down()
  {
    foreach (['users', 'x_users'] as $tblName) {
      $this->table($tblName)
        ->addColumn('changesX', 'blob', ['limit' => MysqlAdapter::BLOB_LONG, 'comment' => 'OLD changes made to the account', 'after' => 'special'])
        ->addColumn('share', 'decimal', ['precision' => 6, 'scale' => 3, 'null' => false, 'default' => '0.000', 'comment' => 'percentage of rebates/bonuses to donate to CG', 'after' => 'minimum'])
        ->addColumn('rebate', 'decimal', ['precision' => 5, 'scale' => 3, 'null' => false,  'default' => '10.000', 'comment' => 'current rebate percentage (sales bonus is proportionate)', 'after' => 'signedBy'])
        ->removeColumn('backing')
        ->removeColumn('backingDate')
        ->removeColumn('food')
        ->update();
    }

    foreach (['r_txs', 'x_txs'] as $tblName) {
      $this->table($tblName)
        ->addColumn('payerReward', 'decimal', ['precision' => 11, 'scale' => 2, 'null' => false, 'default' => '0.00', 'comment' => 'incentive reward for payer', 'after' => 'payeeFor'])
        ->addColumn('payeeReward', 'decimal', ['precision' => 11, 'scale' => 2, 'null' => false, 'default' => '0.00', 'comment' => 'incentive reward for payee', 'after' => 'payerReward'])
        ->update();
    }
  }
}
