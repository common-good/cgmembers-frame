<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Util as u;
use CG\Db as db;

require_once R_ROOT . '/cg-tx-notify.inc';

/**
 * Manage "Transaction Notification Recipients" — up to TX_NOTIFY_MAX_RECIPIENTS
 * extra contacts who receive money-movement notices for this account. Recipients
 * have no login and no other account access.
 */
function formTxNotifyPrefs($form, &$sta, $args = '') {
  global $mya;

  $title = t('Transaction Notification Recipients');
  $subtext = t('Add up to %max extra people who should receive an email or text whenever your account receives or sends money. They will not have access to your account &mdash; just the amount, who it was with, and when.', ray('max', TX_NOTIFY_MAX_RECIPIENTS));

  $existing = \CG\txNotifyRecipients($mya->id);
  $fields = [];

  for ($i = 1; $i <= TX_NOTIFY_MAX_RECIPIENTS; $i++) {
    $rec = $existing[$i - 1] ?? null;
    $suffix = "_$i";

    ${"name$suffix"}      = textFld(t('Name @n:', 'n', $i), '', ['name' => "name$suffix", 'value' => $rec->name ?? '']);
    ${"email$suffix"}     = emailFld(t('Email @n:', 'n', $i), '', ['name' => "email$suffix", 'value' => $rec->email ?? '']);
    ${"phone$suffix"}     = phoneFld(t('Phone @n:', 'n', $i), '', ['name' => "phone$suffix", 'value' => $rec->phone ?? '']);
    ${"wantEmail$suffix"} = boxFld("wantEmail$suffix", t('Email @n', 'n', $i), '', (bool) ($rec->wantEmail ?? 1));
    ${"wantText$suffix"}  = boxFld("wantText$suffix",  t('Text @n',  'n', $i), '', (bool) ($rec->wantText  ?? 0));

    foreach (['name', 'email', 'phone', 'wantEmail', 'wantText'] as $k) $fields[] = "$k$suffix";
  }

  $submit = submit(t('Save Recipients'));

  $formData = compact(ray('title subtext ' . join(' ', $fields) . ' submit'));
  return cgform($formData);
}

function formTxNotifyPrefs_validate($form, &$sta) {
  for ($i = 1; $i <= TX_NOTIFY_MAX_RECIPIENTS; $i++) {
    $rec = txNotifyPrefsReadRow($sta['input'], $i);
    if ($rec === null) continue; // whole row blank — allowed

    if ($rec['name'] === '')                             return txNotifyErr(t('Recipient @n: name is required.', 'n', $i), "name_$i");
    if ($rec['email'] === '' && $rec['phone'] === '')    return txNotifyErr(t('Recipient @n: at least one of email or phone is required.', 'n', $i), "email_$i");
    if ($rec['email'] !== '' && !u\validEmail($rec['email'])) return txNotifyErr(t('Recipient @n: bad email.', 'n', $i), "email_$i");
    if ($rec['phone'] !== '' && ($err = u\badPhone($rec['phone']))) return txNotifyErr("Recipient $i: $err", "phone_$i");
    if (!$rec['wantEmail'] && !$rec['wantText'])         return txNotifyErr(t('Recipient @n: choose at least one channel (email or text).', 'n', $i), "wantEmail_$i");
    if ($rec['wantEmail'] && $rec['email'] === '')       return txNotifyErr(t('Recipient @n: cannot want email without an email address.', 'n', $i), "email_$i");
    if ($rec['wantText']  && $rec['phone'] === '')       return txNotifyErr(t('Recipient @n: cannot want text without a phone number.', 'n', $i), "phone_$i");
  }
}

function formTxNotifyPrefs_submit($form, &$sta) {
  global $mya;

  $rows = [];
  for ($i = 1; $i <= TX_NOTIFY_MAX_RECIPIENTS; $i++) {
    $rec = txNotifyPrefsReadRow($sta['input'], $i);
    if ($rec !== null) $rows[] = $rec;
  }

  $DBTX = \db_transaction();
  db\del('tx_notify_recipients', 'uid=:uid', ['uid' => $mya->id]);
  foreach ($rows as $rec) {
    db\insert('tx_notify_recipients', $rec + ['uid' => $mya->id, 'created' => now()]);
  }
  unset($DBTX);

  say('info saved');
  return w\go('/settings/notice-recipients');
}

/**
 * Read one row's inputs. Returns NULL if the whole row is blank (allowed), or a
 * normalized associative array ready to insert into tx_notify_recipients.
 */
function txNotifyPrefsReadRow($input, $i) {
  $name  = trim((string) nni($input, "name_$i"));
  $email = trim((string) nni($input, "email_$i"));
  $phone = trim((string) nni($input, "phone_$i"));
  $wantEmail = (int) (bool) nni($input, "wantEmail_$i");
  $wantText  = (int) (bool) nni($input, "wantText_$i");

  if ($name === '' && $email === '' && $phone === '') return null;

  return compact('name', 'email', 'phone', 'wantEmail', 'wantText');
}

function txNotifyErr($msg, $field = '') {
  say($msg, 'alert');
  return w\formErr($field);
}
