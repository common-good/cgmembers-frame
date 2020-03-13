<?php


use Phinx\Migration\AbstractMigration;

class CreateTableTxRules extends AbstractMigration
{
  const REF_ANYBODY = 1;
  const REF_ACCOUNT = 2;
  const REF_INDUSTRY = 3;
  const REF_GROUP = 4;
  const REF_LIST = [self::REF_ANYBODY, self::REF_ACCOUNT, self::REF_INDUSTRY, self::REF_GROUP];

  const ACTION_PAYMENT = 1;
  const ACTION_BY_DATE = 2;
  const ACTION_REDEEM = 3;
  const ACTION_LIST = [self::ACTION_PAYMENT, self::ACTION_BY_DATE, self::ACTION_REDEEM];

  const ONLY_ONCE = 1;
  const DAILY = 2;
  const WEEKLY = 3;
  const MONTHLY = 4;
  const QUARTERLY = 5;
  const YEARLY = 6;
  const FOREVER = 7;
  const PERIOD_CODES = [self::ONLY_ONCE, self::DAILY, self::WEEKLY, self::MONTHLY, self::QUARTERLY, self::YEARLY, self::FOREVER];

  const SAME_AS_PAYER = -1;
  const SAME_AS_PAYEE = -2;

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
  public function change()
  {
    $table = $this->table('tx_rules', ['comment' => 'Occurrences of a rule']);
    $table
      ->addColumn('payer', 'biginteger', ['null' => true, 'default' => null,
                                          'comment' => 'Who initiates transaction, null if anybody'])
      ->addColumn('payerType', 'enum', ['values' => self::REF_LIST,
                                        'comment' => 'Type of payer'])
      ->addColumn('payee', 'biginteger', ['null' => true, 'default' => null,
                                          'comment' => 'Payee party to transaction, null if anybody'])
      ->addColumn('payeeType', 'enum', ['values' => self::REF_LIST,
                                        'comment' => 'Type of payee'])
      ->addColumn('fromId', 'biginteger', ['comment' => 'Who to transfer money from'])
      ->addColumn('toId', 'biginteger', ['comment' => 'Who to transfer money to'])
      ->addColumn('action', 'enum', ['values' => self::ACTION_LIST,
                                     'comment' => 'Action that triggers templates of this type'])
      ->addColumn('amount', 'decimal', ['precision' => 11, 'scale' => 2, 'default' => 0, 'signed' => false,
                                        'comment' => 'Fixed amount to transfer'])
      ->addColumn('portion', 'decimal', ['precision' => 7, 'scale' => 6, 'default' => 0, 'signed' => false,
                                         'comment' => 'Proportional amount, e.g., 5%, to transfer, expressed as a decimal, e.g., 0.05'])
      ->addColumn('purpose', 'string', ['length' => 255, 'default' => '',
                                        'comment' => 'Text to appear on statements explaining this'])
      ->addColumn('minimum', 'decimal', ['precision' => 11, 'scale' => 2, 'default' => 0, 'signed' => false,
                                         'comment' => 'Minimum amount of transaction that this template applies to'])
      ->addColumn('ulimit', 'integer', ['null' => true, 'default' => null, 'signed' => false,
                                        'comment' => 'Maximum number of uses per member, NULL if no max'])
      ->addColumn('amtLimit', 'decimal', ['precision' => 11, 'scale' => 2, 'null' => true, 'default' => null,
                                          'signed' => false,
                                          'comment' => 'Maximum amount to transfer, NULL if no limit'])
      ->addColumn('template', 'integer', ['null' => true, 'default' => null,
                                          'comment' => 'Template of which this is an occurrence'])
      ->addColumn('start', 'biginteger', ['null' => false,
                                          'comment' => 'Start of period for which this occurrence applies'])
      ->addColumn('end', 'biginteger', ['null' => true,
                                        'comment' => 'End of period for which this occurrence applies, NULL if it does not end'])
      ->addColumn('code', 'integer', ['null' => true, 'default' => null,
                                      'comment' => 'For gift cards the individual code'])
      /* ->addForeignKey('template', 'tx_templates', 'id', ['delete' => 'restrict']) */
      ->create();
  }
}
