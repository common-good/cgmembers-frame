<?php
namespace CG\Web;
use CG\Web as w;
use CG\Backend as be;
use CG\Util as u;
use CG as r;
use CG\DB as db;
use CG\Txs as x;

include_once __DIR__ . '/../cg-txs.inc';

/**
 * List Invoices
 * @param string $ptf: pending, to, from
 */
function formInvoices($form, &$sta, $ptf, $args = '') {
  $toMe = ($ptf == 'to');
  w\downloadCheck('history/invoices-' . $ptf);

  extract(just('period starting ending download downloaded states nvid do', $args, NULL));

  $choices = array(t('open'), t('paid'), t('denied'));
  extract(just('dateRange url starting ending states', dateRange($args, "history/invoices-$ptf", $choices, '0,1,2')));
  
  $list = item(pendingList($toMe, $starting, $ending, $states, $url, $download)); // must precede starting & ending

  $title = $toMe ? t('Pending Payments TO You') : t('Pending Payments FROM You');
  $subtext = t('Click on a row to edit or delete a pending payment (paid requests are listed last).');
  $ptf = hidFld($ptf);

  //simple advanced
  $form = compact('title', 'subtext') + $dateRange + compact(ray('list ptf'));
  if ($downloaded) js('download', compact('url'));
  
  w\jsx('invoices', ray('args', "&ret=" . u\urlify($url)));
  return cgform($form);
}

function formInvoices_validate($form, &$sta) {w\dateRangeValidate($sta, FALSE);}

function formInvoices_submit($form, &$sta) {
  dateRangeRedirect('history/invoices-' . $sta['input']['ptf'], $sta);
}

/**
 * Prepare the list of pending payments
 * @return string table-based HTML listing the relevant requests
 */
function pendingList($toMe, $starting, $ending, $states, $url, $download) {
  global $mya;
  $headHelp = t('Invoice number,Date created,Who is to be paid,What is the payment for,Total amount,Click a row to change the status of the request');
  $classes = ray('nvid date name purpose amount buttons');
  $headers = t('Inv#,Date,Name,Purpose,Amount,Status');
  $headHelp = array_combine($classes, ray($headHelp));

  $header = array_combine($classes, ray($headers)); // make splices and additions easy and clear
  $downloadHeader = $header;
  foreach ($header as $key => $value) $header[$key] = w\popHelp($value, $headHelp[$key]);

  $maxRows = $download ? PHP_INT_MAX : TX_MAX_ROWS; // don't overwhelm the screen
  if ($download) {
    $way = $toMe ? t('To') : t('From');
    $csv = u\startDown(t('Pending') . "$way$mya->mainQid" . fmtDtRange($starting, $ending));
    u\csv($downloadHeader);
  } else $csv = FALSE;

  $rows = [];
  list ($me, $yu) = u\order($toMe, 'payee', 'payer');
  $where = ":uid=i.$me AND i.created BETWEEN :starting AND :ending";

  $sql = <<<EOF
    SELECT DISTINCT nvid, i.created, u.fullName AS other, i.purpose, i.amount, i.status AS status0, i.data, t.xid
    FROM tx_requests i 
    LEFT JOIN users u ON u.uid=i.$yu
    LEFT JOIN tx_hdrs t ON t.xid=i.status
    LEFT JOIN tx_entries e ON e.xid=i.status AND e.uid=u.uid
    WHERE ($where)
    ORDER BY (i.status>2), nvid DESC
EOF;
//    ORDER BY (i.status=:TX_PENDING OR i.status=:TX_APPROVED) DESC, nvid DESC

  global $mya;
  $q = db\q($sql, ray('uid starting ending', $mya->id, $starting, $ending));
///   debug(compact('sql','starting','ending', 'mya'));
  while (count($rows) < $maxRows and $tx = $q->fetchAssoc()) {
    extract($tx);
    if (!$download) $amount = number_format($amount, 2);
    $row = array($nvid, fmtDt($created, $download ? 'YYYY-MM-dd' : DATE_FMT), $other, $purpose, $amount);
    
    $do = "$url&nvid=$nvid&do";

    if ($status0 > 1) {
//      $button1 = $button2 = $button3 = '';
      $status = t('paid') . " (Tx#$xid)";
    } else {
      $statuses = ray(TX_STATUSES);
      $data = unserialize($data);
      $status = $status1 = ucfirst($statuses[$status0]);
      if ($status0 == TX_DENIED) $status = tags('a', w\popHelp($status, t('Reason: ') . nni($data, 'whyNot')));
    }

//    $buttons = "<div class='txs-buttons'><div>$button1</div><div>$button2</div><div>$status</div></div>";
    $buttons = "<div class='txs-buttons'>$status</div>";
    
    $row[] = $download
    ? ($status0 == TX_DENIED ? $status1 . tr(' (%whyNot)', 'whyNot', nni($data, 'whyNot')) : $status)
    : $buttons;
    $rows[] = $row;
  }
  if ($tx and $q->fetchAssoc()) say('transaction list truncated'); // there was more than we could show

  if ($download) {
    foreach ($rows as $row) u\csv($row);
    if (test()) return; else exit();
  }

  // prepare screen display
  $classes = u\prefixValues('tx-', $classes);
  $attributes = array('id' => 'txlist');
  $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = t('no invoices');
  return theme_table(compact(ray('header rows attributes caption colgroups sticky empty')));
}
