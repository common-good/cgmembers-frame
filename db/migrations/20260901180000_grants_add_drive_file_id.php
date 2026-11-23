<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class GrantsAddDriveFileId extends AbstractMigration {
  public function change() {
    $t = $this->table('grants');
    $t->addColumn('drive_file_id', 'string', ray('length null comment after', 255, TRUE, 'Google Drive file id of the uploaded grant agreement PDF (Phase 3.5 PR B)', 'fund'));
    $t->update();
  }
}
