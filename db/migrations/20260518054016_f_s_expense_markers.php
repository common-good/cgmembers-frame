<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class FSExpenseMarkers extends AbstractMigration {
  public function up(): void {
    $catNicks = ['E: Sponsored Project Expenses' => 'FIRST-FS-EXPENSE', 'E: Sponsored Project Expenses: Travel' => 'LAST-FS-EXPENSE'];
    foreach ($catNicks as $cat => $nick) $this->doSql("UPDATE tx_cats SET nick='$nick' WHERE category='$cat'");
  }

  public function down(): void {
    throw new \RuntimeException('This migration is not reversible.');
  }
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
