<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const CAT_FLDS = 'id category description externalId nick show'; // just the fields we handle here (and omitting pid)
const CAT_URL = 'sadmin/cats/id';

/**
 * Edit Budget Categories (for interface with QuickBooks, 990, etc.)
 */
function formCats($form, &$sta, $args = '') {
  
  if (!db\exists('tx_cats')) return goFirst('qbo/op=accounts');
  extract(crud('tx_cats', $args, CAT_URL, CAT_FLDS, 'ASC')); // get ret or [title, orig, id, and CAT_FLDS]
  if (nn($ret)) return $ret;
  
  $title = t('Accounting Categories');
//  $subtext = t('Details.');
  $id0 = hidFld($id);
  $id = intFld(REQ . t('ID/Order:'), [t('ID'), t('Change this to change display order in dropdowns.')], dft($id)+vmin(-1));
  $category = textFld(REQ . t('Category:'), [t('Name of category')], dft($category));
  $description = textFld(t('Description:'), [t('Description of category')], dft($description));
  $externalId = intFld(REQ . t('External ID:'), [t('ID in QB or whatever')], dft($externalId));
  $nick = textFld(t('Nickname:'), [t('Category Nickname')], dft($nick));
  $show = boolFld(t('Show?'), '', dft($show));

  $submit = t('Save');
  
  return cgform(compact(ray('title id category description externalId nick show submit id0')));
}

function formCats_validate($form, &$sta) {
  extract(just('id id0', $sta['input'], NULL));

  if ($id != $id0 and db\exists('tx_cats', compact('id'))) return err(t('That order ID is already used'), 'id');
}

function formCats_submit($form, &$sta) {
  extract(just('id id0', $input = $sta['input'], NULL));
  if ($id != $id0) db\q('UPDATE tx_cats SET id=:id WHERE id=:id0', compact(ray('id id0')));

  db\updateOrInsert('tx_cats', just(CAT_FLDS, $input), 'id');

  return go('/sadmin/cats');
}
