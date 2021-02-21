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
    'history/recurring' => ['sub', t('Automated Payments'), 'Recurring', 'read', '', t(), 'list'],
    'history/invoices-to' => ['sub', t('Invoices TO You'), 'Invoices to', 'read', '', t(), 'log-in'],
    'history/invoices-from' => ['sub', t('Invoices FROM You'), 'Invoices from', 'read', '', t(), 'log-out'],
    'history/company-reports' => ['sub', t('Company Reports'), 'Data 2 co', 'read co', '', t(), 'signal'],
    'history/company-reports/demographics' => ['call', t('Customer Demographics'), 'Demographics', 'read co'],
    'history/statements' => ['sub', t('Statements'), 'Statements 2', 'read', '', t(), 'list-alt'],
    'history/notices' => ['sub', t('Notices'), 'Notices 2', 'read', '', t(), 'envelope'],
    'history/tax-info' => ['sub', t('Tax Info'), 'TaxInfo 2', 'read', '', t(), 'pawn'],
    'history/changes' => ['sub', t('See Changes'), 'SeeChanges', 'cadmin', '', t(), 'refresh'],
    'history/statement' => ['call', t('Statement'), '2', 'read', 'statement'],
    'history/customer-statement' => ['call', t('Customer Statement'), '2', 'read', 'custStatement'],

    'community' => ['norm', t('Community'), 'Community', ANY],
    'community/endorse' => ['call', t('Invite / Endorse'), 'Endorse 2', ANY, '', t(), 'share-alt'],
    'community/invite' => ['sub', t('Invite / Endorse'), 'Invite 2', ANY, '', t(), 'share-alt'],
    'community/invite/all' => ['call', t('Shouters'), 'Shouters 3', ANY],
    'community/invite/example' => ['call', t('Invite Someone'), '', 'ok manage', 'inviteExample'],
    'community/invite/print' => ['call', t('Print Invite'), '', 'manage', 'printInvite'],
    'community/invite/invited-whom' => ['call', 'Invited Whom', 'InvitedWhom', 'manage'],
    'community/stepup' => ['sub', t('Step Up'), 'StepUp 2', 'manage', '', t(), 'export'],
    'community/crumbs' => ['sub', t('Share When You Receive'), 'Crumbs 2', 'manage', '', t(), 'import'],
    'community/buy-credit' => ['sub', t('Buy Credit for Later'), 'BuyCredit 2', 'manage', '', t(), 'time'],
    'community/donate' => ['sub', t('Donate to %PROJECT'), 'Donate', 'manage', '', t(), 'thumbs-up'],
    'community/coupons' => ['sub', t('Discount / Gift Certificates'), 'Coupons 2', 'manage', '', t(), 'usd'],
    'community/coupons/print' => ['call', t('Print Coupons'), '3', 'ok manage', 'couponsPrint'],
    'community/coupons/list' => ['call', t('List Discounts and Gift Cards'), 'CouponsList 3', 'manage'],
    'community/discount' => ['call', t('Simplified Discounts'), 'Discount', 'manage', '', t(), 'usd'],
    'community/posts' => ['sub', t('Offers, Needs, & Tips'), 'Posts 2', ANY, '', t(), 'heart'],
    'community/data' => ['sub', t('Community & Money Data'), 'Data 2', 'read', '', t(), 'list-alt'],
    'community/events' => ['sub', t('Democracy Events'), 'Events 2', 'ok manage', '', t(), 'bell'],
    
    'community/events/event' => ['call', t('Add/Edit Event'), 'Event 3 4', 'ok'],
    'community/events/question' => ['call', t('Add/Edit Question'), 'Question 3', 'cadmin'],
    'community/events/option' => ['call', t('Add/Edit Option'), 'Option 3', 'cadmin'],
    'community/events/proposal' => ['call', t('Funding Proposal'), 'Proposal 3', ANY],
    'community/events/proposal-report' => ['call', t('Proposal Report'), 'ProposalReport 3', 'manage'],
    'community/events/proposal-notes' => ['call', t('Proposal Notes'), 'ProposalNotes 3', 'manage'],
    
    'community/agreement' => ['sub', t('The Agreement'), 'Agreement', ANY, '', t(), 'globe'],
    'community/backing' => ['call', t('Backing Promise'), 'Backing', ANY, '', t(), 'knight'],
    'community/honors' => ['sub', t('Honored & Remembered'), 'Honors', ANY, '', t(), 'grain'],
    'community/message' => ['sub', t('Message a Member'), 'Message', 'ok refund', '', t(), 'envelope'],
    'community/message/x' => ['call', t('Decode a Secret Message'), 'Message 3', ANY],
    'community/find-company' => ['sub', t('Find a Company'), 'FindCo 2', ANY, '', t(), 'search'],
// temporarily disabled    'community/grant' => ['sub', t('Get Paid to Organize'), 'Grant', 'prose', '', t(), 'apple'],
//    'community/funds' => ['sub', t('Funds in Each Community'), 'CttyFunds', ANY, '', t(), 'usd'],
    'community/graphs' => ['sub', t('%PROJECT Graphs'), 'Graphs 2', 'cadmin', '', t(), 'stats'],
    'community/balance-sheet' => ['sub', t('Balance Sheet'), 'BalanceSheet 2', ANY, '', t(), 'pause'],
    'community/flags' => ['sub', t('Risk Flags'), 'Flags 2', ANY, '', t(), 'flag'],
    'community/chart-data' => ['call', t('Chart Data'), '2', ANY, 'chartData'],
    'community/list' => ['call', t('Community List'), '', ANY, 'cttyList'], // for charts/graphs
    'community/covid' => ['call', t('COVID-19 Response'), 'Covid', ANY],
    
    'co' => ['norm', t('Company'), 'Co', 'manage co'],
    'co/cgbutton' => ['sub', t('Create CGPay Button'), 'CGButton 1', 'manage', '', t(), 'unchecked'],
    'co/coupons' => ['sub', t('Discount / Gift Certificates'), 'Coupons 2', 'manage', '', t(), 'usd'],
    'co/accredit' => ['sub', t('Give a Customer Credit'), 'Accredit', 'manage', '', t(), 'hand-right'],
    'co/relations' => ['sub', t('Authorized Users'), 'Relations 2', 'manage -depends', '', t(), 'magnet'],
    'co/reports' => ['sub', t('Company Reports'), 'Data 2 co', 'read co', '', t(), 'signal'],
    'co/settings' => ['sub', t('Company Settings'), 'Company 2', 'manage co -depends', '', t(), 'calendar'],
    
    'settings' => ['norm', t('Settings'), 'Settings', 'manage -closed -depends'],
//    'settings/settings' => ['dft', t('Settings'), 'Settings', 'manage'],
    'settings/contact' => ['sub', t('Contact Info'), 'Contact', 'manage -depends', '', t(), 'user'],
    'settings/preferences' => ['sub', t('Preferences'), 'Prefs', 'manage -depends', '', t(), 'cog'],
    'settings/security' => ['sub', t('Security'), 'Security 2', 'manage -co -depends', '', t(), 'link'],
    'settings/fund' => ['sub', t('Banking'), 'Connect 2', 'manage -depends', '', t(), 'piggy-bank'],
    'settings/company' => ['sub', t('Company Info'), 'Company 2', 'manage co -depends', '', t(), 'calendar'],
    'settings/relations' => ['sub', t('Relations'), 'Relations 2', 'manage -depends', '', t(), 'magnet'],
// DISABLE FOR now()    'settings/boxes' => ['sub', t('Devices'), 'Boxes', 'manage -depends', '', t(), 'phone'],
    'settings/proxies' => ['sub', t('Proxies'), 'Proxies', 'manage -co -depends', '', t(), 'thumbs-up'],

    'reset' => ['call', t('Choose New Password'), 'ResetPassword 1', ANY],
    'settings/skip' => ['call', t('Skip Step'), '', 'manage', 'skipStep'],
    'settings/notices' => ['call', t('Notices'), 'NoticePrefs', ANY],
    'settings/ssn' => ['call', t('Correct SSN'), 'Ssn 2', 'manage'],
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
    'settings/proxy' => ['call', 'Proxy', 'Proxy 2', 'manage -co'],
    'settings/verifyid' => ['call', 'Correct SSN', 'VerifyId 2', 'manage'],
    'settings/verifyemail' => ['call', 'Verify Email', 'VerifyEmail 2', ANY],
    'settings/work' => ['call', 'Work Life', 'Work 2', 'manage'],
    'settings/agent' => ['call', 'Choose an Agent', 'Agent 2', 'manage'],
    'settings/resend' => ['call', 'Resend Email', 'Resend', 'manage'],
    'settings/food' => ['call', 'Food Fund', 'Food 2', ANY],
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
    'rerisk' => ['call', 'Recalculate Risk', 'Rerisk 1 2 3', 'cadmin2'],
    
    'sadmin' => ['norm', t('Admin'), 'Admin', isDEV ? ANY : 'admin2'],
    'sadmin/panel' => ['sub', t('Panel'), 'AdminPanel 2', 'cadmin'],
    'sadmin/followup' => ['sub', t('Admin Action Page'), 'Followup', 'admin2'],
    'sadmin/member-list' => ['sub', t('Member List'), 'MemberList', 'cadmin2'],
    'sadmin/export-list' => ['sub', t('Export'), 'DownloadMemberList 2', 'cadmin2'],
    'sadmin/deposits' => ['sub', t('Deposits'), 'Deposits', 'superadmin'],
    'sadmin/new-member' => ['sub', t('New Member'), 'Signup code=.(admin)', 'cadmin'],
    'sadmin/make-community' => ['sub', t('Make Ctty'), 'MakeCtty', 'superadmin'],
    'sadmin/groups' => [isDEV ? 'sub' : 'call', t('Groups'), 'Groups 2', 'admin'],
    'sadmin/rules' => [isDEV ? 'sub' : 'call', t('Rules'), 'Rules 2', 'admin'],
    'sadmin/check-sanity' => [isDEV ? 'sub' : 'call', 'Check database consistency', '0', 'admin', 'checkSanity'],
//    'sadmin/sms' => ['sub', t('Test SMS'), 'SMS', 'superadmin'],
    'sadmin/handy' => ['sub', t('Handy'), 'Handy', 'superadminORdev'],
    'sadmin/php' => ['sub', t('PHP'), 'Php 2', 'superadminORdev'],

    'sadmin/tx-info' => ['call', 'Transaction Info', 'TxInfo 2', 'admin2'],
    'sadmin/deposit-details' => ['call', t('Deposit Details'), '2', 'superadmin', 'depositDetails'],
    'sadmin/checks' => ['call', t('Checks'), '2', 'superadmin', 'printChecks'],
    'sadmin/achs' => ['call', t('ACHs'), '2', 'superadmin', 'achBatch'],
    'sadmin/changes' => ['call', t('See Changes'), 'SeeChanges 2', 'cadmin'],
		'sadmin/reconcile-ctty-fund' => ['call', t('Reconcile Community Fund'), 'ReconcileCttyFund', 'superadmin'],
    'sadmin/reconcile-ctty-fund/manually' => ['call', t('Reconcile Manually'), 'ReconcileManually 3', 'superadmin'],
    'rcredits/util' => ['call', 'Util', '2', 'superadminORdev', 'util'],
    'sadmin/delete-account' => ['call', t('Delete Account'), '2', 'superadmin', 'deleteAccount'],
    'sadmin/variables' => ['call', t('List/Edit Variables'), 'Variables', 'superadmin'],
    'sadmin/variables/edit' => ['call', t('Edit Variable'), 'VariableEdit', 'superadmin'],
    'print-rcard' => ['call', t('Print ID Card'), '1 2', 'cadmin', 'memberID'],
    'sadmin/show-deposits' => ['call', t('Show Deposits'), 'ShowDeposits', 'read'],
    'sadmin/show-reconciled' => ['call', t('Show Reconciled'), 'ShowReconciled', 'read'],
    'sadmin/test-results' => ['call', t('Test Results'), 'TestResults', 'dev'], // also go here manually, if needed
    'sadmin/summary' => ['call', t('Summary'), 'Summary', 'admin2'],

    'signout' => ['norm', t('Sign out'), '', 'signedIn', 'signout'],
    'signin' => ['norm', t('Sign in'), 'Signin', '-signedIn'],
//    'signin' => ['call', 'Sign in', 'Signin', ANY],
//    'sinx' => ['call', 'Sign in from elsewhere', 'Sinx', ANY],
    
    'account-photo' => ['call', 'Account Photo', 'AccountPhoto 0 1', ANY],
    'account-thumb' => ['call', 'Account Photo Thumbnail', 'AccountPhoto 0 1', ANY],
    'help' => ['call', t('Help'), 'Help 1 2', ANY],
    'cgbiz' => ['call', t('Common Good Business'), 'CgBiz 1', ANY],
    'change-account' => ['call', t('Change Who'), '1', ANY, 'changeWho'],
    'status' => ['call', 'Status', 'Membership 1', 'manage'],
    'signup' => ['call', t('Sign Up For %PROJECT'), 'Signup', ANY],
    'signup-co' => ['call', t('Open a Company Account'), 'SignupCo', ANY],
    'menu' => ['call', 'Main Menu', 'Menu', ANY],
    'member' => ['call', 'Company Profile', 'Profile', ANY],
    'do' => ['call', 'Do', '1 2', ANY, 'rdo'],
    'coup' => ['call', t('Disable account'), '1', ANY, 'coup'],
    'card' => ['call', t('Card'), 'Card 1 2', ANY], // might wanna be 'sell ok' if user can stay signed in
    'disconnect' => ['norm', t('Disconnect Device'), 'Card disconnect', 'scanner'],

    'whosin' => ['call', t('Who\'s in?'), '1', ANY, 'whosin'], // called from promo site
    'error' => ['call', 'System Error', '1', ANY, 'error'],
    'page-not-found' => ['call', t('Page Not Found'), 'PageNotFound', ANY],
    'go2' => ['call', t('Go to 2 pages'), '1', ANY, 'go2'],
    'pay-with-cg' => ['call', t('Pay With %RCREDITS'), 'Buy 1', ANY], // deprecated
    'authcode' => ['call', t('Supply a one-use auth code'), '1', ANY, 'authcode'], // deprecated
    'cgpay' => ['call', t('Pay With %RCREDITS'), 'CGPay', ANY],
    'cgpay-code' => ['call', t('Supply a CGPay button code'), '', ANY, 'code'],
    'scraps' => ['call', 'Scraps', 'Scraps 1', ANY],   
    'partner' => ['call', 'Signup Through Partner', 'PartnerSignup 1', ANY],
    'accounts' => isDEV ? ['call', 'Account Choices', 'Accounts', 'manage'] : NULL,
    'redirect' => ['call', 'Redirect', '', ANY, 'redirect'],
    'mailus' => ['call', t('Mail %PROJECT'), 'MailUs 1', ANY],
    'unsubscribe' => ['call', t('Unsubscribe'), 'Unsubscribe 1', ANY],  

    'get' => ['call', t('Bank'), 'Get 1', 'manage ok'],
    'pos' => ['call', 'POS', '1', ANY, 'pos'], // called from the CGPay app
    'ajax' => ['call', 'AJAX', '1', ANY, 'ajax'], // called from the CGPay app
    'api' => ['call', 'API', '1', ANY, 'api'],
    'cc' => ['call', 'Credit Card Donation', 'CC 1', ANY],
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
  
  $adminPerms = (strihas($perms, 'admin'));
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
    <h4 class="list-group-item-heading">
      <span class="glyphicon glyphicon-$icon"></span> &nbsp;
      $head
    </h4>
    $detail
EOF;
    $list .= spinLink("/$k", $text, '', 'list-group-item', '');
  }
  return $list;
}
