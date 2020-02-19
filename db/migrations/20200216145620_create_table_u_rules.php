<?php


use Phinx\Migration\AbstractMigration;

const REF_ANYBODY = 1;
const REF_USER = 2;
const REF_INDUSTRY = 3;
const REF_GROUP = 4;
const REF_LIST = [REF_ANYBODY, REF_USER, REF_INDUSTRY, REF_GROUP];

const ACTION_PAYMENT = 1;
const ACTION_BY_DATE = 2;
const ACTION_LIST = [ACTION_PAYMENT, ACTION_BY_DATE];

const ONLY_ONCE = 1;
const DAILY = 2;
const WEEKLY = 3;
const MONTHLY = 4;
const QUARTERLY = 5;
const YEARLY = 6;
const FOREVER = 7;
const PERIOD_CODES = [ONLY_ONCE, DAILY, WEEKLY, MONTHLY, QUARTERLY, YEARLY, FOREVER];

const SAME_AS_ACTOR = -1;
const SAME_AS_OTHER = -2;

class CreateTableURules extends AbstractMigration
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

  public function change()
  {
    $table = $this->table('u_rules', ['comment' => 'Rules for auxiliary transactions']);
    $table
      ->addColumn('actor', 'biginteger', ['null' => true, 'default' => null,
                                          'comment' => 'Who initiates transaction, null if anybody'])
      ->addColumn('actorType', 'enum', ['values' => REF_LIST,
                                        'comment' => 'Type of actor'])
      ->addColumn('other', 'biginteger', ['null' => true, 'default' => null,
                                          'comment' => 'Other party to transaction, null if anybody'])
      ->addColumn('otherType', 'enum', ['values' => REF_LIST,
                                        'comment' => 'Type of other'])
      ->addColumn('from', 'biginteger', ['comment' => 'Who to transfer money from'])
      ->addColumn('to', 'biginteger', ['comment' => 'Who to transfer money to'])
      ->addColumn('action', 'enum', ['values' => ACTION_LIST,
                                     'comment' => 'Action that triggers rules of this type'])
      ->addColumn('start', 'date', ['default' => 'CURRENT_TIMESTAMP',
                                    'comment' => 'Start date of first occurrence of this rule'])
      ->addColumn('end', 'date', ['null' => true, 'default' => null,
                                  'comment' => 'Date after which no more occurrences will be created (NULL if no end)'])
      ->addColumn('amount', 'decimal', ['precision' => 11, 'scale' => 2, 'default' => 0,
                                        'comment' => 'Fixed amount to transfer'])
      ->addColumn('portion', 'decimal', ['precision' => 7, 'scale' => 6, 'default' => 0,
                                         'comment' => 'Proportional amount, e.g., 5%, to transfer, expressed as a decimal, e.g., 0.05'])
      ->addColumn('on', 'string', ['length' => 255,
                                   'comment' => 'Text to appear on statements explaining this'])
      ->addColumn('minimum', 'decimal', ['precision' => 11, 'scale' => 2, 'default' => 0,
                                         'comment' => 'Minimum amount of transaction that this rule applies to'])
      ->addColumn('ulimit', 'integer', ['null' => true, 'default' => null,
                                        'comment' => 'Maximum number of uses per member, NULL if no max'])
      ->addColumn('amtLimit', 'decimal', ['precision' => 11, 'scale' => 2, 'null' => true, 'default' => null,
                                          'comment' => 'Maximum amount to transfer, NULL if no limit'])
      ->addColumn('period', 'enum', ['values' => PERIOD_CODES,
                                     'comment' => 'How often an occurrence will be generated'])
      ->addColumn('duration', 'integer', ['default' => 1,
                                          'comment' => 'How many duration units an occurrence is valid for'])
      ->addColumn('durUnit', 'enum', ['values' => PERIOD_CODES,
                                      'comment' => 'The unit of duration'])
      /* Because of special use of -1 and -2 */
      /* ->addForeignKey('from', 'users', 'uid', ['delete' => 'restrict']) */
      /* ->addForeignKey('to', 'users', 'uid', ['delete' => 'restrict']) */
      ->create();
  }
}
