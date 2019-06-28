<?php

require_once __DIR__ . '/util.inc';

define('E_TYPES', 'bank_only prime donation rebate usd_fee bank'); // these constants are also used in migration files
define('BANK_IDS', ['bankIn' => 256, 'bankOut' => 257, 'bankCharges' => 192]);
define('TX_FLDS', 'xid actorId actorAgentId flags channel boxId goods risk risks recursId reversesXid created');
define('TXE_FLDS', 'id entryType amount description uid agentUid acctTid relType relatedId xid');
define('TXES_FLDS', 'description AS for, uid AS uid, agentUid AS agt, acctTid AS tid, relType AS relType, relatedId AS rel');
  
/**
 * Create (or recreate) all VIEWS, starting from scratch. This makes it easier to add/change/delete fields.
 * @param object $o: phinx AbstractMigration object
 * @param int $stopAt: migration date to stop at (0 = none, used for rolling back a migration)
 */
function createViews($m, $stopAt = 0) {
  
  function allSoFar($w) {return '(' . join(') OR (', $w) . ')';}
  
  function makeTxsView(&$z, $_view, $_where1 = '', $_where2 = '') {
    pr("making $_view");
    $m = $z['m'];
    foreach ([1, 2] as $i) {
      if (${"_where$i"}) $z["w$i"][] = ${"_where$i"}; else ${"_where$i"} = allSoFar($z["w$i"]);
    }
    $subs = compact(ray('_view _where1 _where2'));
    $m->doSql(strtr($z['sql'], $subs));
  }
  
  pr('clearing views');
  $q = $m->query('SELECT table_name FROM information_schema.views');
  while ($table = $q->fetchColumn()) $m->doSql('  DROP VIEW IF EXISTS ' . $table);

  // set up constants
  $txFlds = join(',', ray(TX_FLDS));
  $txeFlds = join(',', ray(TXE_FLDS));
  
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

  $txFlds0 = 't.' . join(', t.', ray(TX_FLDS));
  $txFlds1 = 'e1.' . join('1, e1.', explode(', ', TXES_FLDS)) . '1';
  $txFlds2 = 'e2.' . join('2, e2.', explode(', ', TXES_FLDS)) . '2';
  
  $select0 = 'SELECT e.* FROM tx_entries_all e JOIN tx_entries_all e2 USING(xid)'; // e2 mostly helps identify related prime records)
  $sql = <<< X
    CREATE VIEW _view AS 
    SELECT DISTINCT $txFlds0, e1.entryType AS type, e2.amount AS amt, $txFlds2, $txFlds1, e2.id AS eid
    FROM tx_hdrs t
    JOIN ($select0 WHERE _where1) e1 USING (xid)
    JOIN ($select0 WHERE _where2) e2 USING (xid)
    WHERE e1.id=-e2.id AND e2.id>0
X;

/*
    CREATE VIEW _view AS 
    SELECT DISTINCT $txFlds0, e1.entryType AS type, e2.amount AS amt, $txFlds2, $txFlds1
    FROM tx_hdrs_all t
    JOIN ($select0 WHERE _where1) e1 USING (xid)
    JOIN ($select0 WHERE _where2) e2 USING (xid)
    WHERE e1.id=-e2.id AND e2.id>0 AND t.deleted IS NULL
    */

  $w1 = $w2 = [];
  $z = compact(ray('m sql w1 w2'));

  makeTxsView($z, 'txs_prime', "e.$primey AND e.id<0", "e.$primey AND e.id>0");
  makeTxsView($z, 'txs_donation', "e.$donationy AND e2.$primey AND e2.id<0", "e.$donationy AND e2.$primey AND e2.id>0");
  makeTxsView($z, 'txs_rebate', "e.$rebatey AND e2.$primey AND e2.id<0", "e.$rebatey AND e2.$primey AND e2.id>0");
  makeTxsView($z, 'txs_usd_fee', "e.$usd_feey AND e2.$primey AND e2.id<0", "e.$usd_feey AND e2.$primey AND e2.id>0");
  makeTxsView($z, 'txs_proper');  // all transactions between CG accounts
  makeTxsView($z, 'txs_bank', "e.$banky AND e.$isBank AND NOT e2.$isBank", "e.$banky AND NOT e.$isBank AND e2.$isBank");
  makeTxsView($z, 'txs'); // add bank transfers to and from CG accounts, omit bank charges and transfers betwen CG's banks
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
