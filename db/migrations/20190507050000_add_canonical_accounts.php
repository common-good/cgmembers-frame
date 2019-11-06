<?php

use Phinx\Migration\AbstractMigration;

class AddCanonicalAccounts extends AbstractMigration {
  
  const CG_ADMIN_UID = 1;  // uid of administrator account
  const PLACEHOLDER_1_UID = 2;  // uid of first placeholder
  const PLACEHOLDER_2_UID = 3;  // uid of second placeholder
  const CG_ROUNDUPS_UID = 129;  // Donations start at 128 (128 used for general donations)
  const CG_CRUMBS_UID = 130;  //
  const CG_SERVICE_CHARGES_UID = 192;
  const CG_INCOMING_BANK_UID = 256;  // Bank accounts start at 256
  const CG_OUTGOING_BANK_UID = 257;  //

  public function up() {
    $this->execute("DELETE FROM users where uid IN (" . self::CG_ROUNDUPS_UID . ", " . self::CG_CRUMBS_UID . ", " . self::CG_SERVICE_CHARGES_UID . ", " . self::CG_INCOMING_BANK_UID . ", " . self::CG_OUTGOING_BANK_UID . ")");
    $this->table('users')
      ->insert(['uid'=>self::CG_ROUNDUPS_UID, 'community'=>0, 'name'=>'roundups', 'fullName'=>'Roundup Donations',
                'email'=>'cg@commongood.earth', 'zip'=>'', 'country'=>0, 'minimum'=>0, 'flags'=>6])
      ->insert(['uid'=>self::CG_CRUMBS_UID, 'community'=>0, 'name'=>'crumbs', 'fullName'=>'Crumb Donations',
                'email'=>'cg@commongood.earth', 'zip'=>'', 'country'=>0, 'minimum'=>0, 'flags'=>6])
      ->insert(['uid'=>self::CG_SERVICE_CHARGES_UID, 'community'=>0, 'name'=>'service charges', 'fullName'=>'Service Charges',
                'email'=>'cg@commongood.earth', 'zip'=>'', 'country'=>0, 'minimum'=>0, 'flags'=>6])
      ->insert(['uid'=>self::CG_INCOMING_BANK_UID, 'community'=>0, 'name'=>'bank-in', 'fullName'=>'--',
                'email'=>'cg@commongood.earth', 'zip'=>'', 'country'=>0, 'minimum'=>0, 'flags'=>6]) 
      ->insert(['uid'=>self::CG_OUTGOING_BANK_UID, 'community'=>0, 'name'=>'bank-out', 'fullName'=>'--',
                'email'=>'cg@commongood.earth', 'zip'=>'', 'country'=>0, 'minimum'=>0, 'flags'=>6])
      ->save();
  }

  public function down() {
    $this->execute("DELETE FROM users where uid IN (0, " . self::CG_ROUNDUPS_UID . ", " . self::CG_CRUMBS_UID . ", " . self::CG_SERVICE_CHARGES_UID . ", " . self::CG_INCOMING_BANK_UID . ", " . self::CG_OUTGOING_BANK_UID . ")");
  }
}
