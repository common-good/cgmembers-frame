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
    $note = $mya->stampNote(t('member clicked "Finish"'));
    $mya->update('notes', $note . $mya->notes);

    $mya->setTask($arg);
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