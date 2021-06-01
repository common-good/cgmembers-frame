<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class RecursCentral extends AbstractMigration {
 
  public function up() {
    $t = $this->table('tx_templates');
    $periods = ray('D:day, W:week, M:month, Q:quarter, Y:year');
    
    $tid0 = $this->query('SELECT MAX(id) FROM tx_templates')->fetchColumn();
    $rows = $this->query('SELECT id,payer,payee,amount,purpose,period,created,ended FROM r_recurs ORDER BY id')->fetchAll();
    $count = 0;
    
    foreach ($rows as $row) {
      extract($row);
      $count += 1;
      $tid = $tid0 + $count;
      $info = ray('id action from to amount purpose period start end duration', $tid, 'pay', $payer, $payee, $amount, $purpose, $periods[$period], $created, $ended ?: NULL, 'forever');
      $t->insert($info)->save();
      
      foreach (ray('tx_hdrs_all r_invoices') as $tnm) {
        $this->execute("UPDATE $tnm SET recursId=$tid WHERE recursId=$id");
      }
    }

    echo "Inserted $count templates beginning at template id #$tid0\n";
    $this->execute('RENAME TABLE r_recurs TO legacy_r_recurs');
  }
  
  public function down() {
    $this->execute('RENAME TABLE legacy_r_recurs TO r_recurs');

    $t = $this->table('tx_templates');
    $created1 = $this->query('SELECT created FROM r_recurs ORDER BY id LIMIT 1')->fetchColumn();
    $tid0 = $this->query('SELECT id FROM tx_templates WHERE start=' . $created1)->fetchColumn() - 1;
    $rows = $this->query('SELECT id FROM r_recurs')->fetchAll();
    $count = 0;
    
    foreach ($rows as $row) {
      extract($row);
      $count += 1;
      $tid = $tid0 + $count;
      $this->execute('DELETE FROM tx_templates WHERE id=' . $tid);
      
      foreach (ray('tx_hdrs_all r_invoices') as $tnm) {
        $this->execute("UPDATE $tnm SET recursId=$id WHERE recursId=$tid");
      }
    }

    echo "Deleted $count templates beginning at template id #$tid0\n";
  }
}
