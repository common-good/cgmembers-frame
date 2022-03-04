<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

use QuickBooksOnline\API\Core\ServiceContext;
use QuickBooksOnline\API\DataService\DataService;
use QuickBooksOnline\API\PlatformService\PlatformService;
use QuickBooksOnline\API\Core\Http\Serialization\XmlObjectSerializer;
use QuickBooksOnline\API\Facades\Purchase;
use QuickBooksOnline\API\Data\IPPPurchase;
use QuickBooksOnline\API\QueryFilter\QueryMessage;
use QuickBooksOnline\API\ReportService\ReportService;
use QuickBooksOnline\API\ReportService\ReportName;

use QuickBooksOnline\API\Core\OAuth\OAuth2;
use QuickBooksOnline\API\Core\OAuth\OAuth2\OAuth2AccessToken;
// https://help.developer.intuit.com/s/error-resolver

require_once DRUPAL_ROOT . '/../vendor/autoload.php';

/**
 * Do whatever.
 */
function formQBO($form, &$sta, $args = '') {
  global $mya;

  if (!$dataService = qboConfig($args)) return;

  $title = t('QBO Test');
  $subtext = t('Details.');

  $choices = ray('this that theOther');
  $radio = radiosFld(REQ . t('Radios:'), t('Details here'), dft(-1), $choices); // no default
  $select = selectFld(REQ . t('Selection:'), t('Details here'), dft(2), $choices);
  $tnm = textFld(t('Table to List:'), [t('Object Name'), t('')], dft('Customer'));
//  jsx('model', compact('choices'));

  $args = hidFld($args);
  $submit = t('Go');
  
  return cgform(compact(ray('title tnm submit')));
}

function formQBO_validate($form, &$sta) {
  global $mya;

/*  extract(just('text', $sta['input']));

  if ($err = u\badAmount($text)) return say($err, 'text');
  u\preRay(compact('text'), $sta['input']);
  */
}

function formQBO_submit($form, &$sta) {
  global $mya;
  extract(just('tnm', $sta['input']));

  getQboData($tnm);

  return go('/sadmin/qbo');
}

function getQboData($tnm) {
  $dataService = qboConfig();

  // reports
  /*
  $serviceContext = $dataService->getServiceContext();
  if (!$reportService = new ReportService($serviceContext)) exit("Problem while initializing ReportService.\n");

  if (!$profitAndLossReport = $reportService->executeReport("ProfitAndLoss")) exit("ProfitAndLossReport Is Null.\n");
  $result = json_encode($profitAndLossReport, JSON_PRETTY_PRINT);

  if (!$balancesheet = $reportService->executeReport("BalanceSheet")) exit("BalanceSheet Is Null.\n");
  $result = json_encode($balancesheet, JSON_PRETTY_PRINT);
  */
  
  // data
  if (!$cnt = $dataService->query("SELECT COUNT(*) FROM $tnm")) return debug("Table $tnm is empty.");
  
  $res = $dataService->query("SELECT * FROM $tnm", 1, 10); // or , 1, 100
//  $res = $dataService->getCompanyInfo();
  debug($res);
}

/**
 * Return a QBO dataService object containing config data.
 */
function qboConfig($args = '') {
  extract(just('code realmId', $args, NULL));
  list ($clientId, $secret) = ray(QBO_CREDS);
  
  $dataService = DataService::Configure([
    'auth_mode' => 'oauth2',
    'ClientID' => $clientId,
    'ClientSecret' => $secret,
    'RedirectURI' => isPRODUCTION ? BASE_URL . '/sadmin/qbo' : 'https://cg4.us/qbo.php',
    'scope' => 'com.intuit.quickbooks.accounting', // com.intuit.quickbooks.accounting or com.intuit.quickbooks.payment
//    'baseUrl' => (isPRODUCTION or isSTAGE) ? 'Production' : 'Development',
    'baseUrl' => 'Production',
  ]);

  $dataService->throwExceptionOnError(true);
  $oauth2LoginHelper = $dataService->getOAuth2LoginHelper();

  if ($code) {
    $accessToken = $oauth2LoginHelper->exchangeAuthorizationCodeForToken($code, $realmId);
  } elseif ($qboRefresh = getV('qboRefresh') and $qboRefresh[1] > time() + 60) { // at least a minute left to refresh
    $accessToken = $oauth2LoginHelper->refreshAccessTokenWithRefreshToken($qboRefresh[0]);
    $accessToken->setRealmId($realmId = $qboRefresh[2]); // required (stupid though that is)
    $dataService->updateOAuth2Token($accessToken);
  } else { // out of time to refresh (or first time): get admin to sign in to QBO, then redirect back here with code and realm
    $authUrl = $oauth2LoginHelper->getAuthorizationCodeURL();
    return go($authUrl);
  }

  setV('qboRefresh', [$accessToken->getRefreshToken(), strtotime($accessToken->getRefreshTokenExpiresAt()), $realmId]); // store refreshToken so we don't have to re-authorize
//  die(pr(getV('qboRefresh')));

  return $dataService;
}
