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
    case '': $qb->auth(); break; // may not return
    
    case 'accounts':
      $qbAccts = $qb->getAccounts();
      $id = 0; // tracks each record as we insert or update it
      $redo = !db\exists('tx_cats');
      if (!$redo and $diff = array_diff(db\col('externalId', 'tx_cats', '`show`'), array_keys($qbAccts))) { // if we deleted accounts in QBO, unshow them here
        db\q('UPDATE tx_cats SET `show`=0 WHERE externalId IN (:diff)', compact('diff'));
        err(t('These cats have been deleted in QBO, so they have been hidden here too: ') . join(', ', $diff));
      }
        
      foreach ($qbAccts as $externalId => $category) {
        list ($category, $description) = explode("\t", $category);
        $info = compact(ray('externalId category' . ($description ? ' description' : '')));
        if ($redo) { // building tx_cats from scratch (don't do this on production db!!)
          $id += 100;
          db\insert('tx_cats', $info + compact('id'), 'id');
        } else { // updating tx_cats from QBO
          if ($id0 = db\get('id', 'tx_cats', compact('externalId'))) { // category alread exists here, so just update it
            $id = $id0;
            db\update('tx_cats', $info + compact('id'), 'id');
          } else { // category doesn't exists here yet, so choose an id for it (ideally alphabetical) and insert
            if ($nextId = db\min('id', 'tx_cats', 'id>:id', compact('id'))) {
              $id = round(($id + $nextId) / 2, 0);
              if (db\exists('tx_cats', compact('id'))) {
                $id = db\max('id', 'tx_cats') + 1;
                say(t('Maybe change id (order) of new category "%category" (temporary id %id) to be roughly alphabetical before using it.', compact(ray('id category'))));
              }
            } else $id = round($id + 100, -2);
            db\insert('tx_cats', $info + compact('id'), 'id');
          }
        }
      }
      break;

    case 'txs': $qb->sendTxs(); break;
    
    case 'table': say(pr($qb->getRows($flds ?: 'Id', $tnm, '', 10))); break;
    case 'special': 
//      debug(pr($qb->getRow('*', 'Purchase', 'Id=9293')));
//      debug(pr($qb->getRows('*', 'Deposit', '', 3)));
//      debug(pr($qb->getRows('*', 'Transfer', '', 3)));
      debug(pr($qb->getRows('*', 'JournalEntry', '', 3)));
      break;
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
