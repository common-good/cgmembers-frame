<?php


use Phinx\Migration\AbstractMigration;

class CreateBalanceTriggers extends AbstractMigration
{
  public function up() {
    $this->execute('CREATE TRIGGER insEntry AFTER INSERT ON all_entries FOR EACH ROW '
                   . 'UPDATE users u SET balance=balance+NEW.amount '
                   . 'WHERE NEW.deleted IS NULL AND NEW.uid IN (u.uid, u.jid)');
    $this->execute('CREATE TRIGGER delEntry AFTER DELETE ON all_entries FOR EACH ROW '
                   . 'UPDATE users u SET balance=balance-OLD.amount '
                   . 'WHERE OLD.deleted IS NULL AND OLD.uid IN (u.uid, u.jid)');
    $this->execute('CREATE TRIGGER updEntry AFTER UPDATE ON all_entries FOR EACH ROW '
                   . 'UPDATE users u SET balance=balance-(IF(OLD.deleted IS NULL,0,OLD.amount)) '
                   . '                                  +(IF(NEW.deleted IS NULL,0,NEW.amount)) '
                   . 'WHERE OLD.deleted IS NULL AND OLD.uid IN (u.uid, u.jid)');
  }
  
  public function down() {
    $this->execute('DROP TRIGGER IF EXISTS updEntry');
    $this->execute('DROP TRIGGER IF EXISTS delEntry');
    $this->execute('DROP TRIGGER IF EXISTS insEntry');
  }
}
