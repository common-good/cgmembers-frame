<?php


use Phinx\Migration\AbstractMigration;
require_once 'cgmembers/rcredits/defs.inc';

class AddCanonicalAccounts extends AbstractMigration
{
  public function up() {
    $keys = 'uid community name fullName email zip country minimum flags';
    $this->table('users')
      ->insert(ray($keys, CG_ROUNDUPS_UID, 0, t('roundups'), t('Roundup Donations'), 'cg@commongood.earth', '', 0, 0, 0))
      ->insert(ray($keys, CG_CRUMBS_UID, 0, t('crumbs'), t('Crumb Donations'), 'cg@commongood.earth', '', 0, 0, 0))
      ->insert(ray($keys, CG_INCOMING_BANK_UID, 0, t('bank-in'), t('Incoming bank'), 'cg@commongood.earth', '', 0, 0, 0)) 
      ->insert(ray($keys, CG_OUTGOING_BANK_UID, 0, t('bank-out'), t('Outgoing bank'), 'cg@commongood.earth', '', 0, 0, 0))
      ->save();
  }

  public function down() {
    $keys = 'uid community name fullName email zip country minimum flags';
    $this->execute("DELETE FROM users where uid='" . CG_ROUNDUPS_UID . "'");
    $this->execute("DELETE FROM users where uid='" . CG_CRUMBS_UID . "'");
    $this->execute("DELETE FROM users where uid='" . CG_INCOMING_BANK_UID . "'");
    $this->execute("DELETE FROM users where uid='" . CG_OUTGOING_BANK_UID . "'");
  }
}
