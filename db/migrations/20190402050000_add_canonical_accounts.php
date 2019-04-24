<?php


use Phinx\Migration\AbstractMigration;

define('CG_ADMIN_UID', 1);  // uid of administrator account
define('PLACEHOLDER_1_UID', 2);  // uid of first placeholder
define('PLACEHOLDER_2_UID', 3);  // uid of second placeholder
define('CG_ROUNDUPS_UID', 129);  // Donations start at 128 (128 used for general donations)
define('CG_CRUMBS_UID', 130);  //
define('CG_SERVICE_CHARGES_UID', 192);
define('CG_INCOMING_BANK_UID', 256);  // Bank accounts start at 256
define('CG_OUTGOING_BANK_UID', 257);  //


class AddCanonicalAccounts extends AbstractMigration
{
  public function up() {
    $keys = 'uid community name fullName email zip country minimum flags';
    $this->table('users')
      ->insert(ray($keys, CG_ROUNDUPS_UID, 0, t('roundups'), t('Roundup Donations'), 'cg@commongood.earth', '', 0, 0, 6))
      ->insert(ray($keys, CG_CRUMBS_UID, 0, t('crumbs'), t('Crumb Donations'), 'cg@commongood.earth', '', 0, 0, 6))
      ->insert(ray($keys, CG_SERVICE_CHARGES_UID, 0, t('service charges'), t('Service Charges'), 'cg@commongood.earth', '', 0, 0, 6))
      ->insert(ray($keys, CG_INCOMING_BANK_UID, 0, t('bank-in'), t('--'), 'cg@commongood.earth', '', 0, 0, 6)) 
      ->insert(ray($keys, CG_OUTGOING_BANK_UID, 0, t('bank-out'), t('--'), 'cg@commongood.earth', '', 0, 0, 6))
      ->save();
  }

  public function down() {
    $keys = 'uid community name fullName email zip country minimum flags';
    $this->execute("DELETE FROM users where uid IN (" . CG_ROUNDUPS_UID . ", " . CG_CRUMBS_UID . ", " . CG_SERVICE_CHARGES_UID . ", " . CG_INCOMING_BANK_UID . ", " . CG_OUTGOING_BANK_UID . ")");
  }
}
