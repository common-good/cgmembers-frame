<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class Clickup2 extends AbstractMigration {
  public function change() {
    $t = $this->table('cu_members', ray('id primary_key comment', FALSE, 'id', 'clickup members and guests on the Common Good team'));
    $t->addColumn('id', 'integer', ray('length null identity comment', phx::INT_BIG, FALSE, FALSE, 'record ID'));
    $t->addColumn('uid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'account record ID, if any'));
    $t->addColumn('name', 'string', ray('length null comment', 255, TRUE, 'team member name'));
    $t->addColumn('nick', 'string', ray('length null comment', 5, TRUE, 'team member nickname'));
    $t->create();
    
    $t = $this->table('cu_spaces', ray('id primary_key comment', FALSE, 'id', 'top level categories'));
    $t->addColumn('id', 'integer', ray('length null identity comment', phx::INT_BIG, FALSE, FALSE, 'record ID'));
    $t->addColumn('name', 'string', ray('length null comment', 255, TRUE, 'space name'));
    $t->create();
    
    $t = $this->table('cu_folders', ray('id primary_key comment', FALSE, 'id', 'groups of lists'));
    $t->addColumn('id', 'integer', ray('length null identity comment', phx::INT_BIG, FALSE, FALSE, 'record ID'));
    $t->addColumn('name', 'string', ray('length null comment', 255, TRUE, 'folder name'));
    $t->create();

    $t = $this->table('cu_lists', ray('id primary_key comment', FALSE, 'id', 'lists of tasks'));
    $t->addColumn('id', 'integer', ray('length null identity comment', phx::INT_BIG, FALSE, FALSE, 'record ID'));
    $t->addColumn('name', 'string', ray('length null comment', 255, TRUE, 'list name'));
    $t->addColumn('folder', 'integer', ray('length null comment', phx::INT_MEDIUM, TRUE, 'folder record ID, if any'));
    $t->addColumn('space', 'integer', ray('length null comment', phx::INT_MEDIUM, TRUE, 'space record ID, if not in any folder'));
    $t->create();

    $t = $this->table('cu_tasks', ray('id primary_key comment', FALSE, 'id', 'things to be done'));
    $t->addColumn('id', 'string', ray('length null identity comment', 255, FALSE, FALSE, 'record ID'));
    $t->addColumn('name', 'string', ray('length null comment', 255, TRUE, 'task name'));
    $t->addColumn('parent', 'string', ray('length null comment', 255, TRUE, 'parent task, if this is a subtask'));
    $t->addColumn('list', 'integer', ray('length null comment', phx::INT_MEDIUM, TRUE, 'list record ID'));
    $t->addColumn('status', 'string', ray('length null comment', 255, TRUE, 'task status'));
    $t->addColumn('priority', 'string', ray('length null comment', 255, TRUE, 'task priority'));
    $t->addColumn('tags', 'string', ray('length null comment', 255, TRUE, 'comma-delimted list of tags for this task'));
    $t->addColumn('estimate', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'estimated time to complete this task'));
    $t->addColumn('spent', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'time spent on this task'));
    $t->addColumn('closed', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'date/time task was closed'));
    $t->create();

    $t = $this->table('cu_times', ray('id primary_key comment', FALSE, 'id', 'amount of time spent on tasks'));
    $t->addColumn('id', 'integer', ray('length null identity comment', phx::INT_BIG, FALSE, FALSE, 'record ID'));
    $t->addColumn('task', 'string', ray('length null comment', 255, TRUE, 'task on which time was spent'));
    $t->addColumn('member', 'string', ray('length null comment', 255, TRUE, 'team member record ID'));
    $t->addColumn('start', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'date/time started'));
    $t->addColumn('stop', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'date/time stopped'));
    $t->create();
  }
}
