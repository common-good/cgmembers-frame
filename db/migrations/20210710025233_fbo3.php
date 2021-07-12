<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
require_once __DIR__ . '/recreate-views.inc';

const US_COUNTRY_ID = 1228; // United States (used in settings file and elsewhere)
const REF_LIST = 'anybody account anyCo industry group person'; // values for payerType and payeeType in tx_rules

class Fbo3 extends AbstractMigration {
  public function change() {
    foreach (ray('txs2 x_txs2') as $tnm) {
      $t = $this->table($tnm);
      $t->addColumn('isSavings', 'integer', ray('length null comment after', phx::INT_TINY, TRUE, '1 if bankAccount is a savings account', 'bankAccount'));
      $t->update();
    }

    if ($this->isMigratingUp()) {
      foreach (ray('tx_timed tx_rules') as $tnm) {
        $t = $this->table($tnm);
        $t->changeColumn('payerType', 'enum', ['values' => ray(REF_LIST), 'comment' => 'Type of payer']);
        $t->changeColumn('payeeType', 'enum', ['values' => ray(REF_LIST), 'comment' => 'Type of payee']);
        $t->update();
      }
    }

    $t = $this->table('people');
    $t->addColumn('country', 'integer', ray('length null default comment after', phx::INT_MEDIUM, FALSE, US_COUNTRY_ID, 'country index', 'zip'));
    $t->addColumn('notes', 'text', ray('length null comment after', phx::TEXT_LONG, TRUE, 'miscellaneous notes about the person', 'longitude'));
    $t->addColumn('source', 'text', ray('length null comment', phx::TEXT_MEDIUM, TRUE, 'how did this person hear about us?'));
    $t->update();

    createViews($this, 20210711);
  }
  
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
