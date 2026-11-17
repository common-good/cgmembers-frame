<?php
use Phinx\Migration\AbstractMigration;

/**
 * Give every agent relation that never got an agent number (otherNum 0, from before agent numbers existed) the next free
 * number for its main account. The app encodes agent number 0 as agent A, so unnumbered agents could not sign in to it.
 */
class NumberLegacyAgents extends AbstractMigration {
  public function up() {
    $max = [];
    foreach ($this->fetchAll('SELECT main, MAX(otherNum) AS n FROM u_relations GROUP BY main') as $r) $max[$r['main']] = (int) $r['n'];
    foreach ($this->fetchAll('SELECT reid, main FROM u_relations WHERE otherNum=0 ORDER BY main, reid') as $r) {
      $n = ++$max[$r['main']];
      $this->execute("UPDATE u_relations SET otherNum=$n WHERE reid=" . (int) $r['reid']);
    }
  }
  public function down() {} // the old numbers were all zero and carried no information
}
