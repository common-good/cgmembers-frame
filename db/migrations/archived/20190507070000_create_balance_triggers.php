<?php


use Phinx\Migration\AbstractMigration;

class CreateBalanceTriggers extends AbstractMigration
{
  public function up() {
    $this->execute('DROP TRIGGER IF EXISTS updEntry');
    $this->execute('DROP TRIGGER IF EXISTS delEntry');
    $this->execute('DROP TRIGGER IF EXISTS insEntry');
    $this->execute('CREATE TRIGGER insEntry AFTER INSERT ON tx_entries_all FOR EACH ROW '
                   . 'UPDATE users u SET balance=balance+NEW.amount '
                   . 'WHERE NEW.uid IN (u.uid, u.jid)');
    $this->execute('CREATE TRIGGER delEntry AFTER DELETE ON tx_entries_all FOR EACH ROW '
                   . 'UPDATE users u SET balance=balance-OLD.amount '
                   . 'WHERE OLD.uid IN (u.uid, u.jid)');
    $this->execute('CREATE TRIGGER updEntry AFTER UPDATE ON tx_entries_all FOR EACH ROW BEGIN '
                   . 'UPDATE users u SET balance=balance-(IF(OLD.deleted IS NULL,0,OLD.amount)) '
                   . 'WHERE OLD.uid IN (u.uid, u.jid); '
                   . 'UPDATE users u SET balance=balance+(IF(NEW.deleted IS NULL,0,NEW.amount)) '
                   . 'WHERE NEW.uid IN (u.uid, u.jid); '
                   . 'END');
  }
  
  public function down() {
    $this->execute('DROP TRIGGER IF EXISTS updEntry');
    $this->execute('DROP TRIGGER IF EXISTS delEntry');
    $this->execute('DROP TRIGGER IF EXISTS insEntry');
  }
}
