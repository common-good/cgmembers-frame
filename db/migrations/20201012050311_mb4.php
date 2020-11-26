<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

const TNMS = 'block cache cache_bootstrap cache_form cache_menu flood menu_links menu_router messages people phinxlog post_cats posts queue r_areas r_bad r_ballots r_banks r_boxes r_changes r_company r_countries r_criteria r_do r_events r_honors r_industries r_investments r_invites r_invoices r_ips r_near r_notices r_options r_pairs r_photos r_proposals r_proxies r_questions r_ratings r_regions r_relations r_shares r_stakes r_states r_stats r_tous r_transit r_usd r_usd2 r_user_industries r_votes registry registry_file semaphore sessions signup system test tx_disputes_all tx_entries_all tx_hdrs_all tx_rules tx_templates u_groupies u_groups u_shouters u_track users variable x_invoices x_photos x_relations x_txs x_usd x_users zip3';
const SKIP_TNMS = 'menu_links'; // this one makes MySQL server "go away"

class Mb4 extends AbstractMigration {
  public function change() {
    $dbName = $this->getAdapter()->getOption('name');
    echo "\n\ndb: $dbName\n\n";
    if ($dbName == 'demo') $dbName = 'new_demo';
    $this->execute("ALTER DATABASE $dbName CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci;");

    foreach (ray(TNMS) as $tnm) {
      if (in_array($tnm, ray(SKIP_TNMS))) continue;
      echo "$tnm\n";
      // convert table from utf8 to utf8mb4
      $this->execute("ALTER TABLE $tnm CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;");
    }
    
    // handle special case (menu_links) -- with VARCHAR max 191
    echo ".. menu_name\n";
    $this->execute("ALTER TABLE `menu_links` CHANGE `menu_name` `menu_name` VARCHAR(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'The menu name. All links with the same menu name (such as ’navigation’) are part of the same menu.';");
    echo ".. link_path\n";
    $this->execute("ALTER TABLE `menu_links` CHANGE `link_path` `link_path` VARCHAR(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'The Drupal path or external path this link points to.';");
    echo ".. router_path\n";
    $this->execute("ALTER TABLE `menu_links` CHANGE `router_path` `router_path` VARCHAR(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'For links corresponding to a Drupal path (external = 0), this connects the link to a menu_router.path for joins.';");
    echo ".. link_title\n";
    $this->execute("ALTER TABLE `menu_links` CHANGE `link_title` `link_title` VARCHAR(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'The text displayed for the link, which may be modified by a title callback stored in menu_router.';");
    echo ".. module\n";
    $this->execute("ALTER TABLE `menu_links` CHANGE `module` `module` VARCHAR(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'system' COMMENT 'The name of the module that generated this link.';");
  }
}
