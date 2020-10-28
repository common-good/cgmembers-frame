<?php
namespace CG;
use CG as r;
use CG\DB as db;
use CG\Util as u;

/**
 * @file
 * Activate an account.
 */

/**
 * Activate the account.
 * @param int $helper: account record ID of account that invited this one OR 1 (self) or 0 (unknown)
 */
function activate($a, $helper) {
  $DBTX = \db_transaction();
  $a->update(compact('helper')); // must precede welcome message
  $a->setBit(B_OK, TRUE); // must precede welcome message
  $a->setBit(B_NONUDGE, FALSE);
  
  if ($firstTime = !$a->activated) { // first time activation
    $a->update('activated', now() - 1); // this signals it's time to V-encrypt
    $a->update('vsecure', $a->vsecure); // V-encrypt
    $a->eachPartner(function($coA, $a, $custCode) { // tell partner: account is activated
      if ($api = $coA->api()) u\post($api, ray('partnerCode qid op name', $coA->emailCode, $a->mainQid, 'complete', $a->fullName));
    });
    $msg = 'approved';
//    if (!$a->confirmed) $msg .= ($helper > 1 ? '|must confirm' : '|in person');
//    if (!$a->confirmed) $msg .= ($a->helper == 1 ? '|must confirm uninvited' : '|must confirm');

    if (!$a->stepsDone($a->co ? 'co' : 'card')) $msg .= '|suggest completion';
    
    $subs = ray('inviterName youName', $a->helper ? $a->helperA->fullName : '', $a->fullName);
    
    $a->tell($msg, $subs);
    
  }

  if ($firstTime and !$a->co) { // no bonus or helper bonus for a company signing up
    if ($a->ssnErr and !$a->risk('ssnOff') and !$a->risk('dobOff')) $a->setRisk('ssnOff'); // make SURE this bit gets set (might not if admin signs the person up

    if (!$a->helperA->co and !$a->helperA->superAdmin) { // no helper bonus for company referral or help by superAdmin (cAdmin is okay) [don't require $a->confirmed]
      $fullName = $a->fullName;
      $helped = db\count('users', ':IS_OK AND NOT :IS_CO AND helper=:hId', ray('hId', $a->helper)) + 0;
      if ($helped == ORGANIZER_GRANT_MIN) {
//            r\acct($a->helper)->tell('organizer grant', ['_a' => w\atag('/community/grant')]);
        list ($name, $topic) = [$a->helperA->fullName, t('eligible')];
        r\tellAdmin('eligible for grant', compact(ray('name topic')));
      }
    }
  }
  
  unset($DBTX); 
}
