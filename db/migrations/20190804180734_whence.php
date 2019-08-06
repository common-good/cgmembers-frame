<?php
use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

require_once __DIR__ . '/util.inc';

class Whence extends AbstractMigration {
  
  public function up() {
    foreach (['users', 'x_users'] as $table) {
      $this->execute("ALTER TABLE $table ADD `source` TEXT NULL DEFAULT NULL COMMENT 'how did the member hear about us?' AFTER `special`;");
    }

    $rows = $this->fetchAll("SELECT uid,data FROM users WHERE data IS NOT NULL");
    foreach ($rows as $row) {
      extract($row);
      if (!$data = unserialize($data) or !is_array($data)) die('Bad serialization of data array in account ' . $uid);
      $source = addslashes(@$data['source']);
      unset($data['source']);
      unset($data['lastTx']); // this one was discontinued a couple months ago
      $data = addslashes(serialize($data));
      $this->execute("UPDATE users SET source='$source', data='$data' WHERE uid=$uid");
    }
    
    $this->table('test', ['comment' => 'transient data while testing offline'])
      ->addColumn('test', 'string', ['length' => 255, 'null' => true, 'comment' => 'name of the current test'])
      ->addColumn('type', 'string', ['length' => 255, 'null' => false, 'comment' => 'type of data stored here'])
      ->addColumn('value', 'text', ['limit' => MysqlAdapter::TEXT_LONG, 'null' => true, 'comment' => 'the data'])
      ->addIndex(['test'])
      ->addIndex(['type'])
      ->create();    
  }
  
  public function down() {
    $rows = $this->fetchAll("SELECT uid,source,data FROM users WHERE data IS NOT NULL");
    foreach ($rows as $row) {
      extract($row);
      if (!$data = unserialize($data) or !is_array($data)) die('Bad serialization of data array in account ' . $uid);
      $data['source'] = $source;
      $data = addslashes(serialize($data));
      $this->execute("UPDATE users SET data='$data' WHERE uid=$uid");
    }
      
    foreach (['users', 'x_users'] as $table) $this->execute("ALTER TABLE $table DROP `source`");
    
    $this->execute('DROP TABLE test');
  }
}
