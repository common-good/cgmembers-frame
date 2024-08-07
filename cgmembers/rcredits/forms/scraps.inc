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

  if (in($arg, S_FU_TASKS)) { // called from Dashboard page when member clicks to launch a followup task
    if ($mya->co) return go('', t('This page has been updated for your current account.')); // button for personal account was leftover (in a separate tab)
    $note = r\stampNote(t('member clicked ' . ucfirst($arg)));
    $mya->update('notes', $note . $mya->notes);

    $mya->setTask($arg);
    $nextStep = $mya->nextStep();
    w\go($nextStep ? (ray(S_ALL)[$nextStep] ?: "settings/$nextStep") : '');
  }
  
  if ($arg == 'reopen') {
    r\tellAdmin(t('Reopen account') . " $mya->fullName ($mya->mainQid)");
    return sayEmpty(t('<h2>Request Received</h2>Your request to reopen your account has been received. Expect to hear back from us within a day or two.'));
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