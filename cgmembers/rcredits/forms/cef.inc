<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const CG_CU_ID = 12824113;
const CEF_STAFF = CG_CU_ID . ':CG, 14970696:Alan, 57038485:Bryn, 75357671:Charlie, 4228942:Mary Kay, 14970695:Timothy';
const CEF_WORK_LIST  = 198021752;

/**
 * Provide a dashboard for CEF staff to make limited changes to ClickUp tasks in CEF Work

  require_once R_ROOT . '/classes/clickup.class';
  $cu = new \Clickup();
  $start = strtotime('1oct2023'); // nobody should be adding or editing any times before that
  $end = NOW;
  $cu->updateTimes($start, $end);
  $cu->updateTasks($start, NOW); // no harm in adding all possible tasks
  $cu->updateTasks('SELECT DISTINCT parent AS task FROM cu_tasks WHERE parent IS NOT NULL'); // add parent tasks
*/
function formCEF($form, &$sta, $args = '') {
  extract(just('co', $args, NULL));
  if ($co != 'cef') return softErr(t('That page does not exist.'));
  
  $title = t('CEF ClickUp Dashboard');
  $taskId = textFld(REQ . t('Task:'), [t('Task Id / URL'), t('Paste here the ClickUp task ID or URL')]);
  $assignTo = selectFld(REQ . t('Assign to:'), t('Hold down the Ctrl key to select more than one. CG means "Common Good team please look at the latest comment on this task".'), ray('multiple size', TRUE, 8)+dft(CG_CU_ID), ray(CEF_STAFF));
  $submit = submit(t('Go'));
  
  return cgform(compact(ray('title taskId assignTo submit')));
}

function formCEF_validate($form, &$sta) {
  extract(just('taskId', $sta['input']));
  $ray = explode('/', $taskId);
  tweakSta($sta, ray('taskId', str_replace('#', '', $ray[count($ray) - 1])));
}

function formCEF_submit($form, &$sta) {
  extract(just('taskId assignTo', $sta['input']));
  
  require_once R_ROOT . '/classes/clickup.class';
  $cu = new \Clickup();

  if ($task = $cu->getTask($taskId));
  if (!$task or !$list = nni($task, 'list')) return err(t('That task does not exist or was created today. Cannot proceed.'));
  if ($list->id != CEF_WORK_LIST) return err(t('That task is not in CEF Work.'));
  if (is_object($cu->addAssignees($taskId, $assignTo))) return go('/cef', t('Done!'));
  return err(t('Operation failed. Try again later?'));
}
