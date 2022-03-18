<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class QbAccounts extends AbstractMigration {
  public function change() {
    $t = $this->table('budget_cats');
    $t->addColumn('description', 'text', ray('length null comment after', phx::TEXT_LONG, TRUE, 'description of category', 'category'));
    $t->addColumn('externalId', 'integer', ray('length null comment after', phx::INT_MEDIUM, TRUE, 'account record ID in external accounting program', 'description'));
    $t->addColumn('show', 'integer', ray('length null default comment after', phx::INT_TINY, false, 0, 'whether to show this category in dropdowns', 'externalId'));
    $t->update();
  }
}
