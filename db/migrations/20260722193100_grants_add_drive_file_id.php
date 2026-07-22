<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class GrantsAddDriveFileId extends AbstractMigration {
  public function change() {
    $t = $this->table('grants');
    $t->addColumn('driveFileId', 'string', ray('length null comment after', 64, TRUE, 'Google Drive file ID for uploaded grant agreement, if any', 'fund'));
    $t->update();
  }
}
