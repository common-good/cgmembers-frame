<?php


use Phinx\Migration\AbstractMigration;

class MakeIndustriesRecursive extends AbstractMigration
{
  public function up() {
    $table = $this->table('r_industries');
    $table->changeColumn('parent', 'integer', ['limit' => 11, 'null' => true, 'default' => null]);
    $table->save();
    $table->addIndex(['parent']);
    $n = $this->execute('update r_industries set parent = null where parent = iid');
    $this->execute('CREATE VIEW descendants AS
      WITH RECURSIVE descendants AS (
        SELECT iid AS base, industry AS baseIndustry, iid, industry FROM r_industries
        UNION ALL
        SELECT d.base AS base, d.baseIndustry AS baseIndustry, c.iid, c.industry 
        FROM r_industries c INNER JOIN descendants d ON d.iid = c.parent )
      SELECT base, baseIndustry, iid AS descendant, industry AS descendantIndustry FROM descendants');
    $this->execute('CREATE VIEW ancestors AS
      WITH RECURSIVE ancestors AS (
        SELECT iid AS base, industry AS baseIndustry, iid, industry, parent FROM r_industries
        UNION ALL
        SELECT a.base AS base, a.baseIndustry AS baseIndustry, p.iid, p.industry, p.parent
        FROM r_industries p INNER JOIN ancestors a ON p.iid = a.parent )
      SELECT base, baseIndustry, iid AS ancestor, industry AS ancestorIndustry FROM ancestors');
  }

  public function down() {
    $this->execute('DROP VIEW ancestors');
    $this->execute('DROP VIEW descendants');
    $n = $this->execute('update r_industries set parent = iid where parent is null');
    $table = $this->table('r_industries');
    $table->removeIndex(['parent']);
    $table->changeColumn('parent', 'integer', ['limit' => 11, 'null' => false]);
    $table->save();
  }
}
