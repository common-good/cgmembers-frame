<?php
namespace CG;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Money-movement notification fan-out to per-account "extra recipients"
 * (Phase 3.5 PR C).
 *
 * The primary account holder still gets their notice via Acct::tell() as before.
 * This module adds an *additional* fan-out to up to TX_NOTIFY_MAX_RECIPIENTS
 * staff/contacts that the account has configured on the settings/notice-recipients
 * page. Recipients have no login; they only receive transactional info.
 *
 * Delivery:
 *   - email: uses r\rMail() (already-wired PostMark path). Always fires when the
 *     recipient has an email address and wantEmail is on.
 *   - SMS:   uses direct HTTPS to Twilio (no twilio/sdk dep, matches the cg-drive
 *     pattern). No-op if Twilio config is empty — email still fires.
 *
 * Called from cg-backend.inc's tx-notification path immediately after the primary
 * $a->tell($notice, ...) firing.
 */

/**
 * Fan out a money-movement notification to $a's extra recipients.
 *
 * @param object $a       The account whose transaction fired the notification.
 * @param string $inout   'in' (received) or 'out' (sent).
 * @param array  $args    Substitutions for the primary notice — we cherry-pick
 *                        amount, counterparty, timestamp from here.
 */
function notifyTxExtras($a, $inout, array $args) {
  if (!$a || !$a->id) return;
  $rows = txNotifyRecipients($a->id);
  if (!$rows) return;

  $amount = nni($args, 'amount') ?: nni($args, 'amt');
  $counterparty = nni($args, 'otherName') ?: nni($args, 'other') ?: '';
  $when = date('Y-m-d g:i a', now());
  $verb = $inout === 'out' ? 'sent' : 'received';
  $sponseeName = $a->fullName ?: $a->bestName;

  $subject = "$sponseeName $verb $amount";
  $body = "$sponseeName $verb $amount " . ($counterparty ? ($inout === 'out' ? "to $counterparty" : "from $counterparty") . " " : '') . "on $when.";

  foreach ($rows as $rec) {
    if ($rec->wantEmail && $rec->email) txNotifySendEmail($rec, $subject, $body);
    if ($rec->wantText  && $rec->phone) txNotifySendSms($rec, $body);
  }
}

/**
 * Load the recipient rows for an account, capped at TX_NOTIFY_MAX_RECIPIENTS.
 * Public so txnotifyprefs.inc can list them.
 */
function txNotifyRecipients($uid) {
  $rows = db\q(
    'SELECT id, uid, name, email, phone, wantEmail, wantText FROM tx_notify_recipients WHERE uid=:uid ORDER BY id LIMIT ' . (int) TX_NOTIFY_MAX_RECIPIENTS,
    ['uid' => (int) $uid]
  )->fetchAll(\PDO::FETCH_OBJ);
  return $rows ?: [];
}

function txNotifySendEmail($rec, $subject, $body) {
  $to = ['fullName' => $rec->name, 'email' => $rec->email];
  try {
    r\rMail('tx-notify-recipient', $to, ['subject' => $subject, 'body' => $body], 'ctty');
  } catch (\Throwable $e) {
    r\tellAdmin('tx-notify-email-failed', ray('to err', $rec->email, $e->getMessage()));
  }
}

function txNotifySendSms($rec, $body) {
  if (!twilioConfigured()) return; // silent no-op until Jose approves Twilio budget
  try {
    twilioSend($rec->phone, $body);
  } catch (\Throwable $e) {
    r\tellAdmin('tx-notify-sms-failed', ray('to err', $rec->phone, $e->getMessage()));
  }
}

/*  ---- Twilio wrapper: direct HTTPS, no vendor/twilio dep -------------------- */

function twilioConfigured() {
  return TWILIO_ACCOUNT_SID !== '' && TWILIO_AUTH_TOKEN !== '' && TWILIO_FROM_NUMBER !== '';
}

/**
 * Send an SMS via Twilio's REST API. Throws \RuntimeException on failure.
 * @param string $to    E.164 destination number (e.g. "+14155552671").
 * @param string $body  Message text (Twilio auto-splits at 160 chars for GSM-7).
 * @return string       Twilio message SID on success.
 */
function twilioSend($to, $body) {
  if (!twilioConfigured()) throw new \RuntimeException('Twilio not configured');
  $sid = TWILIO_ACCOUNT_SID;
  $url = "https://api.twilio.com/2010-04-01/Accounts/$sid/Messages.json";

  $ch = curl_init($url);
  curl_setopt_array($ch, [
    CURLOPT_POST           => TRUE,
    CURLOPT_POSTFIELDS     => http_build_query(['From' => TWILIO_FROM_NUMBER, 'To' => $to, 'Body' => $body]),
    CURLOPT_USERPWD        => TWILIO_ACCOUNT_SID . ':' . TWILIO_AUTH_TOKEN,
    CURLOPT_HTTPAUTH       => CURLAUTH_BASIC,
    CURLOPT_RETURNTRANSFER => TRUE,
    CURLOPT_TIMEOUT        => 15,
  ]);
  $raw = curl_exec($ch);
  $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
  $err = curl_error($ch);
  curl_close($ch);

  if ($raw === FALSE) throw new \RuntimeException("Twilio HTTP error: $err");
  if ($code >= 400)   throw new \RuntimeException("Twilio HTTP $code: $raw");

  $data = json_decode($raw, TRUE);
  return $data['sid'] ?? '';
}
