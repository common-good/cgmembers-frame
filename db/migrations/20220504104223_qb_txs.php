<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
const QB_ACCT_NICKS = 'FS-FEE:1300, TX-FEE-BACK:5000, OLD-BANK:11800, OPERATIONS:11900, D-SPONSORED:1200, D-STEPUP:1400, D-CRUMB:600, D-REGULAR:1000, D-ROUNDUP:1100, D-COMPANY:500, D-ONCE:900, TO-PERSON:8900, TO-ORG:5100, LABOR:14900, CONTRACTOR:8200, COMPUTER:8000, ACCOUNTING:7800, INFO-SVC:5200, FBO-TX-FEE:9150, PROCESSOR:10100, NEWAAB:10200, !NEWAAA:12200, FBO-PROCESSOR:12275, AAAAJV:12300, NEWAZV:12500, NEWBTY:12600, NYAAUN:12700, POOL:13500, LABOR:14900';

class QbTxs extends AbstractMigration {
  public function change() {
    $this->table('tx_cats')
    ->addColumn('nick', 'string', ray('length null comment', 255, TRUE, 'nickname for the account, used to set transaction category'))
    ->update();
    
    if ($this->isMigratingUp()) {
      foreach (ray(QB_ACCT_NICKS) as $nick => $id) $this->execute("UPDATE tx_cats SET nick='$nick' WHERE id=$id");
    }
  }
}
