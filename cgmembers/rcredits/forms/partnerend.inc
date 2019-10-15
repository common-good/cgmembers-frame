<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Finish a signup through a partner organization (for now, just show or link to the contract with Co-op Power).
 */
function formPartnerEnd($form, &$sta, $args = '') {
  global $mya;
  extract(just('nothing', $args));

  $title = t('%PROJECT Account Completed');
  $subtext = t('<p>Your %PROJECT account is complete.</p><p>Click the button below to sign a contract with Coop Power, to complete your Community Shared Solar sign-up.</p>');

  $href = 'https://powerforms.docusign.net/23cb08a4-5669-44d1-9341-76f18575fc12?env=na3-eu1';
  $button = item(btn($href, t('Go to Contract'), 'primary', 'md'));

  if ($mya->nextStep() != 'partnerend') return w\go($href);
  $mya->stepDone('partnerend');
  return cgform(compact(ray('title subtext button')));
}
