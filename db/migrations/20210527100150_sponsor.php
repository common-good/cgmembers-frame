<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Sponsor extends AbstractMigration {
  public function change() {
    $t = $this->table('r_company');
    $t->rename('u_company');
    $t->addColumn('mission', 'text', ray('length null comment', phx::TEXT_LONG, TRUE, 'the organization\'s mission'));
    $t->addColumn('activities', 'text', ray('length null comment', phx::TEXT_LONG, TRUE, 'what the (sponsored) organization actually does to advance its mission'));
    $t->addColumn('checksIn', 'integer', ray('length null comment', phx::INT_MEDIUM, TRUE, 'expected number of checks received monthly'));
    $t->addColumn('checksOut', 'integer', ray('length null comment', phx::INT_MEDIUM, TRUE, 'expected number of outgoing payments monthly'));
    $t->update();

    $t = $this->table('budget_cats', ray('comment', 'income and expense categories'));
    $t->addColumn('category', 'string', ray('length null comment', 255, TRUE, 'category'));
    $t->addColumn('type', 'enum', ['values' => ['Income', 'Expense', 'Asset', 'Liability'], 'comment' => 'balance sheet account type']);
    $t->addColumn('line990', 'string', ray('length null comment', 255, TRUE, 'section, part, and line number of this category on IRS Form 990'));
//    $t->addColumn('description', 'text', ray('length null comment', phx::TEXT_LONG, TRUE, 'description of category'));
    $t->create();
    
/* OOPS - not available in this version of phinx
    $this->getQueryBuilder()
      ->insert(ray('category type line990'))
      ->into('budget_cats')
      ->values(['Contributions / Donations / Gifts / Private Grants', 'Income', '1f'])
      ->execute();
      */
  
    $this->execute(<<<X
      INSERT INTO budget_cats (category, type, line990) VALUES 
      ('Contributions / donations / gifts / private grants', 'Income', '1f'),
      ('Government grants', 'Income', '1e'),
      ('In-kind (non-cash) contributions', 'Income', '1g'),

      ('Grants to U.S. organizations', 'Expense', '1'),
      ('Grants to U.S. individuals', 'Expense', '2'),
      ('Foreign grants', 'Expense', '3'),
      ('Compensation of current officers, directors, trustees, and key employees', 'Expense', '5'),
      ('Other salaries and wages', 'Expense', '7'),
      ('Pension plan contributions', 'Expense', '8'),
      ('Other employee benefits', 'Expense', '9'),
      ('Payroll taxes', 'Expense', '10'),
      ('Management fees', 'Expense', '11a'),
      ('Legal fees', 'Expense', '11b'),
      ('Accounting fees', 'Expense', '11c'),
      ('Professional fundraising service fees', 'Expense', '11e'),
      ('Investment management fees', 'Expense', '11f'),
      ('Other fees', 'Expense', '11g'),
      ('Advertising and promotion', 'Expense', '12'),
      ('Utilities', 'Expense', '13'),
      ('Office equipment (computers etc)', 'Expense', '13'),
      ('Office supplies', 'Expense', '13'),
      ('Postage and shipping', 'Expense', '13'),
      ('Printing and copying', 'Expense', '13'),
      ('Information technology (software, internet/web, etc)', 'Expense', '14'),
      ('Royalties', 'Expense', '15'),
      ('Rent / occupancy', 'Expense', '16'),
      ('Travel', 'Expense', '17'),
      ('Transporation or entertainment of public officials', 'Expense', '18'),
      ('Conferences, conventions, and meetings', 'Expense', '19'),
      ('Interest', 'Expense', '20'),
      ('Payments to affiliates', 'Expense', '22'),
      ('Depreciation, depletion, and amortization', 'Expense', '22'),
      ('Insurance', 'Expense', '23');
X
    );
  }
}
