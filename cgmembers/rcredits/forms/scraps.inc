<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Handle a url for a small task.
 */
function formScraps($form, &$sta, $arg = '') {
  global $mya;

  if (in($arg, S_MEM_TASKS)) { // called from Summary page when member clicks to launch a followup task
    $note = $mya->stampNote(t('member clicked "Finish"; account deactivated'));

    $DBTX = db_transaction();
    $mya->setTask($arg);
    $mya->setBit(B_OK, FALSE);
    $mya->setBit(B_MEMBER, FALSE);
    $mya->update('notes activated0 activated', $note . $mya->notes, $mya->activated, 0); // make sure it doesn't look closed (no manage permission)
    unset($DBTX);
    
    $nextStep = $mya->nextStep();
    w\go($nextStep ? (ray(S_ALL)[$nextStep] ?: "settings/$nextStep") : '');
  }
  
  $title = t('');
  $subtext = t('');
}

function formScraps_validate($form, &$sta) {
  global $mya;
}

function formScraps_submit($form, &$sta) {
  global $mya;
}