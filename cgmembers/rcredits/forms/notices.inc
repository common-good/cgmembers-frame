<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * List the months for which notices are available for the current account, with links.
 */
function formNotices($form, $sta, $month = '') {
  return $month ? notice($month) : monthChoices(t('Notices'), 'history/notices');
}

/**
 * List notices for a given month.
 * @param string $month: mmmyyyy
 */
function notice($month = '') {
  if (!$start = strtotime("1$month")) return w\softErr(t('That is not a recognizable month.' . $month));
  $start = u\monthDay1($start);
  $end = strtotime('+1 month', $start) - 1;

  global $mya;
  if ($noticeArgs = r\noticeArgs($mya->id, "created BETWEEN $start AND $end", FALSE, TRUE)) {
    extract(just('messages', $noticeArgs));
  } else $messages = '';
  $title = t('Notices for ') . fmtDt($start, 'MMM yyyy');
  $list = item("<table>$messages</table>");
  return cgform(compact('title', 'list'), 'wide nolabs');
}
