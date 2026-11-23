<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

// Corrects 20260722193200_urelations_tx_notify (omitted x_relations) and 20260901180000_grants_add_drive_file_id (underscored field name).
class CorrectionsToRecentMigrations extends AbstractMigration {
  public function change() {
    $t = $this->table('x_relations');
    $t->addColumn('txNotifyEmail', 'boolean', ray('default null comment', 0, FALSE, 'this relation wants email notifications for the main account\'s money-movement transactions'));
    $t->addColumn('txNotifyText',  'boolean', ray('default null comment', 0, FALSE, 'this relation wants SMS notifications for the main account\'s money-movement transactions'));
    $t->update();

    $this->table('grants')->renameColumn('drive_file_id', 'driveFileId')->update();
  }
}
