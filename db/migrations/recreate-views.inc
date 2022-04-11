<?php

require_once __DIR__ . '/util.inc';

const E_BANK_ONLY = 0; // bank charges and transfers between bank accounts that have no effect on CG account balances
const E_PRIME = 1;  // the main pair in the transaction (payer and payee)
const E_AUX = 2;  // this is an auxiliary payment (e.g., roundup, fee, or rule-based donation) triggered by another transaction
                  // The precise nature can be determined from uid2. Can be related to prime, bank, or outer.
const E_REBATE = 3; // discount rebate to customer
const E_USD_FEE = 4; // fee from customer to merchant for buying (or selling?) Common Good credit
const E_BANK = 5; // CG account transfers to or from the member's bank (replaces E_PRIME)
const E_XFEE = 6; // transfer fee to merchant (UNUSED)
const E_OUTER = 7; // CG account transfers to or from a non-member (replaces E_PRIME)

const E_TYPES = 'bank_only prime aux rebate usd_fee bank rule outer'; // these constants are also used in migration files
const BANK_IDS = ['outer' => 255, 'bankIn' => 256, 'bankOut' => 257, 'bankCharges' => 192];
const TX_EXCLUDE_FLDS = 'deleted';
const TXE_EXCLUDE_FLDS = 'deleted';
  
/**
 * Create (or recreate) all VIEWS, starting from scratch. This makes it easier to add/change/delete fields.
 * @param object $m: phinx AbstractMigration object
 * @param int $migDt: migration date (yyyymmdd)
 */
function createViews($m, $migDt) {
  if (!$m->isMigratingUp()) $migDt -= 1; // when rolling back, don't do the latest changes
  
  cgpr('clearing views');
  $q = $m->query('SELECT table_name FROM information_schema.views');
  while ($table = $q->fetchColumn()) {
/**/ echo "view: $table\n";
    $m->doSql('  DROP VIEW IF EXISTS ' . $table);
  }
  
  $m->doSql('CREATE VIEW descendants AS
      WITH RECURSIVE descendants AS (
        SELECT iid AS base, industry AS baseIndustry, iid, industry FROM r_industries
        UNION ALL
        SELECT d.base AS base, d.baseIndustry AS baseIndustry, c.iid, c.industry 
        FROM r_industries c INNER JOIN descendants d ON d.iid = c.parent )
      SELECT base, baseIndustry, iid AS descendant, industry AS descendantIndustry FROM descendants');
  $m->doSql('CREATE VIEW ancestors AS
      WITH RECURSIVE ancestors AS (
        SELECT iid AS base, industry AS baseIndustry, iid, industry, parent FROM r_industries
        UNION ALL
        SELECT a.base AS base, a.baseIndustry AS baseIndustry, p.iid, p.industry, p.parent
        FROM r_industries p INNER JOIN ancestors a ON p.iid = a.parent )
      SELECT base, baseIndustry, iid AS ancestor, industry AS ancestorIndustry FROM ancestors');
      
  // get current list of columns
  $txFldList = array_map(function($f) {return $f->getName();}, $m->table('tx_hdrs_all')->getColumns());
  $tx_flds = array_diff($txFldList, ray(TX_EXCLUDE_FLDS));
  $txFlds = join(',', $tx_flds);
  $txeFldList = array_map(function($f) {return $f->getName();}, $m->table('tx_entries_all')->getColumns());
  $txe_flds =array_diff($txeFldList, ray(TXE_EXCLUDE_FLDS));
  $txeFlds = join(',', $txe_flds);
  
  extract(BANK_IDS);
  $isBank = "uid IN ($bankIn, $bankOut)";
  $isOuter = "uid=$outer";

  foreach (ray(E_TYPES) as $i => $k) ${$k . 'y'} = "entryType=$i"; // eg "auxy" means entryType is E_AUX
  $e2primish = '(e2.entryType=' . E_PRIME . ' OR e2.entryType=' . E_BANK . ' OR e2.entryType=' . E_OUTER . ')'; // MariaDB converts "CREATE VIEW ... IN ()" to "... = ()" (BUG!!)

  // views of not-deleted records
  cgpr('making non-deleted tx views');
  $m->doSql("CREATE VIEW tx_hdrs AS SELECT $txFlds FROM tx_hdrs_all WHERE deleted IS NULL");
  $m->doSql("CREATE VIEW tx_entries AS SELECT $txeFlds FROM tx_entries_all WHERE deleted IS NULL");
  $m->doSql('CREATE VIEW tx_disputes AS SELECT * FROM tx_disputes_all WHERE deleted IS NULL');

  // individual views for each type of transaction
  $TXES_FLDS = 'description AS for, uid AS uid, agentUid AS agt, cat AS cat'; // one of each of these for each entry in a pair (eg uid1 and uid2)
  $TXS_MAP = 'entryType:type, amount:amt, rule:rule, relType:relType, relatedId:rel, id:eid';

  if ($migDt < 20220408) {
    $TXES_FLDS = 'description AS for, uid AS uid, agentUid AS agt';
    $TXS_MAP = 'entryType:type, cat:cat, amount:amt, rule:rule, relType:relType, relatedId:rel, id:eid';
  }

  $txFlds0 = 't.' . join(', t.', $tx_flds);
  foreach (ray($TXS_MAP) as $fld => $as) {
    if ($migDt < 20210706 and $fld == 'cat') continue;
    if (in_array($fld, $txe_flds)) $txFlds0 .= ", e2.$fld AS $as";
  }

  $txeFlds0 = explode(', ', $TXES_FLDS);

  $txFlds1 = 'e1.' . join('1, e1.', $txeFlds0) . '1';
  $txFlds2 = 'e2.' . join('2, e2.', $txeFlds0) . '2';
  
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
  makeTxsView($z, 'txs_aux', "e.$auxy AND $e2primish AND e2.id<0", "e.$auxy AND $e2primish AND e2.id>0");
  makeTxsView($z, 'txs_rebate', "e.$rebatey AND e2.$primey AND e2.id<0", "e.$rebatey AND e2.$primey AND e2.id>0");
  makeTxsView($z, 'txs_usd_fee', "e.$usd_feey AND $e2primish AND e2.id<0", "e.$usd_feey AND $e2primish AND e2.id>0");
  makeTxsView($z, 'txs_proper');  // all transactions between CG accounts
  makeTxsView($z, 'txs_bank', "e.$banky AND e.$isBank AND NOT e2.$isBank", "e.$banky AND NOT e.$isBank AND e2.$isBank");
  makeTxsView($z, 'txs_outer', "e.$outery AND e.$isOuter AND NOT e2.$isOuter", "e.$outery AND NOT e.$isOuter AND e2.$isOuter");
  makeTxsView($z, 'txs'); // add all transactions that make a difference to CG member accounts, omit bank charges and transfers between CG's banks
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
  
  $m->doSql('CREATE VIEW tx_requests AS SELECT * FROM tx_requests_all WHERE deleted IS NULL');
  if ($migDt < 20210702) return;

  $m->doSql('CREATE VIEW txs2_bank AS SELECT * FROM txs2 WHERE pid IS NULL');
  $m->doSql('CREATE VIEW txs2_outer AS SELECT * FROM txs2 WHERE pid IS NOT NULL');
}
