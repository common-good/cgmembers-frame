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

require_once R_ROOT . '/classes/qb.class';

/**
 * Do some operation in QuickBooks. Especially authorization (QBO API needs a url for OAuth2 callback -- this is it).
 * Call with parameters:
 *   op and other: (POSTed) intended operation and parameters (empty op means get QBO API authorization)
 *   code, state, and realmId: (GET) authorization to connect with Quickbooks API
 */
function formQBO($form, &$sta, $args = '') {
  $qb = new \QB(QBO_CREDS, FALSE);
  extract(just('op tnm flds', $args, ''));

  switch ($op) {
    case '': $qb->auth(); break;
    
    case 'accounts':
      $id = 0;
      foreach ($qb->getAccounts() as $externalId => $category) {
        list ($category, $description) = explode("\t", $category);
        $id += 100;
        $info = compact(ray('id externalId category' . ($description ? ' description' : '')));
        db\updateOrInsert('budget_cats', $info, 'externalId');
      }
      break;
    
    case 'table': say($qb->getRows($flds, $tnm)); break;
  }
  
  $title = t('QBO Test');
  $subtext = t('Details.');

  $tnm = textFld(t('Table:'), [t(''), t('For example: Account, Customer, Vendor')], dft('Account'));
  $flds = textFld(t('Fields:'), t('For example: Id, FullyQualifiedName, Classification'));
//  jsx('model', compact('choices'));

  $args = hidFld($args);
  $submit = t('Go');
  
  return cgform(compact(ray('title tnm flds submit')));
}

function formQBO_validate($form, &$sta) {
}

function formQBO_submit($form, &$sta) {
  extract(just('tnm flds', $sta['input']));

  return go("/qbo/op=table&tnm=$tnm&flds=$flds");
}
