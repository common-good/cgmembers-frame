<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';
const A_LISTX = 'ach activate changeUid chargeFrom close code deleteAcct deletePhoto editNotes editTx export generalInvite helpSignup makeCtty makeRules makeAdmin makeVAdmin manageAccts manageVote mutualAid nonmemberTx panel payFrom printCards printChecks recheckSsn reconcile region reverseBankTx seeAccts seeCanonic seeChanges seeDeposits seeSecure seeSsn seeTxInfo setCredit setStepDone showVotes stopCtty stopServer ten99 v verifyId whileDown'; // admin permissions
const A_LIST = 'ach activate changeUid chargeFrom close code deleteAcct deletePhoto editNotes editTx export followup generalInvite helpSignup makeCtty makeRules makeAdmin makeVAdmin manageAccts manageVote mutualAid nonmemberTx panel payFrom printCards printChecks recheckSsn reconcile region reverseBankTx seeAccts seeCanonic seeChanges seeDeposits seeSecure seeSsn seeTxInfo setCredit setStepDone showVotes stopCtty stopServer ten99 v verifyId whileDown'; // admin permissions

class MinimalAdmin extends AbstractMigration {
  public function change() {
    if ($this->isMigratingUp()) {
      fixFlags($this, 'admins', 'can', A_LISTX, A_LIST);
    } else fixFlags($this, 'admins', 'can', A_LIST, A_LISTX);
  }
}
