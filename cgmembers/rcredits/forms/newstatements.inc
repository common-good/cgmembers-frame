<?php
namespace CG\Web;
use CG\Web as w;

/**
 * List the available account statement dates, with links.
 */
function formNewStatements($form, $sta, $month = '') {
  if ($month) return w\go("history/new-statement/$month");
  
  $mo = date('F');
  $subtext = date('d') < 10 ? t('note|Statements are not final until the 10th of the next month.', compact('mo')) : NULL;
  return cgform(monthChoices(t('New Statements'), 'new-statements', $subtext));
}
