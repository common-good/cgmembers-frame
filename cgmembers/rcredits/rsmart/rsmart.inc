<?php
namespace CG\Smart;
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Web as w;

/**
 * @file
 * Smart phone interface to the %PROJECT system.
 *
 * Coding conventions:
 *   $mya is the current account
 *   $a is some other account
 *
 * RPC API Definition:
 *
 * Basic request params (always sent):
 * op       the requested operation
 * device   the device ID (supplied once by the server) -- empty if none
 * agent    the already-scanned agent's ID (eg NEWAAB-A) or company ID (eg NEWAAB) -- empty if none yet
 * version  the version number of the running app
 *
 * REQUESTS:
 * (see https://docs.google.com/document/d/1fZE_wmEBRTBO9DAZcY4ewQTA70gqwWqeqkUnJSfWMV0/edit)
 * time:  [] => [ok=”1”, time, message]
 * identify:  [member, code] 
 *      => [ok, message, logon="1", name, descriptions[], can, device, default, company, time]
 *   OR  => [ok, message, logon, name, place, company, balance] where logon is 0 or -1
 * photo:  [member, code] => photo
 * charge:  [member, amount, goods, description, created, force] 
 *   => [ok, message, txid, created, balance, undo]
 * undo:  [txid, force] => [ok, message, balance]
 *
 * To create json for app demo, do this from PHP window on STAGE server:
  include_once R_ROOT . '/rsmart/rsmart.inc';
    $args = CG\Util\ray('op device agent version member code', 'identify', '3i2ifsapEjwev3CwBCV7', 'NEWAAB', RPOS_VERSION, 'NEWABB', 'ZzhWMCq0zcBowqw'); // cust
    $args = CG\Util\ray('op device agent version member code', 'identify', '3i2ifsapEjwev3CwBCV7', '', RPOS_VERSION, 'NEWAAB-A', 'WeHlioM5JZv1O9G'); // agent login
    $args = CG\Util\ray('op device agent version member amount goods description', 'charge', '3i2ifsapEjwev3CwBCV7', 'NEWAAB-A', RPOS_VERSION, 'NEWABB', AMOUNT, 1, 'zot');
    $args = CG\Util\ray('op device agent version txid', 'undo', '3i2ifsapEjwev3CwBCV7', 'NEWAAB-A', RPOS_VERSION, '4069');
///   echo CG\Smart\pos($args));
 */

define('RPOS_VERSION', 230); // lastest POS app version available for download (this should match the versionCode in the app's manifest. upload and test the app first (and make sure it is available in Google Play Store), before bumping the number up here.)
define('R_QR_HEADER', 'HTTP://RC2.ME/I/'); // for transient QR IDs
define('R_TRANSIENCE', 60); // number of seconds for device's transient QR to be valid

require_once __DIR__ . '/../cg-strings.inc';

$GLOBALS['tStrings'] += array(
  'old app' => t("IMPORTANT: Your app needs updating.\n\nMake sure your latest transactions are uploaded before you uninstall this old version. Then visit \"cgpay.commongood.earth\" to download the new app."),
  'bad proof' => t('That transaction is not valid.'),
  'bad member' => t('That is not a valid account. The customer used a lost, stolen, or fraudulent %PROJECT card.'),
//  'card deactivated' => t('The %PROJECT card has been de-activated.'),
  'bad agent' => t('That is not a valid %PROJECT company card.'),
  'agent cannot scan' => t('That agent is not authorized to charge customers.'),
  'scan in first' => t('You have to scan yourself in before charging a customer.'),
  'already scanned in' => t('You are already scanned in.'),
  'wrong company' => t('That %PROJECT card is for a different company.'),
  'company inactive' => t('That company is not an active participant.'),
//  'customer balance' => t("Customer: %name\n\nBalance: %balance%spendable\nTradable for cash: %cashable"),
  'customer balance' => "Customer: %name\n\nBalance: %balance\nCredit line: %rewards\n", // (parseable)
  'report exchange' => t("You %did %otherName %amount in exchange for cash."),
  'just did' => t("\n\n%we just %did %amount %forCash."), // "We" not "You" here, in case viewed by manager
  'unknown device' => t('This device is not recognized. Clear your data for the app (or uninstall and reinstall), then try again.'),
  'confirm undo app' => t('Undo transfer of %amount %tofrom %otherName? (Press OK for Yes, Cancel for No.)'), // no creation date, unlike web and SMS
  'member inactive' => t('That account is not yet activated.'),
  'bad icard' => t('The customer has not yet activated his or her %PROJECT Account.'),
  'expired icard' => t('That invitation card has expired. The customer can use their %PROJECT card instead.'),
  'bad pin' => t('That is not your correct PIN.'), // self-service error
  'no such test' => t('That test function does not exist.'),
);

/**
 * Receive and process a REST request from a smart device
 * @param assoc $args: an associative array (defaults $_POST) of a json-encoded assoc that normally includes:
 *   op: what to do (determines what function to call)
 * @return (normally by calling either be\error or doOp):
 *   ok: 1 if successful, otherwise 0 ($op = 'photo' is an exception; see below)
 *   message: an error message or help message to display on the device; or not set
 */
function pos($args = FALSE) {
///  debug("pos(" . pr($args) . ")");

  global $channel; $channel = TX_APP;
  global $scanned; $scanned = true;
  global $mya;

  if (!r\up()) return exitJust('system down'); // app does not (and should not) distinguish this from system failure
  if (is_object($args)) $args = (array) $args; // this helps with debugging when passing object args
  if (is_string($args)) $args = urlq2ray($args); // convert from query string to array
  if ($args) $_POST = $args; else $args = $_POST;
  if (test()) f('t.POST', $args);
  $args0 = $args;
//  $args = u\sanitizeInput($args0 = $args);
  extract(just('op signin device agent member version', $args, NULL));
  if ($op != 'time') u\loga('in', $args); // avoid filling the log with time requests
  $args['version'] = (int) $version;
  if (!$op or !in($op, 'time identify photo charge test')) return posErr('bad op');
  
  if (isDEV and $op == 'test') return smartTest($args0);

  if ($op == 'time') return getTime($args); // no agent or device required
  if (!$mya = getAgent($agent ?: $member, $agentErr)) return posErr($agentErr); // set "current" account for internals
  if ($err = badDevice($device, $signin, $version)) return $err; else $args['device'] = $device;
  if (!$mya->cttyUp) return posErr('your community down', ray('contact', $mya->cAdminContact()));

  $op = 'CG\\Smart\\' . $op;
  return $op($args);
}

/**
 * Return a (normally successful) json result to the remote caller.
 * @param assoc $args: named values to return
 * @param string $ok: "1" for success, "0" for failure
 * @return: does not return (when testing, return the json result)
 */
function ok($args, $ok = "1") {
  if ($msg = nni($args, 'message')) $args['message'] = strip_tags($msg);
  $result = u\jsonize(compact('ok') + $args);
  if (key($args) != 'time') u\loga('out', $args); // avoid filling the log with time requests
  //{t\output($args, 'output'); 
  if (test()) return $result;
  header('Access-Control-Allow-Origin: *'); // allow access from web browsers
  return exitJust($result);
}

/**
 * Return an error message to the remote caller.
 * @param string $message: what to say
 * @param assoc $subs: substitions in message (if any)
 * @param assoc $extra: extra parameters to return (if any)
 * @return: does not return (when testing, return the json result)
 */
function posErr($message, $subs = [], $extra = []) {
  $message = tr($message, $subs);
  return ok(compact('message') + $extra, "0");
}

/**
 * Return information from a test function.
 */
function smartTest($args) {
  global $testOnly;

  if ($err = missingArg($keys = 'function testOnly args', $args)) return posErr($err); 
  extract(just($keys, $args));
  $args = [u\dejsonize($args)];

  require_once __DIR__ . '/rsmart.steps';
  $func = "\\$function";
  if (function_exists($func)) {
    $res = call_user_func_array($func, $args);
    //$reflFunc = new \ReflectionFunction($func);
    //print "\n" . $reflFunc->getFileName() . ':' . $reflFunc->getStartLine();
  } else {
    require_once __DIR__ . '/../cg-testing.inc';
    $func = "\\CG\\Testing\\$function";
    if (function_exists($func)) $res = call_user_func_array($func, $args);
  } 
  
  if (!isset($res)) return posErr('no such test');

  return $res ? ok([], '1') : posErr('step failed', [], \drupal_get_messages());
}

/**
 * Get the current server time
 * time: [data] => [ok=”1”, time, message]
 * @param string $data: optional message string or json-encoded data (perhaps data we requested from the app)
 * @return json:
 *   time: the current unixtime
 *   message: something for the app to say or do (if it begins with "!")
 */
function getTime($args) {
  if ($err = missingArg($keys = 'version', $args)) return posErr($err); 
  extract(just($keys . ' data', $args, NULL));
  $time = now();
  
  if ($version < RPOS_VERSION) $message = $version < 230 ? tr('old app') : '!update';
  if ($data or !nn($message)) $message = handleAppMsg(@$data);

  return ok(compact(ray('time message')));
}
 
/**
 * Sign in (and set up the device, if necessary) OR identify the customer.
 * identify: [member, code, pin?, signin, counter?] 
 *   BEFORE version 2.30:
      => [ok, message, name, device, default, company, time, descriptions[], bad[], can, balance]
   OR => [ok, message, first, name, place, company, balance, rewards, time, descriptions[], bad[], can]
 *   NOW:
      => [ok, message, device, person, company, time, descriptions[], bad[], can, balance, creditLine]
   OR  => [ok, message, first, person, company, place, time, descriptions[], bad[], can, balance, creditLine]
 
 * @params assoc $args:
 *   device: a unique device ID
 *   agent: agent code of current agent -- empty if none yet
 *   member: account ID of customer or new agent
 *   code: rCard security code (possibly encrypted with itself as the key)
 *   signin: 1 if signing in
 * @return json:
 *   device: a unique device identifier (if none passed and signin is true), consisting of the device owner's
 *      full name encrypted with our private key, so customers paying with phone can know the business is legitimate
 *   person/name: the individual's name (if any)
 *   company: the member's (agent's or customer's) company name (if any)
 *   place: city and state of the company if customer is a company agent, otherwise of the individual
 *   descriptions: array of the agent's company's most common charge descriptions (most popular first)
 *   bad: array of invalid qids/cardCodes
 *   can: bit permissions -- what the agent can do
 *   default: the default agent (empty if none allowed)
 *   balance: the customer's current balance
 *   creditLine: the customer's current credit line
 */
function identify($args) {
  global $mya;
  $companyInfo = getCompanyInfo();
  if ($err = missingArg($keys = 'device member code version', $args)) return posErr($err, [], $companyInfo);
  extract(just($keys . ' agent pin signin', $args, NULL)); // no agent on scanin
  u\EXPECT($version > 217, 'archaic version of app');

  if ($agent) { // get other party
    if ($info = machine($member, $code)) return ok($info + $companyInfo); // purchase from fuel pump or other machine
    if (!$a = getMember($member, $code, $err, TRUE)) return posErr($err, [], $companyInfo);
  } else $a = $mya; // get agent

  if ($signin) { //  identifying agent
    if ($agent) {
      if ($a->id != $mya->id) return posErr('wrong company', [], $companyInfo);
      if ($a->agentId == $mya->agentId) return posErr('already scanned in', [], $companyInfo); // $a == $mya doesn't work here because of qo can vary
    }
//    if ($a->proSe and (!$a->co or $a->coCan(CO_REQUIRE_CASHIER))) return posErr('scan in first');
    if ($code != $a->cardCode() and $code != $a->codeHash()) {
      if (test() or $version >= 230) return posErr('bad agent', [], $companyInfo); 
      r\tellAdmin('bad code in identify op: ' . $err, compact(ray('member code')));
    }
    if (!$a->can(B_SELL)) return posErr('agent cannot scan', [], $companyInfo);

    list ($person, $company) = $mya->co 
    ? [$a->proSe ? '' : $a->agentA->fullName, $mya->fullName] 
    : [$a->agentA->fullName, ''];
    if ($version < 230) $name = $person ?: $company;

    $time = now();
    
  } else { // identifying customer
    if (!$agent) return posErr('scan in first', [], $companyInfo);
    if (isset($pin) and $pin != $a->agentA->pin) return posErr('bad pin', [], $companyInfo); // maybe never use this
    $agentCans = 0;

    list ($person, $company) = $a->co 
    ? [$a->proSe ? '' : $a->agentA->fullName, $a->fullName] 
    : [$a->agentA->fullName, ''];

    if ($version < 230) {
      $name = $a->agentA->fullName;
      $company = $a->proSe ? "" : $a->fullName;
    }
    $place = $a->location();
  }

  if ($version < 230) $default = $mya->mainQid;
  $first = (($a->co and $a->proSe) or $a->agentA->ided) ? '0' : '1'; // regular or first-time transaction (proSe co never)
  
  $fields = $version < 230 ? 'first device name place company default time' : 'device first person company place time';
  $info = []; foreach (ray($fields) as $k) if (isset($$k)) $info[$k] = $$k;
  return ok($info + balrew($a, $version < 230) + $companyInfo); // companyInfo last (don't overwrite bal)
}

/**
 * Get an individual's photo.
 * photo:  [member, code] => photo
 * @param array $args:
 *   member: the qid for the customer (individual or company rep)
 *   code: card code
 * @return:
 *   just the photo data (no json) or a short error message (50 characters or less)
 */
function photo($args) {
  if ($err = missingArg($keys = 'member', $args)) return photoReturn($err);
  extract(just($keys . ' code version', $args, NULL));

  if (!$member) return photoReturn(tr('missing member'));
  if (!$code) r\tellAdmin('missing code in photo op', compact(ray('member code')));
  if (!$a = getMember($member, $code, $err, TRUE)) return photoReturn($err);

  $signingIn = (bool) strpos($member, '-'); // allow agents with no photo by faking a photo (>= 100 chars)
  return photoReturn($a->agentA->photo ?: ($signingIn ? file_get_contents(R_ROOT . '/images/idcard/no-photo.jpg') : ''));
}

/**
 * Return the customer's account photo.
 */
function photoReturn($pic) {
  $max = 100; // arbitrary abbreviation length for logging and testing
  $message = strlen($pic) <= $max ? $pic : bin2hex(substr($pic, 0, $max)); // identifies pic in log w/out a big storage hit
  u\loga('out', compact('message'));

  if (test()) {
    $picNone = file_get_contents(R_DFT_PICTURE); // never a company photo
    if (u\starts(bin2hex(substr($picNone, 0, $max)), $message)) $message = 'no photo';
    f('t.output', $message); 
    return $message;
  }

  if (strlen($pic) > $max) header("Content-type: image/jpeg");
  return exitJust($pic);
}

/*
 * Request a charge.
 * charge:  [member, amount, code, goods, description, created, force]
 *   => [ok, message, txid, created, balance, undo]
 * @param array $args:
 *   agent: qid for the seller (company rep)
 *   amount: numeric dollar amount
 *   member: qid for the customer (individual or company rep)
 *   code: card code (phase out in new app 4/1/2016)
 *   goods: <transaction is for real goods and services>
 *   description: description of goods and services sold
 *   created: transaction creation date
 *   proof: hash of agent + amount + member + code + created (proof of card-present tx on or before created date)
 *   force: 0=nothing special, 1=do transaction even if insufficient funds, -1=force reversal of transaction
 * @param string $type: charge or pay (refund)
 * @return: json (ok, message, txid, created, balance, undo)
 */
function charge($args, $type = 'charge') {
  global $mya;
  
  $keys = 'agent member amount goods description created';
  if ($err = missingArg($keys, $args)) return posErr($err);
  extract(just($keys . ' proof force version', $args, NULL));
  $force = (int) $force;
   
  if (!$member) return posErr('missing member'); // handle empty string (versus missing param)
  if (!$proof) return posErr(missingArg('proof', []));
  if ($description === '') return posErr('missing description');
  if (mb_strpos($description, ' ') !== FALSE) {
    r\tellAdmin(t('POS tx description not urlencoded'), $args);
  } else $description = urldecode($description);
  if ($err = u\badAmount($amount, '', 2, MAX_AMOUNT)) return posErr($err); // allow negatives! (for refund)

  if (!$a = getMember($member, '', $err, FALSE)) { // proof is proof enough (no cardCode needed)
    return badTx($args, "Account $member", $err);
  } else $name = $a->fullName; // for use in error messages
  if ($mya->id == $a->id) return posErr('shoulda been login'); // should never happen

  $selfhelp = ($goods == '2');
  $goods = $goods == "3" ? FOR_NONGOODS : ($goods == "0" ? FOR_USD : FOR_GOODS); // selfhelp is FOR_GOODS
  
  $amount = str_replace(',', '', number_format($amount, 2)); // make sure both cents digits are included
  $code = $a->cardCode();
  $hashOfTx = u\hash($wanted = "$mya->mainQid$amount$member$code$created");
  if ($proof != $hashOfTx) if (!$force or $a->badOldProof($proof, $wanted, $created)) {
    r\tellAdmin('bad proof', compact(ray('version proof hashOfTx agent amount member code created force')));
    if (test() or $version > 218 ) {
      if (test()) f('t.output', "Should be proof of $wanted");
      $err = test() ? t('bad proof') : t('bad proof (wanted %hashOfTx as hash of %wanted', compact('hashOfTx', 'wanted'));
      return badTx($args, $name, $err);
    }
  }

  $message = $did = $undo = '';
  $txid = 0;
  
  $info = ray('created uid1 uid2 amt goods for2', $created, $a->id, $mya->id, $amount, $goods, $description);

  if ($force and $xid = db\get('xid', 'txs', $info)) { // forcing and transaction exists
    if ($force == 1) { // does exist and should (always background)
      $txid = $xid;
    } elseif ($force == -1) { // does exist and should be reversed
      $z = be\posUndo($mya, $xid, $force, ($amount < 0 and !$mya->co) ? 'amount>0' : "amount='$amount'");
      if (!$z->ok) return badTx($args, $name, $z->msg, $z->subs);
      $created = $z->subs['created'];

      if (nni($z->subs, 'nvid')) { // invoiced!
        $message = tr($z->msg, $z->subs);
        $undo = $txid = '';
        $did = t('charged');
      } else {
        $message = $z->msg;
        $undo = $xid; // undo = xid of reversed tx
        $txid = nni($z->subs, 'xid');
      }
    } else u\FAIL(t('force param must be 0, 1, or -1'));
    
  } else { // transaction doesn't exist (yet)
    if ($force == -1) { // doesn't exist and shouldn't
      $created = '';
      $message = t('Transaction has been canceled.');
    } else { // doesn't exist and should (forcing or not)
      if ($amount < 0 and !$mya->co) list ($type, $amount) = ['payment', -$amount];
      $z = be\transfer($type, $mya, $a, $amount, $description, $goods, compact(ray('created force')));
      if (!$z->ok) return badTx($args, $name, $z->msg, $z->subs);

      if (!$force and $aa = $a->agentA and $mya->co and !$aa->ided) $aa->setBit(B_IDED);
      $txid = $z->subs['xid'];
      if ($selfhelp) {
        $message = 'selfhelp tx';
        if ($mya->cttyRewardy) $message .= '|for why|reward other';
      } else {
        $message = $z->msg;
        $forCash = $goods == FOR_USD ? t('for USD') : '';
        $we = $mya->co ? t('We') : t('You');
        $did = tr('just did', $z->subs + compact('forCash', 'we'));
      }
      $undo = ($mya->can(B_BUY) and $mya->can(B_SELL)) ? tr('confirm undo app', $z->subs) : '';
    }
  }

  return ok(compact(ray('message txid created did undo')) + balrew($a, $version < 230));
}

/**
 * Handle request for purchase from fuel pump or other machine.
 * @param string $member: account qid of machine (including agent number, which is machine number)
 * @param string $code: M<n>, where <n> is a 1-6 digit radix-36 machine number, from QR code in place of security code
 * @return assoc of parameters to return or FALSE if this is not a machine purchase
 */
function machine($member, $code){
  global $mya;
  if ($mya->co or !$mya->proSe) return FALSE;
  if (strlen($code) > 7 or substr($code, 0, 1) != 'M') return FALSE;
  
}

/**
 * Tell the device owner about a failed background transaction
 */
function badTx($args, $account, $msg, $subs = []) {
  extract(just('force amount created', $args, NULL));
  
  if ($force) {
    global $mya;
    $by = $mya->proSe ? '' : t(' (agent %agent)', 'agent', $mya->agentA->fullName);
    $subs += ray('account amount created by', $account, u\fmtAmt($amount), fmtDt($created), $by);
    $mya->tell("bad forced tx|$msg", $subs);
  }
  
  return posErr($msg, $subs);
}

/**
 * Return info about current company
 */
function getCompanyInfo() {
  global $mya; 
  
  $descriptions = explode("\n", $mya->selling ?: '') ?: ['?'];
  
  if ($mya->co and !$mya->trialco) { // normal company with agent
    $can = ($mya->can(B_SELL) ? u\bit(CO_CHARGE) | u\bit(CO_R4USD) : 0)
         | ($mya->can(B_REFUND) ? u\bit(CO_UNDO) | u\bit(CO_REFUND) | u\bit(CO_USD4R) : 0)
         | ($mya->can(B_BUY) ? u\bit(CO_BUY) : 0)
         | ($mya->can(B_MANAGE) ? u\bit(CO_MANAGE) : 0);

    $can &= (($mya->coFlags >> APP_AGENT_CANS) & APP_CAN_MASK); // limit by agent perms
    $can <<= APP_CAN_BITS + 1; // shift to manager position (extra bit is for "manage" permission)
    $can |= (($mya->coFlags >> APP_CANS) & APP_CAN_MASK); // add in cashier permissions
  } else { // personal account or trialco
    $can = ($mya->can(B_SELL) ? u\bit(CO_CHARGE) | u\bit(CO_R4USD) : 0)
         | ($mya->can(B_BUY) ? u\bit(CO_BUY) | u\bit(CO_REFUND) | u\bit(CO_USD4R) : 0)
         | (($mya->can(B_BUY) and $mya->can(B_SELL)) ? u\bit(CO_UNDO) : 0)
         | u\bit(CO_MANAGE);
    if (!$mya->co) $can <<= APP_CAN_BITS + 1; // personal accounts are manager (signed-in), trialco is always signed out
  }
  
  $bad = db\q("SELECT CONCAT(qid, ',', code) FROM r_bad")->fetchCol() ?: [];
  $balance = $mya->balance;
  return compact(ray('descriptions bad can balance'));
}

/**
 * Report if an argument is missing.
 */
function missingArg($expected, $args = []) {
  foreach (ray($expected) as $one) if (!isset($args[$one])) return "missing $one";
}

/**
 * Return the agent's company acct.
 * @param string $agent: the agent's company qid or company's uid (if company is pro se).
 * @param string $err: (returned) error message, if any
 * @return: the agent's company acct object (false if error)
 */
function getAgent($agent, &$err = '') {
  if (!$mya = r\acct($agent)) return !$err = 'bad agent'; // . ": $agent";
//  if (r\foreignServer(r\uidRegion($mya->agentId))) return !$err = 'wrong server for agent';
  if (!$mya->ok) return !$err = 'company inactive';
  if (!$mya->proSe) r\setAcct($mya->agentId); // get agent first, if not proSe
  return r\setAcct($mya);
}

/**
 * Return the member's acct (the agent's account if signing in).
 * @param string $qid: the customer's qid.
 * @param string $code: hash of the customer's card code (formerly the card code itself)
 * @param string $err: (returned) error message, if any
 * @param bool $requireCode: require card code (FALSE for transactions)
 * @return: the customer's acct object (false if error)
 */
function getMember($qid, $code, &$err = '', $requireCode = TRUE) {
  if (!$a = r\acct($qid) or ($requireCode and $code != $a->cardCode() and $code != $a->codeHash())) {
    r\badCard($qid, $code); 
    return !$err = tr('bad member');
  }
//  }
//  if (r\foreignServer(r\uidRegion($a->agentId))) return !$err = 'wrong server for customer';
  if (!$a->ok) {
    r\badCard($qid, $code); 
    return !$err = tr('member inactive');
  }
  return $a;
  
//    if (!$smartInfo = db\get('owner,defaultAgent,transientId,access', 'r_boxes', 'code=:q_code', compact('q_code'))) return be\error('unknown device');

/*    if ($account_id != $q_account_id) { // this is a transient id scanned from the other party's smart device
      $transientId = basename(dirname($q_account_id));
      $recent = now() - R_TRANSIENCE;
      if (db\get('transientId', 'r_boxes', 'code=:code AND access>=:recent') != $transientId) return be\error('bad transient id');
    } */
}

/**
 * Return the balance and rewards.
 * @param acct $a: the customer account
 * @param boot $old: return credit line as "rewards"
 * @return: assoc [balance, creditLine]
 */
function balrew($a, $old = FALSE) {
  global $mya;
  if (!$mya->co) $a = $mya;
  $balance = (($a->secret and $mya->co) ? '*' : '') . round($a->balance, 2);
  $creditLine = $mya->debt ? -$mya->floor : '';
  if ($old) $rewards = $mya->debt ? $creditLine : 0;
  return compact(ray($old ? 'balance rewards' : 'balance creditLine'));
}

/**
 * Build a list of the account's most popular charge descriptions, beginning with the default.
 * @params acct $a: the company account
 * @return array of descriptions
 */
function getDescriptions($a) {
  $ray = [];
  //if ($default = $a->description) $ray[] = $default;
  $sql = 'SELECT description AS payeeFor, COUNT(description) FROM tx_entries WHERE uid=:id AND amount > 0 GROUP BY payeeFor ORDER BY COUNT(description) DESC';
  return $ray + (db\q($sql, ray('id', $a->id))->fetchCol() ?: []);
}

/**
 * Return the message with adjustments if it contains negative amounts (for refunds or unrefunds)
 * @param string $msg: (returned) the message to adjust, if it has negative amounts
 * @param string $phrase: what to replace
 * @param string $rephrase: what to say instead
 * @return the message, rephrased (if appropriate) without negative amounts
 */
function fixNegative(&$msg, $phrase, $rephrase) {
  if ((mb_strpos($msg, $phrase) === FALSE) or !mb_strpos($msg, '$-')) return $msg;
  $msg = preg_replace('/\$-/', '$', $msg, 1); // just one replacement (leave negative rewards alone)
  return $msg = str_replace($phrase, $rephrase, $msg);
}

/**
 * Handle the data requested and sent from the rPOS app on an account's device.
 * @param string $data: optional json-encoded data (perhaps data we requested from the app)
 * @return string $message: what extra to respond to the app's "time" request: another request for data or ''
 */
function handleAppMsg($data0) {
  global $mya;
  if (!$mya) return '';
  
  $get = $mya->getAppData ?: '';
// NO! this would preclude app-initiated reporting  if (!$get = $mya->getAppData) return '';
  $appData = $mya->appData ?: [];
  if ($data0) {
    if ($getRay = !u\starts($get, '!delete:')) {
      if (is_array($data = u\dejsonize($data0))) {
        ksort($data);
      } else $data = $data0;
        $dpy = pr($data);
    } else list ($data, $dpy) = [$data0, str_replace("'", "\\'", str_replace('\\"', '"', $data0))];
    r\tellAdmin(t('got App Data'), ray('who get zdata', $mya->fullName, $get, $dpy));
    $appData[$get ?: '!report'] = $data;
//    if ($get) $get = $getRay ? (u\nextAfter($get, ray(R_APP_DATA_GETS)) ?: '') : ''; // '' if none (eg for !report)
    $get = ''; // don't request anything automatically
    $mya->update('getAppData appData', $get, $appData);
  }
  return $get;
}

/**
 * Decode the device code using a public key from the appropriate server (just this one for now).
 * @param string $device: the b64encoded encrypted device code received from the device
 * @return: an adequate abbreviation of $device (FALSE if error)
 */
function decodeDevice($device) {
  if (!test() and !u\decryPP(u\b64decode($device), 'public')) return FALSE;
  return substr($device, 0, DEVICE_MAXLEN);
}

/**
 * Check validity of specified device or create one, if none is specified.
 * @param acct $device: the specified device
 * @param acct $device: (RETURNED) the device code to return (undefined if error)
 * @param bool $signin: <signing in>
 * @return the error (FALSE if none)
 */
function badDevice(&$device, $signin, $version) {
  global $mya, $boxId;
  global $nextCode; // tests usurp generated device code, using this instead

  if ($version >= 230 and $device and !decodeDevice($device)) return posErr('unknown device'); // bad crypt

  if (!$device) { // no device specified, set it up
    if ($version < 230) return posErr('old app');
    if (!$signin) return posErr('missing device'); // can't do anything but time or identify without device
    $device = nn($nextCode) ?: u\b64encode(u\cryPP(now() . "/$mya->fullName", 'private')); // let company prove to customer who they're dealing with
  }

  $boxId = w\boxId(substr($device, 0, DEVICE_MAXLEN), $mya, $version); // find device in the db or put it there

  return FALSE;
}
