<?php
namespace CG\Web;
use CG\Web as w;
use CG\Backend as be;
use CG\Util as u;
use CG as r;
use CG\DB as db;
use CG\Txs as x;


/**
 * List Transactions
 */
function formShowRecurring($form, &$sta, $args = '') {
  global $rUrl, $mya;

  extract(just('recId do', $args)); 
  if (isset($do) and $do == 'stop' and isset($recId)) {
    $result = handleStop($recId, $sta, $args);
    // if ($result) return $result;
  }

  $sql = 'SELECT id as recId, payer, payee, amount, period, created, ended FROM r_recurs WHERE payer = :payer';
  $results = db\q($sql, ['payer' => $mya->id]);
  $html = <<<EOF
  <table>
    <th>To</th>
    <th>Amount</th>
    <th>How often?</th>
    <th>Starting</th>
    <th>Next</th>
    <th>Ending</th>
    <th></th>
EOF;
  
  foreach ($results as $row) {
    extract((array)$row);
    $fPayee = r\acct($payee)->bestName;
    $fAmount = u\fmtAmt($amount, '$');
    $fPeriod = t(ray(OFTENS)[$period]);
    $fCreated = u\fmtDate($created, TRUE);
    $finished = (0 < $ended) and ($ended < time());
    $fNext = $finished ? '' : u\fmtDate(nextRecur($created, $period), TRUE);
    $fEnded = $ended <= 0 ? '' : u\fmtDate($ended, TRUE);
    $url0 = BASE_URL . '/history/show-recurring';
    $url = "$url0/recId=$recId";
    $button = $finished ? '' :
              "<a href=\"$url&do=stop\" title=\"Stop recurring transaction\"><img src=\"$rUrl/images/icons/close.png\" border=0 /></a>";

    $html .= <<<EOF
<tr>
  <td class="name">$fPayee</td>
  <td class="amount">$fAmount</td>
  <td>$fPeriod</td>
  <td class="date">$fCreated</td>
  <td class="date">$fNext</td>
  <td class="date">$fEnded</td>
  <td class="buttons">$button</td>
</tr>
EOF;
  }
  $html .= "</table>\n";

  $title = t('Transaction History for ') . $mya->agentId;
  $transactions = item('<div>' . $html . '</div>');
  $title = t('Recurring Transactions for ') . $mya->bestName;
  $form = compact(ray('title transactions'));
  return cgform($form);
}


function nextRecur($starting, $period) {
  if (array_key_exists($period, OFTENS_SECS)) {
    $perSecs = OFTENS_SECS[$period];
    $nPer = ceil((r\rTime() - $starting) / $perSecs);
    return round($starting + ($nPer * $perSecs));
  } else {
    return FALSE;
  }
}

function formShowRecurring_validate($form, &$sta) {}

function formShowRecurring_submit($form, &$sta) {}

function handleStop($recId, &$sta, $args) {
  global $mya;
  
  $url = BASE_URL . '/history/show-recurring';
  $sql = 'SELECT id as recId, payer, amount, ended FROM r_recurs WHERE id = :recId';
  $results = db\q($sql, ['recId' => $recId]);
  if ($results->rowCount() != 1) return w\say('invalid recur id', 'err');

  $row = $results->fetch(\PDO::FETCH_ASSOC);
  extract(just('recId payer ended', $row));
  if (! is_null($ended) and $ended != 0) return w\say('recur already ended', 'err');
  if ($payer != $mya->id) return w\say('recur not yours', 'err');

  db\update('r_recurs', ['id' => $recId, 'ended' => r\rTime()], 'id');
  return w\say('recur stopped');
}

