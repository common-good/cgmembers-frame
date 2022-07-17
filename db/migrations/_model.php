<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Model extends AbstractMigration {
  public function change() {
    $oldTable = $this->table('old');
    $oldTable->rename('legacy_r_txs');
    $hdrTable = $this->table('tx_hdrs_all', ray('id primary_key comment', FALSE, 'xid', 'Record of all rCredits transactions in the region'));
    $hdrTable->addColumn('xid', 'integer', ray('length identity null comment', phx::INT_BIG, TRUE, FALSE, 'the unique transaction ID'));
    $t = $this->table('budget_cats', ray('comment', 'income and expense categories'));
    $t->addColumn('checksOut', 'integer', ray('length null comment', phx::INT, TRUE, 'expected number of outgoing payments monthly'));
    $t->addColumn('category', 'string', ray('length null comment', 255, TRUE, 'category'));
    $t->addColumn('description', 'text', ray('length null comment', phx::TEXT_LONG, TRUE, 'description of category')); // or BLOB_TINY/_REGULAR/_MEDIUM/_LONG
    $t->addColumn('type', 'enum', ray('values comment', ray('Income Expense Asset Liability'), 'balance sheet account type'));
    $t->addColumn('backing', 'decimal', ray('precision scale null default comment after', 11, 2, FALSE, '0', 'amount account-holder chose to back', phx::FIRST));
    $t->addColumn('backingDate', 'integer', ray('length null default comment after', phx::INT_BIG, FALSE, '0', 'date account-holder started backing', 'backing'));
    $t->create();
    $t->update();
    if ($this->isMigratingUp()) {}
  }
//  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
