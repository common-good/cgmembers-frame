<?php


use Phinx\Migration\AbstractMigration;

class MigrateCoupons extends AbstractMigration
{
  const REF_LIST = 'anybody account anyCompany industry group';
  const ACT_LIST = 'pay now redeem';
  const PERIODS = 'once day week month quarter year forever';


  const SAME_AS_PAYER = -1;
  const SAME_AS_PAYEE = -2;

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
    $rules = $this->table('tx_rules');
    $groups = $this->table('u_groups');
    $groupies = $this->table('u_groupies');
    $templates = $this->table('tx_templates');
    
    $coupons = $this->query('SELECT * FROM r_coupons')->fetchAll();
    $this->execute("DELETE FROM tx_rules");
    $count = 0;
    foreach ($coupons as $coupon) {
      $coupon_amount = $coupon["amount"];
      $amount = $coupon_amount > 0 ? $coupon_amount : 0;
      $portion = $coupon_amount < 0 ? -$coupon_amount * 0.01 : 0;  // Convert from % to fraction
      $ruleId = $coupon["coupid"];
      $rule = [ "id" => $ruleId,
                "payer" => null, "payerType" => 'anybody',
                "payee" => $coupon["fromId"], "payeeType" => 'account',
                "from" => $coupon["sponsor"],
                "to" => self::SAME_AS_PAYER,
                "action" => 'pay',
                "start" => $coupon["start"],
                "end" => $coupon["end"],
                "amount" => $amount,
                "portion" => $portion,
                "purpose" => is_null($coupon["on"]) ? '' : $coupon["on"],
                "minimum" => $coupon["minimum"],
                "ulimit" => $coupon["ulimit"],
                "amtLimit" => null ];
      $rules->insert($rule);
      $count += 1;
    }
    $rules->saveData();

    echo "Inserted $count tx_rules\n";

    // relatedId was supposed to be the id of the coupated record, but is actually the id of the coupon record
    // so we can just set rule to relatedId...
    $count = $this->execute("UPDATE tx_entries_all SET rule=relatedId WHERE relType='D'");
    echo "Updated $count tx_entries\n";

    // Handle "restricted", i.e., Food-Fund-like coupons
    $stmt = $this->query("SELECT c.*, sponsor.fullName AS sponsorName FROM r_coupons c JOIN users sponsor ON c.sponsor = sponsor.uid WHERE (c.flags&2)=2");
    $count = $stmt->rowCount();
    echo "We should be processing $count restricted coupons.\n";

    $grpId = 0;
    foreach ($stmt->fetchAll(\PDO::FETCH_ASSOC) as $coupon) {
      print_r($coupon);
      echo "\n";
      $sponsorName = $coupon['sponsorName'];
      echo "The name of the sponsor is '$sponsorName'.\n";
      $groupName = $sponsorName . ' Recipients';
      echo "We're naming the group '$groupName'.\n";

      $coupId = $coupon['coupid'];
      
      // Start by figuring out who's in the group
      $stmt = $this->query("SELECT u.* FROM r_coupated d JOIN users u ON d.uid=u.uid WHERE d.coupid='$coupId'");
      $count = $stmt->rowCount();
      echo "Apparently this group has $count members\n";
      $members = $stmt->fetchAll(\PDO::FETCH_ASSOC);

      // Now create the group and connect users to it.
      $grpId += 1;
      $groups->insert(['id' => $nextGrpId, 'name' => $groupName]);
      $groups->saveData();
      foreach ($members as $member) {
        $memberName = $member['fullName'];
        echo "-- adding $memberName\n";
        $groupies->insert(['uid' => $member['uid'], 'grpId' => $grpId, 'isMember' => true, 'start' => $coupon['start'], 'end' => $coupon['end']]);
      }
      $groupies->saveData();

      // Now create the template and connect it to the group.
      $template = ['id' => $grpId,
                   'payer' => $grpId, 'payerType' => 'group', 'payee' => $coupon['fromId'], 'payeeType' => 'account',
                   'from' => $coupon['sponsor'], 'to' => self::SAME_AS_PAYER, 'action' => 'pay',
                   'start' => $coupon['start'], 'end' => $coupon['end'],
                   'amount' => $coupon['amount'] < 0 ? 0 : $coupon['amount'],
                   'portion' => $coupon['amount'] < 0 ? -$coupon['amount'] * 0.01 : 0,  // convert percentage to portion
                   'purpose' => $coupon['on'], 'minimum' => $coupon['minimum'],
                   'ulimit' => $coupon['ulimit'], 'amtLimit' => max($coupon['amount'], 0),
                   'period' => 1, 'prdUnits' => 'month', 'duration' => 1, 'durUnits' => 'forever' ];
      $templates->insert($template);
      $templates->saveData();
      echo "The coupon sponsored by '$sponsorName' may need manual adjustment, particulary the amount, end date, period, and duration.\n";

      // Generate the appropriate rules -- this code should be closely related to the corresponding code in rcron.
      $shouldStart = $coupon['start'];  // start date of the first rule

      $rule = array_diff_key($template, ['id' => 1, 'period' => 1, 'prdUnits' => 1, 'duration' => 1, 'durUnits' => 1, 'start' => 1, 'end' => 1, 'maxStart' => 1]);
      print_r($rule);
      echo("\n");
      $rule['template'] = $grpId;
      while ($shouldStart < time() and (is_null($template['end']) or $shouldStart < $template['end'])) {
        $rule['start'] = $shouldStart;
        $rule['end'] = $this->dateIncr($shouldStart, $template['duration'], $template['durUnits']);
        $rules->insert($rule);
        echo "Generated rule with starting date $shouldStart.\n";
        $shouldStart = $this->dateIncr($shouldStart, $template['period'], $template['prdUnits']);
      }
      $rules->saveData();
    }
    
    // Unfortunately we also need to handle gift cards, :-(
    $stmt = $this->query('SELECT * FROM r_coupons c WHERE (flags & 1024)=1024');  //
    $count = $stmt->rowCount();
    echo "There are $count gift cards to be handled.\n";
    $giftCards = $stmt->fetchAll(\PDO::FETCH_ASSOC);
    $count = 0;
    foreach ($giftCards as $giftCard) {
      /* print_r($giftCard); */
      /* echo "\n"; */
      foreach (range($giftCard['start'], $giftCard['end']-1) as $number) {
        $newRule = ['payer' => null, 'payerType' => 'anybody',
                    'payee' => null, 'payeeType' => 'anybody',
                    'from' => $giftCard['fromId'], 'to' => self::SAME_AS_PAYEE,
                    'action' => 'redeem',
                    'amount' => $giftCard['amount'],
                    'portion' => 0,
                    'purpose' => $giftCard['on'] ?: 'gift certificate',
                    'minimum' => $giftCard['minimum'],
                    'ulimit' => $giftCard['ulimit'],
                    'amtLimit' => $giftCard['amount'],
                    'template' => null,
                    'start' => 0,
                    'end' => null,
                    'code' => $number ];
        $rules->insert($newRule);
        $count += 1;
      }
    }
    $rules->saveData();
    echo "We created $count rules for gift cards.\n";
  }
  
  public function down()
  {
    $this->execute("UPDATE tx_entries_all SET rule=null");
    $this->execute("DELETE FROM tx_rules");
    $this->execute("DELETE FROM tx_templates");
    $this->execute("DELETE FROM u_groupies");
    $this->execute("DELETE FROM u_groups");
  }

  function dateIncr($start, $number, $units) {
    $startDate = (new \DateTime())->setTimestamp($start);
    if ($units == 'forever') {
      return null;
    }
    if ($units == 'quarter') {
      $units = 'month';
      $number *= 3;
    }
    $unitCode = [ 'once' => 'XX', 'day' => 'D', 'week' => 'W', 'month' => 'M', 'year' => 'Y', 'forever' => 'X' ][$units];
    $interval = new \DateInterval("P$number$unitCode");
    return $startDate->add($interval)->getTimestamp();
  }
}
