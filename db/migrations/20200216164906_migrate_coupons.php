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
  
  public function up()
  {
    $coupons = $this->query('select * from r_coupons')->fetchAll();
    $u_rules = $this->table('u_rules');
    $u_auxtxs = $this->table('u_auxtxs');
    foreach ($coupons as $coupon) {
      $coupon_amount = $coupon["amount"];
      $amount = $coupon_amount > 0 ? $coupon_amount : 0;
      $portion = $coupon_amount < 0 ? -$coupon_amount * 0.01 : 0;  // Convert from % to fraction
      $ruleId = $coupon["coupid"];
      $rule = [ "id" => $ruleId,
                "actor" => null, "actorType" => REF_ANYBODY,
                "other" => $coupon["fromId"], "otherType" => REF_USER,
                "from" => $coupon["sponsor"],
                "to" => SAME_AS_ACTOR,
                "action" => ACTION_PAYMENT,
                "start" => date('Ymd', $coupon["start"]),
                "end" => date('Ymd', $coupon["end"]),
                "amount" => $amount,
                "portion" => $portion,
                "on" => is_null($coupon["on"]) ? '' : $coupon["on"],
                "minimum" => $coupon["minimum"],
                "ulimit" => $coupon["ulimit"],
                "amtLimit" => null,
                "period" => ONLY_ONCE,
                "duration" => 1,
                "durUnit" => FOREVER ];
      $u_rules->insert($rule)->saveData();
      
      $occur = [ "id" => $ruleId,
                 "rule" => $ruleId,
                 "start" => date('Ymd', $coupon['start']),
                 "end" => date('Ymd', $coupon['end']) ];
      $u_auxtxs->insert($occur)->saveData();
    }
    $coupateds = $this->query("SELECT * FROM r_coupated c JOIN tx_entries e ON (c.coupid = e.relatedId AND e.relType = 'D')")->fetchAll();
    foreach ($coupateds as $coupated) {
      $coupid = $coupated['coupid'];
      $id = $coupated['id'];
      $count = $this->execute("UPDATE tx_entries_all SET auxtx=$coupid WHERE id = $id");
      if ($count != 1) throw new Exception("Update failed, count returned is $count");
    }
  }

  public function down()
  {
    $this->execute("UPDATE tx_entries_all SET auxtx=null");
    $this->execute("DELETE FROM u_auxtxs");
    $this->execute("DELETE FROM u_rules");
  }
}
