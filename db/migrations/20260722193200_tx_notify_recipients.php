<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class TxNotifyRecipients extends AbstractMigration {
  public function change() {
    $t = $this->table('tx_notify_recipients', ray('id comment', TRUE, 'extra staff/contacts who receive money-movement notices for an account (up to TX_NOTIFY_MAX_RECIPIENTS per uid)'));

    $t->addColumn('uid', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'account whose transactions trigger notifications'));
    $t->addColumn('name', 'string', ray('length null comment', 255, FALSE, 'recipient display name'));
    $t->addColumn('email', 'string', ray('length null comment', 255, TRUE, 'recipient email address, if any'));
    $t->addColumn('phone', 'string', ray('length null comment', 32, TRUE, 'recipient phone in E.164 format, if any'));
    $t->addColumn('wantEmail', 'boolean', ray('default null comment', 1, FALSE, 'recipient wants email notifications'));
    $t->addColumn('wantText', 'boolean', ray('default null comment', 0, FALSE, 'recipient wants SMS notifications'));
    $t->addColumn('created', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'date the recipient was added'));

    $t->addIndex(['uid']);

    $t->create();
  }
}
