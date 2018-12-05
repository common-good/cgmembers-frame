<?php


use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter;

class TempTest extends AbstractMigration
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
  public function up()
  {
    $this->down();
    $changesTable = $this->table('r_changes', ['comment' => 'Member record changes']);
    $changesTable->addColumn('uid', 'biginteger', ['null' => false, 'comment' => 'id of user record to which this change applies'])
      ->addColumn('created', 'biginteger', ['null' => true, 'comment' => 'Unix date and time that change was made'])
      ->addColumn('field', 'char', ['default' => 'NULL', 'null' => true, 'comment' => 'name of the field that was changed', 'length' => 40])
      ->addColumn('oldValue', 'blob', ['null' => true, 'comment' => 'serialized old value, possibly encrypted'])
      ->addColumn('newValue', 'blob', ['null' => true, 'comment' => 'serialized new value, possibly encrypted'])
      ->addColumn('changedBy', 'biginteger', ['null' => true, 'comment' => 'uid of user who made the change'])
      ->addIndex(['uid'])
      ->create();
    
    $criteriaTable = $this->table('r_criteria', ['comment' => 'Criteria for funding proposals']);
    $criteriaTable->addColumn('name', 'text', ['limit' => MysqlAdapter::TEXT_TINY, 'comment' => 'name of criterion'])
      ->addColumn('text', 'text', ['limit' => MysqlAdapter::TEXT_TINY, 'comment' => 'text of the criterion'])
      ->addColumn('detail', 'text', ['limit' => MysqlAdapter::TEXT_MEDIUM, 'comment' => 'additional detail about the criterion'])
      ->addColumn('points', 'decimal', ['precision' => 11, 'scale' => 2, 'null' => false, 'default' => '0.00', 'comment' => 'how many points for this criterion'])
      ->addColumn('auto', 'integer', ['limit' => MysqlAdapter::INT_TINY, 'null' => false, 'default' => 0, 'comment' => 'is this criterion calculated automatically?'])
      ->addColumn('displayOrder', 'integer', ['limit' => MysqlAdapter::INT_TINY, 'null' => false, 'default' => 0, 'comment' => 'where to display this criterion in the order'])
      ->addColumn('ctty', 'biginteger', ['null' => false, 'default' => 0, 'comment' => 'community or region record id (zero for all)'])
      ->addColumn('modified', 'biginteger', ['null' => false, 'default' => 0, 'comment' => 'date/time last modified'])
      ->addColumn('created', 'biginteger', ['null' => false, 'default' => 0, 'comment' => 'date/time created'])
      ->addIndex('ctty')
      ->create();

    $rows = [
             ['id' => 1, 'name' => 'suitable', 'text' => 'How well does the project support our Common Good community investment priorities?', 'detail' => 'NULL', 'points' => 15.00, 'auto' => 0, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 2, 'name' => 'systemic', 'text' => 'How well does the project promote systemic change?', 'detail' => 'NULL', 'points' => 15.00, 'auto' => 0, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 3, 'name' => 'doable', 'text' => 'Overall, how clearly doable is the project?', 'detail' => 'NULL', 'points' => 15.00, 'auto' => 0, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 4, 'name' => 'mgmt', 'text' => 'How competent is the project team to manage the project and funds effectively?', 'detail' => 'NULL', 'points' => 10.00, 'auto' => 0, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 5, 'name' => 'cope', 'text' => 'How able is the project team to implement the project with less funding than requested?', 'detail' => 'NULL', 'points' => 5.00, 'auto' => 0, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 6, 'name' => 'eval', 'text' => 'How useful is the project\'s evaluation plan?', 'detail' => 'NULL', 'points' => 5.00, 'auto' => 0, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 7, 'name' => 'recovery', 'text' => 'How quickly, surely, and voluminously will the project bring funds back into our Common Good Community Dollar Pool?', 'detail' => 'NULL', 'points' => 10.00, 'auto' => 0, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 8, 'name' => 'goodAmt', 'text' => 'How close is the request to the ideal amount (%idealAmt)?', 'detail' => 'NULL', 'points' => 5.00, 'auto' => 1, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 9, 'name' => 'budgetPct', 'text' => 'What fraction of the total project budget is this funding request? (50% is ideal)', 'detail' => 'NULL', 'points' => 5.00, 'auto' => 1, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 10, 'name' => 'committedPct', 'text' => 'How much of the total project budget has been raised/committed so far? (half is ideal)?', 'detail' => 'NULL', 'points' => 5.00, 'auto' => 1, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 11, 'name' => 'beginSoon', 'text' => 'How soon does the project begin? (soon after funding is best)', 'detail' => 'NULL', 'points' => 2.50, 'auto' => 1, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 12, 'name' => 'endSoon', 'text' => 'How soon does the project end? (soon after funding is best)', 'detail' => 'NULL', 'points' => 2.50, 'auto' => 1, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 13, 'name' => 'local', 'text' => 'How local is the project?', 'detail' => 'NULL', 'points' => 2.50, 'auto' => 1, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548],
             ['id' => 14, 'name' => 'sponsor', 'text' => 'Common Good member sponsorship of the project.', 'detail' => 'NULL', 'points' => 2.50, 'auto' => 1, 'displayOrder' => 0, 'ctty' => 0, 'modified' => 1541171548, 'created' => 1541171548]
             ];
    $criteriaTable->insert($rows)->save();
    
    $honorsTable = $this->table('r_honors', ['comment' => 'gifts in honor or memory']);
    $honorsTable->addColumn('uid', 'biginteger', ['comment' => 'uid of account making the gift to Common Good'])
      ->addColumn('honor', 'char', ['limit' => 10, 'comment' => 'what type of honor'])
      ->addColumn('honored', 'text', ['limit' => MysqlAdapter::TEXT_MEDIUM, 'comment' => 'who is honored'])
      ->addColumn('created', 'biginteger', ['null' => false, 'default' => 0, 'comment' => 'Unixtime of first associated gift'])
      ->addIndex('uid')
      ->create();

    $logTable = $this->table('r_log', ['comment' => 'Development and error log', 'id' => 'logid']);
    $logTable->addColumn('time', 'integer', ['limit' => 11, 'null' => false, 'default' => 0, 'comment' => 'date/time logged'])
      ->addColumn('channel', 'integer', ['limit' => MysqlAdapter::INT_TINY, 'comment' => 'logged from what interface module'])
      ->addColumn('type', 'char', ['limit' => 60, 'default' => 'NULL', 'comment' => 'what type of log entry'])
      ->addColumn('myid', 'biginteger', ['null' => true, 'comment' => 'current account uid'])
      ->addColumn('agent', 'biginteger', ['null' => true, 'comment' => 'agent account uid'])
      ->addColumn('info', 'text', ['limit' => MysqlAdapter::TEXT_MEDIUM, 'null' => true, 'comment' => 'arbitrary serialized data'])
      ->addIndex('type')
      ->addIndex('channel')
      ->addIndex('myid')
      ->addIndex('agent')
      ->create();

    $recursTable = $this->table('r_recurs', ['comment' => 'recurring gifts']);
    $recursTable->addColumn('payer', 'biginteger', ['comment' => 'uid of account making the recurring gifts'])
      ->addColumn('payee', 'biginteger', ['comment' => 'uid of account receiving the gifts'])
      ->addColumn('amount', 'decimal', ['precision' => 11, 'scale' => 2, 'comment' => 'amount of gift'])
      ->addColumn('period', 'char', ['limit' => 1, 'comment' => 'recurring how often (Y, Q, M, W)'])
      ->addColumn('created', 'biginteger', ['null' => false, 'default' => 0, 'comment' => 'Unixtime of gift'])
      ->addColumn('ended', 'biginteger', ['null' => false, 'default' => 0, 'comment' => 'Unixtime of gift'])
      ->addIndex('payer')
      ->addIndex('payee')
      ->create();
  }

  public function down()
  {
    if ($this->hasTable('r_changes'))
      $this->table('r_changes')->drop();
    if ($this->hasTable('r_criteria'))
      $this->table('r_criteria')->drop();
    if ($this->hasTable('r_honors'))
      $this->table('r_honors')->drop();
    if ($this->hasTable('r_log'))
      $this->table('r_log')->drop();
    if ($this->hasTable('r_recurs'))
      $this->table('r_recurs')->drop();
  }
}