<?php
namespace CG\Web;
use CG\Web as w;
use CG\Backend as be;
use CG\Util as u;
use CG as r;
use CG\DB as db;
use CG\Txs as x;

/**
 * Show graphs to clarify a community's (or the region's) finances
 */
function formGraphs($form, &$sta, $args) {
  extract(just('ctty download chart', $args));
  $a = w\eLinkAcct($sta, $args, 'any'); // may be called with ctty=<communityUid>
  u\setDft($ctty, $a ? $a->community : 0);

  if (@$download) { // download a spreadsheet with the chart data
    $dt = strftime('%Y%m%d');
    u\setDft($chart, 'success');
    $cttyName = $ctty ? r\acct($ctty)->name : t('region');
    u\csvStart("$chart-$cttyName-$dt.csv");
    $header[] = ''; // column for data descriptor
    for ($ago = DATA_POINTS; $ago >= 0; $ago--) {
      $dt = strtotime("-$ago months", today());
      $header[] = strftime('%d%h%Y', $dt);
    }
    u\csv($header);
    $chartData = w\showCharts($ctty, 'months', 'download');
    if ($chartData) for ($i = 0; $i < count($chartData[0]); $i++) {
      for ($m = 0; $m < count($chartData); $m++) $row[$m] = $chartData[$m][$i];
      u\csv($row);
    }
    exit();
  } else { // normal (show a chart)
    $title = t('Community Statistics');
    $subtext = t('This page helps us keep an eye on our economy so we can manage it effectively together. Choose a chart. <%b>Hover</b> over a graph for the numbers.', '_b', 'class="loud"');
    
    $charts = item(w\chartFrame('success', TRUE, $ctty));
    if (u\test()) $charts = item(w\showAllCharts($ctty));
    
    return cgform(compact(ray('title subtext charts')));
  }
}

function formGraphs_validate($form, &$sta) {
  formTxs_validate($form, $sta);
}

function formGraphs_submit($form, &$sta) {
  formTxs_submit($form, $sta, 'community');
}