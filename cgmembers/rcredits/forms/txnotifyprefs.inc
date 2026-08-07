<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Util as u;
use CG\Db as db;

/**
 * Manage "Transaction Notification Recipients" for a company account.
 *
 * Design (per William 2026-07-22 / Jose 2026-07-22):
 *   Recipients must be existing CG accounts (u_relations of this company with
 *   permission >= READ). No new table; each relation's channel preference lives
 *   on its u_relations row (txNotifyEmail, txNotifyText).
 *
 * For personal accounts, this form isn't relevant — they set their own tx-notice
 * preferences on the Notices page (forms/noticeprefs.inc).
 */
function formTxNotifyPrefs($form, &$sta, $args = '') {
  global $mya;

  $title = t('Transaction Notification Recipients');

  if (!$mya->co) {
    $subtext = t('This preference is only relevant for company accounts. To manage your own notification preferences, go to the <%a>Notices</a> page.', '_a', atag('/settings/notices'));
    return cgform(compact('title', 'subtext'));
  }

  $subtext = t('Choose which authorized users should receive email or text notifications when money moves into or out of this account. Recipients must already have at least read access on this company (managed via <%a>Relations</a>).', '_a', atag('/settings/relations'));

  $sql = "SELECT r.reid, r.other AS uid, u.fullName, r.txNotifyEmail, r.txNotifyText, u.phone
          FROM u_relations r JOIN users u ON u.uid = r.other
          WHERE r.main = :main AND r.permission >= :readPerm
          ORDER BY u.fullName";
  $rows = db\q($sql, ['main' => (int) $mya->id, 'readPerm' => r\perm(B_READ)])->fetchAll(\PDO::FETCH_OBJ);

  if (!$rows) {
    $list = item(t('No authorized users found. Add users via <%a>Relations</a> before assigning notification preferences.', '_a', atag('/settings/relations')));
    return cgform(compact('title', 'subtext', 'list'));
  }

  $lines = [u\tableRow(ray('Person,Phone,Email,Text'))];
  foreach ($rows as $rec) {
    $reid = (int) $rec->reid;
    $emailKey = "email_$reid";
    $textKey  = "text_$reid";
    ${$emailKey} = boxFld($emailKey, '', '', (bool) $rec->txNotifyEmail);
    ${$textKey}  = boxFld($textKey,  '', '', (bool) $rec->txNotifyText);
    $phone = $rec->phone ?: '<em>' . t('none on file') . '</em>';
    $lines[] = u\tableRow(ray('name phone email text', $rec->fullName, $phone, ${$emailKey}, ${$textKey}));
  }
  $list = "<table>\n" . join('', $lines) . "</table>\n";

  $submit = submit(t('Save Preferences'));
  $sta['no_cache'] = TRUE;

  return cgform(compact('title', 'subtext', 'list', 'submit'));
}

function formTxNotifyPrefs_submit($form, &$sta) {
  global $mya;
  if (!$mya->co) return w\go('/settings/notice-recipients');

  $sql = "SELECT r.reid, u.phone FROM u_relations r JOIN users u ON u.uid = r.other
          WHERE r.main = :main AND r.permission >= :readPerm";
  $reids = db\q($sql, ['main' => (int) $mya->id, 'readPerm' => r\perm(B_READ)])->fetchAll(\PDO::FETCH_OBJ);

  $DBTX = \db_transaction();
  foreach ($reids as $rec) {
    $reid = (int) $rec->reid;
    $txNotifyEmail = (int) (bool) nni($sta['input'], "email_$reid");
    $txNotifyText  = (int) (bool) nni($sta['input'], "text_$reid");
    if ($txNotifyText and empty($rec->phone)) $txNotifyText = 0; // no phone -> can't text; silently correct
    db\update('u_relations', ray('reid txNotifyEmail txNotifyText', $reid, $txNotifyEmail, $txNotifyText), 'reid');
  }
  unset($DBTX);

  say('info saved');
  return w\go('/settings/notice-recipients');
}
