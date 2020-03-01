<?php


use Phinx\Migration\AbstractMigration;

/* const REF_ANYBODY = 1; */
/* const REF_USER = 2; */
/* const REF_INDUSTRY = 3; */
/* const REF_GROUP = 4; */
/* const REF_LIST = [REF_ANYBODY, REF_USER, REF_INDUSTRY, REF_GROUP]; */

/* const ACTION_PAYMENT = 1; */
/* const ACTION_BY_DATE = 2; */
/* const ACTION_LIST = [ACTION_PAYMENT, ACTION_BY_DATE]; */

/* const ONLY_ONCE = 1; */
/* const DAILY = 2; */
/* const WEEKLY = 3; */
/* const MONTHLY = 4; */
/* const QUARTERLY = 5; */
/* const YEARLY = 6; */
/* const FOREVER = 7; */
/* const PERIOD_CODES = [ONLY_ONCE, DAILY, WEEKLY, MONTHLY, QUARTERLY, YEARLY, FOREVER]; */

/* const SAME_AS_ACTOR = -1; */
/* const SAME_AS_OTHER = -2; */

class MigrateCoupons extends AbstractMigration
{
  /**
   * More information on writing migrations is available here:
   * http://docs.phinx.org/en/latest/migrations.html#the-abstractmigration-class
   *
   * Remember to call "create()" or "update()" and NOT "save()" when working
   * with the Table class.
   */
  /* table tx_entries_all:
     +-------------+---------------+------+-----+---------+----------------+
     | Field       | Type          | Null | Key | Default | Extra          |
     +-------------+---------------+------+-----+---------+----------------+
     | id          | int(11)       | NO   | PRI | NULL    | auto_increment |
     | xid         | bigint(20)    | NO   | MUL | 0       |                |
     | entryType   | tinyint(4)    | YES  |     | NULL    |                |
     | amount      | decimal(11,2) | NO   |     | 0.00    |                |
     | uid         | bigint(20)    | NO   | MUL | NULL    |                |
     | agentUid    | bigint(20)    | YES  |     | NULL    |                |
     | description | varchar(255)  | NO   |     | NULL    |                |
     | acctTid     | varchar(255)  | YES  |     | NULL    |                |
     | relType     | varchar(1)    | YES  |     | NULL    |                |
     | relatedId   | bigint(20)    | YES  |     | NULL    |                |
     | deleted     | bigint(20)    | YES  |     | NULL    |                |
     | auxtx       | int(11)       | YES  | MUL | NULL    |                |
     +-------------+---------------+------+-----+---------+----------------+

     table: r_coupons
     +---------+---------------------+------+-----+---------+----------------+
     | Field   | Type                | Null | Key | Default | Extra          |
     +---------+---------------------+------+-----+---------+----------------+
     | coupid  | bigint(20)          | NO   | PRI | NULL    | auto_increment |
     | fromId  | bigint(20)          | NO   | MUL | 0       |                |
     | start   | int(11)             | NO   |     | 0       |                |
     | end     | int(11)             | NO   |     | 0       |                |
     | amount  | decimal(11,2)       | YES  |     | NULL    |                |
     | on      | varchar(255)        | YES  |     | NULL    |                |
     | minimum | decimal(11,2)       | NO   |     | 0.00    |                |
     | ulimit  | int(11)             | NO   |     | 0       |                |
     | flags   | bigint(20) unsigned | NO   |     | 0       |                |
     | sponsor | bigint(11)          | NO   |     | 0       |                |
     +---------+---------------------+------+-----+---------+----------------+

     table: r_coupated
     +--------+------------+------+-----+---------+----------------+
     | Field  | Type       | Null | Key | Default | Extra          |
     +--------+------------+------+-----+---------+----------------+
     | id     | bigint(20) | NO   | PRI | NULL    | auto_increment |
     | uid    | bigint(20) | NO   | MUL | 0       |                |
     | coupid | bigint(20) | NO   | MUL | 0       |                |
     | uses   | int(11)    | NO   |     | 0       |                |
     | when   | int(11)    | NO   |     | 0       |                |
     +--------+------------+------+-----+---------+----------------+
  */

  /* Note that to keep the relationships straight we use the same keys for rules
     as for the original coupons.  This implies that this migration must run 
     prior to any other data migrations involving the tx_rules table. */
  public function up()
  {
    $coupons = $this->query('select * from r_coupons')->fetchAll();
    $this->execute("DELETE FROM tx_rules");
    $tx_rules = $this->table('tx_rules');
    $count = 0;
    foreach ($coupons as $coupon) {
      $coupon_amount = $coupon["amount"];
      $amount = $coupon_amount > 0 ? $coupon_amount : 0;
      $portion = $coupon_amount < 0 ? -$coupon_amount * 0.01 : 0;  // Convert from % to fraction
      $ruleId = $coupon["coupid"];
      $rule = [ "id" => $ruleId,
                "payer" => null, "payerType" => REF_ANYBODY,
                "payee" => $coupon["fromId"], "payeeType" => REF_USER,
                "fromId" => $coupon["sponsor"],
                "toId" => SAME_AS_PAYER,
                "action" => ACTION_PAYMENT,
                "start" => date('Ymd', $coupon["start"]),
                "end" => date('Ymd', $coupon["end"]),
                "amount" => $amount,
                "portion" => $portion,
                "purpose" => is_null($coupon["on"]) ? '' : $coupon["on"],
                "minimum" => $coupon["minimum"],
                "ulimit" => $coupon["ulimit"],
                "amtLimit" => null ];
      $tx_rules->insert($rule);
      $count += 1;
    }
    $tx_rules->saveData();

    echo "Inserted $count tx_rules\n";

    // relatedId was supposed to be the id of the coupated record, but is actually the id of the coupon record
    // so we can just set rule to relatedId...
    $count = $this->execute("UPDATE tx_entries_all SET rule=relatedId WHERE relType='D'");
    echo "Updated $count tx_entries\n";
    
  }

  public function down()
  {
    $this->execute("UPDATE tx_entries_all SET rule=null");
    $this->execute("DELETE FROM tx_rules");
    $this->execute("DELETE FROM tx_templates");
  }
}
