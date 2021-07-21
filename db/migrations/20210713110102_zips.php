<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Zips extends AbstractMigration {
  public function change() {
    if ($this->isMigratingUp() and !$this->hasTable('zips')) {
      $sql = file_get_contents(__DIR__ . '/sql/zips.sql'); // downloaded free from the USPS and converted free online from CSV to SQL
      $sqls = explode(";\n", str_replace("\r", '', $sql));
      foreach ($sqls as $i => $sql) if (trim($sql)) $this->execute($sql);
    }
  }
}
