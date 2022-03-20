<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const BUDGETCAT_FLDS = 'id category description externalId show'; // just the fields we handle here (and omitting pid)
const BUDGETCAT_URL = 'co/budget-cats/id';

/**
 * Edit Budget Categories (for interface with QuickBooks, 990, etc.)
 */
function formBudgetCats($form, &$sta, $args = '') {
  
  if (!db\exists('budget_cats')) return goFirst('qbo/op=accounts');
  extract(crud('budget_cats', $args, BUDGETCAT_URL, BUDGETCAT_FLDS, 'ASC')); // get ret or [title, orig, id, and BUDGETCAT_FLDS]
  if (nn($ret)) return $ret;
  
  $title = t('Accounting Categories');
//  $subtext = t('Details.');
  $id0 = hidFld($id);
  $id = intFld(REQ . t('ID/Order:'), [t('ID'), t('Change this to change display order in dropdowns.')], dft($id));
  $category = textFld(REQ . t('Category:'), [t('Name of category')], dft($category));
  $description = textFld(t('Description:'), [t('Description of category')], dft($description));
  $externalId = intFld(REQ . t('External ID:'), [t('ID in QB or whatever')], dft($externalId));
  $show = boolFld(t('Show?'), '', dft($show));

  $submit = t('Save');
  
  return cgform(compact(ray('title id category description externalId show submit id0')));
}

function formBudgetCats_validate($form, &$sta) {
  extract(just('id id0', $sta['input'], NULL));

  if ($id != $id0 and db\exists('budget_cats', compact('id'))) return err(t('That order ID is already used'), 'id');
}

function formBudgetCats_submit($form, &$sta) {
  extract(just('id id0', $input = $sta['input'], NULL));
  if ($id != $id0) db\q('UPDATE budget_cats SET id=:id WHERE id=:id0', compact(ray('id id0')));

  db\updateOrInsert('budget_cats', just(BUDGETCAT_FLDS, $input), 'id');

  return go('/co/budget-cats');
}
