<?php


use Phinx\Migration\AbstractMigration;

class Invest extends AbstractMigration
{
  /**
   * Change Method.
   *
   * Write your reversible migrations using this method.
   *
   * More information on writing migrations is available here:
   * http://docs.phinx.org/en/latest/migrations.html#the-abstractmigration-class
   *
   * The following commands can be used in this method and Phinx will
   * automatically reverse them when rolling back:
   *
   *    createTable
   *    renameTable
   *    addColumn
   *    renameColumn
   *    addIndex
   *    addForeignKey
   *
   * Remember to call "create()" or "update()" and NOT "save()" when working
   * with the Table class.
   */
  public function up() {
    $this->table('r_investments')
      ->changeColumn('return', 'decimal', ['precision' => 7, 'scale'=>6, 'null' => true, 'comment' => 'predicted or actual APR']) 
      ->save();
    $this->execute("ALTER TABLE `r_ratings` CHANGE comments `comment` MEDIUMTEXT NULL COMMENT 'description of investment'");
    $this->execute('ALTER TABLE `r_shares` DROP `bought`;');
    $this->execute('ALTER TABLE `r_shares` DROP `sell`;');
    $this->execute('DROP TABLE IF EXISTS r_gifts, r_log, r_nonmembers, r_request');
  }
  
  public function down() {
    $this->table('r_investments')
      ->changeColumn('return', 'decimal', ['precision' => 10, 'scale'=>3, 'null' => true, 'comment' => 'predicted or actual APR']) 
      ->save();
    $this->execute("ALTER TABLE `r_ratings` CHANGE `comment` comments MEDIUMTEXT NULL COMMENT 'description of investment'");

    $this->execute("ALTER TABLE `r_shares` ADD `bought` INT(11) NULL DEFAULT NULL COMMENT 'Unixtime investment made';");
    $this->execute("ALTER TABLE `r_shares` ADD `sell` INT(11) NULL DEFAULT NULL COMMENT 'number of shares to sell ASAP';");

    
    $this->execute(<<< o
      CREATE TABLE `r_gifts` (
        `donid` int(11) NOT NULL AUTO_INCREMENT COMMENT 'gift record id',
        `uid` bigint(20) DEFAULT NULL COMMENT 'uid of account that made the gift',
        `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount of gift',
        `often` varchar(1) DEFAULT NULL COMMENT 'recurring how often (Y, Q, M, 1)',
        `honor` varchar(10) DEFAULT NULL COMMENT 'what type of honor',
        `honored` mediumtext COMMENT 'who is honored',
        `share` decimal(6,3) DEFAULT NULL COMMENT 'percentage of rebates/bonuses to donate to CGF',
        `giftDate` int(11) NOT NULL DEFAULT '0' COMMENT 'date/time of gift',
        `completed` int(11) NOT NULL DEFAULT '0' COMMENT 'Unixtime donation was completed',
        PRIMARY KEY (`donid`),
        KEY `uid` (`uid`)
      ) ENGINE=InnoDB AUTO_INCREMENT=5088 DEFAULT CHARSET=utf8 COMMENT='Membership gift details';
o
    );
    
    $this->execute(<<< o
      CREATE TABLE `r_log` (
        `logid` int(11) NOT NULL AUTO_INCREMENT COMMENT 'log record id',
        `time` int(11) NOT NULL DEFAULT '0' COMMENT 'date/time logged',
        `channel` tinyint(4) DEFAULT NULL COMMENT 'logged from what interface module',
        `type` varchar(60) DEFAULT NULL COMMENT 'what type of log entry',
        `myid` bigint(20) DEFAULT NULL COMMENT 'current account uid',
        `agent` bigint(20) DEFAULT NULL COMMENT 'agent account uid',
        `info` mediumtext COMMENT 'arbitrary serialized data',
        PRIMARY KEY (`logid`),
        KEY `type` (`type`),
        KEY `channel` (`channel`),
        KEY `myid` (`myid`),
        KEY `agent` (`agent`)
      ) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Development and error log';
o
    );

    $this->execute(<<< o
      CREATE TABLE `r_nonmembers` (
        `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'non-member company record id',
        `company` varchar(60) DEFAULT NULL COMMENT 'company name',
        `potential` int(11) DEFAULT '0' COMMENT 'number of members who shop there',
        PRIMARY KEY (`id`)
      ) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8 COMMENT='Local companies we want to recruit';
o
    );   

    $this->execute(<<< o
      CREATE TABLE `r_request` (
        `listid` bigint(20) NOT NULL COMMENT 'record id',
        `created` int(11) DEFAULT NULL COMMENT 'Unixtime record created',
        `first` varchar(60) DEFAULT NULL COMMENT 'first name of the individual',
        `last` varchar(60) DEFAULT NULL COMMENT 'last name of the individual',
        `phone` varchar(255) DEFAULT NULL COMMENT 'contact phone (no punctuation)',
        `email` varchar(255) DEFAULT NULL COMMENT 'email of invitee',
        `zip` varchar(60) DEFAULT NULL COMMENT 'postal code (no punctuation)',
        `ctty` bigint(20) DEFAULT NULL COMMENT 'uid of this requester''s Common Good Community',
        `done` tinyint(4) NOT NULL DEFAULT '0' COMMENT 'are we done with this request',
        PRIMARY KEY (`listid`),
        KEY `ctty` (`ctty`)
      ) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Requests to be invited';
o
    ); 
  }  
}
