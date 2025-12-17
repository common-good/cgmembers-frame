<?php
namespace CG;
use CG as r;
use CG\DB as db;
use CG\Util as u;

/**
 * @file
 * Create the menu structure.
 */

/**
 * Return the menu.
 * @param string $submenu: (optional) a subsection of the menu structure to return RAW (unprocessed)
 * @see r\access()
 */
function menu($submenu = '') {
  $items = array(
    'dashboard' => ['norm', t('Dashboard'), 'Dashboard', 'read'],

    'tx/pay' => ['call', t('Pay'), 'Tx 1 2', 'buy ok'],
    'tx/pay/payments' => ['sub', t('Upload Payments'), 'Payments', 'co buy ok'],
    'tx/pay/payroll' => ['sub', t('Upload Payroll'), 'Payroll', 'co buy ok'],

    'tx/charge' => ['call', t('Charge'), 'Tx 1 2', 'sell ok'],
    'tx/charge/invoices' => ['sub', t('Upload Invoices'), 'InvoiceUpload', 'co sell ok'],
    'tx/charge/nonmember' => ['call', t('Charge Nonmember'), 'TxNonmember', 'sell ok'],
    
    'tx/receipt' => ['call', t('Receipt'), 'Tx 1 2', 'sell ok'],
    'tx/undo' => ['call', t('Receipt'), 'Tx 1 2', 'sell ok'],
    'tx/tip' => ['call', t('Receipt'), 'Tx 1 2', 'sell ok'],
   
    'invest' => ['norm', t('Invest'), 'Invest', 'manage ok'],
    'invest/overview' => ['sub', t('Investment Club Overview'), 'Invest 1', 'manage ok', '', t(), 'eye-open'],
    'invest/list' => ['sub', t('List Investments'), 'InvestList 2', 'manage ok investor', '', t(), 'list'],
    'invest/cashout' => ['sub', t('Handle Cashout Requests'), 'InvestCashout 2', 'manage ok iclub', '', t(), 'new-window'],
    'invest/dividends' => ['sub', t('Issue Dividends'), 'InvestDividends 2', 'manage ok iclub', '', t(), 'cloud-download'],
    'invest/propose' => ['sub', t('Propose Investment'), 'InvestPropose 2', 'ok iclub', '', t(), 'fire'],
    'invest/buy-or-sell' => ['call', t('Buy or Sell Investment'), 'InvestBuySell 2', 'manage ok iclub', '', t(), 'refresh'],
    'invest/rate' => ['call', t('Rate Investment'), 'InvestRate 2', 'manage ok investor', '', t(), 'search'],
    'invest/interest' => ['call', t('Investment Club Interest'), 'InvestInterest', ANY],

    'history' => ['norm', t('History'), 'History', 'read'],
    'history/transactions' => ['sub', t('Transactions'), 'Txs 2', 'read', '', t(), 'list'],
    'history/transaction' => ['call', t('Transaction Detail'), 'TxDetail 2', 'read'],
    'history/recurring' => ['sub', t('Automated Payments'), 'Recurring', 'read', '', t(), 'list'],
    'history/pending-to' => ['sub', t('Pending To You'), 'Invoices to', 'read', '', t(), 'log-in'],
    'history/pending-from' => ['sub', t('Pending From You'), 'Invoices from', 'read', '', t(), 'log-out'],
    'history/company-reports' => ['sub', t('Company Reports'), 'Data 2 co', 'read co', '', t(), 'signal'],
    'history/company-reports/demographics' => ['call', t('Customer Demographics'), 'Demographics', 'read co'],
    'history/statements' => ['sub', t('Statements'), 'Statements 2', 'read', '', t(), 'list-alt'],
    'history/notices' => ['sub', t('Notices'), 'Notices 2', 'read', '', t(), 'envelope'],
    'history/tax-info' => ['sub', t('Tax Info'), 'TaxInfo 2', 'read', '', t(), 'pawn'],
    'history/changes' => ['sub', t('See Changes'), 'SeeChanges', 'admSeeChanges', '', t(), 'refresh'],
    'history/statement' => ['call', t('Statement'), '2', 'read', 'statement'],
    'history/customer-statement' => ['call', t('Customer Statement'), '2', 'read', 'custStatement'],

    'community' => ['norm', t('Community'), 'Community', ANY],
    'community/endorse' => ['call', t('Invite People'), 'Endorse 2', 'manage linkable', '', t(), 'share-alt'],
    'community/invite' => ['sub', t('Invite People'), 'Invite 2', 'manage linkable', '', t(), 'share-alt'],
    'community/invite/all' => ['call', t('Shouters'), 'Shouters 3', ANY],
    'community/invite/example' => ['call', t('Invite Someone'), '', 'ok manage', 'inviteExample'],
    'community/invite/print' => ['call', t('Print Invite'), '', 'manage', 'printInvite'],
    'community/invite/invited-whom' => ['call', 'Invited Whom', 'InvitedWhom', 'manage'],
    'community/stepup' => ['sub', t('Step Up'), 'StepUp 2', 'manage linkable', '', t(), 'export'],
    'community/crumbs' => ['sub', t('Share When You Receive'), 'Crumbs 2', 'manage', '', t(), 'import'],
    'community/buy-credit' => ['sub', t('Buy Credit for Later'), 'BuyCredit 2', 'manage', '', t(), 'time'],
    'community/donate' => ['sub', t('Donate to %PROJECT'), 'Pay 2', 'manageIfIn linkable', '', t(), 'thumbs-up'],
    'community/coupons' => ['sub', t('Discount / Gift Certificates'), 'Coupons 2', 'manage', '', t(), 'usd'],
    'community/coupons/print' => ['call', t('Print Coupons'), '3', 'ok manage', 'couponsPrint'],
    'community/coupons/list' => ['call', t('List Discounts and Gift Cards'), 'CouponsList 3', 'manage'],
    'community/discount' => ['call', t('Simplified Discounts'), 'Discount', 'manage', '', t(), 'usd'],
    'community/posts' => ['sub', t('Offers, Needs, & Tips'), 'Posts 2', ANY, '', t(), 'heart'],
    'community/data' => ['sub', t('Community & Money Data'), 'Data 2', '', '', t(), 'list-alt'],
    'community/events' => ['sub', t('Democracy Events'), 'Events 2', 'ok manage linkable', '', t(), 'bell'],
    
    'community/events/event' => ['call', t('Add/Edit Event'), 'Event 3 4', 'ok'],
    'community/events/question' => ['call', t('Add/Edit Question'), 'Question 3', 'admManageVote'],
    'community/events/option' => ['call', t('Add/Edit Option'), 'Option 3', 'admManageVote'],
    'community/events/proposal' => ['call', t('Funding Proposal'), 'Proposal 3', ANY],
    'community/events/proposal-report' => ['call', t('Proposal Report'), 'ProposalReport 3', 'manage linkable'],
    'community/events/proposal-notes' => ['call', t('Proposal Notes'), 'ProposalNotes 3', 'manage'],
    
    'community/agreement' => ['sub', t('The Agreement'), 'Agreement', ANY, '', t(), 'globe'],
    'community/honors' => ['sub', t('Honored & Remembered'), 'Honors', ANY, '', t(), 'grain'],
    'community/message' => ['sub', t('Message a Member'), 'Message', 'ok refund', '', t(), 'envelope'],
    'community/message/x' => ['call', t('Decode a Secret Message'), 'Message 3', ANY],
    'community/find-company' => ['sub', t('Find a Company'), 'FindCo 2', ANY, '', t(), 'search'],
// temporarily disabled    'community/grant' => ['sub', t('Get Paid to Organize'), 'Grant', 'prose', '', t(), 'apple'],
//    'community/funds' => ['sub', t('Funds in Each Community'), 'CttyFunds', ANY, '', t(), 'usd'],
    'community/graphs' => ['sub', t('%PROJECT Graphs'), 'Graphs 2', 'admin', '', t(), 'stats'], // should be ANY, but there are bugs
    'community/balance-sheet' => ['sub', t('Balance Sheet'), 'BalanceSheet 2', ANY, '', t(), 'pause'],
    'community/flags' => ['sub', t('Risk Flags'), 'Flags 2', ANY, '', t(), 'flag'],
    'community/chart-data' => ['call', t('Chart Data'), '2', ANY, 'chartData'],
    'community/list' => ['call', t('Community List'), '', ANY, 'cttyList'], // for charts/graphs
    'community/covid' => ['call', t('COVID-19 Response'), 'Covid', ANY],
    
    'co' => ['norm', t('Company'), 'Co', 'manage co'],
    'co/cgbutton' => ['sub', t('Create CGPay Button'), 'CGButton 1', 'co manage', '', t(), 'unchecked'],
    'co/coupons' => ['sub', t('Discount / Gift Certificates'), 'Coupons 2', 'manage', '', t(), 'usd'],
    'co/accredit' => ['sub', t('Give a Customer Credit'), 'Accredit', 'manage', '', t(), 'hand-right'],
    'co/relations' => ['sub', t('Authorized Users'), 'Relations 2', 'manage -depends', '', t(), 'magnet'],
    'co/reports' => ['sub', t('Company Reports'), 'Data 2 co', 'read co', '', t(), 'signal'],
    'co/settings' => ['sub', t('Company Settings'), 'Company 2', 'manage co -depends', '', t(), 'calendar'],
    'co/grants' => ['sub', t('Grants Expected'), 'FSGrants', 'co sponsored', '', t(), 'inbox'], // see also co/fsgrants below
    'co/fsgrants' => ['call', t('Grants Expected'), 'FSGrants', 'admNonmemberTx', '', t(), 'inbox'],
    'co/sponsor' => ['call', t('Fiscal Sponsorship'), 'Sponsor', ANY],
    
    'settings' => ['norm', t('Settings'), 'Settings', 'manage -closed -depends'],
//    'settings/settings' => ['dft', t('Settings'), 'Settings', 'manage'],
    'settings/contact' => ['sub', t('Contact Info'), 'Contact', 'manage -depends', '', t(), 'user'],
    'settings/preferences' => ['sub', t('Preferences'), 'Prefs', 'manage -depends', '', t(), 'cog'],
    'settings/notices' => ['sub', t('Notices'), 'NoticePrefs', ANY, '', t(), 'envelope'],
    'settings/security' => ['sub', t('Security'), 'Security 2', 'manage -co -depends', '', t(), 'link'],
    'settings/fund' => ['sub', t('Banking'), 'Connect 2', 'manage -depends', '', t(), 'piggy-bank'],
    'settings/company' => ['sub', t('Company Info'), 'Company 2', 'manage co -depends', '', t(), 'calendar'],
    'settings/relations' => ['sub', t('Relations'), 'Relations 2', 'manage -depends', '', t(), 'magnet'],
    'settings/backing' => ['sub', t('Backing Promise'), 'Backing', 'manage linkable', '', t(), 'knight'],
// DISABLE FOR now()    'settings/boxes' => ['sub', t('Devices'), 'Boxes', 'manage -depends', '', t(), 'phone'],
    'settings/proxies' => ['sub', t('Proxies'), 'Proxies', 'manage -co -depends linkable', '', t(), 'thumbs-up'],

    'reset' => ['call', t('Choose New Password'), 'ResetPassword 1', ANY],
    'settings/skip' => ['call', t('Skip Step'), '', 'manage', 'skipStep'],
    'settings/ssn' => ['call', t('Correct SSN'), 'Ssn 2', 'manage linkable'],
    'settings/password' => ['call', t('Request New Password'), 'Pass 2', ANY],
    'settings/security/change-password' => ['call', 'Change Password', 'ChangePassword', 'manage'],
//    'settings/security/change-pin' => ['call', 'Change PIN', 'ChangePin', 'manage'],
    'settings/photo' => ['call', 'Photo', 'Photo 2', 'manage'],
    'settings/photo/upload' => ['call', 'Photo Upload', '3 4', ANY, 'photoUpload'],
//    'settings/verify-phone' => ['call', 'Verify Phone', 'VerifyPhone', 'manage dw'],
//    'settings/readd-phone' => ['call', 'reAdd Phone', '', 'manage dw', 'addPhone'],
    'settings/basic' => ['call', 'Basic', 'Basic', 'manage'],
//    'settings/kba' => ['call', 'Kba', 'Kba', 'manage dw'],
//    'settings/photo-id' => ['call', 'Kba', 'PhotoId', 'manage'],
    'settings/proxy' => ['call', 'Proxy', 'Proxy 2', 'manage -co linkable'],
    'settings/verifyid' => ['call', 'Correct SSN', 'VerifyId 2', 'manage'],
    'settings/verifyemail' => ['call', 'Verify Email', 'VerifyEmail 2', ANY],
    'settings/work' => ['call', 'Work Life', 'Work 2', 'manage'],
    'settings/resend' => ['call', 'Resend Email', 'Resend', 'manage'],
// UNUSED FOR NOW    'settings/food' => ['call', 'Food Fund', 'Food 2', ANY],
    'settings/dft-acct' => ['call', 'Default Account', 'DftAcct', 'read'],
    'settings/partnersteptwo' => ['call', 'Partner Signup Agreement Etc.', 'PartnerStepTwo', 'manage'],
    'settings/partnerend' => ['call', 'Finish Partner Signup', 'PartnerSignup end=1', ANY],

//    'handy' => isDEV ? ['norm', t('Handy'), '', 'dev', 'handyLinks'] : NULL,
    'tests' => isDEV ? ['norm', t('Test'), 'Tests 1', 'dev'] : NULL,
//    'php' => isDEV ? ['norm', t('PHP'), 'Php', 'dev'] : NULL,
    
    'menu' => ['call', t('Menu'), 'Menu', ANY],
    'empty' => ['call', t('Empty'), 'Empty 1', ANY],
    'prox' => ['call', t('Prox'), 'Prox 1 2', ANY], // proxy form called from rdo()

    'request-employee-rcard' => ['call', t('Request Employee %PROJECT Card'), 'RequestRCard 1', 'manage'],
    
    'I' => ['call', 'Scanned QR', 'I', ANY],
    
    'prejoin' => ['call', 'Open a Joint Account', 'Prejoin', 'manage'],
    'handle-invoice' => ['call', 'Handle Invoice', 'HandleInvoice 1', ANY],
    'rerisk' => ['call', 'Recalculate Risk', 'Rerisk 1 2 3', 'admin'],
    
    'sadmin' => ['norm', t('Admin'), 'Admin', isDEV ? ANY : 'admin'],
    'sadmin/panel' => ['sub', t('Panel'), 'AdminPanel 2', 'admPanel'],
    'sadmin/hours' => ['sub', t('Hours'), 'Hours 2', 'admRegion'], // we use Region for all staff (not including local organizers)
    'sadmin/followup' => ['sub', t('Admin Action Page'), 'Followup', 'admFollowup'],
    'sadmin/member-list' => ['sub', t('Member List'), 'MemberList', 'admExport'],
    'sadmin/export-list' => ['sub', t('Export'), 'DownloadMemberList 2', 'admExport'],
    'sadmin/deposits' => ['sub', t('Bank Transfers'), 'Deposits 2', 'admSeeDeposits'],
    'sadmin/new-member' => ['sub', t('New Member'), 'Signup code=.(admin)', 'admHelpSignup'],
    'sadmin/make-community' => ['sub', t('Make Ctty'), 'MakeCtty', 'admMakeCtty'],
    'sadmin/cats' => ['sub', t('Accounting Categories'), 'Cats', 'admCode'],
    'sadmin/groups' => ['sub', t('Groups'), 'Groups 2', 'admMakeRules'],
    'sadmin/rules' => ['sub', t('Rules'), 'Rules 1 2', 'admMakeRules'],
    'sadmin/timed' => ['sub', t('Timed'), 'Rules 1 2', 'admMakeRules'],
    'sadmin/people' => ['sub', t('People'), 'People 2', 'admMutualAid'],
    'sadmin/focuses' => ['sub', t('Focuses'), 'Focuses 2', 'admPrintChecks'],
//    'sadmin/sms' => ['sub', t('Test SMS'), 'SMS', 'admCode'],
    'sadmin/handy' => ['sub', t('Handy'), 'Handy', 'admCode'],
    'sadmin/php' => ['sub', t('PHP'), 'Php 2', isDEV ? ANY : 'admCode'],

    'sadmin/check-sanity' => ['call', 'Check database consistency', '0', 'admCode', 'checkSanity'],
    'sadmin/set-cats' => ['call', t('Set Accounting Categories'), 'SetCats', 'admEditTx'],
    'sadmin/tx-info' => ['call', 'Transaction Info', 'TxInfo 2', 'admSeeTxInfo'],
    'sadmin/deposit-details' => ['call', t('Deposit Details'), 'DepositDetails 2', 'admSeeDeposits'],
    'sadmin/checks' => ['call', t('Checks'), '2', 'admPrintChecks', 'printChecks'],
    'sadmin/achs' => ['call', t('ACHs'), '2', 'admAch', 'achBatch'],
    'sadmin/ofac' => ['call', t('Upload OFAC Files'), 'OFAC', 'admMakeRules'],
    'sadmin/changes' => ['call', t('See Changes'), 'SeeChanges 2', 'admSeeChanges'],
    'sadmin/reconcile-ctty-fund' => ['call', t('Reconcile Community Fund'), 'ReconcileCttyFund', 'admReconcile'],
    'sadmin/reconcile-ctty-fund/manually' => ['call', t('Reconcile Manually'), 'ReconcileManually 3', 'admReconcile'],
    'sadmin/util' => ['call', 'Util', '2', 'admCode', 'util'],
    'sadmin/delete-account' => ['call', t('Delete Account'), '2', 'admDeleteAcct', 'deleteAccount'],
    'sadmin/variables' => ['call', t('List/Edit Variables'), 'Variables', 'admCode'],
    'sadmin/variables/edit' => ['call', t('Edit Variable'), 'VariableEdit', 'admCode'],
    'print-rcard' => ['call', t('Print ID Card'), '1 2 3', 'ok', 'printCard'],
    'sadmin/show-reconciled' => ['call', t('Show Reconciled'), 'ShowReconciled', 'read'],
    'sadmin/test-results' => ['call', t('Test Results'), 'TestResults', 'dev'], // also go here manually, if needed
    'sadmin/summary' => ['call', t('Summary'), 'Summary', 'admin'],
    'sadmin/print-cards' => ['call', t('Print Cards'), 'PrintCards 2', 'admPrintCards'],
    'sadmin/stats' => ['call', t('Stats'), 'Stats 2', 'admRegion'],
    'sadmin/iereport' => ['call', t('Income/Expense Report'), 'IEReport 2', 'admRegion'],
    'sadmin/cef-billing' => ['call', t('CEF Billing'), 'CEFBilling 2', 'read'],
    'sadmin/tx-entry' => ['call', t('TxEntry'), 'TxEntry 2', 'admEditTx'],
    'sadmin/merge-accounts' => ['call', t('Merge Accounts'), 'MergeAccts', 'admDeleteAcct'],
    'sadmin/our-accounts' => ['call', t('Our Accounts'), 'OurAccounts', 'admRegion admSeeAccts'],

    'signout' => ['norm', t('Sign out'), '', 'signedIn', 'signout'],
    'signin' => ['norm', t('Sign in'), 'Signin 1', '-signedIn'],
//    'signin' => ['call', 'Sign in', 'Signin', ANY],
//    'sinx' => ['call', 'Sign in from elsewhere', 'Sinx', ANY],
    
    'qbo' => ['call', t('QBO'), 'QBO 1', 'admEditTx'],
    'account-photo' => ['call', 'Account Photo', 'AccountPhoto 0 1', ANY],
    'account-thumb' => ['call', 'Account Photo Thumbnail', 'AccountPhoto 0 1', ANY],
    'account-logo' => ['call', 'Account Logo', '1', ANY, 'logo'],
    'help' => ['call', t('Help'), 'Help 1 2', ANY],
    'cgbiz' => ['call', t('Common Good Business'), 'CgBiz 1', ANY],
    'change-account' => ['call', t('Change Who'), '1', ANY, 'changeWho'],
    'status' => ['call', 'Status', 'Membership 1', 'manage'],
    'signup' => ['call', t('Sign Up For %PROJECT'), 'Signup 1', ANY],
    'signup-co' => ['call', t('Open a Company Account'), 'SignupCo', ANY],
    'menu' => ['call', 'Main Menu', 'Menu', ANY],
    'member' => ['call', 'Company Profile', 'Profile', ANY],
    'coup' => ['call', t('Disable account'), '1', ANY, 'coup'],
    'app' => ['call', t('Web App'), 'Card', ANY], // call app from mobile icon (no params)
//    'card' => ['call', t('Card'), 'Card 1 2', ANY],
//    'card-scanned' => ['call', t('Customer Photo Pate'), 'Card', ANY], // customer photo etc. that comes up after scan

    'whosin' => ['call', t('Who\'s in?'), '1', ANY, 'whosin'], // called from promo site
    'error' => ['call', 'System Error', '1', ANY, 'error'],
    'page-not-found' => ['call', t('Page Not Found'), 'PageNotFound', ANY],
    'go2' => ['call', t('Go to 2 pages'), '1', ANY, 'go2'],
    'authcode' => ['call', t('Supply a one-use auth code'), '1', ANY, 'authcode'], // deprecated
    'pay' => ['call', t('Pay or Donate from Within or Without'), 'Pay 1', 'okManageIfIn linkable'],
    'cgpay-code' => ['call', t('Supply a CGPay button code'), '', ANY, 'code'],
    'show-qr' => ['call', t('Show My QR Code'), 'ShowQr', 'read'],
    'scan-qr' => ['call', t('Scan a QR Code'), 'ScanQr', ANY],
    'show-doc' => ['call', t('Show an encrypted document'), 'ShowDoc 1', ANY],
    'scraps' => ['call', 'Scraps', 'Scraps 1', 'signedIn'],
    'partner' => ['call', 'Signup Through Partner', 'PartnerSignup 1', ANY],
    'accounts' => isDEV ? ['call', 'Account Choices', 'Accounts', 'manage'] : NULL,
    'mailus' => ['call', t('Mail %PROJECT'), 'MailUs 1', ANY],
    'unsubscribe' => ['call', t('Unsubscribe'), 'Unsubscribe 1', ANY],  
    'cef' => ['call', t('CEF Dashboard'), 'CEF 1', ANY],
    'do' => ['call', 'Do', '1 2', ANY, 'rdo'],

    'get' => ['call', t('Bank'), 'Get 1', 'manage ok'],
    'pos' => ['call', 'POS', '1', ANY, 'pos'], // called from the CGPay app
    'ajax' => ['call', 'AJAX', '1', ANY, 'ajax'], // called from our JS scripts
    'api' => ['call', 'API', '1', ANY, 'api'],
    'cc' => ['call', 'Credit Card Donation', 'Pay 1', ANY],
    'donate-fbo' => ['call', t('Donate'), 'Pay 1', ANY], // deprecated (use donate instead), but still used by Kibilio as of 11/22/2021
  );

  foreach ($items as $k => $v) {
    if ($submenu) {
      if (u\starts($k, "$submenu/")) $menu[$k] = $v;
    } elseif ($v) $menu[$k] = makeMenuItem($k, $v);
  }

  return $menu;
}

/**
 * Construct a menu item from its list of parts.
 * The last two parts (detail and glyphicon) are used only for submenus (see subMenuHtml())
   -- "bootstrap glyphicon components" choices can be googled
    MENU_CALLBACK  Menu type -- A hidden, internal callback, typically used for API calls.
    MENU_DEFAULT_LOCAL_TASK  Menu type -- The "default" local task, which is initially active.
    MENU_LOCAL_ACTION  Menu type -- An action specific to the parent, usually rendered as a link.
    MENU_LOCAL_TASK  Menu type -- A task specific to the parent item, usually rendered as a tab.
    MENU_NORMAL_ITEM  Menu type -- A "normal" menu item that's shown in menu and breadcrumbs.
    MENU_SUGGESTED_ITEM  Menu type -- A normal menu item, hidden until enabled by an administrator.
 */
function makeMenuItem($k, $parts) {
  $types = ['norm' => MENU_NORMAL_ITEM, 'sub' => MENU_LOCAL_TASK, 'dft' => MENU_DEFAULT_LOCAL_TASK, 'call' => MENU_CALLBACK];
  list ($type, $title, $args, $perms, $func) = u\padRay($parts, 5);
  return menu1($k, $title, $types[$type], $func ?: 'showForm', $args, $perms);
}

/**
 * Add an item to the menu structure (called when reinstalling or rebuilding menus).
 * @param string $path: URL path to this item (without the domain)
 * @param string $title: human-readable label for operation
 * @param int $type: menu item type (see hook_menu)
 * @param string $function: what function to call when item is activated (with or without namespace qualifiers)
 * @param string $args: space-delimited list of arguments to pass to the function
 * @param string $perms: space-delimited list of permission bit names (lowercase without B_ )
 * @return the menu item array as expected by Drupal for hook_menu
 */
function menu1($path, $title, $type, $function, $args, $perms = '') {
  global $menuWeight; $menuWeight += !nn($menuWeight) ?: 1;

  foreach ($args = ray($args) as $k => $v) if (is_numeric($v)) $args[$k] = (int) $v;
  
  $adminPerms = (strihas($perms, 'admin') or u\starts($perms, 'adm'));
  $admin = (u\starts($path, 'sadmin') or $perms == 'dev' or $adminPerms);
  $smart = ($path == 'pos');

  $formName = ($args and !is_numeric($args[0])) ? $args[0] : $function;
  $file = strtolower("forms/$formName.inc");
  if (!file_exists(R_ROOT . "/$file")) $file = $smart ? 'rsmart/rsmart.inc' : ($admin ? 'admin/admin-forms.inc' : 'rweb/rweb.inc');
  
  $item = array(
    'title' => $title,
    'type' => $type,
    'page callback' => ($smart ? 'CG\\Smart\\' : 'CG\\Web\\') . $function,
    'page arguments' => $args,
    'access callback' => $perms ? 'CG\\access' : TRUE,
    'access arguments' => $perms ? array($perms) : NULL,
    'weight' => $menuWeight - 1,
    'menu_name' => 'main-menu', 
    'module' => $smart ? 'rsmart' : 'rweb',
    'file' => $file,
    'options' => ray('class', $title),
  );

  return $item;
}

function subMenuHtml($menu) {
  global $base_url;

  if (!is_array($menu)) $menu = menu($menu);

  $list = '';
  foreach ($menu as $k => $v) {
    list ($type, $head, $args, $perms, $func, $detail, $icon) = u\padRay($v, 7);
    if ($type == 'call' or !access($perms)) continue;
    if (nn($detail)) $detail = "<p class=\"list-group-item-text\">$detail</p>";
    $text = <<<EOF
    <span class="list-group-item-heading">
      <span class="glyphicon glyphicon-$icon"></span> &nbsp;
      $head
    </span>
    $detail
EOF;
    $list .= spinLink("/$k", $text, '', 'list-group-item', '');
  }
  return $list;
}
