<?php
/**
 * @file
 * Definitions required for any part of the Common Good system, including app testing
 */

define('R_VERSION', '3.51v'); // software version

global $db_name, $cryHows;
global $configRay; // the configuration array currently being examined -- used only in this file

$configFlnm = __DIR__ . '/../../config/config.json';
$configRay = (array) json_decode(utf8_encode(file_get_contents($configFlnm)));
/**/ if (!$stage = config('stage')) die('There is no valid config file ' . $configFlnm);

define('isDEV', $stage == 'development'); // (used in settings file and elsewhere)
define('isSTAGE', $stage == 'staging');
define('isPRODUCTION', $stage == 'production');
/**/  if (!isDEV and !isSTAGE and !isPRODUCTION) die('Invalid value for stage in configuration file.');

define('US_COUNTRY_ID', 1228); // United States (used in settings file and elsewhere)
require_once __DIR__ . '/cg-settings.inc'; // custom settings for this community's CG server

$base_url = config('baseUrl');
define('GEOCODE_KEY', config('geocodeKey'));
define('BASE_URL', $base_url);
define('LOCAL_URL', config('localUrl', $base_url));
define('PROMO_URL', config('promoUrl'));
define('CO_API_URL', config('coApiUrl'));
define('CGF_EMAIL', config('cgfEmail'));
define('CG_SMTP_SERVER', config('cgSmtpServer', 'smtp.gmail.com'));
define('CG_SMTP_TLS', config('cgSmtpTls', 'tls'));
define('CG_SMTP_PORT', config('cgSmtpPort', 587));
define('CG_SMTP_USER', config('cgSmtpUser', CGF_EMAIL));
define('CG_SMTP_PASS', config('cgSmtpPass'));
define('DKIM_PRIVATE', config('dkimPrivate'));
define('R_INVITE_KEY', config('inviteKey'));
define('R_SSN_USER', config('ssnUser'));
define('R_SSN_PASS', config('ssnPass'));
define('R_SSN_REQUEST', config('ssnRequest'));

$configRay = (array) config('db'); // now look at just the database config array
foreach (explode(' ', 'name driver host port user pass salt word encryption') as $k) ${"db_$k"} = config($k);

define('R_SALTY_PASSWORD', $db_pass);
define('R_WORD', hex2bin($db_word));
$cryHows = (array) $db_encryption;

unset($configRay); // don't leave this hanging around

$databases['default']['default'] = [ 
  'database' => $db_name,
  'username' => $db_user,
  'password' => $db_pass,
  'collation' => 'utf8_general_ci', // or utf8mb4_general_ci
  'host' => $db_host,
  'port' => $db_port,
  'driver' => $db_driver,
  'prefix' => '',
  'pdo' => [PDO::MYSQL_ATTR_INIT_COMMAND => 'SET NAMES utf8'], // or utf8mb4
];

define('PROJECT', t('Common Good')); // project name
define('CG_DOMAIN', 'CommonGood.earth');
define('RCREDITS', t('Common Good credit')); // don't use %PROJECT here
define('BANK_DBA', PROJECT);
define('PROJECT_ID', str_replace(' ', '', strtolower(PROJECT)));
define('PROJECT_LOGO', 'logo80.png');

define('SCRIPTS_TOP', 'x/jquery-3.3.1.min x/jquery.once x/jquery-ui.min x/bootstrap.min x/spin.min x/ladda.min x/ie10-viewport-bug-workaround x/jquery.confirm.min x/alert');
//define('SCRIPTS', 'form goo-jsapi misc charts chart-data download croppic crop-setup ballot x/bootstrap.slider.min countries print-country focus-on x/typeahead.bundle.min suggest-who on-submit chimp followup-email get-ssn change-ctty advanced-dates advanced-prefs bank-prefs which funding-criteria signup prejoint invite-link gift drupal user extend-settings'); // all possible scripts, in order (after SCRIPTS_TOP)
define('SCRIPTS', 'x/bootstrap-toggle.min parse-query misc croppic crop-setup x/bootstrap-slider.min ballot countries x/typeahead.bundle.min on-submit scraps drupal user'); // all possible scripts, in order (after SCRIPTS_TOP). drupal and user are for password.

define('DEV_PW2', 'devPassword#two!'); // second password on DEV machine (len must be 16, 24, or 32)
define('DEV_VKEYPW', 'wt38BqWMslj3uUHUOto7fNi5ExL9CC7S7ffjn_q6zCY'); // password for very secure key, on DEV machine
define('DEV_ADMIN_PASS', '123'); // admin password when isDEV
define('DEFAULT_PROXIES', '2, 3'); // account record IDs for place-holder proxies
define('TRACK_DELETED', 'r_invoices r_photos r_relations r_usd users'); // remember rows in these tables if we delete them
define('DELETED_BY_VIEW', 'tx_hdrs tx_entries tx_disputes'); // these tables are actually views on the _all version of
                                                             // themselves, and deletion is just by setting "deleted"
define('DEV_ONLY_MSG', 'only on development machine');

$host = strtolower(@$_SERVER['HTTP_HOST']);
if (!isDEV) if (mb_strlen($host) > 30 or !preg_match('/^\[?([a-zA-Z0-9-:\]_]+\.?)+$/', $host)) {header('HTTP 400 Bad Request'); exit();}
//define('R_PATH', '/sites/all/modules/rcredits');
define('R_PATH', '/rcredits');
global $rUrl; $rUrl = BASE_URL . R_PATH;
define('R_ROOT', __DIR__);
define('ROOT', getcwd() . '/..'); // avoid using DRUPAL_ROOT here (else gives confusing warning when tests fail)
$parts = parse_url($base_url);
$base_path = @$parts['path'] . '/';
$base_root = mb_substr($base_url, 0, mb_strlen($base_url) - mb_strlen(@$parts['path'])); // everything until first slash after "scheme://"

define('NOT_PRODUCTION', !isPRODUCTION); // safer to use than !isPRODUCTION (in case of typo)

// time periods are defined here because this script uses them
define('MIN_SECS', 60);
define('HOUR_SECS', 60 * MIN_SECS);  // 3600
define('DAY_SECS', 24 * HOUR_SECS);  // 86,400
define('WEEK_SECS', 7 * DAY_SECS);   // 604,800
define('WEEK7_SECS', 52 * DAY_SECS); // 4,492,900
define('MONTH_SECS', 30.4368 * DAY_SECS);  // 18,408,176.64
define('QUARTER_SECS', round(3 * MONTH_SECS));  // 55,224,530
define('YEAR_SECS', round(12 * MONTH_SECS));    // 220,898,120
define('FLOOR_DAYS', 183); // how many days to consider for credit line calculation

define('REGION_DOMAIN', R_SERVER_ID . '.' . CG_DOMAIN);
define('EMAILX', 'example.com'); // domain for unknown, placeholder, test, or example emails

// Common Good Finance email and other data
define('CGF_LEGALNAME', 'Society to Benefit Everyone, Inc.');
define('CGF_PHONE', '+1 413 628 1723');
define('CGF_ADDRESS', '48 Buckland Road');
define('CGF_CITY', 'Ashfield');
define('CGF_POSTALADDR', 'PO Box 21, Ashfield, MA 01330-0021');
define('CGF_EIN', '205280780');
define('SLOGANS', 'for truth and justice,for community power,for community democracy,for community investment,for a sustainable future,to empower communities,for community abundance,for a better future,for economic democracy,for democratic economics');
define('R_LAUNCH', strtotime('12/15/2013')); // original launch date
define('EMAIL_DOMAIN', CG_DOMAIN); // replace this with CG_DOMAIN when ready
define('SYS_EMAIL', 'system@' . EMAIL_DOMAIN); // old address so we don't get marked as spam (later at REGION_DOMAIN)
define('BOUNCE_EMAIL', 'bounce@' . EMAIL_DOMAIN); // old address so we don't get marked as spam (later at REGION_DOMAIN)
define('PUBLIC_KEY', dirname($_SERVER['DOCUMENT_ROOT']) . '/.publicKey');
define('PRIVATE_KEY', dirname($_SERVER['DOCUMENT_ROOT']) . '/.privateKey');
define('ANY', 'any'); // accessible to anyone (if you change this, rebuild menus immediately)
define('SVAR_HEADER', 'rcredits_'); // session variable header
define('TEST_PDF_FLNM', 'testOutput.pdf'); // pdf output during testing

define('R_BULLET', '&#8226;'); // bullet in emails (where CSS is difficult)
define('STATS_GRAIN', 10); // granularity of statistics, in days
define('INFLATION_DESC', t('inflation adjustment'));
define('COMPANY_FREEZE', FALSE); // companies cannot spend rewards yet
define('R', '<span class="r">r</span>');
define('US', '<span class="us">us</span>');
/// define('GAME_COMPANIES', '.,.,.,.,Gift Shop,Religious Institution,For Rent,Empty,Vacant Lot,Land,Cinema,Printing & Office Supply,Car Repair,Health Club/Gym,Restaurant,Hardware & Supply,Grocer,Farm,Food Processing,Department Store,Electric Company,Oil Company,Town Government,Community College,Hospital');
define('R_DATE_FLDS', 'created completed deposit sent invited giftDate signed login access joined yesterday dob when sold'); // date field names for testing and administrative display
define('CO_FLDS', 'coType website description shortDesc selling coFlags gross serviceArea employees payrollStart payrollEnd staleNudge contact'); // account fields stored in r_company
define('DATA_FLDS', 'legalName shortName newAccount by calling wants giftPot giftCoupons notifyNowBy signupCo stepsDone stepsLeft task achMin pinThreshold lastiCode tenure moves mediaConx getAppData appData snap fundBy isSavings idVerifiedBy partnerInfo'); // fields stored in the account's "data" array field (serialized) -- UNUSED: stepsDone
define('MASTER_NUMERIC_FLDS', 'minimum achMin savingsAdd saveWeekly');
define('MASTER_FLDS', MASTER_NUMERIC_FLDS . ' hasBank bankAccount last4bank refill'); // only in master, for joint accounts
define('R_MONEY_FLDS', 'balance rewards floor giftPot committed'); // financial amount fields in account record
define('JOINT_FLDS', R_MONEY_FLDS . ' savings'); // add these together for joint accounts
define('PSECURE_FLDS', 'phone email'); // personal fields that get encrypted in a searchable way (P)
define('VSECURE_FLDS', 'federalId dob bankAccount'); // very secure fields (always soon pgp-encrypted within Acct::vsecure)
define('SECURE_FLDS', 'pw2 vKeyPw ssnData address postalAddr faxetc emailCode cardCode cardCode2 pin question answer idProof oneTimePass last4bank yob'); // non-personal encrypted fields (must be complete list)
define('R_HIDE_CHANGES', R_MONEY_FLDS . ' pw2 vKeyPw ssnData hasBank changes init login access activated data secure vsecure stepsDone steps stepsLeft lastiCode pass oneTimePass notes picture photo thumb trust risk risks stats getAppData appData special last4bank lastip tickle payrollStart payrollEnd'); // do not record changes to these fields: anything often-changed (automatically or not)
define('R_APP_DATA_GETS', '!log !device !members !txs '); // data requests from the rPOS app
//  define('R_AVAIL_BUY', '(balance-IF(u.:IS_DEBT, floor, GREATEST(floor, 0)))'); // balance available to spend
define('R_SHARE_MAX', 50); // minimum sharing percentage
define('R_SHARE_DFT', 25); // default sharing percentage
define('R_SHARING_MSG', t('sharing rewards with CGF'));
define('ICLUB_TITHE', .1); // fraction of profits that go to the community and the common good
define('USERNAME_MAX_LENGTH', '100'); // max length of "name" field

define('R_ADMIN_CAN_MANAGE_CTTYS', TRUE); // should be false once communities can manage themselves
define('R_CRON_PERIOD', 5*60); // how many seconds between cron runs (must divide an hour evenly)
define('R_MAX_TASK_TIME', 1*60); // what's the longest a queued cron task might take
define('R_NOLOG', SECURE_FLDS . ' ' . VSECURE_FLDS . ' data secure vsecure secret oneTimeLoginUrl password pass pass1 pass2 vKeyE newPass external_input form_build_id bad args'); // don't log these (external_input may contain passwords and is duplicated as individual fields)
define('R_NOLOG_FORMS', 'Admin Test Handy footer accounts history settings settings/icon community Summary Php'); // don't log visits to these forms
define('R_ANON_FORMS', R_NOLOG_FORMS . ' Signup Signin PageNotFound Sinx Pass Empty TestRecovery ChangePassword Help Community Democracy Agreement CttyFunds Reports Flags Game Menu I ResetPassword Profile Accept Buy HandleInvoice FindCo'); // no login required
define('SSN_ERR_INDEX', 'CGerror'); // where to put/find error messages in ssnData
define('BY_CASH', '(cash)'); // app includes this in description, for a purchase of CG credit by cash
define('BY_CHECK', '(check)'); // app includes this in description, for a purchase of CG credit by check
define('BY_CARD', '(card)'); // app includes this in description, for a purchase of CG credit by credit card
define('R_ON_FILE', t('(on file)'));
// (NO) define('DEPOSIT_SQL', '(amount>0 XOR txid<0)'); // SQL fragment for "transfer is from bank to CG"

define('DOLLAR_POOL', 1); // uid of dollar pool account
define('CANONIC_ACCTS', 512); // number of canonic accounts (admin/dollar pool, placeholders,...)
define('CANONIC', 'BETWEEN 0 AND ' . (CANONIC_ACCTS - 1));
define('AGGREGATE_MIN', 1); // minimum number of members to show aggregate data

define('CG_ADMIN_UID', 1);  // uid of administrator account
define('PLACEHOLDER_1_UID', 2);  // uid of first placeholder
define('PLACEHOLDER_2_UID', 3);  // uid of second placeholder
define('CG_ROUNDUPS_UID', 129);  // Donations start at 128 (128 used for general donations)
define('CG_CRUMBS_UID', 130);  //
define('CG_SERVICE_CHARGES_UID', 192);
define('MINIMUM_BANK_UID', 256);
define('BANK_IN_ID', 256);  // Bank accounts start at 256
define('BANK_OUT_ID', 257);  //
define('MAXIMUM_BANK_UID', 511);

define('R_TEMP_NAME_PREFIX', 'unknown ');
define('CO_PHOTO_FACTOR', 3); // make company photos this many times as big as individual photos
define('MAX_UPLOAD_SIZE', 4 * 1024000); // maximum upload size in bytes
define('MAX_ID_SIZE', 10); // max ID size in MB

define('GEOCODE_URL', 'https://api.opencagedata.com/geocode/v1/json?'); // use in acct::setLocus()
define('MI_PER_DEGREE', 69); // varies from 68.703 at the equator to 69.407 at the poles
define('NEIGHBOR_MAX', 40); // how far away (in miles) can a member see names of other individual members?
define('DISTANCE_BETWEEN', 'SQRT(DEGREES(ACOS(LEAST(1.0, COS(RADIANS(latitude)) * COS(RADIANS(:lat)) * COS(RADIANS(longitude - :lon)) + SIN(RADIANS(latitude)) * SIN(RADIANS(:lat)))))) * ' . MI_PER_DEGREE); // distance from other, in miles
define('SIGNUP_PARTNERS', 'cooppower nyccec'); // these organizations sign people up for Common Good

define('R_URL_SLASH', '%_%'); // Drupal gets confused by slashes in a url, even if urlencoded
define('R_EXCHANGE_LIMITS', '1:5000,7:20000,52:80000,365:500000'); // per day, week, 7-week, year
define('STALE_NUDGE', 7); // default number of days to wait before reminding a customer to pay an invoice
define('R_INVITE_DAYS', 30); // number of days to accept invitation to membership
//define('R_MAXR_DEFAULT', -1); // suggested maximum amount to hold in CG credit (-1 = no limit)
define('R_CARDCODE_LEN', 14); // length of rCard security code (long to deter easy copying, max 14 for small QR)
define('R_CARD_COST', 5); // cost to order a company or replacement rCard
define('ORGANIZER_GRANT_MIN', 10); // minimum number of individuals to recruit, to be eligible for a grant
define('ORGANIZER_GRANT_MAX', '$5,000'); // maximum organizer grant amount
define('TICKLE_WELCOME', 2 * 7); // number of days to wait after activation, before followup
define('TICKLE_DELAYED', 6 * 7); // number of days to wait after activation, before followup if no gift or no CGC
define('OFTENS', 'W:Weekly,M:Monthly,Q:Quarterly,Y:Yearly,X:Once (now)');
define('OFX_TX_FLDS', 'trnType dtPosted trnAmt fitId name memo');
define('OFTENS_SECS', ['W' => WEEK_SECS, 'M' => MONTH_SECS, 'Q' => QUARTER_SECS, 'Y' => YEAR_SECS]);

define('ICARD_EXPIRES', 14); // how many days until invitation card expires, after account is activated
define('ICARD_CODELEN_MIN', 10); // minimum length for an invitation code
define('ILEN_REGION', 15); // 3 letters (radix 26-32 -- in other words, 1-6 can be used in region also)
define('ILEN_TAIL', 19); // 3-4 letters (radix 26)
define('ILEN_SECURITY', 16); // 3 alphanumerics (radix 35)
define('ILEN', ILEN_REGION + ILEN_TAIL + ILEN_SECURITY);
define('ILEN_DIV', 25); // this results in two 5-digit codes (otherwise the integer gets too big)

// codes for iCode field (invitation code)
define('IBY0', -2); // lowest IBY number
define('IBY_FORM', -2); // invited by form (including companies) (the ONLY pre-confirmed signup)
define('IBY_LATE', -1); // invited by form, but accepted late (needs to be confirmed)
define('IBY_SELF', 0); // signed up without an invitation/confirmation yet
define('IBY_EMAIL', 1); // invited by email (this invitation # is re-usable, but needs to be confirmed)
define('IBY_ICARD', 2); // >=this is invitation number, also needing confirmation
define('IBYS', 'form late self email icard');

// notification preferences
$GLOBALS['notifyWhens'] = t('N:Never,I:Immediately,D:Daily,W:Weekly,M:Monthly');
$GLOBALS['notifyNowBys'] = t('E:Email,T:Text message,B:Both'); // how to send immediate messages
define('N_DEFAULTS', 'MMNIIMDIDWD');
define('N_EXPLICIT_OUT', 0); // manual payment or transfer out
define('N_AUTO_OUT', 1); // automatic payment made
define('N_AUTO_IN', 2); // auto-refill initiated
define('N_IN_DONE', 3); // transfer from bank account complete
define('N_INCOME', 4); // payment received
define('N_INFLATION', 5); // inflation adjustment received
define('N_REWARD', 6); // other incentive reward received
define('N_OTHER_IN', 7); // fine, grant, loan, or investment received
define('N_CHANGED', 8); // transaction/invoice edited, canceled, or disputed
define('N_NUDGE', 9); // your invitee needs a nudge
define('N_SUGGESTION', 10); // suggestion from the system

define('R_WHYS', t('goods and services,exchange of US Dollars or other currency,loan/reimbursement/etc.')); // . R_SHARING_MSG); // transaction purposes
define('FOR_GOODS', 0); // index into R_WHYS for goods
define('FOR_USD', 1); // index into R_WHYS for USD exchange
define('FOR_NONGOODS', 2); // index into R_WHYS for loan, etc.
//define('FOR_SHARE', 3); // "sharing" gift to CGF (comes out of rewards)
//define('FOR_INVESTMENT', 4); // only in invoices (use B_INVESTMENT in txs)
//define('FOR_STAKE', 5); // only in invoices (use B_STAKE in txs)

define('R_CHUNK', 10); // chunk size for buying CG credit (sales amounts are a multiple of this)
define('R_MIN_DEFAULT', 100); // suggested minimum amount to hold in CG credit
define('R_ACHMIN', 10); // minimum granularity of withdrawals from bank account
define('R_ACHMIN_DEFAULT', 50); // suggested achMin
//define('R_ACH_TRIES', 3); // how many times to retry an ACH
define('CASHOUT_MIN', 100); // minimum dollar amount to cash out automatically
define('VERIFICATION_DELAY', 3); // number of days to wait after requesting a zero transfer, before requesting a real transfer

define('R_SUGGEST_BUMP', 1.25); // suggested minimum multiplier, so as not to overdraw
define('CO_ASPECT', [4, 3]); // width-to-height ratio for commercial profile picture
define('FACE_ASPECT', [3, 4]); // width-to-height ratio for personal profile picture
define('R_PHOTO_WIDTH', 300); // rCard photo width, in pixels
define('R_PHOTO_HEIGHT', 400);
define('R_PIXEL_FACTOR', 200); // what to multiply the aspect ratio components by, to get pixel dimensions
define('THUMB_FACTOR', 12); // what to multiply the aspect ratio by, for thumbnail photo
define('R_INDIVIDUAL_GIFT', 50); // suggested donation for individuals
define('R_COMPANY_GIFT', 250); // suggested donation for companies
define('R_BUYS_BEFORE_BONUS', 1); // number of purchases required before signup bonus (should probably be 3)
define('R_REGION_MAX', 3000000000); // maximum uids in a region (46k,3T ~sqrt(PHP_INT_MAX))
define('R_ACCT_REGION', '(-FLOOR((uid-1)/' . R_REGION_MAX . ')-1)'); // SQL region expression
define('R_SIGNIN_HOURS', 24); // window for signing in after registering or resetting password
define('R_MEMBER_MARK', '.'); // divides region from specific identifier in member IDs
//define('R_AGENT_MARK', ':'); // divides region from specific identifier in relational IDs
define('AGT_MARK', '-'); // divides main QID from agent code
//define('R_AGENT_URL_MARK', '-'); // use this as a substitute for R_AGENT_MARK in URLs (else domain forwarding fails)
//define('R_MARKS', '[\\' . R_MEMBER_MARK . '\\' . R_AGENT_MARK . '\\' . R_AGENT_URL_MARK . ']'); // any of those
define('R_ANCIENT_EFT', 10); // how long, at most, an EFT might take (transferring funds to/from bank)
define('R_REQUIRED', ' <span class="form-required" title="This field is required.">*</span>');
define('R_CC_RATE', 2.5); // typical credit card percentage rate
define('R_CODE_LEN', 32); // length of most random identifiers
define('R_DUPTX_MINS', 4); // number of minutes before a duplicate transaction is allowed
define('HAS_XFEE', "data LIKE '%s:4:\"xfee\";%'"); // sql to recognize a transaction that has an exchange fee
define('R_DEPOSIT_RETURN_FEE', 20); // what we charge members for a second bounced transfer
define('HAS_REVERSER', "(EXISTS (SELECT tRev FROM tx_hdrs WHERE tRev.reverses=t.xid))");
define('TRIALCO_LIMIT', 1000); // maximum payments received by a trial (dependent company) account
define('EQUIP_DEPOSIT', 50); // dollar cost of equipment deposit
define('MATURITY', 21); // minimum age to have a credit limit

// Filenames
define('R_TEMPLATE_DIR', '/templates');
define('R_EMAIL_FRAME', 'email-frame');

define('R_PICTURE_DIR', ROOT . '/cgPhotoTemp/');
//define('R_PROOF_DIR', '/proofs/'); UNUSED
define('R_DFT_PICTURE', R_ROOT . '/images/no-photo-available.jpg'); // default profile picture
define('R_DFT_PICTURE_WIDE', R_ROOT . '/images/no-photo-available-wide.jpg'); // default profile picture
define('R_NOT_VALID', R_ROOT . '/images/not-valid.png'); // says rcard is not valid for purchases
define('LOG_DIR', ROOT . '/cgLogs/'); // where to store log files

// account type bits
define('CO_TYPES', t('personal, sole proprietor, partnership, LLC, private corporation, publicly-traded corporation, tax-exempt nonprofit, government, co-operative, unincorporated association or club, estate, trust, custodian'));

define('CO_PERSONAL', 0); // phase out (use type field in r_company instead)
define('CO_SOLE_PROPRIETOR', 1);
define('CO_PARTNERSHIP', 2);
define('CO_LLC', 3); // used in testing
define('CO_CUSTODIAN', 12);

// Company Account flags (bits)
define('CO_PRIVATE', 13); // company is not to be listed publicly (on promo site, etc.)
define('CO_ATM', 14); // can trade cg credit for cash, even if balance is less than floor

define('APP_CANS', 15); // company permission bits when signed OUT: bits 15-21
define('CO_CHARGE', 0);
define('CO_UNDO', 1);
define('CO_R4USD', 2);
define('CO_USD4R', 3);
define('CO_REFUND', 4);
define('CO_BUY', 5);
define('CO_u21', 6); // unused
define('CO_MANAGE', 7); // this bit is not actually stored here, but pass B_MANAGE to the app in this position
define('APP_CAN_BITS', 7); // number of bits for permissions (each: signed in and out)
define('APP_AGENT_CANS', APP_CANS + APP_CAN_BITS); // company permission bits when signed IN (to be limited by agent perms)
define('APP_CAN_MASK', pow(2, APP_CAN_BITS + 1) - 1); // the CANS bits -- signed-IN permission bits are 22-28
define('CO_DFTS', '0 1 7 8 9 11 12'); // set these bits by default

define('CO_FAST', 29); // (deprecated as of app v 2.16) fast checkout -- never ask for photo Id (set by cAdmin)
define('CO_TIP', 30); // ask customer what percent tip to add

define('APP_CAN_BIT_NAMES', 'charge undo r4usd usd4r refund buy');
define('APP_CAN_BIT_DESCS', t('charge customers,undo last transaction,give %RCREDITS for USD,give USD for %RCREDITS,refund any amount,make payments'));

// Membership steps
define('R_STEPS_OLD', 'signup:signup, verify, ssn, sign:community/agreement, donate:community/donate, proxies, prefs:settings/preferences, connect, company, relations, photo, food, discount:community/discount, backing:community/backing');

define('S_ALL', 'signup, verifyemail, verifyid, ssn, agent, fund, contact, agree:community/agreement, donate:community/donate, invite:community/invite, proxies, preferences, company, photo, discount:community/discount, backing:community/backing, titheout, tithein, work, partnersteptwo, partnerend, food, wantsPrint, printed'); // where to go to complete each step (unordered) -- add new steps to the end
define('S_PERSON', 'signup verifyid agree preferences fund verifyemail'); // soon replace "preferences" with titheOut
define('S_CO', 'signup discount verifyemail');
define('S_CO2', 'agent agree contact backing photo donate company tithein discount');
define('S_CARD', 'photo contact donate');
define('S_SELL', 'ssn contact tithein');
define('S_VOICE', 'contact donate proxies work backing invite'); // add comment
define('S_PARTNER', 'signup fund partnersteptwo partnerend');
//define('S_PARTNERCO', 'signup fund agree partnerend');

define('S_MEM_TASKS', 'card sell voice co2'); // followup tasks for members
define('S_PTASKS', 'person card sell voice partner');
define('S_COTASKS', 'co co2 partner');

// Permissions, options, and flags (flags field in account record)
// If we need more bits, create a flags2 field and make the B_ defs negative or 32-63 for setBit
// for documentation of IS_OK etc. see u\SUBS()
define('B_MEMBER', 0); // fulfilled membership requirements (ready to be approved).
define('B_CONFIRMED', 1); // invitation has been confirmed (in advance by form invite OR explicitly OR implicitly by newb first buying from the inviter in person)
define('B_OK', 2); // can use CG for real (set by admin when ready). Set FALSE to freeze ctty funds (NYI)
define('B_CARDED', 3); // card has been printed and mailed
define('B_IDED', 4); // a cashier has verified the member's government-issued photo ID
define('B_REFILL', 5); // refill the account automatically (up to a target amount)
define('B_CO', 6); // company account
define('B_DRAWS', 7); // this account draws on one or more others
define('B_ROUNDUP', 8); // round payments up to nearest dollar, donate the cents to the community fund
define('B_NONUDGE', 9); // don't send reminders
define('B_NOSEARCH', 10); // user wants to be not findable by other members, by phone or email
define('B_DEPENDS', 11); // dependent account (can't change settings, federalId is sponsor's)
define('B_BANKOK', 12); // connected bank account has been verified
define('B_WEEKLY', 13); // send notices only weekly
define('B_MONTHLY', 14); // send notices only monthly
define('B_u15', 15); //
define('B_BYSMS', 16); // NYI notifications by SMS
define('B_PAPER', 17); // will not accept electronic statements
define('B_SECRET', 18); // don't let merchants tell me my balance on request
define('B_UNDERAGE', 19); // too young to have a credit line (needed because dob is accessible only to superAdmin)
define('B_DEBT', 20); // member chose to permit sub-zero balances
define('B_REINVEST', 21); // member chose to automatically reinvest investment club dividends and inflation adjustments
define('B_SAVINGS', 22); // the connected bank account is a savings or money market account
define('B_CASHOUTW', 23); // cash out automatically every 7 days
define('B_CASHOUTM', 24); // cash out automatically every 30 days
define('B_BETA', 25); // beta tester
define('B_u26', 26); //
define('B_ADMIN2', 27); // is an overall admin assistant or bank partner or government regulator (read-only except notes)
define('B_CADMIN2', 28); // is a community sub-administrator (read-only except notes)
define('B_CADMIN', 29); // is a community administrator
define('B_ADMIN', 30); // is a system administrator -- can manage server, including assigning ctty admins, but cannot manage communities or individual accounts (except temporarily)
define('B_MAX', 62); // maximum bit number
define('B_ADMINABLE', 'member confirmed carded beta cAdmin cAdmin2 nonudge cgc invites rewardy noneg'); // for cAdmin to set
define('B_LIST', 'member confirmed ok carded ided refill co draws roundup nonudge nosearch depends bankOk weekly monthly u15 bysms paper secret underage debt reinvest savings cashoutW cashoutM beta u26 admin2 cAdmin2 cAdmin admin');

// community-specific flags
define('B_CGC', 0); // ctty: is an official CGC (currently has no effect)
define('B_UP', 1); // ctty: is operational (up) (see also B_CTTYUP)
// B_CO (see above)
define('B_INVITES', 3); // require invitations
define('B_REWARDY', 4); // community issues rewards explicitly
define('B_NONEG', 5); // temporary ban on accounts going negative (set automatically in r\stats)
define('B_CTTY_BITS', 'cgc up co invites rewardy noneg');
define('B_ADMIN_BITS', 'member:cgc,confirmed:up,co:co,u3:invites,ided:rewardy,refill:noneg');

define('R_CTTY_BENE', 'cttyBuys+signups+rebates+bonuss+inflations+givebacks+grants+loans+invests+fines+tradeOut'); // benefit (ever) to the community (fines and givebacks are negative)

// Relationship flags
define('B_EMPLOYEE', 0); // is employed by main account
define('B_OWNER', 1); // owns the main account
define('B_CUSTOMER', 2); // is a customer of main account
define('B_AUTOPAY', 3); // preapproves any invoice from main account
define('B_r4', 4); //  by main account
define('B_r5', 5); //  by main account
define('B_r6', 6); //  by main account
define('B_DRAW', 7); // can draw funds from main account (same number as B_DRAWS in the users table)
define('REL_FLAGS', 'employee owner customer autopay r4 r5 r6 draw');
define('REL_FLDS', 'main other otherNum permission code');
define('REL_DATA_FLDS', 'fullName email phone address city state zip address2 city2 state2 zip2 partner partnerCode customer source qid country postalAddr'); // pseudo-fields typically stored in data field

// Relationship permissions used in calls to r\Acct::can() -- stored (less B_RELATED) in r_relations
define('B_RELATED', 100); // has a relationship record
define('B_NONE', 100); // no access
define('B_READ', 101); // can view Transaction History
define('B_SCAN', 102); // can accept CG Card payments for goods or cash for CG (or view Transaction History -- permissions are cummulative)
define('B_REFUND', 103); // can undo transactions and make refunds or trade cash at point of sale
define('B_SELL', 104); // can send invoices (or scan or view)
define('B_BUY', 105); // can make purchase for the account (or sell or scan or view)
define('B_MANAGE', 106); // can manage all aspects of the account
define('B_JOINT', 107); // make this a joint account (subsuming the other, with permission) -- no companies!
$GLOBALS['share permissions'] = array(t('no access'), t('view transactions'), t('accept %PROJECT card payments'), t('give refunds & give USD for %RCREDITS'), t('send invoices'), t('make purchases'), t('manage account'), t('joint account'));
define('B_PERMISSIONS', 'related read scan refund sell buy manage joint');

// Other special pseudo-bit values for r\Acct::can() and rweb.module menus
define('B_MANAGING_CTTY', 120); // cadmin is actually managing a qualified account
define('B_PERSON', 121); // personal account (!co)
define('B_DEV', 122); // isDEV or is admin
define('B_u123', 123); //
define('B_CTTYUP', 124); // account's community is operating (see also B_UP)
define('B_BANK', 125); // account has a connected bank account
define('B_CLOSED', 126); // account is closed
define('B_SHOW_ALL', 999); // special signal to r\Acct::can() to return all permissions

// Canonic strings
define('S_REFUND', t('refund'));
define('S_USD_OUT', t('USD out'));
define('S_USD_IN', t('USD in'));

// Risk definitions (for accounts and transactions) -- suspiciousness for possible reporting to FinCEN
define('K_THRESHOLD', 200); // how big an amount in a single transaction is suspicious
/*define('K_DAY_THRESHOLD', 300); // how big an amount spent or received by an individual in one day is suspicious
define('K_WEEK_THRESHOLD', 600); // how large an amount is suspicious in one week
define('K_7WEEK_THRESHOLD', 1200); // how large an amount is suspicious in one 52-day period
define('K_YEAR_THRESHOLD', 2400); // how large an amount is suspicious in one year */
define('K_THRESHOLD_PCT', 5); // what percent of top accounts transaction volume per employee to flag
define('K_LIMIT_FACTOR', 3); // how many times the various thresholds is a prohibited amount
define('K_UNUSUAL_BUMP', 20); // what percentage 7-week increase (in or out) is suspicious
define('K_BIT_MIN', .05); // lowest risk factor that sets a bit
define('K_RED', 100); // lowest score to flag for investigation
define('K_GREEN', -500); // threshold for an account or transaction to be unquestionably beyond reproach
define('K_LINES', 20); // number of lines to show for each type of each potentially risky record (acct or tx)
define('K_MISER_DONATION', 5); // minimum donation to not be a miser
define('K_MISER_SHARE', 25); // minimum sharing percentage, to not be a miser
define('K_CTTY_THRESHOLD', 100); // minimum size of community transactions to consider for risk analysis

// names and weights of account and transaction risks (and, for negative weights, assurance)
// weight is the number of such risks that together would make the account or transaction suspicious
// a weight of "X" means the risk is named but does not contribute to the total risk score (UNUSED)
define('K_ACCT_RISKS', 'adminOk:-0.3,trusted:-4,UNUSED:-3,hasBank:-3,geography:4,cashCo:2,new:5,moves:3,rents:10,badConx:2,homeCo:20,shady:3,miser:5,UNUSED2:1,addrOff:5,ssnOff:1,dobOff:4,poBox:5,fishy:2,moreIn:2,moreOut:2,bigDay:1,bigWeek:1,big7Week:1,bigYear:2');
define('K_ACCT_RISKS_EDITABLE', 'adminOk cashCo rents shady photoOff addrOff ssnOff dobOff fishy');
define('K_TX_RISKS', 'txAdminOk:-0.3,redo:2,exchange:2,cashIn:5,cashOut:2,fromBank:5,toBank:2,b2p:2,p2p:2,inhouse:2,fromSuspect:3,toSuspect:3,absent:4,invoiceless:4,bigFrom:3,biggestFrom:3,oftenFrom:3,bigTo:3,biggestTo:3,oftenTo:3,offline:10,firstOffline:3,origins:16,suspectOut:2'); // names and size of transaction risks
define('K_ADMINOK', 0); // community admin blessed the account or transaction
define('K_HASBANK', 3); // account has a connected bank account
define('K_ADDROFF', 14); // account's physical address is wonky (can't be geolocated or whatever)
define('HAS_BANK', 'risks&(1<<' . K_HASBANK . ')'); // mysql criterion (not surrounding parens, so we can do u.:HAS_BANK)
define('MASTER', '(jid=0 OR uid < jid)'); // account is the master in a joint account
define('U_FOODY', '(SELECT 1 FROM r_industries i JOIN r_user_industries ui USING (iid) WHERE ui.uid=u.uid AND u.name<>\'foodfund\' AND i.parent=2 LIMIT 1)'); // MySQL for "the account with record ID u.uid is a food-related business"
define('T_FOODY', '(SELECT ' . U_FOODY . " FROM users u WHERE u.uid=uid2)"); // MySQL involving txs VIEW, meaning "payee is FOODY"

define('FLOW_NONE', 0); // which way can credit flow, with another account
define('FLOW_TO', 1);
define('FLOW_FROM', 2);
define('FLOW_BOTH', 3);

// Coupon flags
define('B_SOME', 1); // coupon is only for some accounts (those in r_coupated)
// define('B_GIFT', 10); // gift coupon (defined for transactions below)

// Transaction flags (for flags field in transaction records)
define('B_TAKING', 0); // payee initiated the transaction
define('B_DISPUTED', 1); // non-originator disputes the transaction -- signaled by existence of tx_disputes (NOT LEGAL FOR TRANSACTIONS, BUT STILL USED BY INVOICES
define('B_OFFLINE', 2); // transaction was taken offline (or was forced?)
define('B_SHORT', 3); // transaction was taken (offline) despite credit shortfall
define('OLD_B_UNDONE', 4); // undone by another transaction -- signaled by existence of reversing transaction
define('OLD_B_UNDOES', 5); // undoes another transaction -- signaled by reversesXid not null
define('B_CRUMBS', 6); // monthly donation of percentage of receipts
define('OLD_B_ROUNDUPS', 7); // monthly donation of rounded up cents -- no longer occurs
// B_ROUNDUP is 8 (defined above) // payer donated the change to the community fund -- not used here, 
define('B_RECURS', 9); // recurring transaction
define('B_GIFT', 10); // grant or gift (of any type)
define('B_LOAN', 11); // community loan (UNUSED?)
define('B_INVESTMENT', 12); // community investment (UNUSED?) or investment club investment
define('B_STAKE', 13); // member buying or selling stake in investment club
define('B_FINE', 14); // community fine (UNUSED?)
define('B_NOASK', 15); // transaction was taken with ID checking OFF
define('B_FUNDING', 16); // invoice has already instigated an appropriate bank transfer request
define('B_FOOD', 17); // contribution to food fund
define('TX_FLAGS', 'offline short crumbs recurs gift loan investment stake fine noask funding food');

// Invoice status (txid >= TX_PAID if paid)
define('TX_DENIED', -2); // invoice rejected by the non-originator
define('TX_PENDING', -1); // charged, waiting for confirmation
define('TX_APPROVED', 0); // invoice is approved but not paid
define('TX_PAID', 1); // any status this amount or more is a payment transaction record ID (xid)
define('TX_STATUSES', t('-2:denied,-1:OPEN,0:approved,1:paid'));

// Transaction channels (roughly in order of simplicity and generality of messages) (CGF)
define('TX_SYS', 0);
define('TX_SMS', 1); 
define('TX_WEB', 2);
define('TX_POS', 3); // smart phone or other computer-like device
define('TX_TONE', 4); // touch tone phone
define('TX_CRON', 5);
define('TX_LINK', 6); // user clicked a no-signin link
define('TX_AJAX', 7);
define('TX_FOREIGN', 8); // user clicked a "Pay with CG" button or request an app to charge them
define('TX_CHANNELS', 'Sys SMS Web App Tone Cron Link Ajax Foreign'); // can't translate, because used in t() (find a solution)

// Transaction types
define('TX_TRANSFER', 0); // normal fund transfer (usually for actual goods and services) -- not creating rC
define('TX_BANK', 99); // used only internally, to mark bank transfers

define('TX_DEFAULT_PERIOD', 90); // number of days to show in transaction history, by default
define('TX_DEFAULT_INV_PERIOD', -2); // number of days to show in invoice history, by default (all of them)

define('MIN_TX_ROWS', 3); // minimum number of transaction lines to show
define('NORMAL_TX_ROWS', 30); // normal number of transaction lines to show
define('TX_MAX_ROWS', 500); // (unused) maximum number of transactions to load at once
define('TX_MAX_MOS', 18); // maximum transaction months, for display
define('TX_MAX_DAYS', 30.5 * TX_MAX_MOS + 2); // maximum days in a transaction query (18 months) -- 2 for leap day & daylight
define('TX_DATA_FLDS', 'undoneBy undoes force changes'); // serialized pseudo-fields in the data field of a transaction record

define('DS_OPEN', 1);  // dispute not resolved
define('DS_ACCEPTED', 2);  // dispute accepted and transaction reversed
define('DS_DENIED', 3);  // dispute denied

define('E_BANK_ONLY', 0); // bank charges and transfers between bank accounts that have no effect on CG account balances
define('E_PRIME', 1);  // the main pair in the transaction (payer and payee)
define('E_PAYER', 'UNUSED');  // the uid in this entry is for the payer
define('E_PAYEE', 'UNUSED');  // the uid in this entry is for the payee (this type is not used in txs_prime or txs)
define('E_DONATION', 2);  // this is a transaction-related donation (e.g., roundup and Food Fund donations)
                          // the precise nature can be determined from uid2
define('E_REBATE', 3); // discount rebate to customer
define('E_USD_FEE', 4); // fee from customer to merchant for buying (or selling?) Common Good credit
define('E_BANK', 5); // CG account transfers to or from the member's bank
define('E_XFEE', 6); // transfer fee to merchant (UNUSED)
define('E_TYPES', 'bank_only prime donation rebate usd_fee bank');

$GLOBALS['emailSubjects'] = array(
  'nonmember' => t('%PROJECT invitation followup'),
  'partner-signup' => t('new solar subscription'),
  'partner-contract' => t('Continue your Community Solar sign-up'),
  'partner-end' => t('Your Community Solar sign-up is complete'),
  'partner-solar1end' => t('Your Community Solar sign-up is complete'),
  'partner-report' => t('Solar Signup COMPLETE'),
  'partner-solar1report' => t('Solar Signup COMPLETE'),
  'verify' => t('%PROJECT NEXT STEP: Verify your email address.'),
  'verify-co' => t('%PROJECT NEXT STEP: Verify your company email address.'),
//  'verify-trial' => t('%PROJECT NEXT STEP: Verify your company email address.'),
  'password-reset' => t('New %PROJECT PASSWORD'),
  'tell-staff' => t('%PROJECT STAFF ALERT: %topic'),
  'notices' => t('%PROJECT Notices for %qid for %range'),
  'message' => t('%PROJECT %topic'),
	'annual-gift' => t('your chosen annual gift to %PROJECT'),
);

$GLOBALS['topics'] = array( // topics for 'message' email
  'confirm invite' => t('Confirm invitation to %otherName'),
  'not confirmed' => t('Your %PROJECT membership requires confirmation.'),
  'new invoice' => t('INVOICE from %otherName'),
  'account suspended' => t('Account SUSPENDED'),
  'account funded' => t('Account FUNDED'),
  'approved' => t('Account APPROVED!'),
  'join accounts' => t('Invitation to Create a Joint Account'),
  'force without perm' => t('Unauthorized transaction'),
);

if (TRUE) { // can't use if(u\test()) here (this block should be moved somewhere)
  global $specialSubs;
  foreach (explode(' ', B_PERMISSIONS) as $k) {
    $perm = constant('B_' . strtoupper($k));
    $specialSubs["%can_$k"] = $GLOBALS['share permissions'][$perm - B_RELATED];
  }
  $specialSubs["%can_none"] = $GLOBALS['share permissions'][0];
  $specialSubs['%chk'] = '&#10004;';
}

/**
 * Extract a value from the $configRay array (read in from a configuration file or from the database part of it).
 * @param string $k: the key in the configuration array, eg 'db'
 * @param mixed $default: (OPTIONAL) the value to return if the key is not in the array
 */
function config($k, $default = NULL) {
  global $configRay;
  if (isset($configRay[$k])) return $configRay[$k];
  if (isset($default)) return $default;
  if ($k != 'db' and empty($configRay['db'])) $k .= ' in the database definition';
/**/  die("The config file is missing an entry for $k.");
}
