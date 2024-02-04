<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Comments extends AbstractMigration {
  public function up() {
    $t = $this->table('tx_bads', ray('id primary_key comment', FALSE, 'id', 'transactions that cannot be completed, requested by our app'));
    $t->addColumn('id', 'integer', ray('length null identity comment', phx::INT_BIG, FALSE, TRUE, 'record ID'));
    $t->addColumn('version', 'integer', ray('length null comment', phx::INT_MEDIUM, FALSE, 'app version number'));
    $t->addColumn('deviceId', 'string', ray('length null comment', 255, TRUE, 'ID of the device submitting the transaction'));
    $t->addColumn('actorId', 'string', ray('length null comment', 255, TRUE, 'account ID of the transaction initiator'));
    $t->addColumn('otherId', 'string', ray('length null comment', 255, TRUE, 'other account ID'));
    $t->addColumn('amount', 'decimal', ray('precision scale default signed comment', 11, 2, 0, FALSE, 'amount to pay or charge'));
    $t->addColumn('description', 'text', ray('length null comment', phx::TEXT_MEDIUM, TRUE, 'description of transactions'));
    $t->addColumn('created', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'date/time of transaction'));
    $t->addColumn('proof', 'string', ray('length null comment', 255, TRUE, 'various parameters hashed together with cardCode'));
    $t->addColumn('offline', 'integer', ray('length null comment', phx::INT_TINY, FALSE, 'transaction taken online (0) or offline (1)'));
    $t->create();
  }
  
  public function down() { doSql('DROP TABLE tx_bads'); }
    
  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);}
}
