<?php
use Phinx\Migration\AbstractMigration;

define ('U_SQL', "CHANGE `community` `community` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'account ID of this account\'s Common Good Community', CHANGE `state` `state` INT(5) NOT NULL DEFAULT '0' COMMENT 'state/province index', CHANGE `country` `country` INT(4) NOT NULL DEFAULT '0' COMMENT 'country index', CHANGE `helper` `helper` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'account that invited this person or company', CHANGE `iCode` `iCode` INT(11) NOT NULL DEFAULT '0' COMMENT 'sequence number of helper invitation', CHANGE `crumbs` `crumbs` DECIMAL(6,3) NOT NULL DEFAULT '0' COMMENT 'percentage of each transaction to donate to CG', CHANGE `trust` `trust` FLOAT NOT NULL DEFAULT '0' COMMENT 'how much this person is trusted by others in the community (0 for companies)';");

class CouponFixes extends AbstractMigration {
  
  public function up() {
    foreach (explode(' ', 'community state country helper iCode crumbs trust') as $k) $this->execute("UPDATE users SET $k=0 WHERE $k IS NULL");
    foreach (['users', 'x_users'] as $table) $this->execute("ALTER TABLE $table " . U_SQL);      
  }

  public function down() {
    $usersSql = str_replace("NOT NULL DEFAULT '0'", 'NULL DEFAULT NULL', U_SQL);
    foreach (['users', 'x_users'] as $table) $this->execute("ALTER TABLE $table " . $usersSql);
  }
}
