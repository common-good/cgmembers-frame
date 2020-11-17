<?php
/**
 * @file
 * Definitions required for any part of the Common Good system, including app testing
 */

const R_VERSION = '3.55x'; // software version
define('R_ROOT', __DIR__);
define('SHARED_DIR', R_ROOT . '/../..'); // for logs and other protected info, persistent across updates

global $db_name, $cryHows;
global $configRay; // the configuration array currently being examined -- used only in this file

$configFlnm = SHARED_DIR . '/config/config.json';
$configRay = (array) json_decode(utf8_encode(file_get_contents($configFlnm)));
/**/ if (!$stage = config('stage')) die('There is no valid config file ' . $configFlnm);

define('isDEV', $stage == 'development'); // (used in settings file and elsewhere)
define('isSTAGE', $stage == 'staging');
define('isPRODUCTION', $stage == 'production');
/**/  if (!isDEV and !isSTAGE and !isPRODUCTION) die('Invalid value for stage in configuration file.');

const US_COUNTRY_ID = 1228; // United States (used in settings file and elsewhere)
const CA_COUNTRY_ID = 1039; // Canada
require_once __DIR__ . '/cg-settings.inc'; // custom settings for this community's CG server
const TEST_INVITE_KEY = '111';

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
define('R_INVITE_KEY', basename(strstr($_SERVER['REQUEST_URI'], '?', TRUE)) == 'tests' ? TEST_INVITE_KEY : config('inviteKey'));
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
  'collation' => 'utf8mb4_general_ci',
  'host' => $db_host,
  'port' => $db_port,
  'driver' => $db_driver,
  'prefix' => '',
  'pdo' => [PDO::MYSQL_ATTR_INIT_COMMAND => 'SET NAMES utf8mb4'],
];

define('PROJECT', t('Common Good')); // project name
define('CG_DOMAIN', 'CommonGood.earth');
define('RCREDITS', t('Common Good credit')); // don't use %PROJECT here
const BANK_DBA = PROJECT;
define('PROJECT_ID', str_replace(' ', '', strtolower(PROJECT)));
const PROJECT_LOGO = 'logo80.png';

const SCRIPTS_TOP = 'x/jquery-3.3.1.min x/jquery.once x/jquery-ui.min x/bootstrap.min x/spin.min x/ladda.min x/ie10-viewport-bug-workaround x/jquery.confirm.min x/alert';
//const SCRIPTS = 'form goo-jsapi misc charts chart-data download croppic crop-setup ballot x/bootstrap.slider.min countries print-country focus-on x/typeahead.bundle.min suggest-who on-submit chimp followup-email get-ssn change-ctty advanced-dates advanced-prefs bank-prefs which funding-criteria signup prejoint invite-link gift drupal user extend-settings'; // all possible scripts, in order (after SCRIPTS_TOP)
const SCRIPTS = 'x/bootstrap-toggle.min parse-query misc croppic crop-setup x/bootstrap-slider.min ballot countries x/typeahead.bundle.min on-submit scraps drupal user'; // all possible scripts, in order (after SCRIPTS_TOP). drupal and user are for password.

const DEV_PW2 = 'devPassword#two!'; // second password on DEV machine (len must be 16, 24, or 32)
const DEV_VKEYPW = 'wt38BqWMslj3uUHUOto7fNi5ExL9CC7S7ffjn_q6zCY'; // password for very secure key, on DEV machine
const DEV_ADMIN_PASS = '123'; // admin password when isDEV
const DEFAULT_PROXIES = '2, 3'; // account record IDs for place-holder proxies
const TRACK_DELETED = 'r_invoices r_photos r_relations r_usd users'; // remember rows in these tables if we delete them
const DELETED_BY_VIEW = 'tx_hdrs tx_entries tx_disputes'; // these tables are actually views on the _all version of
                                                             // themselves, and deletion is just by setting "deleted"
const DEV_ONLY_MSG = 'only on development machine';

$host = strtolower(@$_SERVER['HTTP_HOST']);
if (!isDEV) if (mb_strlen($host) > 30 or !preg_match('/^\[?([a-zA-Z0-9-:\]_]+\.?)+$/', $host)) {header('HTTP 400 Bad Request'); exit();}
const R_PATH = BASE_URL . '/rcredits';
global $rUrl; $rUrl = R_PATH;
$parts = parse_url($base_url);
$base_path = @$parts['path'] . '/';
$base_root = mb_substr($base_url, 0, mb_strlen($base_url) - mb_strlen(@$parts['path'])); // everything until first slash after "scheme://"

define('NOT_PRODUCTION', !isPRODUCTION); // safer to use than !isPRODUCTION (in case of typo)

// time periods are defined here because this script uses them
const MIN_SECS = 60;
const HOUR_SECS = 60 * MIN_SECS;  // 3600
const DAY_SECS = 24 * HOUR_SECS;  // 86,400
const WEEK_SECS = 7 * DAY_SECS;   // 604,800
const WEEK7_SECS = 52 * DAY_SECS; // 4,492,900
const MONTH_SECS = 30.4368 * DAY_SECS;  // 18,408,176.64
define('QUARTER_SECS', round(3 * MONTH_SECS));  // 55,224,530
define('YEAR_SECS', round(12 * MONTH_SECS));    // 220,898,120
const FLOOR_DAYS = 183; // how many days to consider for credit line calculation
const RECENT_DAYS = 60; // back how long is "recent" (don't change this without recreating r_stats data)
const RECENT_SECS = RECENT_DAYS * DAY_SECS;

const REGION_DOMAIN = R_SERVER_ID . '.' . CG_DOMAIN;
const EMAILX = 'example.com'; // domain for unknown, placeholder, test, or example emails
const GFLD_LAT = 42.3791167; // Greenfield, MA latitude (in tests)
const GFLD_LON = -73.2819463; // Greenfield, MA latitude
const GFLD_ZIP = '01301'; // Greenfield, MA zipcode
define('NULL', NULL); // for tests (cannot be const)

// Common Good Finance email and other data
const CGF_LEGALNAME = 'Society to Benefit Everyone, Inc.';
const CGF_PHONE = '+1 413 628 1723';
const CGF_ADDRESS = '48 Buckland Road';
const CGF_CITY = 'Ashfield';
const CGF_POSTALADDR = 'PO Box 21, Ashfield, MA 01330-0021';
const CGF_EIN = '205280780';
const SLOGANS = 'for truth and justice,for community power,for community democracy,for community investment,for a sustainable future,to empower communities,for community abundance,for a better future,for economic democracy,for democratic economics';
define('R_LAUNCH', strtotime('12/15/2013')); // original launch date
const EMAIL_DOMAIN = CG_DOMAIN; // replace this with CG_DOMAIN when ready
const SYS_EMAIL = 'system@' . EMAIL_DOMAIN; // old address so we don't get marked as spam (later at REGION_DOMAIN)
const BOUNCE_EMAIL = 'bounce@' . EMAIL_DOMAIN; // old address so we don't get marked as spam (later at REGION_DOMAIN)
define('PUBLIC_KEY', dirname($_SERVER['DOCUMENT_ROOT']) . '/.publicKey');
define('PRIVATE_KEY', dirname($_SERVER['DOCUMENT_ROOT']) . '/.privateKey');
const ANY = 'any'; // accessible to anyone (if you change this, rebuild menus immediately)
const SVAR_HEADER = 'rcredits_'; // session variable header
const TEST_PDF_FLNM = 'testOutput.pdf'; // pdf output during testing
const QR_DOMS = 'RC4.ME RC2.ME'; // domains that redirect to staging and production, respectively (for QR codes)

const TEN99MISC = 'A'; // code for 1099-MISC
const TEN99B = 'B'; // code for 1099-B
const TEN99K = 'MC'; // code for 1099-K
const MINIMA1099 = 'X:20000/200, MA:600/1, VT:600.01/1'; // no 1099-K for gross/txCount under this amount (by state)
const MINIMA1099MAVT = 600; // 1099K minimum in MA and VT (for display)

const NBSP = '&nbsp;'; // non-breaking space
const R_BULLET = '&#8226;'; // bullet in emails (where CSS is difficult)
const STATS_GRAIN = 10; // granularity of statistics, in days
define('INFLATION_DESC', t('inflation adjustment'));
const COMPANY_FREEZE = FALSE; // companies cannot spend rewards yet
const R = '<span class="r">r</span>';
const US = '<span class="us">us</span>';
const SUBMENUS = 'community invest history co settings sadmin';

/// const GAME_COMPANIES = '.,.,.,.,Gift Shop,Religious Institution,For Rent,Empty,Vacant Lot,Land,Cinema,Printing & Office Supply,Car Repair,Health Club/Gym,Restaurant,Hardware & Supply,Grocer,Farm,Food Processing,Department Store,Electric Company,Oil Company,Town Government,Community College,Hospital';
const ITEM_FLDS = 'special title subtext midtext midtext1 midtext2 midtext3 posttext buttons list back'; // names of common fields that are wrapped automatically in w\item()
const R_DATE_FLDS = 'start end start created completed deposit sent invited giftDate signed login access joined yesterday dob when sold deleted now'; // date field names for testing and administrative display
const CO_FLDS = 'coType website description shortDesc selling coFlags gross serviceArea employees payrollStart payrollEnd staleNudge contact'; // account fields stored in r_company
const DATA_FLDS = 'legalName shortName newAccount by calling wants giftPot giftCoupons notifyNowBy signupCo stepsDone stepsLeft task achMin pinThreshold lastiCode tenure moves mediaConx getAppData appData snap fundBy isSavings idVerifiedBy partnerInfo otherAssets otherLiabilities fyEnd'; // fields stored in the account's "data" array field (serialized) -- UNUSED: stepsDone
const MASTER_NUMERIC_FLDS = 'minimum achMin savingsAdd saveWeekly';
const MASTER_FLDS = MASTER_NUMERIC_FLDS . ' hasBank bankAccount last4bank refill'; // only in master, for joint accounts
const R_MONEY_FLDS = 'balance rewards floor giftPot committed'; // financial amount fields in account record
const JOINT_FLDS = R_MONEY_FLDS . ' savings'; // add these together for joint accounts (except balance)
const PSECURE_FLDS = 'phone email'; // personal fields that get encrypted in a searchable way (P)
const VSECURE_FLDS = 'federalId dob bankAccount altId'; // very secure fields (always soon pgp-encrypted within Acct::vsecure)
const SECURE_FLDS = 'pw2 vKeyPw ssnData address postalAddr faxetc emailCode cardCode cardCode2 pin question answer idProof oneTimePass last4bank yob smtPass'; // non-personal encrypted fields (must be complete list)
const R_HIDE_CHANGES = R_MONEY_FLDS . ' pw2 vKeyPw ssnData changes init login access activated data secure vsecure stepsDone steps stepsLeft lastiCode pass oneTimePass notes picture photo thumb trust risk risks stats getAppData appData special last4bank lastip tickle payrollStart payrollEnd'; // do not record changes to these fields: anything often-changed (automatically or not)
const R_APP_DATA_GETS = '!log !device !members !txs '; // data requests from the rPOS app
//  const R_AVAIL_BUY = '(balance-IF(u.:IS_DEBT, floor, GREATEST(floor, 0)))'; // balance available to spend
const R_SHARE_MAX = 50; // minimum sharing percentage
const R_SHARE_DFT = 25; // default sharing percentage
define('R_SHARING_MSG', t('sharing rewards with CGF'));
const ICLUB_TITHE = .1; // fraction of profits that go to the community and the common good
const USERNAME_MAX_LENGTH = '100'; // max length of "name" field

const LOCUS_CITYST = -1; // means locus is city/st (so don't use lat/lon)
const LOCUS_PARTZIP = -2; // means locus is partial or whole zipcode (so don't use lat/lon)
const LOCUS_BAD = -3; // means locus is bad format
const LOCUS_ZIP = -4; // means locus cannot be found so just zip is returned
//    $zippat = "(?:$st,?\\s?)?(\\d{5})(-\\d{4})?";
const LOCPAT_PARTZIP = '|^\\d{1,5}$|';
const LOCPAT_CITYST = '|^([^,/\\s]+[^,/]+)\\s*[,/]?([a-zA-Z]{2})$|';

const R_ADMIN_CAN_MANAGE_CTTYS = TRUE; // should be false once communities can manage themselves
const R_CRON_PERIOD = 5*60; // how many seconds between cron runs (must divide an hour evenly)
const R_MAX_TASK_TIME = 1*60; // what's the longest a queued cron task might take
const R_NOLOG = SECURE_FLDS . ' ' . VSECURE_FLDS . ' data secure vsecure secret oneTimeLoginUrl password pass pass1 pass2 vKeyE newPass external_input form_build_id bad args'; // don't log these (external_input may contain passwords and is duplicated as individual fields)
const R_NOLOG_FORMS = 'Admin Test Handy footer accounts history settings settings/icon community Console Summary Php'; // don't log visits to these forms
const R_ANON_FORMS = R_NOLOG_FORMS . ' Signup Signin PageNotFound Sinx Pass Empty TestRecovery ChangePassword Help Community Democracy Agreement CttyFunds Reports Flags Game Menu I ResetPassword Profile Accept Buy HandleInvoice FindCo'; // no login required
const SSN_ERR_INDEX = 'CGerror'; // where to put/find error messages in ssnData
const BY_CASH = '(cash)'; // app includes this in description, for a purchase of CG credit by cash
const BY_CHECK = '(check)'; // app includes this in description, for a purchase of CG credit by check
const BY_CARD = '(card)'; // app includes this in description, for a purchase of CG credit by credit card
define('R_ON_FILE', t('(on file)'));
// (NO) const DEPOSIT_SQL = '(amount>0 XOR txid<0)'; // SQL fragment for "transfer is from bank to CG"

const DOLLAR_POOL = 1; // uid of dollar pool account
const CANONIC_ACCTS = 512; // number of canonic accounts (admin/dollar pool, placeholders,...)
const CANONIC = 'BETWEEN 0 AND ' . (CANONIC_ACCTS - 1);
const AGGREGATE_MIN = 1; // minimum number of members to show aggregate data

const CG_ADMIN_UID = 1;  // uid of administrator account
const PLACEHOLDER_1_UID = 2;  // uid of first placeholder
const PLACEHOLDER_2_UID = 3;  // uid of second placeholder
const CG_ROUNDUPS_UID = 129;  // Donations start at 128 (128 used for general donations)
const CG_CRUMBS_UID = 130;  //
const CG_SERVICE_CHARGES_UID = 192;
const MINIMUM_BANK_UID = 256;
const BANK_IN_ID = 256;  // Bank accounts start at 256
const BANK_OUT_ID = 257;  //
const MAXIMUM_BANK_UID = 511;

const R_TEMP_NAME_PREFIX = 'unknown ';
const CO_PHOTO_FACTOR = 3; // make company photos this many times as big as individual photos
const MAX_UPLOAD_SIZE = 4 * 1024000; // maximum upload size in bytes
const MAX_ID_SIZE = 10; // max ID size in MB

//define('NOTICE_OFTENS', t('t:Immediate by Text,i:Immediate by Email,d:Daily,w:Weekly,m:Monthly'));
define('NOTICE_OFTENS', t('i:Immediate by Email,d:Daily,w:Weekly,m:Monthly,n:Never'));
define('NOTICE_TYPES', t('offer:Offers,need:Needs,tip:Tips,in:Transfers IN,out:Transfers OUT,misc:Other messages'));
define('POST_NOTICE_TYPE_KEYS', 'offer need tip');
const NOTICE_DFTS = 'offer:d,need:d,tip:w,in:w,out:d,misc:d';
const SEND_NOW = 'i';
const SEND_IN = 'in';
const SEND_OUT = 'out';
const SEND_MISC = 'misc';

const VIDEO_URL = 'https://cg4.us/video'; // Nuts & Bolts video
const TRANSLATE_URL = 'https://translation.googleapis.com/language/translate/v2'; // google translate API URL
const GEOCODE_URL = 'https://api.opencagedata.com/geocode/v1/json?'; // use in acct::setLocus()
const MI_PER_DEGREE = 69; // varies from 68.703 at the equator to 69.407 at the poles
const NEIGHBOR_MAX = 40; // how far away (in miles) can a member see names of other individual members?
const DISTANCE_BETWEEN = 'DEGREES(ACOS(LEAST(1.0, COS(RADIANS(latitude))
  * COS(RADIANS(:lat))
  * COS(RADIANS(longitude - :lon))
  + SIN(RADIANS(latitude))
  * SIN(RADIANS(:lat))))) * ' . MI_PER_DEGREE; // distance from other, in miles (requires pre-processing)
const NO_DISTANCE = '(latitude=0 AND longitude=0) OR (:lat=0 AND :lon=0)';

const TEXT_BUTTON_STYLE = 'display:inline-block; background-color:darkgreen; border-radius:5px; border:1px solid forestgreen; color:white; font-family:Arial; font-size:17px; padding:8px 15px; text-decoration-line:none;';
const R_URL_SLASH = '%_%'; // Drupal gets confused by slashes in a url, even if urlencoded
const R_EXCHANGE_LIMITS = '1:5000,7:20000,52:80000,365:500000'; // per day, week, 7-week, year
const STALE_NUDGE = 7; // default number of days to wait before reminding a customer to pay an invoice
const R_INVITE_DAYS = 30; // number of days to accept invitation to membership
//const R_MAXR_DEFAULT = -1; // suggested maximum amount to hold in CG credit (-1 = no limit)
const R_CARDCODE_LEN = 14; // length of rCard security code (long to deter easy copying, max 14 for small QR)
const R_CARD_COST = 5; // cost to order a company or replacement rCard
const ORGANIZER_GRANT_MIN = 10; // minimum number of individuals to recruit, to be eligible for a grant
const ORGANIZER_GRANT_MAX = '$5,000'; // maximum organizer grant amount
const TICKLE_WELCOME = 2 * 7; // number of days to wait after activation, before followup
const TICKLE_DELAYED = 6 * 7; // number of days to wait after activation, before followup if no gift or no CGC
const OFTENS = 'W:Weekly,M:Monthly,Q:Quarterly,Y:Yearly,X:Once (now)';
const OFX_TX_FLDS = 'trnType dtPosted trnAmt fitId name memo';
const OFTENS_SECS = ['W' => WEEK_SECS, 'M' => MONTH_SECS, 'Q' => QUARTER_SECS, 'Y' => YEAR_SECS];

const ICARD_EXPIRES = 14; // how many days until invitation card expires, after account is activated
const ICARD_CODELEN_MIN = 10; // minimum length for an invitation code
const ILEN_REGION = 15; // 3 letters (radix 26-32 -- in other words, 1-6 can be used in region also)
const ILEN_TAIL = 19; // 3-4 letters (radix 26)
const ILEN_SECURITY = 16; // 3 alphanumerics (radix 35)
const ILEN = ILEN_REGION + ILEN_TAIL + ILEN_SECURITY;
const ILEN_DIV = 25; // this results in two 5-digit codes (otherwise the integer gets too big)

// codes for iCode field (invitation code)
const IBY0 = -2; // lowest IBY number
const IBY_FORM = -2; // invited by form (including companies) (the ONLY pre-confirmed signup)
const IBY_LATE = -1; // invited by form, but accepted late (needs to be confirmed)
const IBY_SELF = 0; // signed up without an invitation/confirmation yet
const IBY_EMAIL = 1; // invited by email (this invitation # is re-usable, but needs to be confirmed)
const IBY_ICARD = 2; // >=this is invitation number, also needing confirmation
const IBYS = 'form late self email icard';

// notification preferences
$GLOBALS['notifyWhens'] = t('N:Never,I:Immediately,D:Daily,W:Weekly,M:Monthly');
$GLOBALS['notifyNowBys'] = t('E:Email,T:Text message,B:Both'); // how to send immediate messages
const N_DEFAULTS = 'MMNIIMDIDWD';
const N_EXPLICIT_OUT = 0; // manual payment or transfer out
const N_AUTO_OUT = 1; // automatic payment made
const N_AUTO_IN = 2; // auto-refill initiated
const N_IN_DONE = 3; // transfer from bank account complete
const N_INCOME = 4; // payment received
const N_INFLATION = 5; // inflation adjustment received
const N_REWARD = 6; // other incentive reward received
const N_OTHER_IN = 7; // fine, grant, loan, or investment received
const N_CHANGED = 8; // transaction/invoice edited, canceled, or disputed
const N_NUDGE = 9; // your invitee needs a nudge
const N_SUGGESTION = 10; // suggestion from the system

define('R_WHYS', t('goods and services,exchange of US Dollars or other currency,loan/reimbursement/etc.')); // . R_SHARING_MSG); // transaction purposes
const FOR_GOODS = 0; // index into R_WHYS for goods
const FOR_USD = 1; // index into R_WHYS for USD exchange
const FOR_NONGOODS = 2; // index into R_WHYS for loan, etc.
const SELF_CREDIT = 'credit'; // (string) indicates (internally and in invoice data) a purchase of credit for oneself
//const FOR_SHARE = 3; // "sharing" gift to CGF (comes out of rewards)
//const FOR_INVESTMENT = 4; // only in invoices (use B_INVESTMENT in txs)
//const FOR_STAKE = 5; // only in invoices (use B_STAKE in txs)

const R_CHUNK = 10; // chunk size for buying CG credit (sales amounts are a multiple of this)
const R_MIN_DEFAULT = 100; // suggested minimum amount to hold in CG credit
const R_ACHMIN = 10; // minimum granularity of withdrawals from bank account
const R_ACHMIN_DEFAULT = 50; // suggested achMin
//const R_ACH_TRIES = 3; // how many times to retry an ACH
const CASHOUT_MIN = 100; // minimum dollar amount to cash out automatically
const VERIFICATION_DELAY = 3; // number of days to wait after requesting a zero transfer, before requesting a real transfer

const R_SUGGEST_BUMP = 1.25; // suggested minimum multiplier, so as not to overdraw
const CO_ASPECT = [4, 3]; // width-to-height ratio for commercial profile picture
const FACE_ASPECT = [3, 4]; // width-to-height ratio for personal profile picture
const R_PHOTO_WIDTH = 300; // rCard photo width, in pixels
const R_PHOTO_HEIGHT = 400;
const R_PIXEL_FACTOR = 200; // what to multiply the aspect ratio components by, to get pixel dimensions
const THUMB_FACTOR = 12; // what to multiply the aspect ratio by, for thumbnail photo
const R_INDIVIDUAL_GIFT = 50; // suggested donation for individuals
const R_COMPANY_GIFT = 250; // suggested donation for companies
const R_BUYS_BEFORE_BONUS = 1; // number of purchases required before signup bonus (should probably be 3)
const R_REGION_MAX = 3000000000; // maximum uids in a region (46k,3T ~sqrt(PHP_INT_MAX))
const R_ACCT_REGION = '(-FLOOR((uid-1)/' . R_REGION_MAX . ')-1)'; // SQL region expression
const R_SIGNIN_HOURS = 24; // window for signing in after registering or resetting password
const R_MEMBER_MARK = '.'; // divides region from specific identifier in member IDs
//const R_AGENT_MARK = ':'; // divides region from specific identifier in relational IDs
const AGT_MARK = '-'; // divides main QID from agent code
//const R_AGENT_URL_MARK = '-'; // use this as a substitute for R_AGENT_MARK in URLs (else domain forwarding fails)
//const R_MARKS = '[\\' . R_MEMBER_MARK . '\\' . R_AGENT_MARK . '\\' . R_AGENT_URL_MARK . ']'; // any of those
const R_ANCIENT_EFT = 10; // how long, at most, an EFT might take (transferring funds to/from bank)
const R_REQUIRED = ' <span class="form-required" title="This field is required.">*</span>';
const R_CC_RATE = 2.5; // typical credit card percentage rate
const R_CODE_LEN = 32; // length of most random identifiers
const R_DUPTX_MINS = 4; // number of minutes before a duplicate transaction is allowed
const HAS_XFEE = "data LIKE '%s:4:\"xfee\";%'"; // sql to recognize a transaction that has an exchange fee
const R_DEPOSIT_RETURN_FEE = 20; // what we charge members for a second bounced transfer
const TRIALCO_AMT_LIMIT = 1000; // maximum payments received by a trial (dependent company) account
//const TRIALCO_DAY_LIMIT = 14; // maximum number of days for a trial company
const EQUIP_DEPOSIT = 50; // dollar cost of equipment deposit
const MATURITY = 21; // minimum age to have a credit limit
const SELLING_MAXLEN = 25; // maximum length of any short description of something being sold
const RENEW_ADVANCE_DAYS = 7; // how many days in advance to ask people to renew something annual

// Filenames
const R_TEMPLATE_DIR = '/templates';
const R_EMAIL_FRAME = 'email-frame';

const R_PICTURE_DIR = SHARED_DIR . '/cgPhotoTemp/';
//const R_PROOF_DIR = '/proofs/'; UNUSED
const R_DFT_PICTURE = R_ROOT . '/images/no-photo-available.jpg'; // default profile picture
const R_DFT_PICTURE_WIDE = R_ROOT . '/images/no-photo-available-wide.jpg'; // default profile picture
const R_NOT_VALID = R_ROOT . '/images/not-valid.png'; // says rcard is not valid for purchases
const LOG_DIR = SHARED_DIR . '/cgLogs/'; // where to store log files
define('LOG_FLNM', LOG_DIR . date('Ym') . '.log');
define('SQL_LOG', LOG_DIR . date('Ym') . '.logsql');
const PHOTOID_DIR = R_PICTURE_DIR . 'photoids';

// account type bits
define('CO_TYPES', t('personal, sole proprietor, partnership, LLC, private corporation, publicly-traded corporation, nonprofit, government, co-operative, unincorporated association or club, estate, trust, custodian'));

const CO_PERSONAL = 0; // phase out (use type field in r_company instead)
const CO_SOLE_PROPRIETOR = 1;
const CO_PARTNERSHIP = 2;
const CO_LLC = 3; // used in testing
const CO_NONPROFIT = 6;
const CO_CUSTODIAN = 12;

// Company Account flags (bits)
const CO_SELLCG = 0; // company sells Common Good credit
const CO_FLIP = 1; // flip the normal Donate vs Buy Credit button choice

const CO_PRIVATE = 13; // company is not to be listed publicly (on promo site, etc.)
const CO_ATM = 14; // can trade cg credit for cash, even if balance is less than floor

const APP_CANS = 15; // company permission bits when signed OUT: bits 15-21
const CO_CHARGE = 0;
const CO_UNDO = 1;
const CO_R4USD = 2;
const CO_USD4R = 3;
const CO_REFUND = 4;
const CO_BUY = 5;
const CO_u21 = 6; // unused
const CO_MANAGE = 7; // this bit is not actually stored here, but pass B_MANAGE to the app in this position
const APP_CAN_BITS = 7; // number of bits for permissions (each: signed in and out)
const APP_AGENT_CANS = APP_CANS + APP_CAN_BITS; // company permission bits when signed IN (to be limited by agent perms)
const APP_CAN_MASK = (1 << (APP_CAN_BITS + 1)) - 1; // the CANS bits -- signed-IN permission bits are 22-28

const CO_FAST = 29; // (deprecated as of app v 2.16) fast checkout -- never ask for photo Id (set by cAdmin)
const CO_TIP = 30; // ask customer what percent tip to add

const CO_APP_DFTS = '0 1 7 8 11 12'; // set these app bits by default (R4USD only if chosen in signupco.inc)

const APP_CAN_BIT_NAMES = 'charge undo r4usd usd4r refund buy';
define('APP_CAN_BIT_DESCS', t('charge customers,undo last transaction,give %RCREDITS for USD,give USD for %RCREDITS,refund any amount,make payments'));

const FUNDBY_BANK = 0; // radio button number for bank connection
const FUNDBY_PAY = 1; // paid by employer
const FUNDBY_CASH = 2; // buy with cash at a participating business

// Membership steps
const S_ALL = 'signup, verifyemail, verifyid, ssn, agent, fund, contact, agree:community/agreement, donate:community/donate, invite:community/invite, proxies, preferences, company, photo, discount:community/discount, backing:community/backing, stepup:community/stepup, crumbs:community/crumbs, work, partnersteptwo, partnerend, food, wantsPrint, printed'; // where to go to complete each step (unordered) -- add new steps to the end
const S_PERSON = 'signup verifyid fund preferences verifyemail';
const S_CARD = 'photo contact donate crumbs proxies work backing stepup invite';
const S_CO = 'signup verifyemail fund contact backing photo donate company crumbs discount';
const S_CO2 = 'agent contact backing photo donate company crumbs discount';
const S_SELL = 'ssn contact crumbs'; // keep this for when an individual member goes over the limit for no 1099
const S_VOICE = 'contact donate proxies work backing invite'; // add comment
const S_PARTNER = 'signup fund partnersteptwo partnerend';

const S_MEM_TASKS = 'card co'; // followup tasks for members
const S_PTASKS = 'person card sell voice partner';
const S_COTASKS = 'co co2 partner';

const S_OPTIONAL = 'discount stepup'; // optional steps

// Permissions, options, and flags (flags field in account record)
// If we need more bits, create a flags2 field and make the B_ defs negative or 32-63 for setBit
// for documentation of IS_OK etc. see u\SUBS()
const B_MEMBER = 0; // fulfilled membership requirements (ready to be approved).
const B_CONFIRMED = 1; // invitation has been confirmed (in advance by form invite OR explicitly OR implicitly by newb first buying from the inviter in person)
const B_OK = 2; // can use CG for real (set by admin when ready). Set FALSE to freeze ctty funds (NYI)
const B_CARDED = 3; // card has been printed and mailed
const B_IDED = 4; // a cashier has verified the member's government-issued photo ID
const B_REFILL = 5; // refill the account automatically (up to a target amount)
const B_CO = 6; // company account
const B_DRAWS = 7; // this account draws on one or more others
const B_ROUNDUP = 8; // round payments up to nearest dollar, donate the cents to the community fund
const B_NONUDGE = 9; // don't send reminders
const B_NOSEARCH = 10; // user wants to be not findable by other members, by phone or email
const B_DEPENDS = 11; // dependent account (can't change settings, federalId is sponsor's)
const B_BANKOK = 12; // connected bank account has been verified
const B_u13 = 13; // 
const B_u14 = 14; // 
const B_CONTACTABLE = 15; // okay for members to contact this member about their calling
const B_PARTNER = 16; // is a signup partner
const B_PAPER = 17; // will not accept electronic statements
const B_SECRET = 18; // don't let merchants tell me my balance on request
const B_UNDERAGE = 19; // too young to have a credit line or DOB empty (needed because dob is accessible only to superAdmin)
const B_DEBT = 20; // member chose to permit sub-zero balances
const B_REINVEST = 21; // member chose to automatically reinvest investment club dividends and inflation adjustments
const B_SAVINGS = 22; // the connected bank account is a savings or money market account
const B_CASHOUTW = 23; // cash out automatically every 7 days
const B_CASHOUTM = 24; // cash out automatically every 30 days
const B_ICLUBQ = 25; // interested in investment club
const B_OADMIN = 26; // can moderate the Offers/Needs/Tips board in a community
const B_ADMIN2 = 27; // is an overall admin assistant or bank partner or government regulator (read-only except notes)
const B_CADMIN2 = 28; // is a community sub-administrator (read-only except notes)
const B_CADMIN = 29; // is a community administrator
const B_ADMIN = 30; // is a system administrator -- can manage server, including assigning ctty admins, but cannot manage communities or individual accounts (except temporarily)

const B_MAX = 62; // maximum bit number
const B_ADMINABLE = 'member confirmed carded oAdmin cAdmin cAdmin2 nonudge partner iclubq cgc invites rewardy noneg'; // for cAdmin to set
const B_LIST = 'member confirmed ok carded ided refill co draws roundup nonudge nosearch depends bankOk u13 u14 contactable partner paper secret underage debt reinvest savings cashoutW cashoutM iclubq oAdmin admin2 cAdmin2 cAdmin admin';
const CLOSED = 'flags&(1<<' . B_OK . ')=0 AND activated'; // this must be enclosed in () if negated or ORed

// community-specific flags
const B_CGC = 0; // ctty: is an official CGC (currently has no effect)
const B_UP = 1; // ctty: is operational (up) (see also B_CTTYUP)
// B_CO (see above)
const B_INVITES = 3; // require invitations
const B_REWARDY = 4; // community issues rewards explicitly
const B_NONEG = 5; // temporary ban on accounts going negative (set automatically in r\stats)
const B_CTTY_BITS = 'cgc up co invites rewardy noneg';
const B_ADMIN_BITS = 'member:cgc,confirmed:up,co:co,u3:invites,ided:rewardy,refill:noneg';

const R_CTTY_BENE = 'cttyBuys+signups+rebates+bonuss+inflations+givebacks+grants+loans+invests+fines+tradeOut'; // benefit (ever) to the community (fines and givebacks are negative)

// Relationship flags
const B_EMPLOYEE = 0; // is employed by main account
const B_OWNER = 1; // owns the main account
const B_CUSTOMER = 2; // is a customer of main account
const B_AUTOPAY = 3; // preapproves any invoice from main account
const B_r4 = 4; //  by main account
const B_r5 = 5; //  by main account
const B_r6 = 6; //  by main account
const B_DRAW = 7; // can draw funds from main account (same number as B_DRAWS in the users table)
const REL_FLAGS = 'employee owner customer autopay r4 r5 r6 draw';
const REL_FLDS = 'main other otherNum permission code';
const REL_DATA_FLDS = 'fullName email phone address city state zip address2 city2 state2 zip2 partner partnerCode customer source qid country postalAddr'; // pseudo-fields typically stored in data field

// Relationship permissions used in calls to r\Acct::can() -- stored (less B_RELATED) in r_relations
const B_RELATED = 100; // has a relationship record
const B_NONE = 100; // no access
const B_READ = 101; // can view Transaction History
const B_SCAN = 102; // can accept CG Card payments for goods or cash for CG (or view Transaction History -- permissions are cummulative)
const B_REFUND = 103; // can undo transactions and make refunds or trade cash at point of sale
const B_SELL = 104; // can send invoices (or scan or view)
const B_BUY = 105; // can make purchase for the account (or sell or scan or view)
const B_MANAGE = 106; // can manage all aspects of the account
const B_JOINT = 107; // make this a joint account (subsuming the other, with permission) -- no companies!
$GLOBALS['share permissions'] = array(t('no access'), t('view transactions'), t('accept %PROJECT card payments'), t('give refunds & give USD for %RCREDITS'), t('send invoices'), t('make purchases'), t('manage account'), t('joint account'));
const B_PERMISSIONS = 'related read scan refund sell buy manage joint';

// Other special pseudo-bit values for r\Acct::can() and rweb.module menus
const B_MANAGING_CTTY = 120; // cadmin is actually managing a qualified account
const B_PERSON = 121; // personal account (!co)
const B_DEV = 122; // isDEV or is admin
const B_u123 = 123; //
const B_CTTYUP = 124; // account's community is operating (see also B_UP)
const B_BANK = 125; // account has a connected bank account
const B_CLOSED = 126; // account is closed
const B_SHOW_ALL = 999; // special signal to r\Acct::can() to return all permissions

// Canonic strings
define('S_REFUND', t('refund'));
define('S_USD_OUT', t('USD out'));
define('S_USD_IN', t('USD in'));

// Risk definitions (for accounts and transactions) -- suspiciousness for possible reporting to FinCEN
const K_THRESHOLD = 200; // how big an amount in a single transaction is suspicious
/*const K_DAY_THRESHOLD = 300; // how big an amount spent or received by an individual in one day is suspicious
const K_WEEK_THRESHOLD = 600; // how large an amount is suspicious in one week
const K_7WEEK_THRESHOLD = 1200; // how large an amount is suspicious in one 52-day period
const K_YEAR_THRESHOLD = 2400; // how large an amount is suspicious in one year */
const K_THRESHOLD_PCT = 5; // what percent of top accounts transaction volume per employee to flag
const K_LIMIT_FACTOR = 3; // how many times the various thresholds is a prohibited amount
const K_UNUSUAL_BUMP = 20; // what percentage 7-week increase (in or out) is suspicious
const K_BIT_MIN = .05; // lowest risk factor that sets a bit
const K_RED = 100; // lowest score to flag for investigation
const K_GREEN = -500; // threshold for an account or transaction to be unquestionably beyond reproach
const K_LINES = 20; // number of lines to show for each type of each potentially risky record (acct or tx)
const K_MISER_DONATION = 5; // minimum donation to not be a miser
const K_MISER_SHARE = 25; // minimum sharing percentage, to not be a miser
const K_CTTY_THRESHOLD = 100; // minimum size of community transactions to consider for risk analysis

// names and weights of account and transaction risks (and, for negative weights, assurance)
// weight is the number of such risks that together would make the account or transaction suspicious
// a weight of "X" means the risk is named but does not contribute to the total risk score (UNUSED)
const K_ACCT_RISKS = 'adminOk:-0.3,trusted:-4,UNUSED:-3,hasBank:-3,geography:4,cashCo:2,new:5,moves:3,rents:10,badConx:2,homeCo:20,shady:3,miser:5,UNUSED2:1,addrOff:5,ssnOff:1,dobOff:4,poBox:5,fishy:2,moreIn:2,moreOut:2,bigDay:1,bigWeek:1,big7Week:1,bigYear:2';
const K_ACCT_RISKS_EDITABLE = 'adminOk cashCo rents shady photoOff addrOff ssnOff dobOff fishy';
const K_TX_RISKS = 'txAdminOk:-0.3,redo:2,exchange:2,cashIn:5,cashOut:2,fromBank:5,toBank:2,b2p:2,p2p:2,inhouse:2,fromSuspect:3,toSuspect:3,absent:4,invoiceless:4,bigFrom:3,biggestFrom:3,oftenFrom:3,bigTo:3,biggestTo:3,oftenTo:3,offline:10,firstOffline:3,origins:16,suspectOut:2'; // names and size of transaction risks
const K_SKETCHY = 1; // ratio of risk to amount that is considered "sketchy"
const K_ADMINOK = 0; // community admin blessed the account or transaction
const K_HASBANK = 3; // account has a connected bank account
const K_ADDROFF = 14; // account's physical address is wonky (can't be geolocated or whatever)
const HAS_BANK = 'risks&(1<<' . K_HASBANK . ')'; // mysql criterion (not surrounding parens, so we can do u.:HAS_BANK)
const MASTER = '(jid=0 OR uid < jid)'; // account is the master in a joint account

/* const U_FOODY = '(SELECT 1 FROM r_industries i JOIN r_user_industries ui USING (iid) WHERE ui.uid=u.uid AND u.name<>\'foodfund\' AND i.parent=2 LIMIT 1)'; // MySQL for "the account with record ID u.uid is a food-related business" */
const FOOD_INDUSTRY = 2; // any industry with this base ancestor is food-related
const U_FOODY = '(SELECT 1 FROM r_user_industries ui JOIN ancestors a ON ui.iid=a.base WHERE ui.uid=u.uid AND u.name<>\'foodfund\' AND a.ancestor=' . FOOD_INDUSTRY . ' LIMIT 1)'; // MySQL for "the account u is a food-related business"

const T_FOODY = '(SELECT ' . U_FOODY . " FROM users u WHERE u.uid=uid2)"; // MySQL involving txs VIEW, meaning "payee is FOODY"
const CAT_FOOD = 2; // food industry (used in signup-co and/or company feature)
const CAT_RETAIL = 14; // retail industry (used in signup-co and/or company feature)

define('SHOUT_TEXT', t('"%PROJECT could have a huge positive impact on our region, if a lot of people and businesses sign up and participate."'));
define('ORG_DISCLAIMER', t(' Organizational affiliations are for identification purposes only and do not necessarily reflect the views of the organization.'));
  
const FLOW_NONE = 0; // which way can credit flow, with another account
const FLOW_TO = 1;
const FLOW_FROM = 2;
const FLOW_BOTH = 3;

// Transaction flags (for flags field in transaction records)
const B_TAKING = 0; // payee initiated the transaction
const B_DISPUTED = 1; // non-originator disputes the transaction -- signaled by existence of tx_disputes (NOT LEGAL FOR TRANSACTIONS, BUT STILL USED BY INVOICES
const B_OFFLINE = 2; // transaction was taken offline (or was forced?)
const B_SHORT = 3; // transaction was taken (offline) despite credit shortfall
const OLD_B_UNDONE = 4; // undone by another transaction -- signaled by existence of reversing transaction
const OLD_B_UNDOES = 5; // undoes another transaction -- signaled by reversesXid not null
const B_CRUMBS = 6; // monthly donation of percentage of receipts
const OLD_B_ROUNDUPS = 7; // monthly donation of rounded up cents -- no longer occurs
// B_ROUNDUP is 8 (defined above) // payer donated the change to the community fund -- not used here, 
const B_RECURS = 9; // recurring transaction
const B_GIFT = 10; // grant or gift (of any type)
const B_LOAN = 11; // community loan (UNUSED?)
const B_INVESTMENT = 12; // community investment (UNUSED?) or investment club investment
const B_STAKE = 13; // member buying or selling stake in investment club
const B_FINE = 14; // community fine (UNUSED?)
const B_NOASK = 15; // transaction was taken with ID checking OFF
const B_FUNDING = 16; // invoice has already instigated an appropriate bank transfer request
const B_FOOD = 17; // contribution to food fund
const B_PARTIAL = 18; // partial payment of invoice
const TX_FLAGS = 'offline short crumbs recurs gift loan investment stake fine noask funding food partial';

// Invoice status (txid >= TX_PAID if paid)
const TX_CANCELED = -9; // invoice canceled
const TX_NOSUCH = -4; // no such charge operation (used only in forms/buy.inc)
const TX_DELAYED = -3; // waiting for both accounts to qualify
const TX_DENIED = -2; // invoice rejected by the non-originator
const TX_PENDING = -1; // charged, waiting for confirmation
const TX_APPROVED = 0; // invoice is approved but not paid
const TX_PAID = 1; // any status this amount or more is a payment transaction record ID (xid)
define('TX_STATUSES', t('-9:canceled,-4:no such,-3:delayed,-2:denied,-1:OPEN,0:approved,1:paid'));

// Transaction channels (roughly in order of simplicity and generality of messages) (CGF)
const TX_SYS = 0;
const TX_SMS = 1; 
const TX_WEB = 2;
const TX_POS = 3; // smart phone or other computer-like device
const TX_TONE = 4; // touch tone phone
const TX_CRON = 5;
const TX_LINK = 6; // user clicked a no-signin link
const TX_AJAX = 7;
const TX_FOREIGN = 8; // user clicked a "Pay with CG" button or request an app to charge them
const TX_API = 9;
const TX_CHANNELS = 'Sys SMS Web App Tone Cron Link Ajax Foreign API'; // can't translate, because used in t() (find a solution)

// Transaction types
const TX_TRANSFER = 0; // normal fund transfer (usually for actual goods and services) -- not creating rC
const TX_BANK = 99; // used only internally, to mark bank transfers

const TX_DEFAULT_PERIOD = 90; // number of days to show in transaction history, by default
const TX_DEFAULT_INV_PERIOD = -2; // number of days to show in invoice history, by default (all of them)

const MIN_TX_ROWS = 3; // minimum number of transaction lines to show
const NORMAL_TX_ROWS = 30; // normal number of transaction lines to show
const TX_MAX_ROWS = 500; // (unused) maximum number of transactions to load at once
const TX_MAX_MOS = 18; // maximum transaction months, for display
const TX_MAX_DAYS = 30.5 * TX_MAX_MOS + 2; // maximum days in a transaction query (18 months) -- 2 for leap day & daylight
const TX_DATA_FLDS = 'undoneBy undoes force changes'; // serialized pseudo-fields in the data field of a transaction record

const DS_OPEN = 1;  // dispute not resolved
const DS_ACCEPTED = 2;  // dispute accepted and transaction reversed
const DS_DENIED = 3;  // dispute denied

const E_BANK_ONLY = 0; // bank charges and transfers between bank accounts that have no effect on CG account balances
const E_PRIME = 1;  // the main pair in the transaction (payer and payee)
const E_DONATION = 2;  // this is a transaction-related donation (e.g., roundup or rule-based donation)
                          // the precise nature can be determined from uid2
const E_REBATE = 3; // discount rebate to customer
const E_USD_FEE = 4; // fee from customer to merchant for buying (or selling?) Common Good credit
const E_BANK = 5; // CG account transfers to or from the member's bank
const E_XFEE = 6; // transfer fee to merchant (UNUSED)
const E_TYPES = 'bank_only prime donation rebate usd_fee bank rule';

const REF_LIST = 'anybody account anyCo industry group'; // values for payerType and payeeType in tx_rules
const REF_ANYBODY = 'anybody';
const REF_ACCOUNT = 'account';
const REF_ANYCO = 'anyCo';
const REF_INDUSTRY = 'industry';
const REF_GROUP = 'group';

const ACT_LIST = 'pay charge surtx redeem'; // values for action field in tx_rules
const ACT_PAY = 'pay';
const ACT_CHARGE = 'charge';
const ACT_SURTX = 'surtx';
const ACT_REDEEM = 'redeem';

const PERIODS = 'once day week month quarter year forever'; // periods and durations
define('PERIODLY', t('once daily weekly monthly quarterly yearly always'));
define('PERIODS_PLURAL', t('n/a days weeks months quarters years n/a'));

const MATCH_PAYER = -1; // Special values for from and to (which are normally uids) in tx_rules
const MATCH_PAYEE = -2;

const STEPUP_MIN = 2; // minimum number of stepup rules for a company to appear on the step-up page
define('REBATE_DESC', t('discount (rebate)'));

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
  'notices' => t('%PROJECT Notices for %qid %todayDesc'),
  'message' => t('%PROJECT %topic'),
	'annual-gift' => t('your chosen annual gift to %PROJECT'),
  'renew-backing' => t('your %PROJECT backing is ready to renew'),
  'confirm-post' => t('Confirm your post to %PROJECT Offers & Needs'),
  'confirm-message' => t('Confirm your message'),
  'post-message' => t('Response to your Offers & Needs post'),
  'post-notice' => t('recent Offers & Needs near you'),
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
