<?php
/**
 * @file
 * Database functions.
 */

namespace CG\DB; // typically abbreviated as "db"
use CG as r;
use CG\DB as db;
use CG\Util as u;

require_once __DIR__ . '/cg-util.inc';

define('NO_LOGSQL_TABLES', 'r_do queue sessions r_invites r_near cache_form r_stats'); // don't log deletions from these files
define('CGID', getV('cgf_uid')); // Common Good's account ID (has to be here rather than in defs.inc)

/**
 * Do a database query with easy substitutions.
 * @param string $sql: the raw query with substitution markers preceded by ':'
 *   These can be keys supplied in $subs OR constants (all uppercase and underscores)
 * @param assoc $subs: array of substitutions to make in $sql
 *   Each key, with a colon tacked on the front, is replaced (in $sql) with its value
 * @param bool $retId: <return an inserted record ID rather than a recordset>
 * @param bool $log: <log the operation>
 * @return: if inserting, then the id of the inserted record, else the query object (FALSE if failure)
 */
function q($sql, $subs = [], $retId = FALSE, $log = FALSE) {
  global $channel;
  //  \db_query("SET NAMES 'utf8'"); // only needed once per connection, but hey
  u\EXPECT(compact(ray('sql subs')), 'notempty assoc');
  if ($retId) $return = \Database::RETURN_INSERT_ID;
  if (preg_match('/^DELETE FROM ([^ ]+)( |$)/', $sql, $match) and !in(NO_LOGSQL_TABLES, @$match)) logSql(compact(ray('sql subs retId')));
  if ($log and mb_strpos($sql, 'SELECT ') === FALSE) u\loga('db', $subs + compact('sql'));
  try {
    return \db_query(u\SUBS($sql), u\prefixKeys(':', $subs), @compact('return'));
  } catch (\Exception $e) {
/**/ if (@$channel == TX_WEB) r\Web\exception_handler($e); else die('in q');
  }
}

/**
 * Simplified interactive admin version.
 * Call this in a "try" block, if you want to catch errors.
 */
function adminQ($sql) {
  $word1 = mb_strtolower(mb_strstr($sql, ' ', TRUE));
  u\EXPECT(in($word1, 'select show'), 'bad admin query');
  $sql = str_replace(';', '', $sql); // don't allow concatenation of queries
  return \db_query(u\SUBS($sql));
}

/**
 * Look the field up in the table, returning the first record matching the criteria.
 * @param string $fieldName: field name, function, or list of field names (* for all)
 *   A list of field names cannot contain a function or a left paren
 * @param string $table: table to look in
 * @param mixed $where: criteria (defaults to ALL) -- can be an assoc of required field values
 * @param assoc $subs: optional substitutions -- keys will be prefixed by ':'
 * @return: the value of what was looked up -- an assoc of values if a list of fields was requested
 *   FALSE if none
 * @see also: db\rows()
 */
function get($fieldName, $table, $where = '1', $subs = []) {
  u\EXPECT(compact(ray('fieldName table where subs')), 'notempty notempty notempty assoc');
  
  if (is_array($where)) list ($where, $subs) = [u\where($where), $where];
  if (!mb_strpos($where, ' LIMIT ')) $where .= ' LIMIT 1';
  $sql = "SELECT $fieldName FROM $table WHERE $where";
///   debug(compact('sql','subs'));
  $result = db\q($sql, $subs);

  list ($cnt, $f) = [1, $fieldName];
  while ($cnt) $f = preg_replace('/\\([^\\(\\)]+\\)/', '', $f, -1, $cnt); // ignore commas in functions
  $fieldCount = substr_count($f, ',') + 1;

//  $fieldCount = strpos($fieldName, '(') ? 1 : substr_count($fieldName, ',') + 1; // function or array, not both
  $many = (substr($fieldName, -1, 1) == '*');
  $several = (!$many and $fieldCount > 1);
  $result = ($several or $many) ? $result->fetchAssoc() : $result->fetchField();
  if ($several and $result) u\EXPECT(\count($result) == $fieldCount, 'field count is off: result=' . pr($result) . ' count=' . $fieldCount); // eg, if $fieldName == 'a,a' (\count because this class has a count function!)
  return $result;
}
function exists($table, $where = '1', $subs = []) {return db\get(1, $table, $where, $subs);}

/**
 * Update a database record 
 * (replaces the functionality of drupal_write_record, which does not update bigint fields properly)
 * @param string $table: table name
 * @param assoc $info: data to update (must include record key)
 * @param string $key: record key name (must not be empty)
 * @return: TRUE (FALSE on failure)
 */
function update($table, $info, $key) {
  logSql(compact(ray('table info key')));

  if (!isset($info['modified']) and hasField($table, 'modified')) {
    $info['modified'] = now();
  }
	
  $sql = '';
  foreach ($info as $field => $v) {
    if ($field != $key) $sql .= ",`$field`=:$field";
    if (is_array($v)) u\FAIL('array in update param'); // exception fails if called in $DBTX
  }
  $sql = "UPDATE $table SET " . substr($sql, 1) . " WHERE `$key`=:$key";
  try {
    $result = db\q($sql, $info, !$key);
    return TRUE;
  } catch (PDOException $e) {u\FAIL("PDOException: $e->message");} 
}

/**
 * Insert a database record (just one)
 * (replaces half the functionality of drupal_write_record)
 * @param string $table: table name
 * @param assoc $info: data to insert
 * @param string $key: key field name, defaults to 'id'
 * @return: record key of the inserted record (FALSE on failure)
 */
function insert($table, $info, $key = 'id') {
  logSql(compact(ray('table info')));

  foreach (ray('start created modified') as $k) if (!isset($info[$k]) and hasField($table, $k)) {
    $info[$k] = now(); // set these automatically
  }
	
  $sql = '';
  foreach ($info as $field => $v) {
    /* if ($field == $key) u\FAIL('key value specified in database insert'); // may actually be needed */
    if (is_array($v)) u\FAIL("array in insert param for field $field"); // exception fails if called in $DBTX
    $sql .= ",`$field`=:$field";
  }
  $sql = "INSERT INTO $table SET " . substr($sql, 1);  // get rid of leading comma
  try {
    $result = db\q($sql, $info, TRUE);
    if ($result) return $result; // if there is no serial field, result will be false, so check

    u\EXPECT($key and isset($info[$key]), 'no primary key ' . $key);
    return db\get($key, $table, "`$key`=:key", ray('key', $info[$key])); // return the supplied id if success

  } catch (PDOException $e) {u\FAIL("PDOException: $e->message");} 
}

/**
 * Insert or update a database record 
 * @param string $table: table name
 * @param assoc $info: data to update (must include record key)
 * @param string $key: record key name (must not be empty)
 * @return: if inserting, then the id of the inserted record, else TRUE (FALSE on failure)
 */
function updateOrInsert($table, $info, $key) {
  if (@$info[$key] and db\exists($table, "$key=:$key", [$key => $info[$key]])) {
    return db\update($table, $info, $key);
  } else {
    return db\insert($table, $info, $key);
  }
}

/**
 * Return an array of record objects.
 * @param string $fields: comma-separated list of fields (actual or calculated) -- use * for all
 */
function rows($fields, $table, $where = '1', $subs = [], $labeled = TRUE) {
  if (is_array($where)) list ($where, $subs) = [u\where($where), $where];
  $res = db\q("SELECT $fields FROM $table WHERE $where", $subs)->fetchAll(); // fetchAll means fetchAllObject
  if (!$labeled) foreach ($res as $i => $one) $res[$i] = array_values((array) $one);
  return $res;
}

/**
 * Return an array of matching data.
 */
function col($field, $table, $where = '1', $subs = []) {
  if (is_array($where)) list ($where, $subs) = [u\where($where), $where];
  return db\q("SELECT $field FROM $table WHERE $where", $subs)->fetchCol();
}
function lst($field, $table, $where = '1', $subs = [], $joint = ', ') { // lst because "list" is a reserved word
  return join($joint, db\col($field, $table, $where, $subs));
}
function keyed($fields, $table, $where = '1', $subs = []) {
  if (is_array($where)) list ($where, $subs) = [u\where($where), $where];
  return db\q("SELECT $fields FROM $table WHERE $where", $subs)->fetchAllKeyed();
}

/**
 * Log the given update, insert, or delete query
 */
function logSql($args) {
  global $mya;
  $args = date('d') . ' ' . @$mya->mainQid . ': ' . u\jsonEncode($args);
  $args = strtr($args, ['\\"' => '', '"' => '', '\\n' => '|', ';}' => '}', ';' => '; ', ',' => ', ']);
  if (!is_dir($logdir = LOG_DIR)) mkdir($logdir);
  file_put_contents(SQL_LOG, "$args\n", FILE_APPEND); // new log file every month
  return SQL_LOG;
}  

/**
 * Set new values in the data field of a record.
 * @param string $table: what record the table is in
 * @param string $id: the record ID
 * @param assoc $values: values to add to the data field in that record
 */
function setData($table, $id, $values) {
  $keys = ray('users r_invoices tx_hdrs', 'uid', 'nvid', 'xid');
  $key = $keys[$table];
  u\EXPECT($key, 'bad table');
  $data = db\get('data', $table, "$key=:id", compact('id'));
  $data = $values + ($data ? unserialize($data) : []);
  if ($table == 'users') {
    return r\acct($id)->update(compact('data'));
  } else {
    $data = serialize($data);
    return db\update($table, ray("data $key", $data, $id), $key);
  }
}

/**
 * Set (or unset) a bit in the flags field of a record.
 * @param string $table: what record the table is in
 * @param string $id: the record ID
 * @param int $bit: which bit to change
 * @param bool $bool: TRUE or FALSE (how to set the bit -- defaults to TRUE)
 */
function setFlag($table, $id, $bit, $bool = TRUE) {
  if ($table == 'users') return r\acct($id)->setBit($bit, $bool);

  $keys = ray('users_txs r_invoices tx_hdrs', 'uid', 'nvid', 'xid');
  $key = $keys[$table];
  u\EXPECT($key, 'bad table');
  $flags = db\get('flags', $table, "$key=:id", compact('id'));
  u\setBit($flags, $bit, $bool);
  return db\update($table, ray("flags $key", $flags, $id), $key);
}

/**
 * Make a backup copy of the record before deleting it if appropriate (then delete it).
 * Call with:
 *   db\del(tnm, ray)
 *   db\del(tnm, where)
 *   db\del(tnm, where, subs)
 *   db\del(tnm, where, subs, override)
 *   db\del(tnm, idnm, id)
 * @param string $tnm: delete a record from this table
 * @param assoc $ray: assoc of criteria; for example:
 *   foreach (ray('tx_entries tx_hdrs r_usd') as $tnm) db\del($tnm, ['xid' => 12345]);
 * @param string $where: criteria string
 * @param assoc $subs: substitutions in $where
 * @param bool $override: allow deletions from tx_hdrs_all and tx_entries_all
 * @param string $idnm: name of key field of table
 * @param mixed $id: id of record to delete (not empty)
 * @param assoc $subs: parameters for selection criteria
 */
//function del($tnm, $idnm, $id = '', $subs = []) {
function del($tnm, $where, $subs = [], $override = FALSE) {
  if (strhas($tnm, 'tx_hdrs')
    or strhas($tnm, 'tx_entries')
    or strhas($tnm, 'txs_')
  ) u\EXPECT($override and strhas($tnm, '_all'), t('you must delete transactions only from txs or r_usd, which delete all things with that xid'));
  
  if (is_array($where)) {
    u\EXPECT(empty($subs), t('mixed parameter sets in db\\del()'));
    list ($where, $subs) = [u\where($where), $where];
  } elseif (!is_array($subs)) { list ($where, $subs) = ["$where=:id", ['id' => $subs]]; } // del(tnm, idnm, id)
  
  $DBTX = \db_transaction();
  
  if (in($tnm, DELETED_BY_VIEW)) {
    $q = db\q("UPDATE $tnm SET deleted=:NOW WHERE (deleted IS NULL OR deleted=0) AND $where", $subs);
    $ok = ($q->rowCount() > 0);
    return;
    
  } elseif (in($tnm, TRACK_DELETED)) {
    $xtnm = $tnm == 'users' ? 'x_users' : str_replace('r_', 'x_', $tnm);
    $q = db\q("INSERT INTO $xtnm SELECT :NOW AS deleted,a.* FROM $tnm a WHERE $where", $subs);
    $ok = ($q->rowCount() > 0);
    if ($ok and $tnm == 'r_usd') { // need to delete related txs records as well as r_usd records
      if ($rows = db\col('xid', $tnm, $where, $subs)) db\del('txs', 'xid IN (:rows)', compact('rows'));
    }
    
  } elseif ($tnm == 'txs') {
    if ($rows = db\col('xid', $tnm, $where, $subs)) foreach (ray('tx_entries_all tx_hdrs_all') as $allTnm) {
      db\del($allTnm, 'xid IN (:rows)', compact('rows'), (bool) 'OVERRIDE');
    }
    return; // txs IS tx_hdrs_all and tx_entries_all, so no further action needed
    
  } else { $ok = TRUE; }
  
  if ($ok) db\q("DELETE FROM $tnm WHERE $where", $subs);
  unset($DBTX);
}

function hasField($table, $field) {return hasCol($table, $field);}
function hasCol($table, $field) {global $db_name; return db\exists('INFORMATION_SCHEMA.COLUMNS', 'TABLE_SCHEMA=:db_name AND TABLE_NAME=:table AND COLUMN_NAME=:field', compact(ray('db_name table field')));}
function cols($table, $except = '') {global $db_name; return db\col('COLUMN_NAME', 'INFORMATION_SCHEMA.COLUMNS', 'TABLE_SCHEMA=:db_name AND TABLE_NAME=:table AND COLUMN_NAME<>:except', compact(ray('table db_name except')));}
        
function count($table, $where = '1', $subs = []) {return db\get('COUNT(1)', $table, $where, $subs);}
function max($field, $table, $where = '1', $subs = []) {return db\get("MAX($field)", $table, $where, $subs);}
function min($field, $table, $where = '1', $subs = []) {return db\get("MIN($field)", $table, $where, $subs);}
function sum($field, $table, $where = '1', $subs = []) {return db\get("SUM($field)", $table, $where, $subs);}
function avg($field, $table, $where = '1', $subs = []) {return db\get("AVG($field)", $table, $where, $subs);}
function med($field, $table, $where = '1', $subs = []) {return db\get("MEDIAN($field) OVER ()", $table, $where, $subs);}
