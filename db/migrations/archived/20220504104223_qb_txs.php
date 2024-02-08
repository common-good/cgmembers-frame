<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
const QB_ACCT_NICKS = 'D-COMPANY:500, D-CRUMB:600, D-ONCE:900, D-REGULAR:1000, D-ROUNDUP:1100, D-FBO:1200, D-FBO-STEPUP:1250, FS-FEE:1300, D-STEPUP:1400, TX-FEE-BACK:5000, TO-ORG:5100, ACCOUNTING:7800, COMPUTER:8000, CONTRACTOR:8200, TX-FEE:4900, INFO-SVC:5200, TO-PERSON:8900, FBO-LABOR:9100, FBO-TX-FEE:9150, PROCESSOR:10100, NEWAAB:10200, OLD-BANK:11800, OPERATIONS:11900, !NEWAAA:12200, FBO-PROCESSOR:12275, AAAAJV:12300, NEWAZV:12500, NEWBTY:12600, NYAAUN:12700, POOL:13500, LABOR:14900';

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
