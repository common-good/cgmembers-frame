<?php

require_once __DIR__ . '/util.inc';

const E_TYPES = 'bank_only prime donation rebate usd_fee bank'; // these constants are also used in migration files
const BANK_IDS = ['bankIn' => 256, 'bankOut' => 257, 'bankCharges' => 192];
const TX_EXCLUDE_FLDS = 'deleted';
const TXE_EXCLUDE_FLDS = 'deleted';
const TXES_FLDS = 'description AS for, uid AS uid, agentUid AS agt, acctTid AS tid';
const TXS_MAP = 'entryType:type, amount:amt, rule:rule, relType:relType, relatedId:rel, id:eid';
const VIEWS_NOT_TO_DROP = ['ancestors', 'descendants'];
  
/**
 * Create (or recreate) all VIEWS, starting from scratch. This makes it easier to add/change/delete fields.
 * @param object $o: phinx AbstractMigration object
 * @param int $stopAt: migration date to stop at (0 = none, used for rolling back a migration)
 */
function createViews($m, $stopAt = 0) {
  pr('clearing views');
  $q = $m->query('SELECT table_name FROM information_schema.views');
  while ($table = $q->fetchColumn()) {
    echo "view: $table\n";
//    print_r(VIEWS_NOT_TO_DROP); echo "\n";
//    echo in_array($table, VIEWS_NOT_TO_DROP); echo "\n";
    if (! in_array($table, VIEWS_NOT_TO_DROP)) { // don't drop
      $m->doSql('  DROP VIEW IF EXISTS ' . $table);
    }
  }
  
  // get current list of columns
  $txFldList = array_map(function($f) { return $f->getName(); },
                         $m->table('tx_hdrs_all')->getColumns());
  $tx_flds = array_diff($txFldList, ray(TX_EXCLUDE_FLDS));
  $txFlds = join(',', $tx_flds);
  $txeFldList = array_map(function($f) { return $f->getName(); },
                         $m->table('tx_entries_all')->getColumns());
  $txe_flds =array_diff($txeFldList, ray(TXE_EXCLUDE_FLDS));
  $txeFlds = join(',', $txe_flds);
  
  extract(BANK_IDS);
  $isBank = "uid IN ($bankIn, $bankOut)";

  foreach (ray(E_TYPES) as $i => $k) ${$k . 'y'} = "entryType=$i"; // eg "donationy" means entryType is E_DONATION

  // views of not-deleted records
  pr('making non-deleted tx views');
  $m->doSql("CREATE VIEW tx_hdrs AS SELECT $txFlds FROM tx_hdrs_all WHERE deleted IS NULL");
  $m->doSql("CREATE VIEW tx_entries AS SELECT $txeFlds FROM tx_entries_all WHERE deleted IS NULL");
  $m->doSql('CREATE VIEW tx_disputes AS SELECT * FROM tx_disputes_all WHERE deleted IS NULL');

  if ($stopAt == 20190622) return;

  // individual views for each type of transaction

  $txFlds0 = 't.' . join(', t.', $tx_flds);
  foreach (ray(TXS_MAP) as $fld => $as) {
    if (in_array($fld, $txe_flds)) $txFlds0 .= ", e2.$fld AS $as";
  }

  $txFlds1 = 'e1.' . join('1, e1.', explode(', ', TXES_FLDS)) . '1';
  $txFlds2 = 'e2.' . join('2, e2.', explode(', ', TXES_FLDS)) . '2';
  
  $select0 = 'SELECT e.* FROM tx_entries_all e JOIN tx_entries_all e2 USING(xid)'; // e2 mostly helps identify related prime records)
  $sql = <<< X
    CREATE VIEW _view AS 
    SELECT DISTINCT $txFlds0, $txFlds2, $txFlds1
    FROM tx_hdrs t
    JOIN ($select0 WHERE _where1) e1 USING (xid)
    JOIN ($select0 WHERE _where2) e2 USING (xid)
    WHERE e1.id=-e2.id AND e2.id>0
X;

  $w1 = $w2 = [];
  $z = compact(ray('m sql w1 w2'));

  makeTxsView($z, 'txs_prime', "e.$primey AND e.id<0", "e.$primey AND e.id>0");
  makeTxsView($z, 'txs_donation', "e.$donationy AND e2.$primey AND e2.id<0", "e.$donationy AND e2.$primey AND e2.id>0");
  makeTxsView($z, 'txs_rebate', "e.$rebatey AND e2.$primey AND e2.id<0", "e.$rebatey AND e2.$primey AND e2.id>0");
  makeTxsView($z, 'txs_usd_fee', "e.$usd_feey AND e2.$primey AND e2.id<0", "e.$usd_feey AND e2.$primey AND e2.id>0");
  makeTxsView($z, 'txs_proper');  // all transactions between CG accounts
  makeTxsView($z, 'txs_bank', "e.$banky AND e.$isBank AND NOT e2.$isBank", "e.$banky AND NOT e.$isBank AND e2.$isBank");
  makeTxsView($z, 'txs'); // add bank transfers to and from CG accounts, omit bank charges and transfers between CG's banks
  makeTxsView($z, 'txs_bank_only', "e.$bank_onlyy AND IF(e.$isBank AND e2.$isBank, e.amount<0, e.$isBank)",
                                   "e.$bank_onlyy AND IF(e.$isBank AND e2.$isBank, e.amount>0, e2.$isBank)");
  makeTxsView($z, 'txs_all'); 

  $delMap = ['SELECT DISTINCT ' => 'SELECT DISTINCT t.deleted,', 't.deleted IS NULL' => 't.deleted IS NOT NULL'];
//  $z['sql'] = strtr($sql, $delMap);
//  makeTxsView($z, 'txs_deleted'); 
//  $z['sql'] = str_replace('AND t.deleted IS NOT NULL', '', $z['sql']);
  makeTxsView($z, 'txs_ever'); // both deleted and not deleted records

  $m->doSql('CREATE VIEW txs_noreverse AS SELECT * FROM txs t WHERE reversesXid IS NULL AND NOT EXISTS(SELECT xid FROM tx_hdrs tr WHERE tr.reversesXid=t.xid)');
  
  $m->doSql('CREATE VIEW tx_entries_o AS SELECT * FROM tx_entries ORDER BY ABS(id), id'); // for debugging 
}
