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
require_once R_ROOT . '/classes/qb.class';

/**
 * Do some operation in QuickBooks.
 * Call with parameters:
 *   op and other: (POSTed) intended operation and parameters to pass to new QB()
 *   code, state, and realmId: (GET) authorization to connect with Quickbooks and retry new QB()
 */
function formQBO($form, &$sta, $args = '') {
  global $mya;

  $qb = new QB(QBO_CREDS, $args ?: $_GET);
  extract(just('op', $qb->intent(), NULL));

  if ($op == 'accounts') {
    debug($qb->getIEAccounts());
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
  global $mya;

/*  extract(just('text', $sta['input']));

  if ($err = u\badAmount($text)) return say($err, 'text');
  u\preRay(compact('text'), $sta['input']);
  */
}

function formQBO_submit($form, &$sta) {
  global $mya;
  extract(just('tnm flds', $sta['input']));

  getQboData($tnm, $flds);

  return go('/qbo');
}
