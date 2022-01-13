<?php
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

include DRUPAL_ROOT . '/rcredits/admin/admin-achify.inc';

/**
 * Create an ACH Batch file for all relevant transfer requests.
 * @params string $args: URL argument list including:
 *   date: 0 for pending transfers (the default), unixtime for a redo
 *   way: in or out (defaults to neither, meaning both)
 *   mark: 1 means set deposit date to NOW, 0 (or omitted) means don't change anything
 *   bank: BSL or Citizens
 */
function achBatch($args) {
  extract(just('date way mark bank', $args, ''));

  u\setDft($date, 0);
  if ($way) $way = ' AND amount' . ($way == 'in' ? '>=0' : '<=0');
  w\achify("deposit=$date AND created<=:NOW" . $way, $mark, $bank); // <=:NOW lets us verify with $0 transfers before a scheduled real one

  if (test()) return;
}
