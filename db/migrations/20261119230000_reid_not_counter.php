<?php
use Phinx\Migration\AbstractMigration;

/**
 * Drop AUTO_INCREMENT from u_relations.reid (and its deleted-records copy). The app assigns reid itself as a regional id,
 * like users.uid, so relation ids stay unique across regional servers; the counter was never meant to be used.
 */
class ReidNotCounter extends AbstractMigration {
  public function up() {$this->reid('');}
  public function down() {$this->reid(' AUTO_INCREMENT');}
  private function reid($auto) {
    foreach (['u_relations', 'x_relations'] as $t) $this->execute("ALTER TABLE $t MODIFY reid bigint(20) NOT NULL$auto COMMENT 'relationship record id'");
  }
}
