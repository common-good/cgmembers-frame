<?php

/**
 * Create (or recreate) all TRIGGERS, starting from scratch. This makes it easier to keep track of how things work.
 * @param object $m: phinx AbstractMigration object
 * @param int $stopAt: migration date to stop at (0 = none, used for rolling back a migration)
 */
function createTriggers($m, $stopAt = 0) {
  clearTriggers($m);
  
  $sql = <<< o
    CREATE TRIGGER insEntry AFTER INSERT ON tx_entries_all FOR EACH ROW 
    UPDATE users u SET balance=balance+NEW.amount
    WHERE NEW.uid IN (u.uid, u.jid)
o;
  $m->query($sql);
  
  $sql = <<< o
    CREATE TRIGGER delEntry AFTER DELETE ON tx_entries_all FOR EACH ROW
    UPDATE users u SET balance=balance-OLD.amount
    WHERE OLD.uid IN (u.uid, u.jid)
o;
  $m->query($sql);

  $sql = <<< o
    CREATE TRIGGER updEntry AFTER UPDATE ON tx_entries_all FOR EACH ROW
    UPDATE users u SET balance=balance
      -IF(OLD.uid IN (u.uid, u.jid), IF(OLD.deleted,0,OLD.amount), 0)
      +IF(NEW.uid IN (u.uid, u.jid), IF(NEW.deleted,0,NEW.amount), 0)
    WHERE OLD.uid IN (u.uid, u.jid) OR NEW.uid IN (u.uid, u.jid)
o;
  $m->query($sql);
}

function clearTriggers($m) {
  $q = $m->query('SELECT trigger_name FROM information_schema.triggers');
  while ($trigger = $q->fetchColumn()) {
    $m->doSql('  DROP TRIGGER IF EXISTS ' . $trigger);
  }
}
