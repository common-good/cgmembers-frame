<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Cttys extends AbstractMigration {
  public function up() { // no down necessary

    $this->execute("ALTER TABLE `r_company` CHANGE `serviceArea` `zips` TEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NULL DEFAULT NULL COMMENT 'zip regex for geographic region the company serves'");
    $this->execute("ALTER TABLE `r_regions` CHANGE `zip` `zips` TEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NULL COMMENT 'zip regex for this geographic region'");
    $this->execute("UPDATE r_regions SET zips=NULL WHERE zips=''");

    $regexs = [
      -45240000000002 => '^53',
      -26742000000002 => '^013([012346789]|5[012346789])',
      -25026000000002 => '^492|^48',
      -17238000000002 => '^46|^47',
    ];
    
    $zips = [
      -45240000000002 => '53703',
      -26742000000002 => '01330',
      -25026000000002 => '48103',
      -17238000000002 => '46526',
    ];
    
    foreach ($regexs as $uid => $regex) {
      $zip = $zips[$uid];
      $this->execute("UPDATE r_company SET zips='$regex' WHERE uid=$uid");
      $this->execute("UPDATE users SET zip='$zip' WHERE uid=$uid");
    }
    
  }
  
}
