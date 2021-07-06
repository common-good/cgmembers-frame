<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class MakeOuter extends AbstractMigration {
  public function up() {
    $this->execute(<<< X
    INSERT IGNORE INTO `users` (`uid`, `name`, `pass`, `email`, `created`, `access`, `login`, `picture`, `data`, `flags`, `jid`, `steps`, `changes`, `community`, `secure`, `vsecure`, `fullName`, `phone`, `city`, `state`, `zip`, `country`, `latitude`, `longitude`, `notes`, `tickle`, `activated`, `helper`, `iCode`, `signed`, `signedBy`, `savingsAdd`, `saveWeekly`, `floor`, `minimum`, `crumbs`, `backing`, `backingDate`, `backingNext`, `food`, `balance`, `rewards`, `committed`, `risk`, `risks`, `trust`, `stats`, `notices`, `lastip`, `preid`, `special`, `source`) 
    VALUES (255, NULL, NULL, NULL, 1625588666, 1625588666, 1625588666, 0, 'a:1:{s:9:\"legalName\";s:9:\"Txs Outer\";}', 71, 0, 0, NULL, -26742000000001, NULL, NULL, 'Txs Outer', NULL, NULL, 0, NULL, 1228, '0.00000000', '0.00000000', NULL, 0, 0, 0, 0, 0, NULL, '0.00', '0.00', '0.00', NULL, '0.000', '0.00', 0, NULL, '0.000', '0.00', '0.00', '0.00', NULL, 0, 0, NULL, 'offer:d,need:d,tip:w,in:w,out:d,misc:d', NULL, 0, NULL, NULL);
X
    );
  }
  public function down() {
    $this->execute('DELETE FROM users WHERE uid=255');
  }
}
