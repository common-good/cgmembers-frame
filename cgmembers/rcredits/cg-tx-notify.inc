<?php
namespace CG;
use CG as r;
use CG\Db as db;

/**
 * @file
 * Money-movement notification fan-out for company accounts (Phase 3.5 PR C).
 *
 * Design (per William 2026-07-22 / Jose 2026-07-22):
 *   - Recipients are existing Common Good accounts (u_relations entries), not
 *     free-form contacts. Only relations with permission >= READ are eligible.
 *   - Each relation stores channel preferences on its u_relations row
 *     (txNotifyEmail, txNotifyText — added by migration 20260722193200).
 *   - When Acct::tell fires on a company account for a money-movement notice
 *     (SEND_IN or SEND_OUT), it recurses here to notify each opted-in relation
 *     via r\rMail (email) and/or the Twilio wrapper (text).
 *
 * Twilio config (defs.inc): TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN,
 * TWILIO_FROM_NUMBER. Empty => SMS is a silent no-op (email still fires).
 */

/**
 * For a company account's money-movement notice, notify each opted-in relation.
 * Called from Acct::tell(), after the primary tell path completes.
 *
 * @param object $a       Company account whose transaction fired the notice.
 * @param string $index   Message template key (e.g. 'paid you', 'you paid').
 * @param array  $subs    Message substitutions.
 * @param string $type    SEND_IN or SEND_OUT (already validated by caller).
 */
function notifyTxRelations($a, $index, array $subs, $type) {
  if (!$a || !$a->id) return;

  $sql = "SELECT r.other AS uid, r.txNotifyEmail, r.txNotifyText
          FROM u_relations r
          WHERE r.main = :main AND r.permission >= :readPerm
          AND (r.txNotifyEmail > 0 OR r.txNotifyText > 0)";
  $rows = db\q($sql, ['main' => (int) $a->id, 'readPerm' => r\perm(B_READ)])->fetchAll(\PDO::FETCH_OBJ);
  if (!$rows) return;

  foreach ($rows as $rec) {
    $other = r\acct((int) $rec->uid);
    if (!$other || !$other->ok) continue;

    if ($rec->txNotifyEmail) {
      try {
        r\rMail($index, $other, $subs, nni($subs, 'reply') ?: 'ctty');
      } catch (\Throwable $e) {
        r\tellAdmin('tx-notify-email-failed', ray('to err', $other->email, $e->getMessage()));
      }
    }

    if ($rec->txNotifyText and $other->phone) {
      try {
        twilioSend($other->phone, txNotifyBodyForSms($subs, $type));
      } catch (\Throwable $e) {
        r\tellAdmin('tx-notify-sms-failed', ray('to err', $other->phone, $e->getMessage()));
      }
    }
  }
}

/**
 * SMS body: brand + amount + direction + counterparty + timestamp.
 * Example: "Common Good: $50.00 in from Jane Doe (Aug 25 3:15pm)"
 * Kept short to fit one SMS segment (160 chars GSM-7).
 *
 * @param array  $subs  Message substitutions (typical keys: amount, otherName).
 * @param string $type  SEND_IN or SEND_OUT - drives "in from" vs "out to" wording.
 */
function txNotifyBodyForSms(array $subs, $type = '') {
  $amt = nni($subs, 'amount') ?: nni($subs, 'amt') ?: '';
  $other = nni($subs, 'otherName') ?: nni($subs, 'other') ?: '';
  $direction = $type === SEND_IN ? 'in from' : ($type === SEND_OUT ? 'out to' : '');
  $when = date('M j g:ia', now());

  $parts = ['Common Good:'];
  if ($amt !== '') $parts[] = $amt;
  if ($direction and $other) $parts[] = "$direction $other";
  elseif ($other) $parts[] = "w/ $other";
  $parts[] = "($when)";

  return join(' ', $parts);
}

/*  ---- Twilio wrapper: direct HTTPS, no vendor/twilio dep -------------------- */

function twilioConfigured() {
  return TWILIO_ACCOUNT_SID !== '' && TWILIO_AUTH_TOKEN !== '' && TWILIO_FROM_NUMBER !== '';
}

/**
 * Send an SMS via Twilio's REST API. Silent no-op when Twilio is unconfigured.
 * @param string $to    E.164 destination number (e.g. "+14155552671").
 * @param string $body  Message text (Twilio splits at 160 chars for GSM-7).
 * @return string       Twilio message SID on success, '' on no-op / failure.
 * @throws \RuntimeException on Twilio HTTP failure.
 */
function twilioSend($to, $body) {
  if (!twilioConfigured()) return '';
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
