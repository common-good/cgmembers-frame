<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class PiecemealDiscounts extends AbstractMigration {
  public function up() {
    createViews($this);
    $this->execute("ALTER TABLE `r_coupons` ADD `sponsor` BIGINT(11) NOT NULL DEFAULT '0' COMMENT 'account that pays the rebate';");
    $this->execute("ALTER TABLE `r_coupons` CHANGE `start` `start` INT(11) NOT NULL DEFAULT '0' COMMENT 'Unixtime coupon is first valid OR first gift certificate number';");
    $this->execute("ALTER TABLE `r_coupons` CHANGE `end` `end` INT(11) NOT NULL DEFAULT '0' COMMENT 'Unixtime after which coupon is no longer valid OR last gift certificate number plus one';"); 

    $this->execute('UPDATE r_coupons SET sponsor=fromId WHERE sponsor=0');    
  }
  
  public function down() {
    $this->execute("ALTER TABLE `r_coupons` DROP `sponsor`");
  }
  
  public function doSql($sql) {
//    cgpr("$sql\n");
    $this->execute($sql);
  }  
}
